# Smartwatch on microcontrollers

A new market in the consumer electronics domain has risen the last few years : the smartwatches. True companion to the smartpone, the connected watch is designed to allow its user to be informed and to seek informations stored on the phone in a blink of an eye.

Interested by the subject, I started to search for different models of existing smartwatches : their communication modes and componants. A huge majority of them work with sophisticated application processors, from the same architecture than our smartphones. I started to wonder if it was possible to build a smartwatch using more basic microcontrollers that would be less powerfull and therefore consume less energy. 

## Components

### Sparkfun Microview

The Microview is a Arduino-compatible microcontroller board developped by Sparkfun (https://www.sparkfun.com/products/12923). It has an ATMega 328P microcontroller and a built-in 64x48 pixels OLED screen. That screen is used to show informations to the user.

Its program is located in Microview/. 

If you want to compile it, you first need to install the Arduino IDE and Microview specific additions by following the instructions at http://microview.io/intro/getting-started.html#install-arduino-ide

### RedBear BLE Nano

The BLE Nano is a microcontroller board developped by RedBear. It has a Nordic nRF51822 micrcontroller based on a ARM Cortex-M0 core and a Bluetooth Low Energy chip. It can be programmed using Arduino or mbed (https://developer.mbed.org/). The Nano manages the connection with the smartphone.

Its program is located in BLE-Nano/

If you want to compile it, you need to :
* Subscribe to the mbed developper website (https://developer.mbed.org)
* Register the RedBear BLE Nano as one of your platforms
* Create a new project targetted at the RedBear BLE Nano
* Copy paste the main.cpp file
* Add required library (BLE_API)


### Android Application

In order to send data to the smartwatch, an Android application has been developped. It connects to the watch, sends its internal clock value and forward receiving SMS contents to the smartwatch.

Its program is located in MicroSmartwatch/

The application has a minimum SDK version of 21, so you need a smartphone running Android 5.0 or newer to run it. You also need to have a Bluetooth Low Energy (also known as Bluetooth Smart) compatible device.

## Rights

Copyright Nathan Olff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.