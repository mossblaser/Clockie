#include "WProgram.h"
#include "UnicomReceiver.h"

int analogPin = 0;
int brightnessThreshold = 500;

UnicomReceiver reciever = UnicomReceiver(analogPin, brightnessThreshold);

void setup() {
	Serial.begin(115200);
}

void loop() {
	reciever.refresh();
}
