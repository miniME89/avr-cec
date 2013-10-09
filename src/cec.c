#include "includes/cec.h"

Level shouldLevel = HIGH;

/************************************************************************/
/* pull cec line LOW                                                     */
/************************************************************************/
void cecLow()
{
    setDataDirectionCEC(OUTPUT);
    shouldLevel = LOW;
}

/************************************************************************/
/* high impedance state                                                 */
/************************************************************************/
void cecHigh()
{
    setDataDirectionCEC(INPUT);
    shouldLevel = HIGH;
}

Level cecShouldLevel()
{
    return shouldLevel;
}
