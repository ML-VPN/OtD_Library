/**
  ******************************************************************************
  * @file    otd_Analog.cpp
  * @author  OtomaDUINO Team
  * @brief   This file contains analog data input functions.
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

#include "otd_Analog.h"

#ifdef __cplusplus
extern "C"{
#endif 

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

static enum OTD_ANALOG_TYPE sAnalogType = OTD_ANALOG_TYPE_NOT_SET;
static enum OTD_ANALOG_CHANNEL sAnalogChannel = OTD_ANALOG_CHAN_NOT_SET;
static enum OTD_ANALOG_DATARATE sAnalogDataRate = OTD_ANALOG_DATARATE_15Hz;
static enum OTD_ANALOG_GAIN sAnalogGain = OTD_ANALOG_GAIN_1;
static float sAnalogGainValue = 1;



#define IC1242_CS_PORT		PORTD
#define IC1242_CS_PIN		0
#define IC1242_CS_ENABLE   (IC1242_CS_PORT &= ~(1 << IC1242_CS_PIN))
#define IC1242_CS_DISABLE	(IC1242_CS_PORT |= (1 << IC1242_CS_PIN))


// Command Definitions
#define IC1242_CMD_READ_DATA           (0x01)
#define IC1242_CMD_READ_CONT           (0x03)
#define IC1242_CMD_READ_CSTOP          (0x0F)
#define IC1242_CMD_READ_REGISTER       (0x10)
#define IC1242_CMD_WRITE_REGISTER      (0x50)
#define IC1242_CMD_SELF_OFFSET_CALIB   (0xF1)
#define IC1242_CMD_SELF_GAIN_CALIB     (0xF2)
#define IC1242_CMD_SYSTEM_OFFSET_CALIB (0xF3)
#define IC1242_CMD_RESET               (0xFE)

// Register Addresses
#define IC1242_REG_SETUP               (0x00)  /* Setup Register */
#define IC1242_REG_MUX                 (0x01)  /* Multiplexer Control Register */
#define IC1242_REG_ACR                 (0x02)  /* Analog Control Register */
#define IC1242_REG_ODAC                (0x03)  /* Offset DAC */
#define IC1242_REG_DIO                 (0x04)  /* Data I/O */
#define IC1242_REG_DIR                 (0x05)  /* Direction Control for Data I/O */
#define IC1242_REG_IOCON               (0x06)  /* I/O Configuration Register */
//
#define IC1242_REG_FSR2               	(0x0C)  /* Gain Calibration Coefficient (Most Significant Byte) */
#define IC1242_REG_DOR2               	(0x0D)  /* Data Output Register (Most Significant Byte) */
#define IC1242_REG_DOR1               	(0x0E)  /* Data Output Register (Middle Byte) */
#define IC1242_REG_DOR0               	(0x0F)  /* Data Output Register (Least Significant Byte) */

// SETUP
#define IC1242_REG_SETUP_BIT_PGA2       2
#define IC1242_REG_SETUP_BIT_PGA1       1
#define IC1242_REG_SETUP_BIT_PGA0       0

// ACR
#define IC1242_REG_ACR_BIT_NDRDY       7
#define IC1242_REG_ACR_BIT_UnB         6
#define IC1242_REG_ACR_BIT_SPEED       5
#define IC1242_REG_ACR_BIT_BUFEN       4
#define IC1242_REG_ACR_BIT_BITORDER    3
#define IC1242_REG_ACR_BIT_RANGE       2
#define IC1242_REG_ACR_BIT_DR1         1
#define IC1242_REG_ACR_BIT_DR0         0



#define OSC_FREQ_HZ		(49152*1000)
static const float sOscPeriod_uS = (1000.0 * 1000.0) /OSC_FREQ_HZ;
//
#define ADC_V_COEF				0.000000268
#define ANALOG_VADC_OFFSET		2.25
#define ANALOG_IF_SCALE			4.444444444
#define ANALOG_VIN_OFFSET		10



