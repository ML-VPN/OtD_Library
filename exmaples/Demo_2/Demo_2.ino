#include "otd_CorePeri.h"
#include <util/delay.h>

void setup() {
  // Call this function even to reset the MCUSR
  getLastResetCause();
  
  // Initialize core peripherals
  otd_InitCorePeri();
}

void loop() {

  uint8_t canBlock = 0;
  int16_t tmpChar = -1;

  // Enter infinite loop
  while(1){
    // Read UART
    tmpChar = otd_UartRead(canBlock);

    // Check whether we had a character
    if (tmpChar >= 0){
      // Send the character back to UART
      otd_UartWrite(tmpChar);
    }else{
      // Wait 100 ms
      _delay_ms(100);
    }
  }
  
}
