#ifndef __LCD__H
#define __LCD__H
#include <pico/stdlib.h>
#include <hardware/i2c.h>
#include <pico/binary_info.h>

#ifdef __cplusplus
extern "C"
{
#endif
//// flag for backlight control
#define REG_MODE1	0x00
#define REG_MODE2	0x01
#define REG_OUTPUT	0x08

#define LCD_BACKLIGHT   0x08
#define LCD_ENABLE_BIT   0x04

//// Modes for lcd_send_byte
#define LCD_CHARACTER  1	// to send data
#define LCD_COMMAND    0	// to send command
				//
#define MAX_LINES      2	// max line possible
#define MAX_CHARS      16	// max char per line

// Device I2C Arress
#define LCD_I2C_ADDRESS     	(0x7c>>1) // = 0x3E
#define LCD_RGB_ADDRESS     	(0xc4>>1) // = 0x62
#define RGB_ADDRESS_V5   	(0x30)


// color define
#define WHITE           0
#define RED             1
#define GREEN           2
#define BLUE            3

#define REG_MODE1       0x00
#define REG_MODE2       0x01
#define REG_OUTPUT      0x08

// commands
#define LCD_CLEARDISPLAY 	0x01
#define LCD_RETURNHOME 		0x02
#define LCD_ENTRYMODESET 	0x04
#define LCD_DISPLAYCONTROL 	0x08
#define LCD_CURSORSHIFT 	0x10
#define LCD_FUNCTIONSET 	0x20
#define LCD_SETCGRAMADDR 	0x40
#define LCD_SETDDRAMADDR 	0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 		0x00
#define LCD_ENTRYLEFT 		0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 	0x04
#define LCD_DISPLAYOFF 	0x00
#define LCD_CURSORON 	0x02
#define LCD_CURSOROFF 	0x00
#define LCD_BLINKON 	0x01
#define LCD_BLINKOFF 	0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 	0x00
#define LCD_MOVERIGHT 	0x04
#define LCD_MOVELEFT 	0x00

// flags for function set
#define LCD_8BITMODE 	0x10 	// send data 8 bit D0-D7
#define LCD_4BITMODE 	0x00 	// send data 4 bit D4-D7
#define LCD_1LINE 	0x00	// one line
#define LCD_2LINE	0x08	// two lines
#define LCD_5x10DOTS 	0x04	// Font size matrix 5x10
#define LCD_5x8DOTS 	0x00	// Font size matrix 5x8

#define DEFAULT_I2C_CLOCK	100000UL

#define I2C1_SDA_PIN		PICO_DEFAULT_I2C_SDA_PIN + 2
#define I2C1_SCL_PIN		PICO_DEFAULT_I2C_SDA_PIN + 2

#define LCD_CMD_REG_ADDR	0x40
#define LCD_DATA_REG_ADDR	0x80

typedef struct lcd_instance
{
	i2c_inst_t *	i2c_bus;
	uint8_t 	lcd_address;
	uint8_t 	rgb_address;
	uint8_t		currentRegisterValue;
	uint8_t 	_functionset;
	uint8_t		_display;
	uint8_t		_mode;
} LCD;

extern void lcd_init(i2c_inst_t *i2c_instance, uint8_t address, uint8_t rgb_address);
extern void lcd_set_cursor(int line, int position);
extern void lcd_char(char val);
extern void lcd_print(const char *s);
extern void lcd_clear(void);
extern void lcd_send_byte(uint8_t val, int mode);
extern void lcd_set_rgb(uint8_t r, uint8_t g, uint8_t b);
extern bool reserved_addr(uint8_t addr);
extern void lcd_set_rgb_reg(uint8_t reg, uint8_t data);
extern void lcd_i2c_scan();
#ifdef __cplusplus
}
#endif
#endif
