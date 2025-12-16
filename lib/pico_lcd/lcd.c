#include "lcd.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "hardware/resets.h"
#ifdef __cplusplus
extern "C" 
{
#endif
LCD _lcd_instance; 
LCD *_self = &_lcd_instance; 
const static uint8_t row_address_offset[2] = {0x80, 0xC0};

static int line = 0; 
static int column = 0; 
static inline void init_i2c_if_necessary(i2c_inst_t * i2c_instance)
{
	// Enable I2C1	
	gpio_pull_up(I2C1_SDA_PIN); 
	gpio_pull_up(I2C1_SCL_PIN); 
	gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN + 2, GPIO_FUNC_I2C); 
	gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN + 2, GPIO_FUNC_I2C); 
	printf("after set_function\n"); 

	i2c_init(i2c_instance, 100*1000);
}


void lcd_init(i2c_inst_t *i2c_bus, uint8_t address, uint8_t rgb_address)
{
	// LCD Instance initialization
	_self->i2c_bus 		= i2c_bus; 
	_self->lcd_address 	= address;
	_self->rgb_address 	= rgb_address; 
	_self->_functionset	= 0; 
	_self->_display 	= 0; 
	_self->_mode    	= 0;

	// check if I2C1 pin are initialized
	if (gpio_get_function(I2C1_SDA_PIN) != GPIO_FUNC_I2C ||
	    gpio_get_function(I2C1_SCL_PIN) != GPIO_FUNC_I2C)
	{
		printf("I'll init i2c\n"); 
		init_i2c_if_necessary(_self->i2c_bus); 
	}
	
	// Initialization for the Hitachi HD44780 microcontroller
	// https://fr.wikipedia.org/wiki/HD44780 see initialization section.
	
	_self->_functionset = LCD_FUNCTIONSET | LCD_2LINE| LCD_5x8DOTS; 
    sleep_ms(500);
    lcd_send_byte(_self->_functionset, LCD_COMMAND); 
	sleep_ms(50); 
    lcd_send_byte(_self->_functionset, LCD_COMMAND); 
	sleep_ms(50); 
    lcd_send_byte(_self->_functionset, LCD_COMMAND); 
	sleep_ms(50);     
    lcd_send_byte(_self->_functionset, LCD_COMMAND); 


	// display
	_self->_display = LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
    lcd_send_byte(_self->_display, LCD_COMMAND); 

	lcd_clear(); 
	_self->_mode = LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	lcd_send_byte(_self->_mode, LCD_COMMAND);

	lcd_set_rgb_reg(REG_MODE1,0); 
	lcd_set_rgb_reg(REG_OUTPUT, 0xFF); 
	lcd_set_rgb_reg(REG_MODE2, 0x20); 
    
	lcd_set_rgb(255, 255,255); 
}

void lcd_clear(void)
{
	lcd_send_byte(LCD_CLEARDISPLAY, LCD_COMMAND); 
	column = 0; 
	line = 0; 
	sleep_ms(20); 
}

void lcd_set_cursor(int line, int position)
{
	int val = row_address_offset[line]+position; 
	lcd_send_byte(val, LCD_COMMAND); 
}

void lcd_char(char val)
{
	if (val == '\n') {
		line+=1; 
		column = 0; 
		return;
	}
	lcd_send_byte(val, LCD_CHARACTER);
	column++;
	lcd_set_cursor(line, column); 
}

   
void lcd_print(const char *s)
{
	while(*s)
		lcd_char(*s++); 
}

   
void lcd_send_byte(uint8_t val, int mode)
{
	if (mode == LCD_CHARACTER) {
		uint8_t data[2] = {0x40, val};
		i2c_write_blocking(_self->i2c_bus, _self->lcd_address, &data[0], 2, false); 
	} else if (mode == LCD_COMMAND) { 	
		uint8_t data[2] = {0x80, val};
		i2c_write_blocking(_self->i2c_bus, _self->lcd_address, &data[0], 2, false); 
	}
}

void lcd_set_rgb_reg(uint8_t reg, uint8_t data)
{
	uint8_t rgb_data[2] = {reg, data}; 
	i2c_write_blocking(_self->i2c_bus, _self->rgb_address, &rgb_data[0], 2, false); 
}

    
void lcd_set_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	lcd_set_rgb_reg(0x04, r); 
	lcd_set_rgb_reg(0x03, g); 
	lcd_set_rgb_reg(0x02, b); 
}

    
#ifdef __cplusplus
}
#endif
