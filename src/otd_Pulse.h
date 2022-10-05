/**
  ******************************************************************************
  * @file    otd_Pulse.h
  * @author  OtomaDUINO Team
  * @brief   This file contains pulse output functions prototypes and data types.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 ML-VPN.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */ 

#ifndef OTD_PULSE_H_
#define OTD_PULSE_H_

#ifdef __cplusplus
extern "C"{
#endif 

#include <stdint.h>


#define OTD_FREQ_MAX	160000	// 160KHz
#define OTD_FREQ_MIN	20		// 20Hz
#define OTD_DUTY_MAX	256		// 100%
#define OTD_DUTY_MIN	1		// 0.4%


enum PULSE_OUTPUT_PINS{
	PULSE_OUTPUT_1 = 0,
	PULSE_OUTPUT_2
};


void otd_InitPulse();
int8_t otd_SetPulseEnabled(enum PULSE_OUTPUT_PINS inPulseOutPin, uint8_t inIsEnabled);
uint8_t otd_GetPulseEnabled(enum PULSE_OUTPUT_PINS inPulseOutPin);
void otd_SetPulseFreqDuty(enum PULSE_OUTPUT_PINS inPulseOutPin, unsigned long inFreqHz, unsigned int inDutyCycle);
void otd_GetPulseFreqDuty(enum PULSE_OUTPUT_PINS inPulseOutPin, uint16_t *outFreq, uint8_t *outDuty);
//
void otd_SetMaxPulseCount(enum PULSE_OUTPUT_PINS inPulseOutPin, uint16_t inPulseMaxCount);
void otd_ResetMaxPulseCount(enum PULSE_OUTPUT_PINS inPulseOutPin);
uint16_t otd_GetPulseCount(enum PULSE_OUTPUT_PINS inPulseOutPin);

#ifdef __cplusplus
}
#endif

#endif /* OTD_PULSE_H_ */
