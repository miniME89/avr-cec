## Goal ##
Most video cards these days don't support the CEC (Consumer Electronics Control) feature of HDMI. This feature allows communication between all connected HDMI devices. To take advantage of this feature on the computer a workaround is needed.

This projects goal is to use an AVR microcontroller which manages the CEC communication between the computer and all connected HDMI devices. The AVR microcontroller is connected to the CEC bus to read and write CEC messages. A connection to the target computer and corresponding software makes it possible to bind actions to specific CEC messages. On this way the computer is able to take advantage of the CEC feature to control other devices or be controlled by other devices and there remotes.

![goal](http://s14.directupload.net/images/131122/vehsyhy6.png)

## Status ##
**avr-cec-client:** For the client software the initial code for a QT application was created.

**avr-cec-lib:** The USB connection to the AVR microcontroller is possible. Decoding the CEC messages will be the next step.

**avr-cec-target:** Most of the CEC low level application is implemented. Reading and writing of CEC messages is possible. A communication between the microcontroller and the computer is possible via USB.


## Requirements ##
The following microcontrollers are supported:

* ATMega8

For other microcontrollers than the supported ones, the microcontroller specific code may needs to be changed. All microcontroller specific code can be found in the peripherals.c file. Any microcontrollers meeting the following requirements can be used:

* 16 bit timer unit with 2 compare registers and a input capture pin
* One edge triggered interrupt pin
* About 6KB of program memory
* About 1KB of RAM (Using less RAM is possible when the CEC queue sizes are reduced and debugging is disabled)


## Circuit ##
...
