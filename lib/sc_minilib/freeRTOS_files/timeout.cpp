#include "includes/timeout.h"
#include "includes/sm_defs.h"
#include "includes/ptime.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "pico/cyw43_arch.h"

TaskHandle_t  send_event_handle = NULL;

typedef enum {
    FREE,
    INIT
} Status;

int ids=0;
SemaphoreHandle_t m= xSemaphoreCreateMutex();

typedef struct {
    int id;
    int evt_id;
    StateMachine *sm;
    xTimerHandle timer;
    Status status;
} TimerElement;

TimerElement timerElements[MAX_TIMEOUTS];

void timer_list_init() {
    for (int i = 0; i < MAX_TIMEOUTS; i++) {
        timerElements[i].id = 0;
        timerElements[i].evt_id = 0;
        timerElements[i].sm = NULL;
        timerElements[i].timer = NULL;
        timerElements[i].status = FREE;
    }
}

TimerElement* find_element_by_id(int timer_id) {
    for (int i = 0; i < MAX_TIMEOUTS; i++) {
        if (timerElements[i].id == timer_id) {
            return &timerElements[i];
        }
    }
    return NULL;
}

void elements_free(TimerElement* element)
{
    element->id = 0;
    element->evt_id = 0;
    element->sm = nullptr;
    element->timer = nullptr;
    element->status = FREE;
}

TimerElement* get_free_element() {
    for (int i = 0; i < MAX_TIMEOUTS; i++) {
        if (timerElements[i].status == FREE) {
            return &timerElements[i];
        }
    }
    return NULL;
}


void send_event_task(void* xTimer){
    int timer_id = (int) pvTimerGetTimerID( (TimerHandle_t)xTimer );

    TimerElement* element=find_element_by_id(timer_id);
    if(element!=NULL){
        element->sm->send(element->evt_id);
        elements_free(element);
    }

    vTaskDelete(NULL);
}

void vTimerCallback( TimerHandle_t xTimer )
{
    xTaskCreate(send_event_task, "send_event_thread", PICO_STACK_SIZE, xTimer ,5,&send_event_handle);
}



int set_timeout(ptime howlong, int unit, int evt_id, StateMachine *sm)
{

    tspec now;
    xSemaphoreTake(m,portMAX_DELAY);
    get_custom_tspec(&now);

    auto free_element = get_free_element();
    if (free_element == NULL) {
        xSemaphoreGive(m);
        return ERR_MAX_TIMEOUTS;
    }
    free_element->id = ids++;
    free_element->evt_id = evt_id;
    free_element->sm = sm;
    free_element->status=INIT;

    tspec t= tspec_add_delta(&now, howlong, unit);

    free_element->timer = xTimerCreate (
                    "Timer",
                    elapsedTicksBetweenTimespecs(&now, &t),
                    pdFALSE,
                    ( void * ) free_element->id,
                    vTimerCallback
            );
    xTimerStart( free_element->timer, 0 );
    xSemaphoreGive(m);

    return free_element->id;
}

void remove_timeout(int id_elem)
{
    xSemaphoreTake(m,portMAX_DELAY);
    auto it = find_element_by_id(id_elem);
    xTimerDelete(it->timer,0);
    elements_free(it);
    xSemaphoreGive(m);
}
