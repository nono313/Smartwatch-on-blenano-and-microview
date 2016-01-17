# Smartwatch on microcontrollers

A new market in the consumer electronics domain has risen the last few years : the smartwatches. True companion to the smartpone, the connected watch is designed to allow its user to be informed and to seek informations stored on the phone in a blink of an eye.

Interested by the subject, I started to search for different models of existing smartwatches : their communication modes and componants. A huge majority of them work with sophisticated application processors, from the same architecture than our smartphones. I started to wonder if it was possible to build a smartwatch using more basic microcontrollers that would be less powerfull and therefore consume less energy. 

## Components

### Sparkfun Microview

The Microview is a Arduino-compatible microcontroller board developped by Sparkfun (https://www.sparkfun.com/products/12923). It has an ATMega 328P microcontroller and a built-in 64x48 pixels OLED screen. That screen is used to show informations to the user.

Its program is located in Microview/

### RedBear BLE Nano

The BLE Nano is a microcontroller board developped by RedBear. It has a Nordic nRF51822 micrcontroller based on a ARM Cortex-M0 core and a Bluetooth Low Energy chip. It can be programmed using Arduino or mbed (https://developer.mbed.org/). The Nano manages the connection with the smartphone.

Its program is located in BLE-Nano/

### Android Application

In order to send data to the smartwatch, an Android application has been developped. It connects to the watch, sends its internal clock value and forward receiving SMS contents to the smartwatch.

Its program is located in MicroSmartwatch/
