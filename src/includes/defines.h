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

#define DEBUG_AVR_CEC               0
#define F_CLK                       16000000                                //CPU clock speed

//IO
#define INFO_LED_PORT_DDR           DDRB
#define INFO_LED_PORT               PORTB
#define INFO_LED_PIN                PINB4

#define CEC_BUS_PULL_PORT_DDR       DDRD
#define CEC_BUS_PULL_PORT           PORTD
#define CEC_BUS_PULL_PIN            PIND6

#define CEC_INPUT_CAPTURE_PORT      PINB
#define CEC_INPUT_CAPTURE_PIN       PINB0

//Timer1
#define TIMER1_PRESCALER            64                                      //prescaler (1, 8, 64, 256 or 1024)
#define TIMER1_TICK_VALUE           (1000000 / (F_CLK/TIMER1_PRESCALER))    //time value of 1 timer tick (in us)

//UART
#define BAUD                        19200                                   //baud rate
#define UBRR_REGISTER               (F_CLK/16/BAUD-1)                       //content of UBRR register
#define FLUSH_MAX_CHARS             5                                       //max characters send when output buffer is flushed

//CEC
#define START_BIT_T1                ((3.7 * 1000) / TIMER1_TICK_VALUE)      //start bit: rising edge time t1 (in timer1 ticks => 3.7ms) (HDMI CEC specification p. 8)
#define START_BIT_T2                ((4.5 * 1000) / TIMER1_TICK_VALUE)      //start bit: falling edge time t2 (in timer1 ticks => 4.5ms) (HDMI CEC specification p. 8)
#define START_BIT_TOLERANCE         ((0.2 * 1000) / TIMER1_TICK_VALUE)      //start bit: rising/falling edge time tolerance (in timer1 ticks => 0.2ms) (HDMI CEC specification p. 8)

#define DATA_BIT_LOGIC_0            ((1.5 * 1000) / TIMER1_TICK_VALUE)      //data bit: rising edge time (in timer1 ticks => 1.5ms) to represent a logic 0 (HDMI CEC specification p. 9)
#define DATA_BIT_LOGIC_1            ((0.6 * 1000) / TIMER1_TICK_VALUE)      //data bit: rising edge time (in timer1 ticks => 0.6ms) to represent a logic 1 (HDMI CEC specification p. 9)
#define DATA_BIT_FOLLOWING          ((2.4 * 1000) / TIMER1_TICK_VALUE)      //data bit: start time (in timer1 ticks => 2.4ms) for a following data bit (HDMI CEC specification p. 9)
#define DATA_BIT_FOLLOWING_TIMEOUT  ((2.75 * 1000) / TIMER1_TICK_VALUE)     //data bit: latest time for start of a following data bit (in timer1 ticks => 2.75ms) (HDMI CEC specification p. 9)
#define DATA_BIT_SAMPLE_TIME        ((1.05 * 1000) / TIMER1_TICK_VALUE)     //data bit: sample time (in timer1 ticks => 1.05ms) (HDMI CEC specification p. 9)

#define SFT_PRESENT_INITIATOR       ((7 * 2.4 * 1000) / TIMER1_TICK_VALUE)  //signal free time: present initiator sends another frame immediately after its previous frame (in timer1 ticks => 16.8ms) (HDMI CEC specification p. 15)
#define SFT_NEW_INITIATOR           ((5 * 2.4 * 1000) / TIMER1_TICK_VALUE)  //signal free time: new initiator wants to send a frame (in timer1 ticks => 12ms) (HDMI CEC specification p. 15)
#define SFT_RETRANSMISSION          ((3 * 2.4 * 1000) / TIMER1_TICK_VALUE)  //signal free time: retransmission after a previous unsuccessful attempt (in timer1 ticks => 7.2ms) (HDMI CEC specification p. 15)

#define PHYSICAL_ADDRESS            0xFFFF
#define LOGICAL_ADDRESS             0x4

//macros
#define debug(message) if (DEBUG_AVR_CEC) {uartSendString(message); uartSendChar('\n');}

#endif
