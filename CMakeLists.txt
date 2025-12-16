# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/home/m1io/hanan-miftah.agraw.etu/pico-sdk")

# Locate FreeRTOS
set(FREERTOS_KERNEL_PATH "/home/m1io/hanan-miftah.agraw.etu/FreeRTOS-Kernel")

# The type of physical board is pico_w 
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

# Pull FreeRTOS specific definitions
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

set(FREERTOS "True")

###########################################################
# The Project
###########################################################

project(project C CXX ASM)

# Exceptions in C++ are enabled
set(PICO_CXX_ENABLE_EXCEPTIONS 1)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(project
	project.cpp)

pico_set_program_name(project "project")
pico_set_program_version(project "0.1")

pico_enable_stdio_uart(project 1)
pico_enable_stdio_usb(project 1)

add_subdirectory(lib/sc_minilib)
add_subdirectory(lib/pico_lcd)
add_subdirectory(lib/pico_buzzer)

# Add the standard library to the build
target_link_libraries(project
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(project PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add the FreeRTOS library 
target_link_libraries(project
        hardware_timer
		hardware_adc            # for ADC based sensors
		hardware_i2c            # for the LCD
		sc_minilib
		pico_lcd
		pico_buzzer)
#        FreeRTOS-Kernel-Heap4

# Here you can add additional libraries
# target_link_libraries(project ...)

pico_add_extra_outputs(project)

