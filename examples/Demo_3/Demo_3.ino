
#include "otd_CorePeri.h"
#include "otd_DigitalIO.h"

void setup() {
  // Call this function even to reset the MCUSR
  getLastResetCause();
  
  // Initialize core peripherals
  otd_InitCorePeri();

  // Initialize digital IO
  otd_InitDigitalIO();
}

void loop() {
  unsigned long switchLastTS = 0;   // Time stamp for last switch check in ms
  unsigned long buttonLastTS = 0;   // Time stamp for last button check in ms
  unsigned long ledLastTS = 0;    // Time stamp for last LED update in ms
  unsigned long currentTS;      // Current time stamp in ms
  //
  const unsigned int switchCheckDiff = 250; // Switch status is checked every 250ms
  const unsigned int buttonCheckDiff = 50;  // Button status is checked every 50ms
  const unsigned int ledCheckDiff = 100;    // LED is updated every 100ms


  uint8_t switchState = 0;
  uint8_t buttonState = 0;


  // Enable digital output
  otd_OutputEnable();

  // Enter infinite loop
  while(1){
    // Get current time stamp
    currentTS = getUptime_ms();
    /*
     * Switch Check Task
     */
    // Is switch check time
    if (currentTS-switchLastTS > switchCheckDiff){
      switchState = otd_DigitalRead(DIGITAL_INPUT_7);
      // Update switch last time stamp
      switchLastTS = currentTS;
    }


    /*
     * Button Check Task
     */
    // Is button check time
    if (currentTS-buttonLastTS > buttonCheckDiff){
      buttonState = otd_DigitalRead(DIGITAL_INPUT_1);
      // Update button last time stamp
      buttonLastTS = currentTS;
    }


    /*
     * LED update Task
     */
    // Is LED update time
    if (currentTS-ledLastTS > ledCheckDiff){

      // Check whether switch is on
      if (switchState == 1){
        // Check whether button is pressed
        if(buttonState == 1){
          // Turn on the led
          otd_DigitalWrite(DIGITAL_OUTPUT_6, 1);
        }else{
          // Blink the led
          if (otd_GetDigitalWriteState(DIGITAL_OUTPUT_6) == 1){
            // Turn off the LED
            otd_DigitalWrite(DIGITAL_OUTPUT_6, 0);
          }else{
            // Turn on the LED
            otd_DigitalWrite(DIGITAL_OUTPUT_6, 1);
          }
        }
      }else{
        // Turn off the led
        otd_DigitalWrite(DIGITAL_OUTPUT_6, 0);
      }

      // Update led last time stamp
      ledLastTS = currentTS;
    }
  }
}
