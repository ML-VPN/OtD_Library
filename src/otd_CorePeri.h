#ifndef OTD_COREPERI_H_
#define OTD_COREPERI_H_

#ifdef __cplusplus
extern "C"{
#endif 


#include <inttypes.h>


enum LAST_RESET_TYPE{
	LAST_RESET_POWERON = 0,
	LAST_RESET_EXT,
	LAST_RESET_BROWNOUT,
	LAST_RESET_WATCHDOG,
	LAST_RESET_NONE
};


// CORE
enum LAST_RESET_TYPE getLastResetCause();
void otd_InitCorePeri();
void otd_SoftReset();
// TIMER
unsigned long getUptime_ms();
// UART
void otd_UartPrintByte(uint8_t inData);
void otd_UartPrint(char *inStr);
void otd_UartPrintInt(int inData);
void otd_UartPrintFloat(float inData);
void otd_UartPrintln(char *inStr);
int16_t otd_UartRead(uint8_t inCanBlock);
void otd_UartWrite(uint8_t inData);


#ifdef __cplusplus
}
#endif

#endif /* OTD_COREPERI_H_ */
