/*
 * MotorAnalyzer Serial communications
 * Commands: 
 * Getting data from motor analyzer:
 * //0 - not used (literally does nothing, used to continue doing current task)
 * //1 - begin primaryTest
 * //2 - begin hover test (50% throttle for 15 secs)
 * //b - print current battery data (continuous stream)
 * //c - calibrate ESC (requires removing power from ESC and applying after a moment)
 * f - print pwm frequency
 * //g - print main loop delta time
 * //i - initEEPROM() and recalculate gain/pwm period
 * k - print load cell calibration values
 * //l - print load cell values
 * //p - pulse motor 3 times
 * //t - print last test start and end times (if no test run yet, returns 0,0)
 * //z - zero out or TARE load cell
 * Sending data to motor analyzer:
 * L - new Load Cell calibration - usage: Lweight where weight is the weight of the object. ex: L50.3
 * P - change PWM frequency - usage Pfreq where freq is frequency in Hz. ex: P400 (default is 400Hz)
 * //W - write all values to EEPROM - any calibration you do is not stored until W is sent
 * Stop all action:
 * x - stop everything
 */
 
/*Serial.print("a -                                 | A - \n");
Serial.print("b -                                 | B - \n");
Serial.print("c -                                 | C - \n");
Serial.print("d -                                 | D - \n");
Serial.print("e -                                 | E - \n");
Serial.print("f -                                 | F - \n");
Serial.print("g -                                 | G - \n");
Serial.print("h -                                 | H - \n");
Serial.print("i -                                 | I - \n");
Serial.print("j -                                 | J - \n");
Serial.print("k -                                 | K - \n");
Serial.print("l -                                 | L - \n");
Serial.print("m -                                 | M - \n");
Serial.print("n -                                 | N - \n");
Serial.print("o -                                 | O - \n");
Serial.print("p -                                 | P - \n");
Serial.print("q -                                 | Q - \n");
Serial.print("r -                                 | R - \n");
Serial.print("s -                                 | S - \n");
Serial.print("t -                                 | T - \n");
Serial.print("u -                                 | U - \n");
Serial.print("v -                                 | V - \n");
Serial.print("w -                                 | W - \n");
Serial.print("x -                                 | X - \n");
Serial.print("y -                                 | Y - \n");
Serial.print("z -                                 | Z - \n");*/





