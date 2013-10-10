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

#include "includes/defines.h"
#include "includes/utils.h"
#include "includes/peripherals.h"
#include "includes/state_machine.h"

int main(void)
{
    initUart();
    initTimer1();
    initIO();

    setInterrupts(true);

    while (1)
    {
        stateMachine();

        uartFlush();
    }
}
