#include "otd_CorePeri.h"
#include "otd_Analog.h"

void setup() {

  // Call this function even to reset the MCUSR
  getLastResetCause();
  
  // Initialize core peripherals
  otd_InitCorePeri();

  // Initialize analog interface
  otd_InitAnalog();

}

void loop() {

  unsigned long proxSensLastTS = 0;     // Time stamp for last LED update in ms
  unsigned long currentTS;          // Current time stamp in ms
  unsigned long lastSampleTS = 0;       // Used for verifying sampling period
  //
  const unsigned int proxSensCheckDiff = 10;  // Switch status is checked every 15ms


  // Configure analog type
  otd_SetAnalogType(OTD_ANALOG_CURRENT);
  // Configure analog channel
  otd_SetAnalogChannel(OTD_ANALOG_SINGLE_1);
  // Set data rate to 3.75Hz
  otd_SetAnalogDataRate(OTD_ANALOG_DATARATE_3p75Hz);

  uint8_t isDataReady;
  union OTD_ANALOG_VALUE anaValue;
  enum OTD_ANALOG_TYPE anaType = otd_GetAnalogType();

  // Infinite loop
  while(1){
    // Get current time stamp
    currentTS = getUptime_ms();

    // Proximity sensor task
    if (currentTS-proxSensLastTS > proxSensCheckDiff){

      isDataReady = otd_IsAnalogDataReady();
      if (isDataReady == 1){
        // Get analog value
        anaValue = otd_AnalogRead();

        if(anaType == OTD_ANALOG_VOLTAGE){
          // Display analog value with sampling period (in ms)
          otd_UartPrint("> V: ");
          otd_UartPrintFloat(anaValue.voltage_V);
          otd_UartPrint("   [");
          otd_UartPrintInt(currentTS-lastSampleTS);
          otd_UartPrint("ms]");
          otd_UartPrintByte('\n');
        }

        if(anaType == OTD_ANALOG_CURRENT){
          // Display analog value with sampling period (in ms)
          otd_UartPrint("> mA: ");
          otd_UartPrintFloat(anaValue.current_mA);
          otd_UartPrint("   [");
          otd_UartPrintInt(currentTS-lastSampleTS);
          otd_UartPrint("ms]");
          otd_UartPrintByte('\n');
        }

        lastSampleTS = currentTS;
      }

      proxSensLastTS = currentTS;
    }
  }
  
}
