/*
 * AQ Motors_PWM_Timer.h condensed
 */


int16_t pwmFreq;
float pwmCounterPeriod;

int16_t minArmedThrottle;

// function prototypes
void writeMotor(int command);

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

void pulseMotor(byte nbPulse) 
{
  for (byte i = 0; i < nbPulse; i++) 
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

//#define PWM_FREQUENCY 400 // in Hz
#define PWM_PRESCALER 8
//#define PWM_COUNTER_PERIOD (F_CPU/PWM_PRESCALER/PWM_FREQUENCY)



void initializeMotor() 
{
  DDRB = DDRB | B00000010; // set port to output PB1 - digital pin 9
  
  // set motor to off
  writeMotor(MINCOMMAND);
  
  // initialize PWM timer 1 - 16 bit
  TCCR1A = ((1<<WGM11)|(1<<COM1A1));            // fast PWM mode, output on a1
  TCCR1B = ((1<<WGM13)|(1<<WGM12)|(1<<CS11));
  pwmCounterPeriod = F_CPU/PWM_PRESCALER/pwmFreq;
  ICR1 = pwmCounterPeriod;
}

void writeMotor(int command)
{
  OCR1A = command * 2;  // pin 9.
}

void changePWMfreq(int newpwmFreq) 
{
  pwmFreq = newpwmFreq;
  initializeMotor();
}

