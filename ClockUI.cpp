#include "WProgram.h"
#include "LiquidCrystal.h"
#include "UnicomReceiver.h"
#include "ClockUI.h"
#include "Time.h"

#define PLURAL(num) (((num) == 1) ? "" : "s")

//                      |----------------|
#define MSG_PRE_UPDATE1 "       :o"
#define MSG_PRE_UPDATE2 "  Update Time!"

//                      |----------------|
#define MSG_MID_UPDATE1 " It tickles! :D"

//                      |----------------|
#define MSG_GD_UPDATE1  "       :D"
#define MSG_GD_UPDATE2  "I updated myself"

//                      |----------------|
#define MSG_BD_UPDATE1  "       :'("
#define MSG_BD_UPDATE2  "I couldn't do it"

//                      |----------------|
#define MSG_NO_TIME1A   "  I forgot the"
#define MSG_NO_TIME2A   "     time :("

//                      |----------------|
#define MSG_NO_TIME1B   "Please update me"
#define MSG_NO_TIME2B   "amp.jhnet.co.uk"

//                      |----------------|
#define MSG_NO_MEETING1 "See you soon! <3"
#define MSG_NO_MEETING2 "       -- rabbit"

//                      |----------------|
#define MSG_EGG1A       "  I LOVE YOU!!  "
#define MSG_EGG2A       " (So, so much!) "
//                      |----------------|
#define MSG_EGG1B       "      Mwah!     "
#define MSG_EGG2B       "   (Snuggle!)   "


ClockUI::ClockUI(int analogPin,
	               int rs, int rw, int enable, int d0, int d1, int d2, int d3,
	               int d4, int d5, int d6, int d7) :
	lcd(rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7),
	receiver(analogPin),
	updateSuccess(0),
	nextDisplayUpdate(0),
	nextMeeting(0),
	nextModeChange(0),
	displayMode(MODE_DATE_TIME)
{
	// Do nothing
} // ClockUI::ClockUI


ClockUI::~ClockUI()
{
	// Do nothing
} // ClockUI::~ClockUI



void
ClockUI::init()
{
	lcd.begin(16, 2);
} // ClockUI::init


void
ClockUI::refresh()
{
	//int intensity = analogRead(0) << 4;
	//static int min = 0;
	//static int max = 0;
	//min = MIN(intensity, min+1);
	//max = MAX(intensity, max-1);
	//if (millis() > nextDisplayUpdate) {
	//	lcd.clear();
	//	lcd.print(intensity>>4);
	//	lcd.print(" ");
	//	lcd.print(min>>4);
	//	lcd.print("-");
	//	lcd.print(max>>4);
	//	lcd.setCursor(0,1);
	//	lcd.print((min+max)>>5);
	//	nextDisplayUpdate = millis() + 500;
	//}
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
			if (newState != oldState) {
				receiveStarted = millis();
				nextDisplayUpdate = 0;
			}
			displayReceiver();
			break;
		
		case UnicomReceiver::STATE_SYNCING:
			if (oldState != newState)
				displayUpdateComplete();
			
			if (nextModeChange < millis()) {
				displayMode++;
				if (displayMode == MODE_LAST)
					displayMode = MODE_DATE_TIME;
				nextModeChange = millis() + 3000;
			}
			
			switch (displayMode) {
				case MODE_DATE_TIME:     displayDateTime(); break;
				case MODE_WAIT_DURATION: displayWaitDuration(); break;
			}
			break;
	}
} // ClockUI:;refresh


void
ClockUI::displayLocked() {
	if (millis() > nextDisplayUpdate) {
		lcd.clear();
		lcd.print(MSG_PRE_UPDATE1);
		lcd.setCursor(0,1);
		lcd.print(MSG_PRE_UPDATE2);
		
		nextDisplayUpdate = millis() + 50000;
	}
	
	bytesReceived = 0;
	updateSuccess = false;
	dataProcessed = false;
} // ClockUI::displayLocked


void
ClockUI::displayReceiver() {
	
	if (bytesReceived < BUF_LEN && receiver.getByte(dataBuffer + bytesReceived))
		bytesReceived++;
	
	size_t messageLength = 2*sizeof(unsigned long);
	
	if (bytesReceived == messageLength && !dataProcessed) {
		setTime(*((unsigned long *)dataBuffer));
		nextMeeting = (*(((unsigned long *)dataBuffer) + 1));
		
		adjustTime((long)((millis() - receiveStarted) / 1000ul));
		
		dataProcessed = true;
		updateSuccess = true;
	}
	
	if (millis() > nextDisplayUpdate) {
		
		lcd.clear();
		lcd.print(MSG_MID_UPDATE1);
		
		int messageLengthBits = messageLength << 3;
		int bitsReceived = (bytesReceived << 3) + receiver.getBitsReceived();
		lcd.setCursor(0,1);
		for (int i = 0; i < ((bitsReceived <<4) / messageLengthBits); i++)
			lcd.print('\xff');
		
		nextDisplayUpdate = millis() + 200;
	}
} // ClockUI::displayyReceiver


