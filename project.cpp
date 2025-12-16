#include "pico/cyw43_arch.h"
#include "sm_defs.h"
#include "timeout.h"
#include "display.h"
#include "state_chart.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include "i2c_scan.h"
#include "lcd.h"
#include "buzzer.h"
#include "hardware/adc.h"
#include <string.h>
#include <stdlib.h> // For abs()
#include "pico/stdlib.h"

// The potentiometer is connected to A0 (repurposing the old LIGHT_ADC)
#define POT_ADC_INPUT 0
// The LED is connected to D16
#define LED_PIN 16
// The button is connected to D18
#define BUTTON_PIN 18
// The buzzer is connected to A1
#define BUZZ_PIN 27

// Timeout constants (in ms, then converted to ticks)
#define TIMEOUT_MODE_SELECT_MS 5000
#define TIMEOUT_TIME_SET_MS 2000
#define TIMEOUT_ALARM_SET_MS 2000
#define TIMEOUT_ALARM_MAX_MS 15000
#define TIMEOUT_ALARM_STOPPING_MS 2000

// Global Task Handles
TaskHandle_t th_pot = NULL;
TaskHandle_t th_tick = NULL;
TaskHandle_t th_led = NULL;
TaskHandle_t th_buzzer = NULL;
TaskHandle_t th_temp = NULL;
TaskHandle_t th_button = NULL;
TaskHandle_t th_serial = NULL;

// initialize Ids for remove_timeout_function
int id1 = -1;
int id2 = -1;
int id3 = -1;
int id4 = -1;
int id5 = -1;
int id6 = -1;
buzzer_t buzz;

/*
  The main state machine. This is always the same, do not change it.
 */
class MainSM : public StateMachine
{
    SimpleState *root_state;

public:
    MainSM(SimpleState &root) : root_state(&root) {}
    void handler(Evt_t evt)
    {
        root_state->handler(evt);
    }
};

enum ModeEnum
{
    MODE_HOME,
    MODE_TIME_SET,
    MODE_ALARM_SET,
    MODE_PLAY
};

enum BuzzerMode
{
    BUZZ_OFF,
    BUZZ_PLAY,
    BUZZ_PLAY1,
    BUZZ_ALARM
};

volatile BuzzerMode buzzer_mode = BUZZ_OFF; // default is off

ModeEnum selected_mode = MODE_HOME;
volatile uint32_t alarm_delay = 0; // Countdown timer (5 to 60 seconds)
bool alarm_running = false;        // to indicate if the alarm countdown is active
volatile uint32_t current_time_seconds = 0;
volatile float current_temp = 0.0f;

// The list of possible events in the system
enum Events
{
    EVT_BUTTON,
    EVT_POT_MOVE,
    EVT_TICK,
    EVT_TIMEOUT,
    EVT_TIME_RECEIVED,
    EVT_ALARM_TRIGGERED,
    EVT_ENTER_HOME,
    EVT_ENTER_TIME_SET,
    EVT_ENTER_ALARM_SET,
    EVT_ENTER_PLAY,
    EVT_ALARM_STOPPED,
    EVT_TIMEOUT_ALARM_CONFIRMED
};

// the root state
CompositeState Root("Root");
// The main state machine, takes the root state as parameter
MainSM sm(Root);

void lcd_print_line2(const char *text)
{
    for (int i = 0; text[i] != '\0'; i++)
    {
        lcd_set_cursor(1, i); // row 1 (second line), column i
        lcd_char(text[i]);    // print one character
    }
}

void display_time_temp()
{
    char time_str[10];
    char temp_str[10];

    // Simple time conversion (seconds to HH:MM:SS)
    uint32_t h = (current_time_seconds / 3600) % 24;
    uint32_t m = (current_time_seconds / 60) % 60;
    uint32_t s = current_time_seconds % 60;

    sprintf(time_str, "%02lu:%02lu:%02lu", h, m, s);

    sprintf(temp_str, "%.0f%cC", current_temp, (char)223);
    char line2[20];
    sprintf(line2, "%s %s", time_str, temp_str);

    // Line 1: Mode
    lcd_set_cursor(0, 0);
    lcd_print("HOME");

    // Line 2: Time and Temperature
    lcd_print_line2(line2);

    printf("ACTION: Clock displayed: %s, Temp: %s\n", time_str, temp_str);
}

