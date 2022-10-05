/**
  ******************************************************************************
  * @file    otd_DigitalIO.h
  * @author  OtomaDUINO Team
  * @brief  This file contains digital input / output functions prototypes and data types.
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

#ifndef OTD_DIGITALIO_H_
#define OTD_DIGITALIO_H_

#ifdef __cplusplus
extern "C"{
#endif 

#include <stdint.h>

enum DIGITAL_INPUT_PINS{
	DIGITAL_INPUT_1 = 0,
	DIGITAL_INPUT_2,
	DIGITAL_INPUT_3,
	DIGITAL_INPUT_4,
	DIGITAL_INPUT_5,
	DIGITAL_INPUT_6,
	DIGITAL_INPUT_7,
	DIGITAL_INPUT_8
};

enum DIGITAL_OUTPUT_PINS{
	DIGITAL_OUTPUT_1 = 0,
	DIGITAL_OUTPUT_2,
	DIGITAL_OUTPUT_3,
	DIGITAL_OUTPUT_4,
	DIGITAL_OUTPUT_5,
	DIGITAL_OUTPUT_6
};



void otd_InitDigitalIO();
uint8_t otd_DigitalRead(enum DIGITAL_INPUT_PINS inInputPin);
uint8_t otd_DigitalReadAll();
void otd_DigitalWrite(enum DIGITAL_OUTPUT_PINS inOutputPin, uint8_t inValue);
uint8_t otd_GetDigitalWriteState(enum DIGITAL_OUTPUT_PINS inOutputPin);
void otd_OutputEnable();
void otd_OutputDisable();

#ifdef __cplusplus
}
#endif

#endif /* OTD_DIGITALIO_H_ */
