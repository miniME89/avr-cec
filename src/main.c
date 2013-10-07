#include "includes/defines.h"
#include "includes/utils.h"
#include "includes/peripherals.h"
#include "includes/state_machine.h"

/************************************************************************/
/* main                                                                 */
/************************************************************************/
int main(void)
{
	initUart();
	initTimer1();
	initIO();

	setInterrupts(true);

	char c;
	while(1)
	{
		stateMachine();
		
		if (getFIFO(bufferUart, &c))
		{
			uart_write_char(c);
		}
	}
}
