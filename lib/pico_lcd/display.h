/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <vector>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "FreeRTOS.h"
#include "task.h"
#include "i2c_scan.h"
#include "lcd.h"
#include "onboard_temp.h"
#include "grove_map.h"

// Déclaration des variables externes
extern char *oldtemp;

// Déclaration des fonctions
void vsetup(void);
void vDisplay_onboard_temperature(void *args);
void vDisplayHello(char *args);
void vGetLightSample(char *args);
void display_temp(void);
void stop_display_temp(void);

#endif // DISPLAY_H
