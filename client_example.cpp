#include "WProgram.h"
#include "UnicomReceiver.h"

int analogPin = 0;
int brightnessThreshold = 500;


UnicomReceiver reciever = UnicomReceiver(analogPin, brightnessThreshold);


void
setup()
{
	Serial.begin(115200);
} // setup


void
loop()
{
	char byte;
	
	// Run the receiver's mainloop
	reciever.refresh();
	
	// Get any new data
	if (reciever.getByte(&byte))
		Serial.print(byte);
} // loop