void display_mode_home()
{
    printf("ENTRY: HOME. Displaying Time and Temp.\n");
    lcd_set_rgb(255, 255, 255); // Black background

    display_time_temp();
    vTaskResume(th_tick);
}

void update_clock_display()
{
    current_time_seconds++;
    lcd_clear();
    display_time_temp();
    printf("ACTION: Tick. Time updated.\n");
}

void show_mode_selection()
{
    printf("ENTRY: MODE_SELECT. Use pot to select mode.\n");
    lcd_clear();
    vTaskSuspend(th_tick);

    // Start Mode Select Timeout (5s)
    id1 =
        set_timeout(TIMEOUT_MODE_SELECT_MS, 1, EVT_TIMEOUT, &sm);

    // Initial display of selected_mode (HOME)
    lcd_set_cursor(0, 0);
    lcd_print("^ HOME");
}

void update_selected_mode()
{
    adc_select_input(POT_ADC_INPUT);
    uint16_t adc = adc_read();

    // Map ADC range (0-4095) to 4 modes (0-3)
    int mode_index = (adc * 4) / 4096;

    // Map index: 0->TIME_SET, 1->ALARM_SET, 2->PLAY, 3->HOME
    switch (mode_index)
    {
    case 0:
        selected_mode = MODE_TIME_SET;
        break;
    case 1:
        selected_mode = MODE_ALARM_SET;
        break;
    case 2:
        selected_mode = MODE_PLAY;
        break;
    case 3:
        selected_mode = MODE_HOME;
        break;
    }

    remove_timeout(id1);
    id1 = -1;
    id1 = set_timeout(TIMEOUT_MODE_SELECT_MS, 1, EVT_TIMEOUT, &sm);

    lcd_clear();
    lcd_set_cursor(0, 0);
    char buf[20];
    const char *mode_names[] = {"TIME_SET", "ALARM_SET", "PLAY", "HOME"};
    sprintf(buf, "^ %s", mode_names[mode_index]);
    lcd_print(buf);

    printf("ACTION: POT_MOVE. Selected mode updated to %s.\n", mode_names[mode_index]);
}

#define TIME_STR_LEN 8 // HH:MM:SS

void request_time_sync()
{
    char time_str[TIME_STR_LEN + 2]; // +1 for null terminator
    int idx = 0;

    printf("ENTRY: TIME_SET. Waiting for serial time input (HH:MM:SS).\n");

    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("TIME_SET");

    printf("Enter time (HH:MM:SS): ");

    // Read characters one by one
    int c = getchar();
    while (c != '\n' && idx < TIME_STR_LEN)
    {
        time_str[idx++] = (char)c;
        putchar(c); // echo back
        c = getchar();
    }
    time_str[idx] = '\0'; // null terminate
    int hours, minutes, seconds;

    // Parse time string using sscanf
    if (sscanf(time_str, "%2d:%2d:%2d", &hours, &minutes, &seconds) == 3)
    {
        // Validate
        if (hours >= 0 && hours < 24 &&
            minutes >= 0 && minutes < 60 &&
            seconds >= 0 && seconds < 60)
        {

            printf("Valid time: %02d:%02d:%02d\n", hours, minutes, seconds);

            current_time_seconds = hours * 3600 + minutes * 60 + seconds;
            lcd_print_line2(time_str);
            id2 = set_timeout(TIMEOUT_TIME_SET_MS, 1, EVT_TIMEOUT, &sm);
        }
        else
        {
            lcd_print_line2("INVALID TIME FORMAT!!!!!");
            id2 = set_timeout(3000, 1, EVT_TIMEOUT, &sm);

            printf("Invalid time values!\n");
        }
    }
    else
    {
        lcd_print_line2("INVALID FORMAT!!!!!");
        id2 = set_timeout(3000, 1, EVT_TIMEOUT, &sm);
        printf("Invalid format! Use HH:MM:SS\n");
    }
}

