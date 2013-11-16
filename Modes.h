/*
 * Modes
 * Chip Wood Mar 2013
 */
 


void printTestStart();
void printTestTime(); 
void calculatePrimaryRampTimes();

boolean rampUp = true; // true for up, false for down

float primaryUpRamp, primaryDownRamp;
int16_t primaryMaxThrottleStartTime, primaryThrottle = 0;


void modeHandler()
{
  switch (mode) 
  {
    case IDLE:
      // do nothing, waiting for command to start
      throttle = MINCOMMAND;      
      break;
      
    case PRIMARY_TEST:                           
      // Primary Test: primaryTest()
      if (firstIteration) {
        startTest();
        calculatePrimaryRampTimes();
        primaryThrottle = MINCOMMAND;
        rampUp = true;
      }
      primaryTest();
      break;
      
    case CALIBRATE_ESC:
      // calibrate ESC, see motor.h
      calibrateESC();
      break;   
    
    case TARE_LOAD:
      // tare load cell - see loadCell.h
      if (firstIteration)
        startLoadTare();
      tareLoadCell();
      break;
      
    case STOP:
      // stop everything ASAP
      stopMotor();
      mode = IDLE;
      break;
      
      
  }
  
  if (testRunning && batteryIsAlarm()) stopTest(); 
}

/*
 * primaryTest()
 * 
 * over user defined # of seconds, increase throttle to max and then back steadily
 *   - increase throttle from min to max in primaryUpTime
 *   - hold throttle at max for primaryMaxThrottleTime
 *   - ramp down to min throttle in primaryDownTime
 * primaryTest() called @ 100Hz
 *   - how to adjust throttle over time
 *   - need se
 * 1000/30 = 33.333333 times a second we need to increase throttle
 * divide counter by 3 to get 33 times a second, close enough
 */
void primaryTest() 
{
  if ((throttle <= MAXCOMMAND) && (throttle >= minArmedThrottle)) 
  {
    if (throttle == MAXCOMMAND) {
      rampUp = false;
      primaryMaxThrottleStartTime = currentTime;
    }
    
    if (rampUp && (throttle != MAXCOMMAND)) {
      primaryThrottle += primaryUpTime;
    } 
    else if (throttle == MAXCOMMAND) {
      if ((currentTime - primaryMaxThrottleStartTime) > (primaryMaxThrottleTime / 1000000))
        primaryThrottle = MAXCOMMAND - 1;
    }
    else 
      primaryThrottle -= primaryDownTime;
    
    throttle = (int16_t)primaryThrottle; 
  }
  
  if ((throttle <= MINCOMMAND) && (!rampUp)) 
    stopTest();
}

void responseTest()
{
  
}

void hoverTest()
{
  
}



/* Calculate Ramp Times
 * Calculating amount to add to throttle every loop (100Hz)
 * formula - (max throttle - min throttle) / (seconds * 100)
 *
 */
void calculatePrimaryRampTimes()
{
  primaryUpRamp   = (float)((MAXCOMMAND - MINCOMMAND) / (primaryUpTime   * 100));
  primaryDownRamp = (float)((MAXCOMMAND - MINCOMMAND) / (primaryDownTime * 100));
}


void startTest() 
{
  firstIteration = false;
  testRunning = true;
  testStartTime = currentTime;
  batteryData.testStartUsedCapacity = batteryData.usedCapacity;
  printTestStart();
}

void stopTest() 
{
  testEndTime = currentTime;
  mode = STOP;
  stopMotor();
  testRunning = false;
  printTestTime();
  query = 'x';
}
