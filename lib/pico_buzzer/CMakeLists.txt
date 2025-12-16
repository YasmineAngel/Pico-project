add_library(pico_buzzer
	buzzer.c
)

target_include_directories(pico_buzzer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# pull in common dependencies
target_link_libraries(pico_buzzer
	hardware_adc
	hardware_pwm)

pico_enable_stdio_usb(pico_lcd 1)
pico_enable_stdio_uart(pico_lcd 0)
