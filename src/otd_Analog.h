#ifndef OTD_ANALOG_H_
#define OTD_ANALOG_H_

#ifdef __cplusplus
extern "C"{
#endif 

#include <stdint.h>



enum OTD_ANALOG_TYPE{
	OTD_ANALOG_VOLTAGE = 0,
	OTD_ANALOG_CURRENT,
	OTD_ANALOG_TYPE_NOT_SET
};


enum OTD_ANALOG_CHANNEL{
	OTD_ANALOG_SINGLE_1 = 0,
	OTD_ANALOG_SINGLE_2,
	OTD_ANALOG_SINGLE_3,
	OTD_ANALOG_DIFF_1,
	OTD_ANALOG_DIFF_2,
	OTD_ANALOG_CHAN_NOT_SET
};


enum OTD_ANALOG_DATARATE{
	OTD_ANALOG_DATARATE_30Hz = 0,
	OTD_ANALOG_DATARATE_15Hz,
	OTD_ANALOG_DATARATE_7p5Hz,
	OTD_ANALOG_DATARATE_3p75Hz
};


enum OTD_ANALOG_GAIN{
	OTD_ANALOG_GAIN_1 = 0,
	OTD_ANALOG_GAIN_2,
	OTD_ANALOG_GAIN_4,
	OTD_ANALOG_GAIN_8,
	OTD_ANALOG_GAIN_16,
	OTD_ANALOG_GAIN_32,
	OTD_ANALOG_GAIN_64,
	OTD_ANALOG_GAIN_128,
};


union OTD_ANALOG_VALUE {
	float current_mA;
	float voltage_V;
};






void otd_InitAnalog();
void otd_SetAnalogType(enum OTD_ANALOG_TYPE inAnaType);
enum OTD_ANALOG_TYPE otd_GetAnalogType();
void otd_SetAnalogChannel(enum OTD_ANALOG_CHANNEL inAnaChan);
enum OTD_ANALOG_CHANNEL otd_GetAnalogChannel();
void otd_SetAnalogDataRate(enum OTD_ANALOG_DATARATE inAnaDataRate);
enum OTD_ANALOG_DATARATE otd_GetAnalogDataRate();
void otd_SetAnalogGain(enum OTD_ANALOG_GAIN inAnaGain);
enum OTD_ANALOG_GAIN otd_GetAnalogGain();
uint8_t otd_IsAnalogDataReady();
union OTD_ANALOG_VALUE otd_AnalogRead();


#ifdef __cplusplus
}
#endif

#endif /* OTD_ANALOG_H_ */
