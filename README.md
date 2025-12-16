# Example of using FreeRTOS on the Raspberry Pico 

## Requirements

To run this example you will need: 
- A Raspberry Pico W
- A Grove Shield 
  https://wiki.seeedstudio.com/Grove-Starter-Kit-for-Raspberry-Pi-Pico/
- The following sensors/actuators
  - light sensor 
  - button
  - led
  - buzzer
  - lcd 

Regarding the software you need:
- The Pico SDK 
  https://github.com/raspberrypi/pico-sdk
- FreeRTOS Kernel
  https://github.com/FreeRTOS/FreeRTOS-Kernel
- A recente version of CMake

- The minicom program. In Ubuntu like systems, 
```
	sudo install minicom
```	
  

## Compiling

Before starting to compile, set up the correct path variables in the
`CMakeLists.txt` 

	set(PICO_SDK_PATH <path to sdk>)
	set(FREERTOS_KERNEL_PATH <path to freertos kernel>)

Then, configure the build system: 

	mkdir build
	cd build 
	cmake ..
	
If everything goes ok, you can compile with 

	make 
	
This will compule the sdk, freertos and the example application.  To
recompile, you only need to relaunch `make`, unless you change one of
the configuration files (the `CMakeLists.txt` files).

If everything compiles correctly, the resulting application will be
found in file :

	build/example_sc_minilib.uf2

## The SC Mini Lib

Currently there is no documentation for SC-MiniLib library, you need
to look at the comments in the header files, for example the

	lib.sc_minilib.state_chart.hpp
	
Also read carefully the example code to understand how to use the library. 

# Flash and running 

Keep pressed the button on the Pico board while connecting it to the
USB port of your PC; in this way, the board will be connected as a USB
drive. Copy the application file `build/example_sc_minilib.uf2` to the
USB drive. As soon as the copy is completed, the USB drive will
disconnect and the board will reboot to launch your application.

# Monitoring 

To see the output, you need the minicom program :

	minicom -b 115200 -o -D /dev/ttyACM0
	
A convenient script is available in file `mcom.sh`.


  
