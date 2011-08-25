#include "WProgram.h"
#include "UnicomReceiver.h"
#include "ClockUI.h"
#include "Time.h"


ClockUI::ClockUI(int analogPin, int brightnessThreshold) :
	receiver(analogPin, brightnessThreshold),
	updateSuccess(0),
	nextDisplayUpdate(0)
{
	// Do nothing
} // ClockUI::ClockUI


ClockUI::~ClockUI()
{
	// Do nothing
} // ClockUI::~ClockUI


void
ClockUI::refresh()
{
	UnicomReceiver::state_t oldState = receiver.getState();
	receiver.refresh();
	UnicomReceiver::state_t newState = receiver.getState();
	
	switch (newState) {
		case UnicomReceiver::STATE_LOCKED:
			if (newState != oldState)
				nextDisplayUpdate = 0;
			displayLocked();
			break;
		
		case UnicomReceiver::STATE_RECEIVING:
			if (newState != oldState)
				nextDisplayUpdate = 0;
			displayReceiver();
			break;
		
		case UnicomReceiver::STATE_SYNCING:
			if (oldState != newState)
				displayUpdateComplete();
			displayDateTime();
			break;
	}
	
	
} // ClockUI:;refresh


void
ClockUI::displayLocked() {
	if (millis() > nextDisplayUpdate) {
		Serial.println("Ready to update!");
		
		nextDisplayUpdate = millis() + 50000;
	}
	
	bytesReceived = 0;
	updateSuccess = false;
	dataProcessed = false;
} // ClockUI::displayLocked


void
ClockUI::displayReceiver() {
	if (millis() > nextDisplayUpdate) {
		Serial.println("Updating...");
		
		nextDisplayUpdate = millis() + 50000;
	}
	
	if (bytesReceived < BUF_LEN && receiver.getByte(dataBuffer + bytesReceived))
		bytesReceived++;
	
	if (bytesReceived == sizeof(unsigned long) && !dataProcessed) {
		setTime(*((unsigned long *)dataBuffer));
		dataProcessed = true;
		updateSuccess = true;
	}
	
} // ClockUI::displayyReceiver


void
ClockUI::displayDateTime() {
	if (millis() > nextDisplayUpdate) {
		Serial.print(hour());
		Serial.print(":");
		Serial.print(minute());
		Serial.print(":");
		Serial.print(second());
		Serial.print(" ");
		Serial.print(day());
		Serial.print(" ");
		Serial.print(month());
		Serial.print(" ");
		Serial.print(year());
		Serial.println();
		
		nextDisplayUpdate = millis() + 1000;
	}
} // ClockUI::displayDateTime


void
ClockUI::displayUpdateComplete()
{
	if (updateSuccess) {
		Serial.println("Yey, updated!");
	} else {
		Serial.println("Update Failed!");
	}
	
	nextDisplayUpdate = millis() + 2000;
} // ClockUI::displayUpdateComplete