void processSerial() 
{
  if (Serial.available() && !validQuery) 
    query = Serial.read();
    
  switch (query) 
  {
    case '?':
      // help
      cliBusy = true;
      
      Serial.println("Motor Analyzer v0.2 CLI");
      Serial.println("Lower case letters print data, capitals used to change settings");
      Serial.println("1 - Primary test - see \"Primary Test CLI\"");
      Serial.println("2 - Response test - see \"Response Test CLI\"");
      Serial.println("3 - Hover test - see \"Hover Test CLI\"");
      Serial.println("a -                                          | A -");
      Serial.println("b - print current battery data               | B -");
      Serial.println("c - print current load cell data             | C -");
      Serial.println("d -                                          | D - calibrate ESC - REMOVE PROPS FIRST!!!");
      Serial.println("e -                                          | E -");
      Serial.println("f - print ESC PWM frequency in Hz            | F - Adjust ESC signal PWM frequency in Hz    | F400");
      Serial.println("g - print battery voltage/current pins       | G - Adjust battery voltage/current pins      | G0;1;");
      Serial.println("h - print battery vscale,vbias,cscale,cbias  | H - Adjust battery vscale,vbias,cscale,cbias | H78.5;0;136.36;0;");
      Serial.println("i -                                          | I - re-initialize EEPROM (restore defaults)");
      Serial.println("j -                                          | J -");
      Serial.println("k -                                          | K -");
      Serial.println("l - print timing loop deltas (ms)            | L -");
      Serial.println("\nPress space bar for more or enter a command....");
      
      while (!Serial.available());
      query = Serial.read();
      if (query != ' ') {
        validQuery = true;
        cliBusy = false;
        return; 
      }
      
      Serial.println();
      Serial.println("m - show primary test ramp up/down/max times | M - adjust primary ramp times (up/down/max)  | M20;20;5;");
      Serial.println("n -                                          | N -| ");
      Serial.println("o -                                          | O -");
      Serial.println("p -                                          | P - pulse motor n times \"Pn\" where n = int | P12;");
      Serial.println("q -                                          | Q -");
      Serial.println("r -                                          | R -");
      Serial.println("s -                                          | S -");
      Serial.println("t - print last test start and end times      | T -");
      Serial.println("u -                                          | U -");
      Serial.println("v -                                          | V -");
      Serial.println("w -                                          | W - Write all values to EEPROM");
      Serial.println("x - stop EVERYTHING                          | X - stop EVERYTHING");
      Serial.println("y -                                          | Y -");
      Serial.println("z - zero out or tare load cell               | Z -");

      query = 'x';
      cliBusy = false;
      break;
      
    case '+':
      // continue doing whatever is currently happening
      break;
    
    case '1':
      // begin primary data collection
      mode = PRIMARY_TEST;
      if (!testRunning) { firstIteration = true; }
      sendData();
      validQuery = false;
      break;
    
    case '2':
      // begin response data collection
      mode = RESPONSE_TEST;
      firstIteration = true;
      validQuery = false;
      break;
    
    case '3':
      // begin hover data collection
      mode = HOVER_TEST;
      firstIteration = true;
      validQuery = false;
      break;
      
    case 'a':
      // print load cell values continuously
      printLoad();
      validQuery = false;
      break;
    
    case 'b':
      // print current battery levels, total current usage, and current used during last test
      printBatteryData();
      validQuery = false;
      break;
    
    case 'f':
      // print pwm freq
      Serial.print("PWM freq (Hz): ");
      Serial.print(pwmFreq);
      Serial.println();
      query = 'x';
      validQuery = false;
      break;
      
    case 'g':
      // print battery voltage/current pins
      Serial.print("Battery Voltage pin: ");
      Serial.println(VPin); 
      Serial.print("Battery Current pin: ");
      Serial.println(CPin);
      query = 'x';
      validQuery = false;
      break;
      
    case 'i':
      // init EEPROM
      Serial.println("Initializing EEPROM");
      initEEPROM();
      writeEEPROM();
      // gain and pwm freq need to be recalculated
      initializeMotor();
      calculateLoadGain();
      query = 'x';
      validQuery = false;
      break;
      
    case 'k':
      // print load cell calibration
      printLoadCal();
      query = 'x';
      validQuery = false;
      break;
      
    case 'l':
      query = 'x';
      validQuery = false;
      break;
      
    case 'm':
      // display primary test ramp up/down/max times
      Serial.print("Primary test ramp up, ramp down, and max throttle times: ");
      Serial.print(primaryUpTime);
      Serial.print(", ");
      Serial.print(primaryDownTime);
      Serial.print(", ");
      Serial.println(primaryMaxThrottleTime);
      query = 'x';
      validQuery = false;
      break;
      
    case 't':    // send test start and end time
      printTestTime();
      query = 'x';
      validQuery = false;
      break;
      
    case 'z':
      // zero out, or TARE, load cell
      mode = TARE_LOAD;
      firstIteration = true;
      query = 'x';
      validQuery = false;
      break;
      
    
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    //////////////////////////// Writing values ///////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    
    case 'D':
      // calibrate ESC
      mode = CALIBRATE_ESC;
      query = 'x';
      validQuery = false;
      break;
      
    case 'F':
        // change motor pwm frequency
        int8_t temppwmFreq;
        temppwmFreq = readIntegerSerial();
        Serial.print("Old PWM Freq: "); Serial.print(pwmFreq); Serial.print("\n");
        changePWMfreq(temppwmFreq);
        Serial.print(", new PWM Freq: "); Serial.print(pwmFreq); Serial.print("\n");
        Serial.print("Re-initializing motors at new frequency...\n");
        query = 'f';
        validQuery = false;
        break;
        
    case 'G':
      // adjust Battery pins
      int8_t tempVPin, tempCPin;
      tempVPin = readIntegerSerial();
      tempCPin = readIntegerSerial();
      if (((0 <= tempVPin) || (tempVPin <= 7)) && ((0 <= tempCPin) || (tempCPin <= 6)) && (tempVPin != tempCPin)) 
      {
        VPin = tempVPin;
        CPin = tempCPin;
      } else if (tempVPin == tempCPin) {
        Serial.println("Cannot set C and V pins to the same pin, please try again."); 
      } else {
        Serial.println("Available pins are analog pins 0-6. Please try again."); 
      }    
      query = 'g';     
      validQuery = false;
      break;
    
    case 'H':
      // adjust battery voltage/curent scaling and bias
      float tempVScale, tempVBias, tempCScale, tempCBias;
      tempVScale = readFloatSerial();
      tempVBias  = readFloatSerial();
      tempCScale = readFloatSerial();
      tempCBias  = readFloatSerial();
      VScale = tempVScale;
      VBias  = tempVBias;
      CScale = tempCScale;
      CBias  = tempCBias;
      query = 'h';     
      validQuery = false;
      break;
  
    case 'L':
      //change load cell calibration
      mode = IDLE;
      
      newLoadCalWeight = readFloatSerial();
      
      // catch if we already have this calibration
      // or if we're obviously replacing low or high cal
      if (newLoadCalWeight <= loadCal[AWEIGHT]) 
      {
        newLoadCalNum = AREAD;
        Serial.print("Replacing low calibration\n");
      } 
      else if (newLoadCalWeight >= loadCal[BWEIGHT]) 
      {
        newLoadCalNum = BREAD;
        Serial.print("Replacing high calibration\n");
      }
      else if ((newLoadCalWeight > loadCal[AWEIGHT]) && (newLoadCalWeight < loadCal[BWEIGHT]))
      {
        // new calibration weight is in between previous readings
        // find which it's closer to, replace that one
        int tempDeltaCal0 = abs(newLoadCalWeight - loadCal[AWEIGHT]);
        int tempDeltaCal1 = abs(newLoadCalWeight - loadCal[BWEIGHT]);
        if (tempDeltaCal0 > tempDeltaCal1) {
          newLoadCalNum = BREAD;
          Serial.print("new cal closer to high cal, replacing high cal\n");
        } 
        else 
        {
          newLoadCalNum = AREAD;
          Serial.print("new cal closer to low cal, replacing low cal\n");
        }
      } /*else { // timed out - user didn't enter a number
        Serial.print("To calibrate the load cell, two measurements are taken with known weights.\n");
        Serial.print("Place a known weight on the scale and use the \"L\" command to initiate the calibration\n");
        Serial.print("Command \"L\" usage example: \"L50.3\" - 50.3 means the object weighs 50.3.\n");
        Serial.print("Unit of weight does not matter to this program - if you enter in grams, output load will be in grams.\n");
        newCalNum = NOCAL;
        break;
      }*/
      
      loadCal[newLoadCalNum + 1] = newLoadCalWeight;
      Serial.print("Weight");
      Serial.print(newLoadCalNum);
      Serial.print(" = ");
      Serial.println(newLoadCalWeight);
      rawLoadRead = true;
      digitalWrite(LEDPIN, LOW);
      query = 'A';
      validQuery = false;
      break;
      
    case 'M':
      // adjust primary test ramp up/down/max times
      int8_t tempPrimaryUpTime, tempPrimaryDownTime, tempPrimaryMaxThrottleTime;
      tempPrimaryUpTime          = readIntegerSerial();
      tempPrimaryDownTime        = readIntegerSerial();
      tempPrimaryMaxThrottleTime = readIntegerSerial();
      primaryUpTime          = tempPrimaryUpTime;
      primaryDownTime        = tempPrimaryDownTime;
      primaryMaxThrottleTime = tempPrimaryMaxThrottleTime;
      
      query = 'm';
      validQuery = false;
      break;
      
    case 'P':
      // pulse motor n times
      int8_t numPulses;
      numPulses = readIntegerSerial();
      pulseMotor(numPulses);
      query = 'x';
      validQuery = false;
      break;
      
    /*case 'P':
      query = 'x';
      validQuery = false;
      break;*/
      
    case 'W':
      // write all values to EEPROM
      Serial.println("Writing values to EEPROM . . .");
      writeEEPROM();
      query = 'x';
      break;
      
    
    case 'x':
      // stop everything
      if (mode != STOP) {
        stopTest();
        query = '+';
      }
      mode = STOP;
      firstIteration = false;
      validQuery = false;
      break;
  }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////


void sendData() 
{
  Serial.print((float)(currentTime - testStartTime)/1000);    // currentTime stored in uS, displayed in mS
  Serial.print(",");
  Serial.print(throttle / 10 - 100);                   // percentage. range of 1000-2000 -> 1000/10 = 100 - 100 = 0%
  Serial.print(",");
  Serial.print((float)batteryData.voltage/100);        // voltage internally stored at 10mV per, displayed in V
  Serial.print(",");
  Serial.print((float)batteryData.current/100);        // current stored at 10mA, displayed in A
  Serial.print(","); 
  Serial.print((float)(batteryData.usedCapacity - batteryData.testStartUsedCapacity)/1000);  // used curr stored in uAh, displayed in mAh
  Serial.print(",");
  Serial.println(load);                                // load in g
}

void printBatteryData() 
{
  Serial.print("Cells: ");
  Serial.print(batteryData.cells);
  Serial.print(", V: ");
  Serial.print((float)batteryData.voltage/100);
  Serial.print(", V per cell: ");
  Serial.print(((float)batteryData.voltage/100)/batteryData.cells);
  Serial.print(", I: ");
  Serial.print((float)batteryData.current);
  Serial.print(", Used I: ");
  Serial.print((float)batteryData.usedCapacity/1000);
  Serial.print(", Used I during test: ");
  Serial.println((float)(batteryData.usedCapacity - batteryData.testStartUsedCapacity)/1000);
  //Serial.println();
}

void printTestStart() 
{
  Serial.println("Beginning test");
  Serial.println("elapsedTime (ms),throttle,V,I (amps),used I (mAh),thrust (g)");
}

void printTestTime() 
{
  float testDuration = (float)(testEndTime - testStartTime) / 1000000;
  Serial.print("Start: ");
  Serial.print((float)testStartTime / 1000000); // u
  Serial.print(" sec, End: ");
  Serial.print((float)testEndTime / 1000000);
  Serial.print(" sec, Duration: ");
  Serial.print(testDuration);
  Serial.println(" sec");
}

void printLoad() 
{
  Serial.print("load = ");
  Serial.print(load);
  Serial.print("g, loadNoGain = ");
  Serial.print(loadNoGain);
  Serial.print(", loadAvg = ");
  Serial.println(loadAvg);
}

void printLoadCal() {
  Serial.print("aReading = ");
  Serial.print(loadCal[AREAD]);
  Serial.print(", aWeight = ");
  Serial.print(loadCal[AWEIGHT]);
  Serial.print(" g, bReading = ");
  Serial.print(loadCal[BREAD]);
  Serial.print(", bWeight = ");
  Serial.print(loadCal[BWEIGHT]);
  Serial.print(" g, gain: ");
  Serial.println(loadCalGain);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////


void readValueSerial(char *data, byte size);
void comma();

void readValueSerial(char *data, byte size) {
  byte index = 0;
  byte timeout = 0;
  data[0] = '\0';

  do {
    if (Serial.available() == 0) {
      delay(1);
      timeout++;
    } else {
      data[index] = Serial.read();
      timeout = 0;
      index++;
    }
  } while ((index == 0 || data[index-1] != ';') && (timeout < 10) && (index < size-1));

  data[index] = '\0';
}


// Used to read floating point values from the serial port
float readFloatSerial() {
  char data[15] = "";

  readValueSerial(data, sizeof(data));
  return atof(data);
}

// Used to read integer values from the serial port
long readIntegerSerial() {
  char data[16] = "";

  readValueSerial(data, sizeof(data));
  return atol(data);
}

/*void comma() {
  Serial.print(',');
}*/