static void ic1242_Reset();
static uint8_t ic1242_ReadRegister(uint8_t inRegAddr);
static void ic1242_WriteRegister(uint8_t inRegAddr, uint8_t inRegData);
static float ic1242_ConvertToVolt(uint32_t inData, uint8_t);
//
static void initSpi();
static uint8_t transferSpi(uint8_t inData);



void otd_InitAnalog(){

	uint8_t tmpRegAddr;
	uint8_t tmpDataRead;
	uint8_t tmpDataWrite;

	// Release chip select
	PORTD |= _BV(PORTD0);
	DDRD |= _BV(PORTD0);

	// Initialize SPI interface
	initSpi();

	_delay_us(250);

	// Reset the IC1242
	ic1242_Reset();
	_delay_us(250);
	ic1242_Reset();


	// Read Analog Control Register
	tmpRegAddr = IC1242_REG_ACR;
	tmpDataRead = ic1242_ReadRegister(tmpRegAddr);

	// Set range to "-/+Vref/2"
	tmpDataWrite = tmpDataRead;
	tmpDataWrite |= 1 << IC1242_REG_ACR_BIT_RANGE;
	// Set data rate to 7.5Hz
	tmpDataWrite |= 1 << IC1242_REG_ACR_BIT_SPEED;
	tmpDataWrite |= 1 << IC1242_REG_ACR_BIT_DR0;
	// Write Analog Control Register
	tmpRegAddr = IC1242_REG_ACR;
	ic1242_WriteRegister(tmpRegAddr, tmpDataWrite);


	// Reset analog type and channel
	sAnalogType = OTD_ANALOG_TYPE_NOT_SET;
	sAnalogChannel = OTD_ANALOG_CHAN_NOT_SET;
	// Default data rate is 3.75Hz
	sAnalogDataRate = OTD_ANALOG_DATARATE_7p5Hz;

	return;
}



void otd_SetAnalogType(enum OTD_ANALOG_TYPE inAnaType){

	switch (inAnaType){
	case OTD_ANALOG_VOLTAGE:
	case OTD_ANALOG_CURRENT:
		break;

	default:
		return;
	}

	sAnalogType = inAnaType;
	return;
}


enum OTD_ANALOG_TYPE otd_GetAnalogType(){
	return sAnalogType;
}


void otd_SetAnalogChannel(enum OTD_ANALOG_CHANNEL inAnaChan){

	uint8_t tmpDataWrite = 0;
	uint8_t tmpRegAddr;


	switch (inAnaChan){
	case OTD_ANALOG_SINGLE_1:
		// Positive channel is 1 - Negative channel is 4 (For single end, connected internally to reference)
		tmpDataWrite = 0x03;
		break;

	case OTD_ANALOG_SINGLE_2:
		// Positive channel is 2 - Negative channel is 4 (For single end, connected internally to reference)
		tmpDataWrite = 0x13;
		break;

	case OTD_ANALOG_SINGLE_3:
		// Positive channel is 3 - Negative channel is 4 (For single end, connected internally to reference)
		tmpDataWrite = 0x23;
		break;

	case OTD_ANALOG_DIFF_1:
		// Positive channel is 1 - Negative channel is 2
		tmpDataWrite = 0x01;
		break;

	case OTD_ANALOG_DIFF_2:
		// Positive channel is 3 - Negative channel is 4
		tmpDataWrite = 0x23;
		break;

	default:
		return;
	}


	/*
	 * Just in case write multiplexer value twice
	 */
	// Write Multiplexer Control Register
	tmpRegAddr = IC1242_REG_MUX;
	ic1242_WriteRegister(tmpRegAddr, tmpDataWrite);
	_delay_us(100);
	tmpRegAddr = IC1242_REG_MUX;
	ic1242_WriteRegister(tmpRegAddr, tmpDataWrite);

	sAnalogChannel = inAnaChan;
	return;
}


