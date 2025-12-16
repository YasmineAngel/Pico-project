add_library(pico_lcd
		lcd.c
		i2c_scan.c
)

target_include_directories(pico_lcd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# pull in common dependencies
target_link_libraries(pico_lcd
	pico_stdlib
	hardware_i2c
	hardware_adc)

pico_enable_stdio_usb(pico_lcd 1)
pico_enable_stdio_uart(pico_lcd 1)
