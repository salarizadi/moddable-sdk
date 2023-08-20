/**
 * Copyright (c) 2023
 * @Version    : 1.0.0
 * @Ported     : https://github.com/sui77/rc-switch
 * @Repository : https://github.com/salarizadi/moddable-sdk/tree/main/Modules/rcswitch
 * @Author     : https://salarizadi.github.io
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_timer.h" // for esp-idf v5
#include "rcswitch.h"

static const RCSwitchProtocol RCSwitchProto[] = {
  { 350, {  1, 31 }, {  1,  3 }, {  3,  1 }, false }, // Protocol 1
  { 650, {  1, 10 }, {  1,  2 }, {  2,  1 }, false }, // Protocol 2
  { 100, { 30, 71 }, {  4, 11 }, {  9,  6 }, false }, // Protocol 3
  { 380, {  1,  6 }, {  1,  3 }, {  3,  1 }, false }, // Protocol 4
  { 500, {  6, 14 }, {  1,  2 }, {  2,  1 }, false }, // Protocol 5
  { 450, { 23,  1 }, {  1,  2 }, {  2,  1 }, true },  // Protocol 6 (HT6P20B)
  { 150, {  2, 62 }, {  1,  6 }, {  6,  1 }, false }, // Protocol 7 (HS2303-PT, i. e. used in AUKEY Remote)
  { 200, {  3, 130}, {  7, 16 }, {  3,  16}, false},  // Protocol 8 Conrad RS-200 RX
  { 200, { 130, 7 }, {  16, 7 }, { 16,  3 }, true},   // Protocol 9 Conrad RS-200 TX
  { 365, { 18,  1 }, {  3,  1 }, {  1,  3 }, true },  // Protocol 10 (1ByOne Doorbell)
  { 270, { 36,  1 }, {  1,  2 }, {  2,  1 }, true },  // Protocol 11 (HT12E)
  { 320, { 36,  1 }, {  1,  2 }, {  2,  1 }, true }   // Protocol 12 (SM5212)
};

enum {
    numRCSwitchProto = sizeof(RCSwitchProto) / sizeof(RCSwitchProto[0])
};

void rcswitch_init (RCSWITCH_t *rcswitch) {
	rcswitch->nReceivedValue = 0;
	rcswitch->nReceivedBitlength = 0;
	rcswitch->nReceivedDelay = 0;
	rcswitch->nReceivedProtocol = 0;
	rcswitch->nReceiveTolerance = 60;
	rcswitch->nSeparationLimit = 4300;

	rcswitch->nTransmitterPin = -1;
	rcswitch_set_repeat_transmit(rcswitch, 10);
	rcswitch_set_protocol(rcswitch, 1);
	rcswitch->nReceiverInterrupt = -1;
	rcswitch_set_receive_tolerance(rcswitch, 60);
	rcswitch->nReceivedValue = 0;
}

void rcswitch_set_protocol (RCSWITCH_t *rcswitch, int nProtocol) {
	if (nProtocol < 1 || nProtocol > numRCSwitchProto) {
	    nProtocol = 1;	// TODO: trigger an error, e.g. "bad Protocol" ???
	}
	rcswitch->Protocol = RCSwitchProto[nProtocol-1];
}

void rcswitch_set_pulse_length (RCSWITCH_t *rcswitch, int nPulseLength) {
	rcswitch->Protocol.pulseLength = nPulseLength;
}

void rcswitch_set_repeat_transmit (RCSWITCH_t *rcswitch, int nRepeat_transmit) {
	rcswitch->nRepeat_transmit = nRepeat_transmit;
}

void rcswitch_set_receive_tolerance (RCSWITCH_t *rcswitch, int nPercent) {
	rcswitch->nReceiveTolerance = nPercent;
}

void rcswitch_enable_transmit (RCSWITCH_t *rcswitch, int nTransmitterPin) {
	rcswitch->nTransmitterPin = nTransmitterPin;
	gpio_reset_pin(rcswitch->nTransmitterPin);
	gpio_set_direction(rcswitch->nTransmitterPin, GPIO_MODE_OUTPUT);
}

void rcswitch_disable_transmit (RCSWITCH_t *rcswitch) {
	rcswitch->nTransmitterPin = -1;
}

void rcswitch_send_tristate (RCSWITCH_t *rcswitch, const char *sCodeWord) {
	// turn the tristate code word into the corresponding bit pattern, then send it
	unsigned long code = 0;
	unsigned int length = 0;
	for (const char* p = sCodeWord; *p; p++) {
		code <<= 2L;
		switch (*p) {
			case '0':
				// bit pattern 00
				break;
			case 'F':
				// bit pattern 01
				code |= 1L;
				break;
			case '1':
				// bit pattern 11
				code |= 3L;
				break;
		}
		length += 2;
	}
	rcswitch_send(rcswitch, code, length);
}

void rcswitch_send_binary (RCSWITCH_t *rcswitch, const char *sCodeWord) {
  // turn the tristate code word into the corresponding bit pattern, then send it
  unsigned long code = 0;
  unsigned int length = 0;
  for (const char *p = sCodeWord; *p; p++) {
    code <<= 1L;
    if (*p != '0')
      code |= 1L;
    length++;
  }
  rcswitch_send(rcswitch, code, length);
}

void rcswitch_send (RCSWITCH_t *rcswitch, unsigned long code, unsigned int length) {
	if (rcswitch->nTransmitterPin == -1)
		return;

	// make sure the receiver is disabled while we rcswitch_transmit
	int nReceiverInterrupt_backup = rcswitch->nReceiverInterrupt;
	if (nReceiverInterrupt_backup != -1) {
		rcswitch_disable_receive(rcswitch);
	}

	for (int nRepeat = 0; nRepeat < rcswitch->nRepeat_transmit; nRepeat++) {
		for (int i = length-1; i >= 0; i--) {
			if (code & (1L << i))
				rcswitch_transmit(rcswitch, rcswitch->Protocol.one);
			else
				rcswitch_transmit(rcswitch, rcswitch->Protocol.zero);
		}
		rcswitch_transmit(rcswitch, rcswitch->Protocol.syncFactor);
	}

	// Disable rcswitch_transmit after rcswitch_sending (i.e., for inverted Protocols)
	gpio_set_level( rcswitch->nTransmitterPin, LOW );

	// enable receiver again if we just disabled it
	if (nReceiverInterrupt_backup != -1) {
		rcswitch_enable_receive(rcswitch, nReceiverInterrupt_backup);
	}
}

void rcswitch_transmit (RCSWITCH_t *rcswitch, HighLow pulses) {
	uint8_t firstLogicLevel = (rcswitch->Protocol.invertedSignal) ? LOW : HIGH;
	uint8_t secondLogicLevel = (rcswitch->Protocol.invertedSignal) ? HIGH : LOW;

#if 0
	digitalWrite(this->nTransmitterPin, firstLogicLevel);
	delayMicroseconds( this->Protocol.pulseLength * pulses.high);
	digitalWrite(this->nTransmitterPin, secondLogicLevel);
	delayMicroseconds( this->Protocol.pulseLength * pulses.low);
#endif
	gpio_set_level(rcswitch->nTransmitterPin, firstLogicLevel );
	//ets_delay_us(rcswitch->Protocol.pulseLength * pulses.high);
	esp_rom_delay_us(rcswitch->Protocol.pulseLength * pulses.high);
	gpio_set_level(rcswitch->nTransmitterPin, secondLogicLevel);
	//ets_delay_us(rcswitch->Protocol.pulseLength * pulses.low);
	esp_rom_delay_us(rcswitch->Protocol.pulseLength * pulses.low);
}

void rcswitch_enable_receive (RCSWITCH_t *rcswitch, int interrupt) {
	rcswitch->nReceiverInterrupt = interrupt;
	rcswitch_enable_receive_internal(rcswitch);
}

#define RCSWITCH_ESP_INTR_FLAG_DEFAULT 0

void rcswitch_enable_receive_internal (RCSWITCH_t *rcswitch) {
	uint64_t gpio_pin_sel = (1ULL<<rcswitch->nReceiverInterrupt);

    // Configure the data input
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = gpio_pin_sel,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE
    };
    gpio_config(&io_conf);

	// install gpio isr service
	gpio_install_isr_service(RCSWITCH_ESP_INTR_FLAG_DEFAULT);
	// hook isr handler for specific gpio pin
	gpio_isr_handler_add(rcswitch->nReceiverInterrupt, rcswitch_handle_interrupt, rcswitch);
}

void rcswitch_disable_receive (RCSWITCH_t *rcswitch) {
	gpio_isr_handler_remove(rcswitch->nReceiverInterrupt); // nReceiverInterrupt is GPIO
	rcswitch->nReceiverInterrupt = -1;
}

bool rcswitch_available (RCSWITCH_t *rcswitch) {
	return rcswitch->nReceivedValue != 0;
}

void rcswitch_reset_available (RCSWITCH_t *rcswitch) {
	rcswitch->nReceivedValue = 0;
}

unsigned long rcswitch_received_value (RCSWITCH_t *rcswitch) {
	return rcswitch->nReceivedValue;
}

unsigned int rcswitch_received_bit_length (RCSWITCH_t *rcswitch) {
	return rcswitch->nReceivedBitlength;
}

unsigned int rcswitch_received_delay (RCSWITCH_t *rcswitch) {
	return rcswitch->nReceivedDelay;
}

unsigned int rcswitch_received_protocol (RCSWITCH_t *rcswitch) {
	return rcswitch->nReceivedProtocol;
}

unsigned int* rcswitch_received_raw_data (RCSWITCH_t *rcswitch) {
	return rcswitch->timings;
}

static inline unsigned int rcswitch_diff (int A, int B) {
	return abs(A - B);
}

bool rcswitch_receive_protocol (RCSWITCH_t *rcswitch, const int p, unsigned int changeCount) {
	const RCSwitchProtocol pro = RCSwitchProto[p-1];

	unsigned long code = 0;
	//Assuming the longer pulse length is the pulse captured in timings[0]
	const unsigned int syncLengthInPulses =  ((pro.syncFactor.low) > (pro.syncFactor.high)) ? (pro.syncFactor.low) : (pro.syncFactor.high);
	const unsigned int delay = rcswitch->timings[0] / syncLengthInPulses;
	const unsigned int delayTolerance = delay * rcswitch->nReceiveTolerance / 100;

	/* For Protocols that start low, the sync period looks like
	 *							 _________
	 * _____________|					|XXXXXXXXXXXX|
	 *
	 * |--1st dur--|-2nd dur-|-Start data-|
	 *
	 * The 3rd saved duration starts the data.
	 *
	 * For Protocols that start high, the sync period looks like
	 *
	 *	______________
	 * |							|____________|XXXXXXXXXXXXX|
	 *
	 * |-filtered out-|--1st dur--|--Start data--|
	 *
	 * The 2nd saved duration starts the data
	 */
	const unsigned int firstDataTiming = (pro.invertedSignal) ? (2) : (1);

	for (unsigned int i = firstDataTiming; i < changeCount - 1; i += 2) {
		code <<= 1;
		if (rcswitch_diff(rcswitch->timings[i], delay * pro.zero.high) < delayTolerance &&
			rcswitch_diff(rcswitch->timings[i + 1], delay * pro.zero.low) < delayTolerance) {
			// zero
		} else if (rcswitch_diff(rcswitch->timings[i], delay * pro.one.high) < delayTolerance &&
							 rcswitch_diff(rcswitch->timings[i + 1], delay * pro.one.low) < delayTolerance) {
			// one
			code |= 1;
		} else {
			// Failed
			return false;
		}
	}

	if (changeCount > 7) {		// ignore very short transmissions: no device rcswitch_sends them, so this must be noise
		rcswitch->nReceivedValue = code;
		rcswitch->nReceivedBitlength = (changeCount - 1) / 2;
		rcswitch->nReceivedDelay = delay;
		rcswitch->nReceivedProtocol = p;
		return true;
	}

	return false;
}

