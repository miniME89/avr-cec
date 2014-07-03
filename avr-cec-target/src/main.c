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
#include "cec.h"
#include "usb.h"
#include "debug.h"

int main(void)
{
	//setup
	debugSetup();
    peripheralsSetup();
    cecSetup();
    usbSetup();

    //enable interrupts
    setInterrupts(true);

    //process loop
    while (1)
    {
        cecProcess();
        usbProcess();
    }
}
