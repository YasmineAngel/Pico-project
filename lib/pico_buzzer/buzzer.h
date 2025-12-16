#pragma once

#include "hardware/adc.h"
#include "hardware/pwm.h"

typedef struct buzzer_s {
    uint pin;
    uint slice_num;
    uint channel;
} buzzer_t;

#ifdef __cplusplus
extern "C"
{
#endif
void buzzer_init(buzzer_t *b, int pin);
void buzzer_start(buzzer_t *b, uint freq);
void buzzer_stop(buzzer_t *b);

#ifdef __cplusplus
}
#endif
