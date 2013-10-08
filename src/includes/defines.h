#ifndef DEFINES_H_
#define DEFINES_H_

#define DEBUG_CEC					0
#define F_CLK						16000000								//CPU clock speed

//IO
#define	INFO_LED_PORT_DDR			DDRB
#define INFO_LED_PORT				PORTB
#define	INFO_LED_PIN				PINB4

#define CEC_BUS_PULL_PORT_DDR		DDRD
#define CEC_BUS_PULL_PORT			PORTD
#define CEC_BUS_PULL_PIN			PIND6

#define CEC_INPUT_CAPTURE_PORT		PINB
#define CEC_INPUT_CAPTURE_PIN		PINB0

//Timer1
#define TIMER1_PRESCALER			64										//prescaler (1, 8, 64, 256 or 1024)
#define TIMER1_TICK_VALUE			(1000000 / (F_CLK/TIMER1_PRESCALER))	//time value of 1 timer tick (in us)

//UART
#define BAUD						19200									//baud rate
#define UBRR_REGISTER				(F_CLK/16/BAUD-1)						//content of UBRR register
#define FLUSH_MAX_CHARS				5										//max characters send when output buffer is flushed

//CEC
#define START_BIT_T1				((3.7 * 1000) / TIMER1_TICK_VALUE)		//start bit: rising edge time t1 (in timer2 ticks => 3.7ms) (see HDMI 1.3 CEC specification p. 8)
#define START_BIT_T2				((4.5 * 1000) / TIMER1_TICK_VALUE)		//start bit: falling edge time t2 (in timer2 ticks => 4.5ms) (see HDMI 1.3 CEC specification p. 8)
#define START_BIT_TOLERANCE			((0.2 * 1000) / TIMER1_TICK_VALUE)		//start bit: rising/falling edge time tolerance (in timer2 ticks => 0.2ms) (see HDMI 1.3 CEC specification p. 8)

#define DATA_BIT_LOGIC_0			((1.5 * 1000) / TIMER1_TICK_VALUE)		//data bit: rising edge time (in timer2 ticks => 1.5ms) to represent a logic 0 (see HDMI 1.3 CEC specification p. 9)
#define DATA_BIT_LOGIC_1			((0.6 * 1000) / TIMER1_TICK_VALUE)		//data bit: rising edge time (in timer2 ticks => 0.6ms) to represent a logic 1 (see HDMI 1.3 CEC specification p. 9)
#define DATA_BIT_FOLLOWING			((2.4 * 1000) / TIMER1_TICK_VALUE)		//data bit: start time (in timer2 ticks => 2.4ms) for a following data bit (see HDMI 1.3 CEC specification p. 9)
#define DATA_BIT_FOLLOWING_TIMEOUT	((2.75 * 1000) / TIMER1_TICK_VALUE)		//data bit: latest time for start of a following data bit (in timer2 ticks => 2.75ms) (see HDMI 1.3 CEC specification p. 9)
#define DATA_BIT_SAMPLE_TIME		((1.05 * 1000) / TIMER1_TICK_VALUE)		//data bit: sample time (in timer2 ticks => 1.05ms) (see HDMI 1.3 CEC specification p. 9)

#define PHYSICAL_ADDRESS			0xFFFF
#define LOGICAL_ADDRESS				0x4

//macros
#define debug(message) if (DEBUG_CEC) {uartSendString(message); uartSendChar('\n');}

#endif
