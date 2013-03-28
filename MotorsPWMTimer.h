/*
 * AQ Motors_PWM_Timer.h condensed
 */

int rampUpDown = 0; // 1 for up, 0 for down
long pwmFreq;
float pwmCounterPeriod;

// function prototypes
void writeMotor(int command);

/* processMotor
 * 
 * Controls motor and writes MINCOMMAND unless:
 *   - mode != 0
 *   - battery is not in warning or alarm state.
 */ 
void processMotor() {
  switch (mode) {
    case '0':
      // do nothing, waiting for command to start
      throttle = MINCOMMAND;      
      break;
      
    case '1':                           // case '1': slowMotorRamp()
      if (firstIteration) {
        startTest();
        rampUpDown = 1;                 // set ramping to up
      }
      slowMotorRamp();
      break;
      
    case 'c':
      // calibrate ESC, send MAXCOMMAND for 3s
      Serial.println("SENDING MAXCOMMAND to motors for 3s, make sure props are off!!!!");
      throttle = MAXCOMMAND;
      writeMotor(throttle);
      delay(3000);
      Serial.println("Now sending MINCOMMAND to motors...");
      throttle = MINCOMMAND;
      mode = '0';
      queryType = 'x';
      break;     
  }
  
  if (testRunning && batteryAlarm) {
    stopTest();
  }
  
  writeMotor(throttle);
}

/* pulseMotor
 * 
 * Send a few quick pulses to signal calibration is done.
 */

void pulseMotor(byte nbPulse) {
  for (byte i = 0; i < nbPulse; i++) {
    writeMotor(MINCOMMAND + 100);
    delay(250);
    writeMotor(MINCOMMAND);
    delay(250);
  }
}

//#define PWM_FREQUENCY 400 // in Hz
#define PWM_PRESCALER 8
//#define PWM_COUNTER_PERIOD (F_CPU/PWM_PRESCALER/PWM_FREQUENCY)



void initializeMotor() {
  DDRB = DDRB | B00000010; // set port to output PB1 - digital pin 9
  
  // set motor to off
  writeMotor(MINCOMMAND);
  
  // initialize PWM timer 1 - 16 bit
  TCCR1A = ((1<<WGM11)|(1<<COM1A1));            // fast PWM mode, output on a1
  TCCR1B = ((1<<WGM13)|(1<<WGM12)|(1<<CS11));
  pwmCounterPeriod = F_CPU/PWM_PRESCALER/pwmFreq;
  ICR1 = pwmCounterPeriod;
}

void writeMotor(int command) {
  OCR1A = command * 2;  // pin 9.
}

void changePWMfreq(int newpwmFreq) {
  pwmFreq = newpwmFreq;
  initializeMotor();
}

