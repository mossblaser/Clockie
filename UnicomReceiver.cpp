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
	lastEdgeTime(0),
	syncPulseBufHead(0)
{
	// Do nothing
} // UnicomReceiver::UnicomReceiver


UnicomReceiver::~UnicomReceiver()
{
	// Do nothing
} // UnicomReceiver::~UnicomReceiver


/**
 * Insert this into your main loop.
 */
void
UnicomReceiver::refresh()
{
	// Get current time
	unsigned long time = millis();
	bool currentSample = sample();
	bool edge          = currentSample != lastSample;
	bool edgeDirection = currentSample;
	lastSample = currentSample;
	
	if (ignoreUntilTime < time) {
		// If we've hit an edge, when syncing it must be a positive edge but
		// otherwise it can be any edge.
		if (edge && (state != STATE_SYNCING || edgeDirection == 1)) {
			unsigned long period = time - lastEdgeTime;
			lastEdgeTime = time;
			
			if (isPeriodStable(period)) {
				// If the period goes stable, lock on
				if (state == STATE_SYNCING)
					state = STATE_LOCKED;
			} else {
				// If the period is unstable and we were previously locked, reset the
				// period tracker to ensure we don't quickly reconnect without a propper
				// sync signal.
				if (state != STATE_SYNCING)
					periodTrackerReset();
				
				state = STATE_SYNCING;
			}
			
			if (state == STATE_SYNCING) {
				ignoreUntilTime = time + (MIN_PERIOD >> 1);
			} else {
				ignoreUntilTime = time + ignoreWindow;
				onBitReceived(edgeDirection);
			}
		} else if (state != STATE_SYNCING
		           && (ignoreUntilTime + acceptanceWindow) < time) {
			// If we're not syncing and we don't see an edge before the end of the
			// acceptance window, we've lost the connection and must re-sync.
			periodTrackerReset();
			state = STATE_SYNCING;
		}
	}
	
} // UnicomReceiver::refresh


void
UnicomReceiver::onBitReceived(bool bit)
{
	if (state == STATE_LOCKED && bit == 0) {
		state = STATE_RECEIVING;
		bitsReceived = 0;
	} else if (state == STATE_RECEIVING) {
		bitsReceived++;
		byteBuffer <<= 1;
		byteBuffer |= bit;
		
		if (bitsReceived == 8) {
			byteReady = true;
			byteReceived = byteBuffer;
			bitsReceived = 0;
		}
	}
} // UnicomReceiver::onBitReceived


/**
 * Returns true if a byte has been read since the last time this function was
 * called. Places the byte in the char pointed to by the argument.
 *
 * @param *byte Pointer to a char into which to place the received byte.
 * @return Whether a byte was received since last calling this function.
 */
bool
UnicomReceiver::getByte(char *byte)
{
	if (byteReady) {
		*byte = byteReceived;
		byteReady = false;
		return true;
	} else {
		return false;
	}
} // UnicomReceiver::getByte


UnicomReceiver::state_t
UnicomReceiver::getState()
{
	return state;
} // UnicomReceiver::getState


/**
 * Sample the analog input.
 */
bool
UnicomReceiver::sample()
{
	return analogRead(analogPin) > brightnessThreshold;
} // UnicomReceiver::sample


/**
 * Uses historical values of the period to determine if the period is stable.
 *
 * @return Is the period 'stable'?
 */
bool
UnicomReceiver::isPeriodStable(unsigned long currentPeriod)
{
	// Record the current period
	periodTrackerPush(currentPeriod);
	
	// Calculate average and range of periods
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
	
	// Calculate the acceptanceWindow = period / 2
	acceptanceWindow = period >> 1;
	
	// Calculate the maximum allowed jitter
	unsigned long maxJitter = MIN(acceptanceWindow, MAX_JITTER);
	
	// Range of values currently in the buffer is the "jitter" estimate
	unsigned long jitter = max - min;
	
	// Calculate ignore window = 3/4 * period
	ignoreWindow = acceptanceWindow + (acceptanceWindow>>1);
	
	return (jitter < maxJitter)
	       && (period > MIN_PERIOD)
	       && (period < MAX_PERIOD);
} // UnicomReceiver::isPeriodStable


void
UnicomReceiver::periodTrackerPush(unsigned long period)
{
	// Add period to buffer
	syncPulseBuf[syncPulseBufHead++] = period;
	
	// Wrap around buffer
	if (syncPulseBufHead >= SYNC_DURATION)
		syncPulseBufHead = 0;
} // UnicomReceiver::periodTrackerPush


void
UnicomReceiver::periodTrackerReset()
{
	for (int i = 0; i < SYNC_DURATION; i++)
		syncPulseBuf[i] = 0ul;
} // UnicomReceiver::periodTrackerReset
