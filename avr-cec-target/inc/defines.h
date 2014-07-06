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
 * @file defines.h
 * @author Marcel
 * @brief
 *
 */

#ifndef DEFINES_H_
#define DEFINES_H_

#include "config.h"

//==========================================
// Debug
//==========================================


//==========================================
// IO
//==========================================


//==========================================
// Timer
//==========================================
#define TIMER_TICK_VALUE            (1000000 / (F_CPU/TIMER_PRESCALER))     ///calculated time value of one timer tick (in us)


//==========================================
// CEC
//==========================================
/*
 * The following defines will hold some important CEC timing values for the start bit. There is no need for changing there values.
 *
 * Pulse format and timing of the start bit:
 * __                _______
 *   |              |       |
 *   |______________|       |__
 *   T0             T1      T2
 *
 * T0: Beginning of a start bit.
 * T1: Rising edge of a start bit after 3.7ms (+/- 0.2ms) from beginning.
 * T2: End of a start bit after 4.5ms (+/- 0.2ms) from beginning. This is also the beginning of the first data bit.
 *
 * For further details check the HDMI CEC specification on Page 8.
 */
#define START_BIT_T1                ((3.7 * 1000) / TIMER_TICK_VALUE)       ///start bit: rising edge time t1 (in timer ticks => 3.7ms)
#define START_BIT_T2                ((4.5 * 1000) / TIMER_TICK_VALUE)       ///start bit: falling edge time t2 (in timer ticks => 4.5ms)
#define START_BIT_TOLERANCE         ((0.2 * 1000) / TIMER_TICK_VALUE)       ///start bit: rising/falling edge time tolerance (in timer ticks => 0.2ms)

/*
 * The following defines will hold some important CEC timing values for the data bit. There is no need for changing there values.
 *
 * Pulse format and timing of logical 0 data bit:
 * __                _______
 *   |              |       |
 *   |______________|       |__
 *   T0       Ts    T1      T2
 *
 * T0: Beginning of data bit.
 * Ts: Safe sample time of a data bit after 1.05ms (+/- 0.2ms) from beginning.
 * T1: Rising edge of a logical 0 data bit after 1.5ms (+/- 0.2ms) from beginning.
 * T2: Beginning of next data bit after 2.4ms (+/- 0.35ms) from beginning. If no data bit is following the line will stay high.
 *
 * Pulse format and timing of logical 1 data bit:
 *
 * __      _________________
 *   |    |                 |
 *   |____|                 |__
 *   T0   T1  Ts            T2
 *
 * T0: Beginning of data bit.
 * T1: Rising edge of a logical 1 data bit after 0.6ms (+/- 0.2ms) from beginning.
 * Ts: Safe sample time of a data bit after 1.05ms (+/- 0.2ms) from beginning.
 * T2: Beginning of next data bit after 2.4ms (+/- 0.35ms) from beginning. If no data bit is following the line will stay high.
 *
 * For further details check the HDMI CEC specification on Page 9.
 */
#define DATA_BIT_LOGIC_0            ((1.5 * 1000) / TIMER_TICK_VALUE)       ///data bit: rising edge time (in timer ticks => 1.5ms) to represent a logic 0
#define DATA_BIT_LOGIC_1            ((0.6 * 1000) / TIMER_TICK_VALUE)       ///data bit: rising edge time (in timer ticks => 0.6ms) to represent a logic 1
#define DATA_BIT_FOLLOWING          ((2.4 * 1000) / TIMER_TICK_VALUE)       ///data bit: start time (in timer ticks => 2.4ms) for a following data bit
#define DATA_BIT_FOLLOWING_TIMEOUT  ((2.75 * 1000) / TIMER_TICK_VALUE)      ///data bit: latest time for start of a following data bit (in timer ticks => 2.75ms)
#define DATA_BIT_SAMPLE_TIME        ((1.05 * 1000) / TIMER_TICK_VALUE)      ///data bit: sample time (in timer ticks => 1.05ms)

/*
 * The following defines will hold some important CEC timing values for the signal free time. There is no need for changing there values. For further details check
 * the HDMI CEC specification on Page 15.
 */
#define SFT_PRESENT_INITIATOR       ((7 * 2.4 * 1000) / TIMER_TICK_VALUE)   ///signal free time: present initiator sends another frame immediately after its previous frame (in timer ticks => 16.8ms)
#define SFT_NEW_INITIATOR           ((5 * 2.4 * 1000) / TIMER_TICK_VALUE)   ///signal free time: new initiator wants to send a frame (in timer ticks => 12ms)
#define SFT_RETRANSMISSION          ((3 * 2.4 * 1000) / TIMER_TICK_VALUE)   ///signal free time: retransmission after a previous unsuccessful attempt (in timer ticks => 7.2ms)


//==========================================
// USB
//==========================================
#define COMMAND_GET_MESSAGE         1
#define COMMAND_PUT_MESSAGE         2
#define COMMAND_GET_CONFIG          3
#define COMMAND_PUT_CONFIG          4
#define COMMAND_GET_DEBUG           5

#endif
