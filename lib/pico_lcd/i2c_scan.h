#ifndef __I2C_SCAN__H
#define __I2C_SCAN__H
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdio.h>
#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C"
{
#endif
extern bool reserved_addr(uint8_t addr);
extern void i2c_scan(i2c_inst_t * i2c_bus);
#ifdef __cplusplus
}
#endif
#endif
