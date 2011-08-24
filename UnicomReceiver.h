#ifndef UNICOM_RECEIVER_H
#define UNICOM_RECEIVER_H

#include "WProgram.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))


class UnicomReceiver {
	private:
		// State machine states
		typedef enum state {
			STATE_SYNCING,
			STATE_LOCKED,
			STATE_RECEIVING,
		} state_t;
		
		// Minimum period allowed (msec)
		static const unsigned int MIN_PERIOD = 50;
		static const unsigned int MAX_PERIOD = 2000;
		static const unsigned int MAX_JITTER = 50;
		
		// Number of consecutive sync pulses to require before locking
		static const unsigned int SYNC_DURATION = 8;
		static const unsigned int LOG_SYNC_DURATION = 3;
	
	
	private:
		// Current state
		state_t state;
		
		// Sensing parameters
		int analogPin;
		int brightnessThreshold;
		
		// Last value seen
		bool lastSample;
		
		// Time until which no samples are taken
		unsigned long ignoreUntilTime;
		
		// Time last posedge occurred
		unsigned long lastPosEdgeTime;
		
		// Time last edge seen
		unsigned long lastEdgeTime;
		
		// A buffer of sync pulse durations
		unsigned int  syncPulseBufHead;
		unsigned long syncPulseBuf[SYNC_DURATION];
		
		// Current clock period
		unsigned long period;
		
		// Byte reciever
		int bitsReceived;
		unsigned char byteReceived;
		
		// Time in which an expected edge is allowed to occur
		//   acceptanceWindow = period / 2
		unsigned long acceptanceWindow;
		
		// Time during which the signal is ignored
		//   ignoreWindow = period / 4
		unsigned long ignoreWindow;
		
	public:
		UnicomReceiver(int analogPin, int brightnessThreshold);
		~UnicomReceiver();
		
		void refresh();
		void refreshSync();
	
	private:
		bool sample();
		
		bool syncPulseBufPush(unsigned long duration);
		void syncPulseBufClear();
};


#endif
