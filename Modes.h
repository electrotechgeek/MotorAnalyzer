/*
 * Modes
 * Chip Wood Mar 2013
 */
 

 
/*
 * slowMotorRamp()
 * 
 * over 60 seconds, increase throttle to max and then back steadily
 * increase throttle from 1000 to 2000 in 30 sec and back in 30 sec
 * frameCounter = 100Hz counter
 * 1000/30 = 33.333333 times a second we need to increase throttle
 * divide counter by 3 to get 33 times a second, close enough
 */
void slowMotorRamp() {
  if ((throttle <= MAXCOMMAND) && (throttle >= MINCOMMAND)) 
  {
    if (throttle == MAXCOMMAND) {
      rampUpDown = 0;
    }
    if (rampUpDown == 0) {
      throttle--;
    }
    else if (rampUpDown == 1) {
      throttle++;
    }
  }
  if ((throttle == MINCOMMAND) && (rampUpDown == 0)) {
    stopTest();
  }
}

void startTest() {
  firstIteration = false;
  testRunning = true;
  testStartTime = currentTime;
  batteryData.testStartUsedCapacity = batteryData.usedCapacity;
  printTestStart();
}

void stopTest() {
  testEndTime = currentTime;
  mode = '0';
  throttle = MINCOMMAND;
  testRunning = false;
  printTestTime();
  queryType = 'x';
}
