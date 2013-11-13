/*
 * Modes
 * Chip Wood Mar 2013
 */
 
#define CALIBRATE_ESC 'c'
#define PRIMARY_TEST 'p'
#define RESPONSE_TEST 'r'
#define HOVER_TEST 'h'
#define STOP 'X'

void printTestStart();
void printTestTime(); 

boolean rampUp = true; // true for up, false for down


void modeHandler()
{
  switch (mode) 
  {
    case '0':
      // do nothing, waiting for command to start
      throttle = MINCOMMAND;      
      break;
      
    case 'p':                           // Primary Test: primaryTest()
      if (firstIteration) {
        startTest();
        rampUp = true;                  // set ramping to up
      }
      primaryTest();
      break;
      
    case 'c':
      // calibrate ESC, send MAXCOMMAND for 3s
      Serial.println("SENDING MAXCOMMAND to motors for 3s, make sure props are off!!!!");
      throttle = MAXCOMMAND;
      writeMotor(throttle);
      delay(3000);
      Serial.println("Now sending MINCOMMAND to motors...");
      mode = STOP;
      query = 'x';
      break;     
  }
  
  if (testRunning && batteryAlarm) stopTest(); 
}

/*
 * primaryTest()
 * 
 * over user defined # of seconds, increase throttle to max and then back steadily
 * increase throttle from 1000 to 2000 in 30 sec and back in 30 sec
 * frameCounter = 100Hz counter
 * 1000/30 = 33.333333 times a second we need to increase throttle
 * divide counter by 3 to get 33 times a second, close enough
 */
void primaryTest() 
{
  if ((throttle <= MAXCOMMAND) && (throttle >= minArmedThrottle)) 
  {
    if (throttle == MAXCOMMAND) 
      rampUp = false;
    
    if (rampUp) throttle++; 
    else throttle--; 
  }
  
  if ((throttle <= MINCOMMAND) && (!rampUp)) 
    stopTest();
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
