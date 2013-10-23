/*
 *  Copyright (C) 2013
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file main.c
 * @author Marcel
 * @brief
 *
 */

#include "peripherals.h"
#include "cec_driver.h"
#include "cec_protocol.h"
#include "usbdrv/usbdrv.h"
#include <util/delay.h>

USB_PUBLIC uchar usbFunctionSetup(uchar data[8])
{
    return 0; // do nothing for now
}

int main(void)
{
    initUart();
    initTimer1();
    initIO();
    initDriver();

    usbInit();

    usbDeviceDisconnect();
    for (uint8_t i = 0; i < 250; i++)
    {
        _delay_ms(2);
    }
    usbDeviceConnect();

    setInterrupts(true);

    while (1)
    {
        processDriver();
        processProtocol();

        uartFlush();
        usbPoll();
    }
}
