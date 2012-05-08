#ifndef CLOCK_UI_H
#define CLOCK_UI_H

#include "WProgram.h"
#include "LiquidCrystal.h"
#include "UnicomReceiver.h"
#include "Time.h"


class ClockUI {
	public:
		ClockUI(int analogPin,
		        int rs, int rw, int enable, int d0, int d1, int d2, int d3, int d4,
		        int d5, int d6, int d7);
		~ClockUI();
		
		void init();
		
		void refresh();
	
	private:
		LiquidCrystal lcd;
		UnicomReceiver receiver;
		
		unsigned long receiveStarted;
		
		unsigned long nextDisplayUpdate;
		unsigned long nextModeChange;
		
		unsigned long nextMeeting;
		
		bool updateSuccess;
		
		enum {
			MODE_DATE_TIME,
			MODE_WAIT_DURATION,
			MODE_LAST
		};
		int  displayMode;
		void displayDateTime();
		void displayWaitDuration();
		
		void displayLocked();
		void displayReceiver();
		void displayUpdateComplete();
		
		size_t bytesReceived;
		static const size_t BUF_LEN = 64;
		char dataBuffer[BUF_LEN];
		bool dataProcessed;
};


#endif
