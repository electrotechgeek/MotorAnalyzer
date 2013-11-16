/*
 * AQ Motors_PWM_Timer.h condensed
 */


/* processMotor
 * 
 * Controls motor and writes MINCOMMAND unless:
 *   - mode != 0
 *   - battery is not in warning or alarm state.
 */ 
void processMotor() 
{
  if (!batteryAlarm && testRunning) writeMotor(throttle); 
  else writeMotor(MINCOMMAND);
}

/* pulseMotor
 * 
 * Send a few quick pulses to signal calibration is done.
 */

void pulseMotor(byte nPulse) 
{
  testRunning = true;
  for (byte i = 0; i < nPulse; i++) 
  {
    writeMotor(MINCOMMAND + 200);
    delay(250);
    writeMotor(MINCOMMAND);
    delay(250);
  }
}

void stopMotor()
{
  throttle = MINCOMMAND;
  writeMotor(throttle);
}

void calibrateESC()
{
  testRunning = true;
  Serial.println("SENDING MAXCOMMAND to motors for 3s, make sure props are off!!!!");
  throttle = MAXCOMMAND;
  writeMotor(throttle);
  delay(3000);
  Serial.println("Now sending MINCOMMAND to motors...");
  stopMotor();
}




void initializeMotor() 
{
  DDRB = DDRB | B00000010; // set port to output PB1 - digital pin 9
  
  // set motor to off
  writeMotor(MINCOMMAND);
  
  // initialize PWM timer 1 - 16 bit
  TCCR1A = ((1<<WGM11)|(1<<COM1A1));            // fast PWM mode, output on a1
  TCCR1B = ((1<<WGM13)|(1<<WGM12)|(1<<CS11));
  
  ICR1 = pwmCounterPeriod;
}

void writeMotor(int command)
{
  OCR1A = command * 2;  // pin 9.
}

void changePWMfreq(int newpwmFreq) 
{
  pwmFreq = newpwmFreq;
  pwmCounterPeriod = F_CPU/PWM_PRESCALER/pwmFreq;
  initializeMotor();
}

