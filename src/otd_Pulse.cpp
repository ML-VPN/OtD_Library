#include "otd_Pulse.h"

#ifdef __cplusplus
extern "C"{
#endif 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "otd_CorePeri.h"

#define PULSE_IN_CLOCK_HZ		8000000
#define FREQ_RANGE_THRESH_HZ	4000

static uint16_t sPulseFreq[2] = {0, 0};
static uint8_t sPulseDuty[2] = {0, 0};
static uint16_t sPulseCount[2] = {0, 0};
static uint16_t sPulseMaxCount[2] = {0, 0};

void otd_InitPulse(){

	// Initialize Pulse 1
	PSOC2 |= (1<<POEN2A);		// Enable pulse output
	PCNF2 = (0<<PMODE21)|(0<<PMODE20)|(1<<POP2);	// 1 Ramp mode with active high polarity
	PCNF2 &= ~(1<<PCLKSEL2);	// Use IO clock
	// PSC Input A & B Control Register -not used-
	PFRC2A = 0;
	PFRC2B = 0;
	// Disable count interrupt
	PIM2 &= ~(1 < PEOPE2);

	// Initialize Pulse 2
	PSOC0 |= (1<<POEN0A);		// Enable pulse output
	PCNF0 = (0<<PMODE01)|(0<<PMODE00)|(1<<POP0);	// 1 Ramp mode with active high polarity
	PCNF0 &= ~(1<<PCLKSEL0);	// Use IO clock
	// PSCR Input A & B Control Register -not used-
	PFRC0A = 0;
	PFRC0B = 0;
	// Disable count interrupt
	PIM0 &= ~(1 << PEOPE0);

	return;
}



int8_t otd_SetPulseEnabled(enum PULSE_OUTPUT_PINS inPulseOutPin, uint8_t inIsEnabled){

	// Check whether freq and duty cycle was set
	if (sPulseFreq[inPulseOutPin] == 0 || sPulseDuty[inPulseOutPin] == 0){
		return -1;
	}

	switch(inPulseOutPin){
	case PULSE_OUTPUT_1:
		if (inIsEnabled == 1){
			// Reset Pulse count
			sPulseCount[PULSE_OUTPUT_1] = 0;
			// Start pulse
			PCTL2 |= (1<<PRUN2);		// PSC Run
		}else{
			PCTL2 &= ~(1<<PRUN2);		// PSC Stop
		}
		break;

	case PULSE_OUTPUT_2:
		if (inIsEnabled == 1){
			// Reset Pulse count
			sPulseCount[PULSE_OUTPUT_2] = 0;
			// Start pulse
			PCTL0 |= (1<<PRUN0);		// PSCR Run
		}else{
			PCTL0 &= ~(1<<PRUN0);		// PSCR Stop
		}
		break;

	default:
		break;
	}

	return 0;
}



uint8_t otd_GetPulseEnabled(enum PULSE_OUTPUT_PINS inPulseOutPin){

	switch(inPulseOutPin){
	case PULSE_OUTPUT_1:
		if (PCTL2 & (1 << PRUN2)){
			return 1;
		}
		break;

	case PULSE_OUTPUT_2:
		if (PCTL0 & (1 << PRUN0)){
			return 1;
		}
		break;
	}

	return 0;
}



void otd_SetPulseFreqDuty(enum PULSE_OUTPUT_PINS inPulseOutPin, unsigned long inFreqHz, unsigned int inDutyCycle){

	uint16_t prescaler;
	uint16_t period;
	uint8_t duty;
	uint16_t temp;

	// Check the freq margins
	if (inFreqHz < OTD_FREQ_MIN){
		inFreqHz = OTD_FREQ_MIN;
	}
	if (inFreqHz > OTD_FREQ_MAX){
		inFreqHz = OTD_FREQ_MAX;
	}

	// Check the duty margins
	if (inDutyCycle < OTD_DUTY_MIN){
		inDutyCycle = OTD_DUTY_MIN;
	}
	if (inDutyCycle > OTD_DUTY_MAX){
		inDutyCycle = OTD_DUTY_MAX;
	}



	switch(inPulseOutPin){
	case PULSE_OUTPUT_1:
		// Check the frequency range
		if (inFreqHz < FREQ_RANGE_THRESH_HZ){
			// Set prescaler to 256
			PCTL2 |= (1<<PPRE20);
			PCTL2 |= (1<<PPRE21);
			prescaler = 256;
		}else{
			// Set prescaler to 1
			PCTL2 &= ~(1<<PPRE20);
			PCTL2 &= ~(1<<PPRE21);
			prescaler = 1;
		}
		period = (PULSE_IN_CLOCK_HZ/prescaler)/(inFreqHz/2);	// "/2" is due to using dual counter "OCR0RA" and "OCR0RB"
		duty = inDutyCycle;
		// Enable Lock
		PCNF2 |= (1<<PLOCK2);
		// Set counter compare registers
		temp = ((uint32_t)period * duty) >> 8;
		OCR2RA = temp;
		OCR2SA = 0;
		OCR2RB = period;
		OCR2SB = 0;
		// Release Lock
		PCNF2 &= ~(1<<PLOCK2);
		break;

	case PULSE_OUTPUT_2:
		// Check the frequency range
		if (inFreqHz < FREQ_RANGE_THRESH_HZ){
			// Set prescaler to 256
			PCTL0 |= (1<<PPRE00);
			PCTL0 |= (1<<PPRE01);
			prescaler = 256;
		}else{
			// Set prescaler to 1
			PCTL0 &= ~(1<<PPRE00);
			PCTL0 &= ~(1<<PPRE01);
			prescaler = 1;
		}
		period = (PULSE_IN_CLOCK_HZ/prescaler)/(inFreqHz/2);	// "/2" is due to using dual counter "OCR0RA" and "OCR0RB"
		duty = inDutyCycle;
		// Enable Lock
		PCNF0 |= (1<<PLOCK0);
		// Set counter compare registers
		temp = ((uint32_t)period * duty) >> 8;
		OCR0RA = temp;
		OCR0SA = 0;
		OCR0RB = period;
		OCR0SB = 0;
		// Release Lock
		PCNF0 &= ~(1<<PLOCK0);
		break;

	default:
		return;
	}

	// Set frequency and duty cycle
	sPulseFreq[inPulseOutPin] = inFreqHz;
	sPulseDuty[inPulseOutPin] = inDutyCycle;

	return;
}



void otd_GetPulseFreqDuty(enum PULSE_OUTPUT_PINS inPulseOutPin, uint16_t *outFreq, uint8_t *outDuty){
	*outFreq = sPulseFreq[inPulseOutPin];
	*outDuty = sPulseDuty[inPulseOutPin];
	return;
}




void otd_SetMaxPulseCount(enum PULSE_OUTPUT_PINS inPulseOutPin, uint16_t inPulseMaxCount){

	sPulseMaxCount[inPulseOutPin] = inPulseMaxCount;

	// Enable interrupt
	switch(inPulseOutPin){
	case PULSE_OUTPUT_1:
		PIM2 |= (1 << PEOPE2);
		break;
	case PULSE_OUTPUT_2:
		PIM0 |= (1 << PEOPE0);
		break;
	}

	return;
}



void otd_ResetMaxPulseCount(enum PULSE_OUTPUT_PINS inPulseOutPin){

	sPulseMaxCount[inPulseOutPin] = 0;

	// Disable interrupt
	switch(inPulseOutPin){
	case PULSE_OUTPUT_1:
		PIM2 &= ~(1 << PEOPE2);
		break;
	case PULSE_OUTPUT_2:
		PIM0 &= ~(1 << PEOPE0);
		break;
	}
	return;
}



uint16_t otd_GetPulseCount(enum PULSE_OUTPUT_PINS inPulseOutPin){
	return sPulseCount[inPulseOutPin];
}



// Pin 1 Pulse period end interrupt
ISR(PSC2_EC_vect){
	if (sPulseMaxCount[0] != 0){
		// Increment pulse count
		sPulseCount[0] = sPulseCount[0] +1;
		// Check whether we can terminate
		if (sPulseCount[0] >= sPulseMaxCount[0]){
			// Stop Pulse 1
			PCTL2 &= ~(1<<PRUN2);
		}
	}
}


// Pin 2 Pulse period end interrupt
ISR(PSC0_EC_vect){
	if (sPulseMaxCount[1] != 0){
		// Increment pulse count
		sPulseCount[1] = sPulseCount[1] +1;
		// Check whether we can terminate
		if (sPulseCount[1] >= sPulseMaxCount[1]){
			// Stop Pulse 1
			PCTL0 &= ~(1<<PRUN0);
		}
	}
}


#ifdef __cplusplus
}
#endif