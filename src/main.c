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

	while(1)
	{
		stateMachine();
		
		uartFlush();
	}
}
