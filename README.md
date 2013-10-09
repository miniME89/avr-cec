## Goal ##
Most video cards these days don't support the CEC (Consumer Electronics Control) feature of HDMI (High-Definition Multimedia Interface). Therefore a computer can't communicate with other connected devices. To take advantage of the CEC feature a workaround is needed. This projects goal is to use an AVR microcontroller which manages the CEC bus communication with other devices.

![Google Logo](http://s14.directupload.net/images/131009/pm4jfyx4.png)

## Status ##
In the current state most of the CEC low level bit timing is implemented. CEC messages can be read reliably. Writing of CEC messages is also possible but does ignore any collisions on the line. A communication between the microcontroller and the computer is currently possible via UART.

## Supported microcontrollers ##
* ATMega8
* ...

Easy portable to other microcontrollers with at least a 16 bit timer unit with 2 compare registers and a input capture pin.