void show_alarm_set_screen()
{
    printf("ENTRY: ALARM_SET.\n");
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("Set your alarm!!!");
}

void show_alarm_confirm_screen()
{
    printf("ENTRY: ALARM_CONFIRMED.\n");
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("Alarm Confirmed!!!");

    // Start Timeout (2s)
    id3 = set_timeout(2000, 1, EVT_TIMEOUT, &sm);
    id4 = set_timeout(alarm_delay * 1000, 1, EVT_ALARM_TRIGGERED, &sm);
}

void update_alarm_delay()
{
    // Logic: set alarm_delay based on pot position (5s to 60s)
    adc_select_input(POT_ADC_INPUT);
    uint16_t adc = adc_read();

    // Map ADC range (0-4095) to 5-60 seconds (range of 55)
    alarm_delay = (adc * 55 / 4096) + 5;

    char buf[5];
    sprintf(buf, "%lu s\0", alarm_delay);
    lcd_clear();
    lcd_print("Set alarm");
    lcd_print_line2(buf);

    printf("ACTION: POT_MOVE. Alarm delay updated to %lu s.\n", alarm_delay);
}

void update_songs()
{
    // Logic to cycle through modes based on raw ADC pot value
    adc_select_input(POT_ADC_INPUT);
    uint16_t adc = adc_read();

    int mode_index = (adc * 2) / 4096;

    switch (mode_index)
    {
    case 0:
        buzzer_mode = BUZZ_PLAY;
        break;
    case 1:
        buzzer_mode = BUZZ_PLAY1;
        break;
    }
}

void play_music()
{
    printf("ENTRY: PLAY. Starting music.\n");
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("PLAY");
    lcd_print_line2("Turn to change music");
    // Start music
    vTaskResume(th_buzzer);
}

void stop_music()
{
    printf("ACTION: Stop music.\n");
    // Stop music
    vTaskSuspend(th_buzzer);
    buzzer_mode = BUZZ_OFF;
    buzzer_stop(&buzz);
}

void start_alarm()
{
    printf("ENTRY: ALARM. Starting alarm sequence (15s max).\n");

    // Suspend periodic clock updates
    vTaskSuspend(th_tick);
    stop_music(); // Ensure PLAY music is stopped if alarm triggered from PLAY
    buzzer_mode = BUZZ_ALARM;
    // Start LED flashing task
    vTaskResume(th_led);

    // Start alarm music
    vTaskResume(th_buzzer);

    id6 =
        set_timeout(15000, 1, EVT_TIMEOUT, &sm);
    // Initial alarm display (Red background)
    lcd_set_rgb(255, 0, 0);
    lcd_clear();
    lcd_print("!!! ALARM !!!");
}

void stop_alarm()
{
    printf("ACTION: Stop alarm music and lights.\n");

    // Stop LED flashing
    vTaskSuspend(th_led);
    gpio_put(LED_PIN, false);

    // Stop alarm music
    vTaskSuspend(th_buzzer);
    buzzer_mode = BUZZ_OFF;
    buzzer_stop(&buzz);
}

void show_alarm_stopping()
{
    printf("ENTRY: ALARM_STOPPING.\n");
    buzzer_mode = BUZZ_OFF;
    // Set to yellow background
    lcd_set_rgb(255, 165, 0);
    lcd_clear();
    id5 =
        set_timeout(2000, 1, EVT_TIMEOUT, &sm);

    lcd_print("ALARM STOPPED");
}

