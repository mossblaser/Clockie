#include "WProgram.h"
#include "UnicomReceiver.h"
#include "ClockUI.h"

int analogPin = 0;
int brightnessThreshold = 500;


ClockUI clock = ClockUI(analogPin, brightnessThreshold);


void
setup()
{
	Serial.begin(115200);
} // setup


void
loop()
{
	clock.refresh();
} // loop
