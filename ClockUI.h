#ifndef CLOCK_UI_H
#define CLOCK_UI_H

#include "WProgram.h"
#include "UnicomReceiver.h"


class ClockUI {
	public:
		ClockUI(int analogPin, int brightnessThreshold);
		~ClockUI();
		
		void refresh();
	
	private:
		UnicomReceiver receiver;
		
		unsigned long nextDisplayUpdate;
		
		bool updateSuccess;
		
		void displayDateTime();
		void displayLocked();
		void displayReceiver();
		void displayUpdateComplete();
		
		size_t bytesReceived;
		static const size_t BUF_LEN = 64;
		char dataBuffer[BUF_LEN];
		bool dataProcessed;
};


#endif
