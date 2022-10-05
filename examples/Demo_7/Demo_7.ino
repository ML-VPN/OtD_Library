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

  unsigned long sensorReadLastTS = 0;     // Time stamp for last sensor read in ms
  unsigned long currentTS;          // Current time stamp in ms
  unsigned long lastSampleTS = 0;       // Used for verifying sampling period
  //
  const unsigned int sensorCheckDiff = 10;  // Switch status is checked every 10ms
  //
  uint8_t isLoadCell = 1;


  // Configure analog type
  otd_SetAnalogType(OTD_ANALOG_VOLTAGE);
  // Configure analog channel
  otd_SetAnalogChannel(OTD_ANALOG_DIFF_1);
  // Set data rate to 3.75Hz
  otd_SetAnalogDataRate(OTD_ANALOG_DATARATE_15Hz);
  // Set gain to 128
  otd_SetAnalogGain(OTD_ANALOG_GAIN_128);

  uint8_t isDataReady;
  union OTD_ANALOG_VALUE anaValue;
  union OTD_ANALOG_VALUE anaValue_loadcell;
  union OTD_ANALOG_VALUE anaValue_prox;
  //
  anaValue_loadcell.voltage_V = 0.0;
  anaValue_prox.current_mA = 0.0;

  // Infinite loop
  while(1){
    // Get current time stamp
    currentTS = getUptime_ms();

    // Proximity sensor task
    if (currentTS-sensorReadLastTS > sensorCheckDiff){

      isDataReady = otd_IsAnalogDataReady();
      if (isDataReady == 1){
        anaValue = otd_AnalogRead();

        // Switch analog channel
        if (isLoadCell == 1){
          // Set the analog value for the load cell
          anaValue_loadcell = anaValue;
          // Switch to proximity sensor with current read
          otd_SetAnalogType(OTD_ANALOG_CURRENT);
          otd_SetAnalogChannel(OTD_ANALOG_SINGLE_3);
          otd_SetAnalogGain(OTD_ANALOG_GAIN_1);
          isLoadCell = 0;
        }else{
          // Set the analog value for the proximity sensor
          anaValue_prox = anaValue;
          // Switch to loadcell with differential voltage read
          otd_SetAnalogType(OTD_ANALOG_VOLTAGE);
          otd_SetAnalogChannel(OTD_ANALOG_DIFF_1);
          otd_SetAnalogGain(OTD_ANALOG_GAIN_128);
          isLoadCell = 1;
        }


        // Display the sensor values
        otd_UartPrint("> uV: ");
        otd_UartPrintFloat(anaValue_loadcell.voltage_V*1000000);
        otd_UartPrint("  -  mA: ");
        otd_UartPrintFloat(anaValue_prox.current_mA);
        otd_UartPrint("   [");
        otd_UartPrintInt(currentTS-lastSampleTS);
        otd_UartPrint("ms]");
        otd_UartPrintByte('\n');

        lastSampleTS = currentTS;
      }

      sensorReadLastTS = currentTS;
    }
  }
}