void rcswitch_handle_interrupt (void *arg) {
	RCSWITCH_t *rcswitch = (RCSWITCH_t *) arg;

	static unsigned int changeCount = 0;
	static unsigned long lastTime = 0;
	static unsigned int repeatCount = 0;

	const long time = esp_timer_get_time();
	const unsigned int duration = time - lastTime;

	if (duration > rcswitch->nSeparationLimit) {
		// A long stretch without signal level change occurred. This could
		// be the gap between two transmission.
		if (rcswitch_diff(duration, rcswitch->timings[0]) < 200) {
			// This long signal is close in length to the long signal which
			// started the previously recorded timings; this suggests that
			// it may indeed by a a gap between two transmissions (we assume
			// here that a rcswitch_sender will rcswitch_send the signal multiple times,
			// with roughly the same gap between them).
			repeatCount++;
			if (repeatCount == 2) {
				for(uint8_t i = 1; i <= numRCSwitchProto; i++) {
					if (rcswitch_receive_protocol(rcswitch, i, changeCount)) {
						// receive succeeded for Protocol i
						break;
					}
				}
				repeatCount = 0;
			}
		}
		changeCount = 0;
	}
	// detect overflow
	if (changeCount >= RCSWITCH_MAX_CHANGES) {
		changeCount = 0;
		repeatCount = 0;
	}

	rcswitch->timings[changeCount++] = duration;
	lastTime = time;
}