/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "FreeRTOS.h"
#include "semphr.h"

// FREERTOS
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
// CURRENT include
#include "i2c_scan.h"
#include "lcd.h"
#include "onboard_temp.h"
#include "grove_map.h"

#include <iostream>
#include <vector>
char *oldtemp;
TaskHandle_t temph = NULL;
SemaphoreHandle_t mu= xSemaphoreCreateMutex();
void vsetup(void)
{
	stdio_init_all();
	adc_gpio_init(ADC_SENSOR_SOUND_PIN);
	adc_gpio_init(ADC_SENSOR_LIGHT_PIN);
	adc_init();
	adc_set_temp_sensor_enabled(true);
	sleep_ms(1000);
	lcd_init(i2c1, LCD_I2C_ADDRESS, LCD_RGB_ADDRESS);
	i2c_scan(i2c1);
	lcd_set_cursor(0,0);
}

void vDisplay_onboard_temperature(void * args)
{
    std::vector<float> t;
    for (;;)
    {
        lcd_clear();
        adc_select_input(4);
        float temperature = read_onboard_temperature('C');
        t.push_back(temperature);
        char str[20];
        char countstr[10];
        sprintf(str, "%.2f", temperature);
        sprintf(countstr, "%d", t.size());
        char * str_temp = strstr(str, ".00");
        if (str_temp) str_temp = 0x0;
        if(strcmp(oldtemp,str)!=0) {
            lcd_print("temp: ");
            lcd_print(str);
            oldtemp=str;
            //lcd_print("\nCount: ");
            //lcd_print(countstr);
            vTaskDelay(1000);
        }
        vTaskDelete(NULL);
    }
}

void vDisplayHello(char * args) {

    xSemaphoreTake(mu,portMAX_DELAY);
		lcd_clear();
		lcd_print(args);
    xSemaphoreGive(mu);
}

void vGetLightSample(char * args)
{
		printf("Light sensor: ");
		lcd_clear();
		adc_select_input(1);
		uint adc_raw = adc_read();
		sprintf(args, "%.2f", adc_raw * ADC_SENSOR_SOUND_CONVERT);

}
void display_temp(){
    xTaskCreate(vDisplay_onboard_temperature, "RTOS Temp", 128, NULL, 1, &temph);
}

void stop_display_temp(){
    lcd_clear();
    vTaskDelete(temph);
}
