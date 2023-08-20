/**
 * Copyright (c) 2023
 * @Version    : 1.0.0
 * @Ported     : https://github.com/sui77/rc-switch
 * @Repository : https://github.com/salarizadi/moddable-sdk/tree/main/Modules/rcswitch
 * @Author     : https://salarizadi.github.io
*/

#ifndef MAIN_RCSWITCH_H_
#define MAIN_RCSWITCH_H_

#define	LOW  0
#define HIGH 1

	/**
	 * Description of a single pulse, which consists of a high signal
	 * whose duration is "high" times the base pulse length, followed
	 * by a low signal lasting "low" times the base pulse length.
	 * Thus, the pulse overall lasts (high+low)*pulseLength
	 */
	typedef struct HighLow {
		uint8_t high;
		uint8_t low;
	} HighLow;

	/**
	 * A "Protocol" describes how zero and one bits are encoded into high/low
	 * pulses.
	 */
	typedef struct RCSwitchProtocol {
		/** base pulse length in microseconds, e.g. 350 */
		uint16_t pulseLength;

		HighLow syncFactor;
		HighLow zero;
		HighLow one;

		/**
		 * If true, interchange high and low logic levels in all transmissions.
		 *
		 * By default, RCSwitch assumes that any signals it rcswitch_sends or receives
		 * can be broken down into pulses which start with a high signal level,
		 * followed by a a low signal level. This is e.g. the case for the
		 * popular PT 2260 encoder chip, and thus many switches out there.
		 *
		 * But some devices do it the other way around, and start with a low
		 * signal level, followed by a high signal level, e.g. the HT6P20B. To
		 * accommodate this, one can set invertedSignal to true, which causes
		 * RCSwitch to change how it interprets any HighLow struct FOO: It will
		 * then assume transmissions start with a low signal lasting
		 * FOO.high*pulseLength microseconds, followed by a high signal lasting
		 * FOO.low*pulseLength microseconds.
		 */
		bool invertedSignal;
	} RCSwitchProtocol;

// Number of maximum high/Low changes per packet.
// We can handle up to (unsigned long) => 32 bit * 2 H/L changes per bit + 2 for sync
#define RCSWITCH_MAX_CHANGES 67

typedef struct {
	unsigned long nReceivedValue;
	unsigned int nReceivedBitlength;
	unsigned int nReceivedDelay;
	unsigned int nReceivedProtocol;
	int nReceiveTolerance;
	unsigned nSeparationLimit;
	/*
	 * timings[0] contains sync timing, followed by a number of bits
	 */
	unsigned int timings[RCSWITCH_MAX_CHANGES];
	int nReceiverInterrupt;

	int nTransmitterPin;
	int nRepeat_transmit;

	RCSwitchProtocol Protocol;
} RCSWITCH_t;

	void rcswitch_init(RCSWITCH_t * RCSwitch);

	void rcswitch_enable_receive(RCSWITCH_t * RCSwitch, int interrupt);
	void rcswitch_enable_receive_internal(RCSWITCH_t * RCSwitch);
	void rcswitch_disable_receive(RCSWITCH_t * RCSwitch);
	bool rcswitch_available(RCSWITCH_t * RCSwitch);
	void rcswitch_reset_available(RCSWITCH_t * RCSwitch);

	unsigned long rcswitch_received_value(RCSWITCH_t * RCSwitch);
	unsigned int rcswitch_received_bit_length(RCSWITCH_t * RCSwitch);
	unsigned int rcswitch_received_delay(RCSWITCH_t * RCSwitch);
	unsigned int rcswitch_received_protocol(RCSWITCH_t * RCSwitch);
	unsigned int* rcswitch_received_raw_data(RCSWITCH_t * RCSwitch);

	void rcswitch_handle_interrupt(void* arg);

	void rcswitch_set_protocol(RCSWITCH_t * RCSwitch, int nProtocol);
	void rcswitch_set_pulse_length(RCSWITCH_t * RCSwitch, int nPulseLength);
	void rcswitch_set_repeat_transmit(RCSWITCH_t * RCSwitch, int nRepeat_transmit);
	void rcswitch_set_receive_tolerance(RCSWITCH_t * RCSwitch, int nPercent);

	void rcswitch_enable_transmit(RCSWITCH_t * RCSwitch, int nTransmitterPin);
	void rcswitch_disable_transmit(RCSWITCH_t * RCSwitch);

	void rcswitch_send_binary(RCSWITCH_t * RCSwitch, const char* sCodeWord);
	void rcswitch_send_tristate(RCSWITCH_t * RCSwitch, const char* sCodeWord);
	void rcswitch_send(RCSWITCH_t * RCSwitch, unsigned long code, unsigned int length);

	void rcswitch_transmit(RCSWITCH_t * RCSwitch, HighLow pulses);

#endif /* MAIN_RXB6_RECEIVER_H_ */