enum OTD_ANALOG_CHANNEL otd_GetAnalogChannel(){

	return sAnalogChannel;
}


void otd_SetAnalogDataRate(enum OTD_ANALOG_DATARATE inAnaDataRate){

	uint8_t mask = 0;
	uint8_t tmpRegAddr;
	uint8_t tmpDataRead;
	uint8_t tmpDataWrite;

	switch (inAnaDataRate){
	case OTD_ANALOG_DATARATE_30Hz:
		mask &= ~(1 << IC1242_REG_ACR_BIT_SPEED);
		mask &= ~(1 << IC1242_REG_ACR_BIT_DR0);
		mask &= ~(1 << IC1242_REG_ACR_BIT_DR1);
		break;

	case OTD_ANALOG_DATARATE_15Hz:
		mask |= (1 << IC1242_REG_ACR_BIT_SPEED);
		mask &= ~(1 << IC1242_REG_ACR_BIT_DR0);
		mask &= ~(1 << IC1242_REG_ACR_BIT_DR1);
		break;

	case OTD_ANALOG_DATARATE_7p5Hz:
		mask |= (1 << IC1242_REG_ACR_BIT_SPEED);
		mask |= (1 << IC1242_REG_ACR_BIT_DR0);
		mask &= ~(1 << IC1242_REG_ACR_BIT_DR1);
		break;

	case OTD_ANALOG_DATARATE_3p75Hz:
		mask |= (1 << IC1242_REG_ACR_BIT_SPEED);
		mask &= ~(1 << IC1242_REG_ACR_BIT_DR0);
		mask |= (1 << IC1242_REG_ACR_BIT_DR1);
		break;

	default:
		return;
	}



	// Read Analog Control Register
	tmpRegAddr = IC1242_REG_ACR;
	tmpDataRead = ic1242_ReadRegister(tmpRegAddr);
	// Apply mask to register
	tmpDataWrite = tmpDataRead;
	tmpDataWrite &= 0xDC;		// Clear Speed and datarate bits
	tmpDataWrite |= mask;

	// Write Analog Control Register
	tmpRegAddr = IC1242_REG_ACR;
	ic1242_WriteRegister(tmpRegAddr, tmpDataWrite);


	sAnalogDataRate = inAnaDataRate;

	return;
}


enum OTD_ANALOG_DATARATE otd_GetAnalogDataRate(){
	return sAnalogDataRate;
}


