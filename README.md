
# Table of Contents

1.  [Introduction](#org1c9b8bc)
2.  [Application](#orge40d5b9)
    1.  [Sensors](#orgdf4d2b1)
    2.  [Requirements](#org7df90e8)
    3.  [Serial line](#org0266e62)
    4.  [Bug on the LCD module](#org7adb13f)
3.  [Documentation](#org0167bb7)



<a id="org1c9b8bc"></a>

# Introduction

The goal of this project is to learn how to program a simple embedded
system using a RTOS. In particular, we will make use of FreeRTOS:

<https://www.freertos.org/>

We will use a Raspberry Pico W, with a Grove Shield

<https://wiki.seeedstudio.com/Grove-Starter-Kit-for-Raspberry-Pi-Pico/>

We will program in C/C++ and configure and compile using CMake.


<a id="orge40d5b9"></a>

# Application


<a id="orgdf4d2b1"></a>

## Sensors

-   Buzzer
-   Potentiometer
-   Button
-   Led
-   LCD
-   Light Sensor
-   Temperature Sensor (on board, integrated)


<a id="org7df90e8"></a>

## Requirements

Initially the display shows the current mode on the first line. The
possible working modes are:

-   HOME, TIME\_SET, ALARM\_SET, PLAY, ALARM

The second line shows information relative to the current mode. 

In mode HOME, the display shows the time (hour/minutes/seconds) and
the current temperature on the second line.  The time is updated every
second.

In mode TIME\_SET, the time is obtained by the serial line (USB
interface) and is synchronized with the actual time. When the board
receives the time, it updates it and returns to mode HOME. 

-   **BONUS** (Optionally) you may show the time on the second line for 2
    seconds before returning to HOME.

In mode ALARM\_SET, it is possible to set an alarm. By using the knob
(potentiometer) and the button, it is possible to set a timer after an
interval between 5 and 60 seconds. Two seconds after setting the
alarm, the system returns to HOME automatically.

After the elapsed time, the systems goes into ALARM mode, and

-   a little music is played;
-   the display background light flashes between two colors;
-   the led flashes.

The ALARM mode is kept for a maximum of 15 seconds, then we return to
HOME. If the button is pressed within the 15 seconds, the alarm is
stopped (no music, no flashing lights) and after 2 second the system
goes back to HOME.

In mode PLAY, the system plays a music (different from the alarm
music), until the button is pressed, in which case we return to HOME
and stop the music. If the alarm is fired in PLAY mode, the system
stops playing the music and goes to the ALARM mode.

**BONUS**: Optionally, you can play different tunes, and let the user
chose which tune to play.

How to switch between modes? When in HOME, we use the knob
(potentiometer) and the button:

-   with the potentiometer, we show on the display which mode we want to
    go preceded by a "^" symbol.
-   when the user presses the button, it goes into that mode, shows it
    on the screen without the "^" symbol, and accepts commands for it.

When we exit from any mode, we always go back to HOME.


<a id="org0266e62"></a>

## Serial line

To communicate via USB with the system, you may use minicom, or you
may want to write a simple Python script using the serial library. An
example will be provided to you on how to use the serial line input. 


<a id="org7adb13f"></a>

## Bug on the LCD module

The LCD module has some little bug that has not yet been solved at the
time of this writing, and the lcd\_print() function does not work
properly. I think I will be able to solve it soon, however in the
meantime you can send the output on the serial line with minicom.


<a id="org0167bb7"></a>

# Documentation

-   The Pico SDK library <https://www.raspberrypi.com/documentation/pico-sdk/>
-   Basics of RTOS       <https://www.freertos.org/Documentation/01-FreeRTOS-quick-start/01-Beginners-guide/01-RTOS-fundamentals>
-   FreeRTOS API:        <https://www.freertos.org/Documentation/02-Kernel/04-API-references/01-Task-creation/00-TaskHandle>
-   For the buzzer: how to use the PWM   <https://www.i-programmer.info/programming/148-hardware/14849-the-pico-in-c-basic-pwm.html>
-   The Pico Grove:      <https://wiki.seeedstudio.com/Grove-Starter-Kit-for-Raspberry-Pi-Pico/>

