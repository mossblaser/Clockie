#include "WProgram.h"
#include "UnicomReceiver.h"
#include "ClockUI.h"

int analogPin = 0;


ClockUI clock = ClockUI(analogPin,
                        12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2);


void
setup()
{
	Serial.begin(115200);
	clock.init();
} // setup


void
loop()
{
	clock.refresh();
} // loop
