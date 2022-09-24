#include "otd_CorePeri.h"
#include <util/delay.h>

void setup() {
  // Call this function even to reset the MCUSR
  getLastResetCause();
  
  // Initialize core peripherals
  otd_InitCorePeri();
}

void loop() {

  // Print hello world to uart
  otd_UartPrintln("Hello World");
  
  // Wait 2 seconds
  _delay_ms(2000);
  
  // Apply soft reset
  otd_SoftReset();
  
  // Enter infinite loop
  while(1){
  }

}
