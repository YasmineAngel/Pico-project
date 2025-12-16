#include "hardware/pwm.h"
#include "hardware/adc.h"

typedef struct buzzer_s {
    uint pin;
    uint slice_num;
    uint channel;
} buzzer_t;


uint32_t pwm_set_freq_duty(uint slice_num, uint chan, uint32_t freq, int duty)
{
    uint32_t clock = 125000000;
    uint32_t divider16 = clock / freq / 4096 + 
        (clock % (freq * 4096) != 0);
    
    if (divider16 / 16 == 0)
        divider16 = 16;
    
    uint32_t wrap = clock * 16 / divider16 / freq - 1;
    pwm_set_clkdiv_int_frac(slice_num, divider16/16, divider16 & 0xF);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, chan, wrap * duty / 100);

    return wrap;
}

void buzzer_init(buzzer_t *b, int pin)
{
    b->pin = pin;
    gpio_set_function(pin, GPIO_FUNC_PWM);
    b->slice_num = pwm_gpio_to_slice_num(pin);
    b->channel = pwm_gpio_to_channel(pin);
}

void buzzer_start(buzzer_t *b, uint freq)
{
    pwm_set_freq_duty(b->slice_num, b->channel, freq, 50); 
    pwm_set_enabled(b->slice_num, true);
}

void buzzer_stop(buzzer_t *b)
{
    pwm_set_enabled(b->slice_num, false);
}