void
ClockUI::displayDateTime() {
	if (millis() > nextDisplayUpdate) {
		lcd.clear();
		
		if (year() > 2000) {
			lcd.setCursor(3, 0);
			lcd.print((hourFormat12() < 10) ? "0" : "");
			lcd.print(hourFormat12());
			lcd.print(":");
			lcd.print((minute() < 10) ? "0" : "");
			lcd.print(minute());
			lcd.print(":");
			lcd.print((second() < 10) ? "0" : "");
			lcd.print(second());
			lcd.print(isAM() ? "AM" : "PM");
			
			lcd.setCursor(4, 1);
			lcd.print((day() < 10) ? "0" : "");
			lcd.print(day());
			lcd.print("/");
			lcd.print((month() < 10) ? "0" : "");
			lcd.print(month());
			lcd.print("/");
			lcd.print(year() - 2000);
		} else {
			lcd.print(MSG_NO_TIME1A);
			lcd.setCursor(0, 1);
			lcd.print(MSG_NO_TIME2A);
		}
		
		nextDisplayUpdate = millis() + 1000;
	}
} // ClockUI::displayDateTime


void
ClockUI::displayWaitDuration() {
	if (millis() > nextDisplayUpdate) {
		lcd.clear();
		
		if (nextMeeting != 0) {
			if (nextMeeting >= now()) {
				if (minute() == 14 && second() < 6) {
					lcd.setCursor(0, 0);
					lcd.print(MSG_EGG1B);
					lcd.setCursor(0, 1);
					lcd.print(MSG_EGG2B);
				} else {
					unsigned long delta = nextMeeting - now();
					
					unsigned long days = delta / (24ul*60ul*60ul);
					delta -= days * (24ul*60ul*60ul);
					
					unsigned long weeks = days / 7;
					days -= (weeks * 7);
					
					unsigned long hours = delta / (60ul*60ul);
					delta -= hours * (60ul*60ul);
					
					unsigned long minutes = delta / 60ul;
					delta -= minutes * 60ul;
					
					unsigned long seconds = delta;
					
					if (weeks) {
						lcd.print(weeks);
						lcd.print("wk");
						lcd.print(PLURAL(weeks));
						lcd.print(" ");
					} else {
						lcd.print("Only ");
					}
					
					if (days) {
						lcd.print(days);
						lcd.print("day");
						lcd.print(PLURAL(days));
						lcd.print(" ");
					}
					
					lcd.print(hours);
					lcd.print("hr");
					lcd.print(PLURAL(hours));
					lcd.print(" ");
					
					lcd.setCursor(0,1);
					lcd.print(minutes);
					lcd.print("min");
					lcd.print(PLURAL(minutes));
					lcd.print(" ");
					lcd.print(seconds);
					lcd.print("sec");
					lcd.print(PLURAL(seconds));
					lcd.print(" ");
				}
			} else {
				if (nextMeeting != 0 && second() < 6) {
					lcd.setCursor(0, 0);
					lcd.print(MSG_EGG1A);
					lcd.setCursor(0, 1);
					lcd.print(MSG_EGG2A);
				} else {
					lcd.setCursor(0, 0);
					lcd.print(MSG_NO_MEETING1);
					lcd.setCursor(0, 1);
					lcd.print(MSG_NO_MEETING2);
				}
			}
		} else {
			lcd.print(MSG_NO_TIME1B);
			lcd.setCursor(0, 1);
			lcd.print(MSG_NO_TIME2B);
		}
		
		
		nextDisplayUpdate = millis() + 1000;
	}
} // ClockUI::displayDateTime


void
ClockUI::displayUpdateComplete()
{
	lcd.clear();
	if (updateSuccess) {
		lcd.print(MSG_GD_UPDATE1);
		lcd.setCursor(0,1);
		lcd.print(MSG_GD_UPDATE2);
	} else {
		lcd.print(MSG_BD_UPDATE1);
		lcd.setCursor(0,1);
		lcd.print(MSG_BD_UPDATE2);
	}
	
	nextDisplayUpdate = millis() + 3000;
} // ClockUI::displayUpdateComplete
