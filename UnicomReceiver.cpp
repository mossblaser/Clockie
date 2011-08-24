#include "WProgram.h"
#include "UnicomReceiver.h"


UnicomReceiver::UnicomReceiver(int analogPin, int brightnessThreshold) :
	analogPin(analogPin),
	brightnessThreshold(brightnessThreshold),
	
	// When we start we need to sync
	state(STATE_SYNCING),
	
	// Arbitary defaults
	lastSample(0),
	ignoreUntilTime(0),
	lastPosEdgeTime(0),
	syncPulseBufHead(0)
{
	// Do nothing
} // UnicomReceiver::UnicomReceiver


UnicomReceiver::~UnicomReceiver()
{
	// Do nothing
} // UnicomReceiver::~UnicomReceiver


void
UnicomReceiver::refresh()
{
	// Get current time
	unsigned long time = millis();
	
	switch (state) {
		case STATE_SYNCING:
			// If not ignoring [i.e. debouncing]
			if (ignoreUntilTime < time) {
				bool currentSample = sample();
				
				// Posedge seen, try to lock onto the frequency
				if (currentSample && !lastSample) {
					bool locked = syncPulseBufPush(time - lastPosEdgeTime);
					
					if (locked) {
						state = STATE_LOCKED;
						Serial.print("Locked @ ");
						Serial.print(1000ul / period);
						Serial.println("Hz");
						ignoreUntilTime = time + ignoreWindow;
						lastPosEdgeTime = 0;
						lastEdgeTime = time;
					} else {
						ignoreUntilTime = time + (MIN_PERIOD >> 1);
						lastPosEdgeTime = time;
					}
				}
				
				lastSample = currentSample;
			}
			break;
		
		case STATE_LOCKED:
		case STATE_RECEIVING:
			bool currentSample = sample();
			
			// If not ignoring [i.e. waiting til just before the edge]
			if (ignoreUntilTime < time) {
				if ((ignoreUntilTime + acceptanceWindow) > time) {
					// Within acceptance window, did we see an edge?
					if (lastSample != currentSample) {
						if (syncPulseBufPush(time - lastEdgeTime)) {
							if (state == STATE_RECEIVING) {
								byteReceived = (byteReceived << 1) | currentSample;
								if (++bitsReceived == 8) {
									Serial.print(byteReceived);
									bitsReceived = 0;
								}
							}
							
							if (state == STATE_LOCKED && !currentSample) {
								state = STATE_RECEIVING;
								bitsReceived = 0;
							}
							
							ignoreUntilTime = time + ignoreWindow;
							lastEdgeTime = time;
						} else {
							// Sync lost
							Serial.println("Lost on edge");
							state = STATE_SYNCING;
							syncPulseBufClear();
						}
					}
					
					lastSample = currentSample;
				} else {
					// Acceptance window expired without a pulse - sync lost
					Serial.println("Lost on missing edge");
					state = STATE_SYNCING;
					syncPulseBufClear();
				}
			} else {
				lastSample = currentSample;
			}
			break;
	}
} // UnicomReceiver::refresh


/**
 * Sample the analog input.
 */
bool
UnicomReceiver::sample()
{
	return analogRead(analogPin) > brightnessThreshold;
} // UnicomReceiver::sample


/**
 * Push a pulse duration onto the buffer.
 *
 * @return Is the period 'stable'?
 */
bool
UnicomReceiver::syncPulseBufPush(unsigned long duration)
{
	// Add duration to buffer
	syncPulseBuf[syncPulseBufHead++] = duration;
	
	// Wrap around buffer
	if (syncPulseBufHead >= SYNC_DURATION)
		syncPulseBufHead = 0;
	
	// Calculate average period and range of periods
	unsigned long min = syncPulseBuf[0];
	unsigned long max = syncPulseBuf[0];
	period = 0ul;
	
	for (int i = 0; i < SYNC_DURATION; i++) {
		min = MIN(min, syncPulseBuf[i]);
		max = MAX(max, syncPulseBuf[i]);
		period += syncPulseBuf[i];
	}
	
	// Period = Average(sum of periods)
	period = period >> LOG_SYNC_DURATION;
	
	// acceptanceWindow = period / 2
	acceptanceWindow = period >> 1;
	
	// Maximum allowed range
	unsigned long maxRange = acceptanceWindow;
	
	// Range of values currently in the buffer
	unsigned long range = max - min;
	
	// Ignore window = 3/4 * period
	ignoreWindow = acceptanceWindow + (acceptanceWindow>>1);
	
	return (range < MIN(maxRange, MAX_JITTER))
	       && (period > MIN_PERIOD)
	       && (period < MAX_PERIOD);
} // UnicomReceiver::syncPulseBufPush


void
UnicomReceiver::syncPulseBufClear()
{
	for (int i = 0; i < SYNC_DURATION; i++)
		syncPulseBuf[i] = 0ul;
} // UnicomReceiver::syncPulseBufClear