void otd_SetAnalogGain(enum OTD_ANALOG_GAIN inAnaGain){

	uint8_t mask = 0;
	uint8_t tmpRegAddr;
	uint8_t tmpDataRead;
	uint8_t tmpDataWrite;

	switch(inAnaGain){
	case OTD_ANALOG_GAIN_1:
		mask &= ~(1 << IC1242_REG_SETUP_BIT_PGA0);
		mask &= ~(1 << IC1242_REG_SETUP_BIT_PGA1);
		mask &= ~(1 << IC1242_REG_SETUP_BIT_PGA2);
		sAnalogGainValue = 1;
		break;

	case OTD_ANALOG_GAIN_2:
		mask |= (1 << IC1242_REG_SETUP_BIT_PGA0);
		mask &= ~(1 << IC1242_REG_SETUP_BIT_PGA1);
		mask &= ~(1 << IC1242_REG_SETUP_BIT_PGA2);
		sAnalogGainValue = 2;
		break;

	case OTD_ANALOG_GAIN_4:
		mask &= ~(1 << IC1242_REG_SETUP_BIT_PGA0);
		mask |= (1 << IC1242_REG_SETUP_BIT_PGA1);
		mask &= ~(1 << IC1242_REG_SETUP_BIT_PGA2);
		sAnalogGainValue = 4;
		break;

	case OTD_ANALOG_GAIN_8:
		mask |= (1 << IC1242_REG_SETUP_BIT_PGA0);
		mask |= (1 << IC1242_REG_SETUP_BIT_PGA1);
		mask &= ~(1 << IC1242_REG_SETUP_BIT_PGA2);
		sAnalogGainValue = 8;
		break;

	case OTD_ANALOG_GAIN_16:
		mask &= ~(1 << IC1242_REG_SETUP_BIT_PGA0);
		mask &= ~(1 << IC1242_REG_SETUP_BIT_PGA1);
		mask |= (1 << IC1242_REG_SETUP_BIT_PGA2);
		sAnalogGainValue = 16;
		break;

	case OTD_ANALOG_GAIN_32:
		mask |= (1 << IC1242_REG_SETUP_BIT_PGA0);
		mask &= ~(1 << IC1242_REG_SETUP_BIT_PGA1);
		mask |= (1 << IC1242_REG_SETUP_BIT_PGA2);
		sAnalogGainValue = 32;
		break;

	case OTD_ANALOG_GAIN_64:
		mask &= ~(1 << IC1242_REG_SETUP_BIT_PGA0);
		mask |= (1 << IC1242_REG_SETUP_BIT_PGA1);
		mask |= (1 << IC1242_REG_SETUP_BIT_PGA2);
		sAnalogGainValue = 64;
		break;

	case OTD_ANALOG_GAIN_128:
		mask |= (1 << IC1242_REG_SETUP_BIT_PGA0);
		mask |= (1 << IC1242_REG_SETUP_BIT_PGA1);
		mask |= (1 << IC1242_REG_SETUP_BIT_PGA2);
		sAnalogGainValue = 128;
		break;

	default:
		return;
	}


	// Read Setup Register
	tmpRegAddr = IC1242_REG_SETUP;
	tmpDataRead = ic1242_ReadRegister(tmpRegAddr);


	// Apply mask to register
	tmpDataWrite = tmpDataRead;
	tmpDataWrite &= 0xF8;		// Clear PGA bits
	tmpDataWrite |= mask;

	/*
	 * Just in case write gain twice
	 */
	// Write Setup Register
	tmpRegAddr = IC1242_REG_SETUP;
	ic1242_WriteRegister(tmpRegAddr, tmpDataWrite);
	_delay_us(100);
	tmpRegAddr = IC1242_REG_SETUP;
	ic1242_WriteRegister(tmpRegAddr, tmpDataWrite);

	sAnalogGain = inAnaGain;

	return;
}


enum OTD_ANALOG_GAIN otd_GetAnalogGain(){
	return sAnalogGain;
}


uint8_t otd_IsAnalogDataReady(){

	uint8_t tmpRegAddr;
	uint8_t tmpDataRead;

	IC1242_CS_ENABLE;

	// Read setup register
	tmpRegAddr = IC1242_REG_ACR;
	tmpDataRead = ic1242_ReadRegister(tmpRegAddr);

	IC1242_CS_DISABLE;

	// Data Ready bit is active low
	uint8_t isDataReady = tmpDataRead & (1 << IC1242_REG_ACR_BIT_NDRDY);
	isDataReady = isDataReady == 0;
	return isDataReady;
}


union OTD_ANALOG_VALUE otd_AnalogRead(){


	uint32_t voltData = 0;
	uint8_t *outSeq = (uint8_t *) &voltData;

	// Send READ command
	IC1242_CS_ENABLE;
	transferSpi(IC1242_CMD_READ_DATA);

	// Wait for data to be ready
	_delay_us(150*sOscPeriod_uS);

	// Get the register data
	outSeq[2] = transferSpi(0);
	outSeq[1] = transferSpi(0);
	outSeq[0] = transferSpi(0);

	IC1242_CS_DISABLE;

	union OTD_ANALOG_VALUE outAnalogValue;
	memset(&outAnalogValue, 0, sizeof(outAnalogValue));


