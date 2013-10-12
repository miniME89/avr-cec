## Goal ##
Most video cards these days don't support the CEC (Consumer Electronics Control) feature of HDMI. CEC makes it possible to allow communication between all connected HDMI devices, except for even the most recent video cards. To take advantage of the CEC feature a workaround is needed. This projects goal is to use an AVR microcontroller which manages the CEC bus communication between the computer and other connected HDMI devices. The AVR microcontroller is connected to the computer where software can bind actions to specific CEC messages. On this way the computer is able to take advantage of the CEC feature to control other devices or be controlled by other devices and there remotes.

![Google Logo](http://s14.directupload.net/images/131009/pm4jfyx4.png)

## Status ##
In the current state most of the CEC low level driver application is implemented. Reading and writing of CEC messages is possible. A communication between the microcontroller and the computer is currently possible via UART.

## Supported microcontrollers ##
* ATMega8
* ...

Easy portable to other microcontrollers with at least a 16 bit timer unit with 2 compare registers and a input capture pin.
