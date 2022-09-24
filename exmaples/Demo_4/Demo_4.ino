#include <util/delay.h>
#include "otd_CorePeri.h"
#include "otd_DigitalIO.h"
#include "otd_Pulse.h"

enum MOTOR_STATE{
  MOTOR_STOP = 0,
  MOTOR_SLOW_FRWD,
  MOTOR_FAST_FRWD,
  MOTOR_SLOW_DOWN_FRWD,
  MOTOR_STOP_CHG_DIR,
  MOTOR_SLOW_BACK,
  MOTOR_FAST_BACK,
  MOTOR_SLOW_DOWN_BACK,
};

void setup() {

  // Call this function even to reset the MCUSR
  getLastResetCause();
  
  // Initialize core peripherals
  otd_InitCorePeri();

  // Initialize digital IO
  otd_InitDigitalIO();

  // Initialize Pulse
  otd_InitPulse();
  
}

void loop() {

  unsigned long motorLastTS = 0;    // Time stamp for last LED update in ms
  unsigned long currentTS;      // Current time stamp in ms
  //
  const unsigned int motorCheckDiff = 50; // Switch status is checked every 250ms
  enum MOTOR_STATE curMotorState = MOTOR_STOP;

  // Enable digital output
  otd_OutputEnable();

  // Set Motor direction forward
  otd_DigitalWrite(DIGITAL_OUTPUT_2, 1);

  // Enable motor control (active low signal)
  otd_DigitalWrite(DIGITAL_OUTPUT_1, 0);



  // Enter infinite loop
  while(1){
    // Get current time stamp
    currentTS = getUptime_ms();

    /*
     * Motor state update task
     */
    // Is LED update time
    if (currentTS-motorLastTS > motorCheckDiff){
      // Check whether pulse stoped
      if(otd_GetPulseEnabled(PULSE_OUTPUT_1) == 0){
        // Move to next motor state
        switch(curMotorState){
        case MOTOR_STOP:
          otd_SetPulseFreqDuty(PULSE_OUTPUT_1, 500, 128);
          otd_SetMaxPulseCount(PULSE_OUTPUT_1, 1000);
          otd_SetPulseEnabled(PULSE_OUTPUT_1, 1);
          curMotorState = MOTOR_SLOW_FRWD;
          break;

        case MOTOR_SLOW_FRWD:
          otd_SetPulseFreqDuty(PULSE_OUTPUT_1, 6000, 128);
          otd_SetMaxPulseCount(PULSE_OUTPUT_1, 10000);
          otd_SetPulseEnabled(PULSE_OUTPUT_1, 1);
          curMotorState = MOTOR_FAST_FRWD;
          break;

        case MOTOR_FAST_FRWD:
          otd_SetPulseFreqDuty(PULSE_OUTPUT_1, 500, 128);
          otd_SetMaxPulseCount(PULSE_OUTPUT_1, 1000);
          otd_SetPulseEnabled(PULSE_OUTPUT_1, 1);
          curMotorState = MOTOR_SLOW_DOWN_FRWD;
          break;

        case MOTOR_SLOW_DOWN_FRWD:
          // Set Motor direction backward
          otd_DigitalWrite(DIGITAL_OUTPUT_2, 0);
          // Wait 250 ms
          _delay_ms(250);
          curMotorState = MOTOR_STOP_CHG_DIR;
          break;

        case MOTOR_STOP_CHG_DIR:
          otd_SetPulseFreqDuty(PULSE_OUTPUT_1, 1000, 128);
          otd_SetMaxPulseCount(PULSE_OUTPUT_1, 1000);
          otd_SetPulseEnabled(PULSE_OUTPUT_1, 1);
          curMotorState = MOTOR_SLOW_BACK;
          break;

        case MOTOR_SLOW_BACK:
          otd_SetPulseFreqDuty(PULSE_OUTPUT_1, 2000, 128);
          otd_SetMaxPulseCount(PULSE_OUTPUT_1, 10000);
          otd_SetPulseEnabled(PULSE_OUTPUT_1, 1);
          curMotorState = MOTOR_FAST_BACK;
          break;

        case MOTOR_FAST_BACK:
          otd_SetPulseFreqDuty(PULSE_OUTPUT_1, 1000, 128);
          otd_SetMaxPulseCount(PULSE_OUTPUT_1, 1000);
          otd_SetPulseEnabled(PULSE_OUTPUT_1, 1);
          curMotorState = MOTOR_SLOW_DOWN_BACK;
          break;

        case MOTOR_SLOW_DOWN_BACK:
          // Set Motor direction forward
          otd_DigitalWrite(DIGITAL_OUTPUT_2, 1);
          _delay_ms(250); // Wait 250 ms
          curMotorState = MOTOR_STOP;
          break;
        }

      }
    }
  }
  
}