	if (sAnalogChannel != OTD_ANALOG_DIFF_1 && sAnalogChannel !=OTD_ANALOG_DIFF_2){
		float tmpVolt = ic1242_ConvertToVolt(voltData, 0);
		//
		if (sAnalogType == OTD_ANALOG_VOLTAGE){
			outAnalogValue.voltage_V = tmpVolt;
		}
		if (sAnalogType == OTD_ANALOG_CURRENT){
			// Convert input voltage  to current. Sense resistor 249R.
			float tmpCurrent_mA = tmpVolt/249*1000;
			outAnalogValue.current_mA = tmpCurrent_mA;
		}
	}else{
		float tmpVolt = ic1242_ConvertToVolt(voltData, 1);
		outAnalogValue.voltage_V = tmpVolt;
	}



	return outAnalogValue;
}





static void ic1242_Reset(){

	IC1242_CS_ENABLE;

	// Send Reset command
	transferSpi(IC1242_CMD_RESET);

	IC1242_CS_DISABLE;


	return;
}



static uint8_t ic1242_ReadRegister(uint8_t inRegAddr){


	uint8_t tmpRegVal = 0x00;
	uint8_t tmpCmd;

	IC1242_CS_ENABLE;

	// Form the read command
	tmpCmd = IC1242_CMD_READ_REGISTER | inRegAddr;
	transferSpi(tmpCmd);

	// Set the read count "1 Byte/Register"
	transferSpi(0);

	// Wait for data to be ready
	_delay_us(100*sOscPeriod_uS);

	// Get the register data
	tmpRegVal = transferSpi(0);

	IC1242_CS_DISABLE;

	return tmpRegVal;
}

static void ic1242_WriteRegister(uint8_t inRegAddr, uint8_t inRegData){


	uint8_t tmpCmd;

	IC1242_CS_ENABLE;

	// Form the read command
	tmpCmd = IC1242_CMD_WRITE_REGISTER | inRegAddr;
	transferSpi(tmpCmd);

	// Set the write count "1 Byte/Register"
	transferSpi(0);

	// Set the register data
	transferSpi(inRegData);

	IC1242_CS_DISABLE;

	return;
}


static float ic1242_ConvertToVolt(uint32_t inData, uint8_t inIsDiff){

	float outVolt = 0;

	// Check sign of the data
	int32_t tmpConv = inData;
	if (inData >= 0x800000){
		// Sign is negative
		tmpConv &= ~0x800000;			// Reset sign
		tmpConv = -0x800000+tmpConv;	// Convert Two's Complement to Decimal
	}
	//
	float adcVolt = ((float)tmpConv*ADC_V_COEF);

	if (inIsDiff == 1){
		adcVolt = adcVolt/sAnalogGainValue*ANALOG_IF_SCALE;
		return adcVolt;
	}
	outVolt = -1*((adcVolt/sAnalogGainValue +ANALOG_VADC_OFFSET)*ANALOG_IF_SCALE-ANALOG_VIN_OFFSET);

	return outVolt;
}



/*
 * SPI FUNCTIONS
 */
static void initSpi(){

	// SS
	DDRD |= _BV(PORTD0);
	PORTD |= _BV(PORTD0);
	// SCK
	DDRB |= _BV(PORTB5);
	// MOSI
	DDRB |= _BV(PORTB4);

	// Disable SPI Interrupt
    SPCR &= ~_BV(SPIE);

	// Set SPI as master
    SPCR |= _BV(MSTR);

    // Clock zero when idle
    SPCR &= ~_BV(CPOL);

    // Clock phase setup-sample
    SPCR |= _BV(CPHA);

	// MSB First
    SPCR &= ~_BV(DORD);

    // fclk_io/128
	SPCR |= _BV(SPR0);
	SPCR |= _BV(SPR1);
	SPSR &= ~_BV(SPI2X);


	// Enable SPI
    SPCR |= _BV(SPE);

	return;
}



static uint8_t transferSpi(uint8_t inData){

	// Send data
	SPDR = inData;

	// Wait for data
	while ((SPSR & _BV(SPIF)) == 0);

	// Read data
	return SPDR;
}


#ifdef __cplusplus
}
#endif
