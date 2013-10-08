#include "includes/cec.h"
#include "includes/peripherals.h"

/************************************************************************/
/* pull cec line LOW                                                     */
/************************************************************************/
void cecLow()
{
	setDataDirectionCEC(OUTPUT);
}

/************************************************************************/
/* high impedance state                                                 */
/************************************************************************/
void cecHigh()
{
	setDataDirectionCEC(INPUT);
}