void show_alarm_stopped()
{
    printf("ENTRY: ALARM_STOPPED.\n");
    vTaskSuspend(th_buzzer);
    buzzer_mode = BUZZ_OFF;
    buzzer_stop(&buzz);
    sm.send(EVT_ALARM_STOPPED);
}

// --- Transition Guards
bool guard_enter_time_set(Evt_t e)
{
    return e == EVT_BUTTON && selected_mode == MODE_TIME_SET;
}
bool guard_enter_alarm_set(Evt_t e)
{
    return e == EVT_BUTTON && selected_mode == MODE_ALARM_SET;
}
bool guard_enter_play(Evt_t e)
{
    return e == EVT_BUTTON && selected_mode == MODE_PLAY;
}
bool guard_enter_home(Evt_t e)
{
    return e == EVT_BUTTON && selected_mode == MODE_HOME;
}

///////////////////////////////////////////////////////////////////
/// POTENTIOMETER (KNOB) TASK
///////////////////////////////////////////////////////////////////
void task_pot(void *arg)
{
    adc_select_input(POT_ADC_INPUT);
    uint16_t last_adc = adc_read(); // initialize with actual value

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(100)); // Check every 100ms
        adc_select_input(POT_ADC_INPUT);
        uint16_t current_adc = adc_read();

        // Check for significant movement
        if (abs((int)current_adc - (int)last_adc) > 50)
        {
            sm.send(EVT_POT_MOVE);
            last_adc = current_adc;
        }
    }
}

///////////////////////////////////////////////////////////////////
/// PERIODIC TICK & ALARM MONITOR TASK
///////////////////////////////////////////////////////////////////
void task_tick(void *arg)
{
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000)); // Every 1 second
        sm.send(EVT_TICK);

        // Alarm delay logic: Decrement if running
        if (alarm_running && alarm_delay > 0)
        {
            alarm_delay--;
            if (alarm_delay == 0)
            {
                alarm_running = false;
                sm.send(EVT_ALARM_TRIGGERED); // Send global alarm event
            }
        }
    }
}

///////////////////////////////////////////////////////////////////
/// BUTTON SENSOR TASK (MODIFIED to send EVT_BUTTON)
///////////////////////////////////////////////////////////////////

// handle of the task to wake up in the interrupt

// Interrupt handler routine
void gpio_callback(uint gpio, uint32_t events)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // if a RISE event
    if (events & GPIO_IRQ_EDGE_RISE)
    {
        // notify the task
        vTaskNotifyGiveFromISR(th_button, &xHigherPriorityTaskWoken);
    }
    // call the scheduler if a higher priority task has been woken up
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void task_button(void *arg)
{
    while (1)
    {
        ulTaskNotifyTake(pdFALSE,        // Decrements the notification value
                         portMAX_DELAY); // waits forever
        printf("Button pressed !!!\n");
        sm.send(EVT_BUTTON); // MODIFIED
    }
}

///////////////////////////////////////////////////////////////////
///  ON BOARD TEMPERATURE SENSOR (Unchanged)
///////////////////////////////////////////////////////////////////
#define TEMPERATURE_UNITS 'C'

float read_onboard_temperature(const char unit)
{
    /* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
    const float conversionFactor = 3.3f / (1 << 12);

    adc_select_input(4); // the onboard sensor is on ADC 4
    float adc = (float)adc_read() * conversionFactor;
    float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

    if (unit == 'C')
    {
        return tempC;
    }
    else if (unit == 'F')
    {
        return tempC * 9 / 5 + 32;
    }

    return -1.0f;
}

// handle of the temperature task

void task_temp(void *arg)
{
    while (1)
    {
        // every 4 s
        vTaskDelay(pdMS_TO_TICKS(4000));
        // reads the temperature
        float temperature = read_onboard_temperature(TEMPERATURE_UNITS);
        current_temp = temperature; // Store global temperature
        // printf("Onboard temperature = %.02f %c\n", temperature, TEMPERATURE_UNITS);
    }
}

///////////////////////////////////////////////////////////////////
/// BLINK LED (Unchanged - control via suspend/resume in actions)
///////////////////////////////////////////////////////////////////
void task_led(void *arg)
{
    while (1)
    {
        // waits 500 ms
        vTaskDelay(pdMS_TO_TICKS(500));
        // turn led on
        gpio_put(LED_PIN, true);
        // waits 500 ms
        vTaskDelay(pdMS_TO_TICKS(500));
        // turn led off
        gpio_put(LED_PIN, false);
    }
}

///////////////////////////////////////////////////////////////////
/// SERIAL INPUT TASK (NEW)
/// Reads characters from the USB serial line to sync time
///////////////////////////////////////////////////////////////////
void task_serial(void *arg)
{
    while (1)
    {
        if (stdio_usb_connected() && getchar_timeout_us(0) != PICO_ERROR_TIMEOUT)
        {
            char buffer[32];
            int i = 0;

            while (i < 31)
            {
                int c = getchar_timeout_us(100000);
                if (c == '\n' || c == PICO_ERROR_TIMEOUT)
                    break;
                buffer[i++] = (char)c;
            }
            buffer[i] = '\0';

            if (strstr(buffer, "SYNC") != NULL || strstr(buffer, "TIME:") != NULL)
            {
                printf("Serial input received: %s. Sending EVT_TIME_RECEIVED.\n", buffer);

                sm.send(EVT_TIME_RECEIVED);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

///////////////////////////////////////////////////////////////////
/// PLAY ARPEGE A minor (Unchanged - control via suspend/resume in actions)
///////////////////////////////////////////////////////////////////

void task_buzzer(void *arg)

{
    while (1)
    {
        switch (buzzer_mode)
        {
        case BUZZ_OFF:
            buzzer_stop(&buzz);
            vTaskDelay(pdMS_TO_TICKS(100));
            break;

        case BUZZ_PLAY:
            buzzer_stop(&buzz);
            buzzer_start(&buzz, 440); // A4
            vTaskDelay(pdMS_TO_TICKS(250));
            buzzer_stop(&buzz);
            buzzer_start(&buzz, 523); // C5
            vTaskDelay(pdMS_TO_TICKS(250));
            buzzer_stop(&buzz);
            buzzer_start(&buzz, 659); // E5
            vTaskDelay(pdMS_TO_TICKS(250));
            buzzer_stop(&buzz);
            buzzer_start(&buzz, 523); // C5
            vTaskDelay(pdMS_TO_TICKS(250));
            break;

        case BUZZ_PLAY1:
            buzzer_stop(&buzz);
            buzzer_start(&buzz, 523); // Play C5
            vTaskDelay(250);

            buzzer_stop(&buzz);
            buzzer_start(&buzz, 659); // Play E5
            vTaskDelay(250);

            buzzer_stop(&buzz);
            buzzer_start(&buzz, 784); // Play G5
            vTaskDelay(250);

            buzzer_stop(&buzz);
            buzzer_start(&buzz, 659); // Play E5
            vTaskDelay(250);

            buzzer_stop(&buzz);
            buzzer_start(&buzz, 659); // Play E5
            vTaskDelay(250);

            buzzer_stop(&buzz);
            buzzer_start(&buzz, 784); // Play G5
            vTaskDelay(250);

            buzzer_stop(&buzz);
            buzzer_start(&buzz, 659); // Play E5
            vTaskDelay(250);

            buzzer_stop(&buzz);
            buzzer_start(&buzz, 659); // Play E5
            vTaskDelay(250);

            buzzer_stop(&buzz);
            buzzer_start(&buzz, 784); // Play G5
            vTaskDelay(250);

            buzzer_stop(&buzz);
            buzzer_start(&buzz, 659); // Play E5
            vTaskDelay(250);

            break;

        case BUZZ_ALARM: // ALARM mode music
            buzzer_stop(&buzz);
            buzzer_start(&buzz, 523); // C5
            vTaskDelay(pdMS_TO_TICKS(300));
            buzzer_stop(&buzz);
            buzzer_start(&buzz, 659); // E5
            vTaskDelay(pdMS_TO_TICKS(300));
            buzzer_stop(&buzz);
            buzzer_start(&buzz, 784); // G5
            vTaskDelay(pdMS_TO_TICKS(300));
            buzzer_stop(&buzz);
            buzzer_start(&buzz, 987); // B5
            vTaskDelay(pdMS_TO_TICKS(300));
            buzzer_stop(&buzz);
            buzzer_start(&buzz, 1046); // C6
            vTaskDelay(pdMS_TO_TICKS(500));
            break;
        }
    }
}

//////////////////////////////////////////////////////////////
/// MAIN
//////////////////////////////////////////////////////////////
int main()
{
    // Init input/output via usb
    stdio_init_all();

    // initializes adc
    adc_init();

    // The LCD is connected to I2C1
    lcd_init(i2c1, LCD_I2C_ADDRESS, LCD_RGB_ADDRESS);
    lcd_set_cursor(0, 0);

    // Initialize The LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Initialize the button
    gpio_init(BUTTON_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN,
                                       GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
                                       true, &gpio_callback);

    // Initialize the buzzer
    buzzer_init(&buzz, BUZZ_PIN); // connected to A1

    // --- Statechart Definition ---

    // Top-Level States
    CompositeState Normal("NORMAL");
    CompositeState Alarm("ALARM");

    Root.add_child(Normal);
    Root.add_child(Alarm);
    Root.set_initial(Normal); // Root starts in Normal

    // Normal Substates (attached entry/exit actions)
    SimpleState Home("HOME", display_mode_home, empty_action);
    SimpleState Mode_Select("MODE_SELECT", show_mode_selection, empty_action);
    SimpleState TimeSet("TIME_SET", request_time_sync, empty_action);

    SimpleState AlarmSet("ALARM_SET", show_alarm_set_screen, empty_action);
    SimpleState AlarmConfirmed("ALARM_CONFIRMED", show_alarm_confirm_screen, empty_action);

    SimpleState Play("PLAY", play_music, stop_music);                    // Stop music on exit
    SimpleState Alarm_Running("ALARM_RUNNING", start_alarm, stop_alarm); // Stop alarm on exit
    SimpleState Alarm_Stopping("ALARM_STOPPING", show_alarm_stopping, empty_action);
    SimpleState Alarm_Stopped("ALARM_STOPPED", show_alarm_stopped, empty_action);

    Normal.add_child(Home);
    Normal.add_child(Mode_Select);
    Normal.add_child(TimeSet);

    Normal.add_child(AlarmSet);
    Normal.add_child(AlarmConfirmed);
    Normal.add_child(Play);
    Normal.set_initial(Home); // Normal starts in Home

    Alarm.add_child(Alarm_Running);
    Alarm.add_child(Alarm_Stopping);
    Alarm.add_child(Alarm_Stopped);
    Alarm.set_initial(Alarm_Running); // ALARM starts in AlarmRunning

    Transition NormalToAlarm(Normal, Alarm, [](Evt_t e)
                             { return e == EVT_ALARM_TRIGGERED; }, []()
                             { printf("GLOBAL: Normal → ALARM\n"); });
    Transition AlarmToNormal(Alarm, Normal, [](Evt_t e)
                             { return e == EVT_ALARM_STOPPED; }, []()
                             { printf("GLOBAL: ALARM → Normal\n"); });

    Transition HomeSelf(Home, Home, [](Evt_t e)
                        { return e == EVT_TICK; }, update_clock_display);

    Transition HomeToMode_Select(Home, Mode_Select, [](Evt_t e)
                                 { return e == EVT_POT_MOVE; }, []()
                                 { printf("HOME → MODE_SELECT\n"); });

    Transition Mode_SelectSelf(Mode_Select, Mode_Select, [](Evt_t e)
                               { return e == EVT_POT_MOVE; }, update_selected_mode);

    Transition Mode_SelectToHome(Mode_Select, Home, [](Evt_t e)
                                 { return e == EVT_TIMEOUT; }, []()
                                 { printf("MODE_SELECT → HOME (Timeout)\n"); });

    Transition Mode_SelectToMode_Timeset(Mode_Select, TimeSet, guard_enter_time_set,
                                         []()
                                         { remove_timeout(id1);
    id1 = -1;
    printf("MODE_SELECT → TIME_SET\n"); });

    Transition Mode_SelectToAlarmSet(Mode_Select, AlarmSet, guard_enter_alarm_set,
                                     []()
                                     { remove_timeout(id1);
    id1 = -1;
    printf("MODE_SELECT → ALARM_SET\n"); });

    Transition Mode_SelectToPlay(Mode_Select, Play, guard_enter_play,
                                 []()
                                 { remove_timeout(id1);buzzer_mode = BUZZ_PLAY;
    id1 = -1;
    printf("MODE_SELECT → Play\n"); });

    Transition PlayToHome(Play, Home, [](Evt_t e)
                          { return e == EVT_BUTTON; }, []()
                          { printf("PLAY → HOME\n"); }
                          // stop_music is handled by Play's exit action
    );

    Transition Time_SetToHome(TimeSet, Home, [](Evt_t e)
                              { return e == EVT_TIMEOUT; }, []()
                              {
                                selected_mode = MODE_HOME;
                                printf("TIME_SET → HOME (Timeout)\n"); });

    Transition AlarmSetSelf(AlarmSet, AlarmSet, [](Evt_t e)
                            { return e == EVT_POT_MOVE; }, update_alarm_delay);

    Transition AlarmSetToAlarmConfirmed(AlarmSet, AlarmConfirmed, [](Evt_t e)
                                        { return e == EVT_BUTTON; }, []()
                                        { printf("ALARM_SET → AlarmConfirmed (Button, Alarm countdown started)\n"); });

    Transition AlarmConfirmedToHome(AlarmConfirmed, Home, [](Evt_t e)
                                    { return e == EVT_TIMEOUT; }, []()
                                    { printf("ALARM_Confirmed → HOME (Timeout, Alarm countdown started)\n"); });

    Transition AlarmRunningToStopping(Alarm_Running, Alarm_Stopping, [](Evt_t e)
                                      { return e == EVT_BUTTON; }, stop_alarm);

    Transition AlarmRunningToStopped(Alarm_Running, Alarm_Stopped, [](Evt_t e)
                                     { return e == EVT_TIMEOUT; }, stop_alarm);

    Transition StoppingToStopped(Alarm_Stopping, Alarm_Stopped, [](Evt_t e)
                                 { return e == EVT_TIMEOUT; }, []()
                                 { printf("ALARM_STOPPING → ALARM_STOPPED\n"); });
    Transition PlaySelf(Play, Play, [](Evt_t e)
                        { return e == EVT_POT_MOVE; }, update_songs);

    // --- Task Creation ---
    xTaskCreate(task_pot, "pot task", PICO_STACK_SIZE, NULL, 1, &th_pot);
    xTaskCreate(task_tick, "tick task", PICO_STACK_SIZE, NULL, 1, &th_tick);
    xTaskCreate(task_button, "button task", PICO_STACK_SIZE, NULL, 1, &th_button);
    xTaskCreate(task_led, "led task", PICO_STACK_SIZE, NULL, 1, &th_led);
    xTaskCreate(task_temp, "temp task", PICO_STACK_SIZE, NULL, 1, &th_temp);
    xTaskCreate(task_serial, "serial task", PICO_STACK_SIZE, NULL, 1, &th_serial);
    xTaskCreate(task_buzzer, "buzzer task", PICO_STACK_SIZE, NULL, 1, &th_buzzer);

    vTaskSuspend(th_buzzer);
    vTaskSuspend(th_led);
    vTaskStartScheduler();

    while (1)
        ;
}