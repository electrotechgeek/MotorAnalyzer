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



void motorSetupCLI();
void sendData();
void printBatteryData();
void printTestStart();
void printTestTime();
void printLoad();
void printLoadCal();
void readValueSerial(char *data, byte size);
float readFloatSerial();
long readIntegerSerial();
void comma();

float newCalWeight;

uint8_t cliBusy, validQuery = false;


void processSerial() 
{
  if (Serial.available() && !validQuery) 
    query = Serial.read();
    
  switch (query) 
  {
    case '?':
      // help
      cliBusy = true;
      
      Serial.print("Motor Analyzer v0.2 CLI\n");
      Serial.print("Lower case letters print data, capitals used to change settings\n");
      Serial.print("1 - Primary test - see \"Primary Test CLI\"\n");
      Serial.print("2 - Response test - see \"Response Test CLI\"\n");
      Serial.print("3 - Hover test - see \"Hover Test CLI\"\n");
      Serial.print("a - print current load cell data             | A - open Load Cell CLI\n");
      Serial.print("b - print current battery data               | B - open Battery CLI\n");
      Serial.print("c - calibrate ESC - REMOVE PROPS FIRST!!!    | C - open Motor Setup CLI\n");
      Serial.print("d -                                          | D - Motor Test CLI\n");
      /*Serial.print("e -                                          | E - \n");
      Serial.print("f -                                          | F - \n");
      Serial.print("g -                                          | G - \n");
      Serial.print("h -                                          | H - \n");*/
      Serial.print("i - re-initialize EEPROM (restore defaults)  | I - \n");
      /*Serial.print("j -                                          | J - \n");
      Serial.print("k -                                          | K - \n");*/
      Serial.print("l - print timing loop deltas (ms)            | L - \n");
      Serial.print("\nPress space bar for more or enter a command....\n");
      
      while (!Serial.available());
      query = Serial.read();
      if (query != ' ') {
        validQuery = true;
        cliBusy = false;
        return; 
      }
      
      Serial.print("\n");
      /*Serial.print("m -                                          | M - \n");
      Serial.print("n -                                          | N - \n");
      Serial.print("o -                                          | O - \n");*/
      Serial.print("p - pulse motor three times                  | P - \n");
      /*Serial.print("q -                                          | Q - \n");
      Serial.print("r -                                          | R - \n");
      Serial.print("s -                                          | S - \n");*/
      Serial.print("t - print last test start and end times      | T - \n");
      /*Serial.print("u -                                          | U - \n");
      Serial.print("v -                                          | V - \n");*/
      Serial.print("w -                                          | W - Write all values to EEPROM\n");
      Serial.print("x - stop EVERYTHING                          | X - \n");
      //Serial.print("y -                                          | Y - \n");
      Serial.print("z - zero out or tare load cell               | Z - \n");

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
      // print load cell values
      printLoad();
      //query = 'x';
      break;
    
    case 'b':
      // print current battery levels, total current usage, and current used during last test
      printBatteryData();
      validQuery = false;
      break;
    
    case 'c':
      // calibrate ESC
      mode = CALIBRATE_ESC;
      validQuery = false;
      break;
    
    case 'f':
      // print pwm freq
      Serial.print("PWM freq (Hz): ");
      Serial.println(pwmFreq);
      query = 'x';
      validQuery = false;
      break;
      
    case 'g':
      // print delta time G_dt
      Serial.print("main loop timing (us): ");
      Serial.println(mainG_dt); 
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
      
    case 'p':
      // pulse motor
      pulseMotor(3);
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
      startTare();
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
    
    case 'C':
      // Motor Setup CLI
      stopMotor();
      motorSetupCLI();
      query = 'x';
      validQuery = false;
      break;
      
    case 'L':
      //change load cell calibration
      mode = '0';
      
      newCalWeight = readFloatSerial();
      
      // catch if we already have this calibration
      // or if we're obviously replacing low or high cal
      if (newCalWeight <= cal[AWEIGHT]) 
      {
        newCalNum = AREAD;
        Serial.println("Replacing low calibration");
      } 
      else if (newCalWeight >= cal[BWEIGHT]) 
      {
        newCalNum = BREAD;
        Serial.println("Replacing high calibration");
      }
      else if ((newCalWeight > cal[AWEIGHT]) && (newCalWeight < cal[BWEIGHT]))
      {
        // new calibration weight is in between previous readings
        // find which it's closer to, replace that one
        int tempDeltaCal0 = abs(newCalWeight - cal[AWEIGHT]);
        int tempDeltaCal1 = abs(newCalWeight - cal[BWEIGHT]);
        if (tempDeltaCal0 > tempDeltaCal1) {
          newCalNum = BREAD;
          Serial.println("new cal closer to high cal, replacing high cal");
        } 
        else 
        {
          newCalNum = AREAD;
          Serial.println("new cal closer to low cal, replacing low cal");
        }
      } /*else { // timed out - user didn't enter a number
        Serial.println("To calibrate the load cell, two measurements are taken with known weights.");
        Serial.println("Place a known weight on the scale and use the \"L\" command to initiate the calibration");
        Serial.println("Command \"L\" usage example: \"L50.3\" - 50.3 means the object weighs 50.3.");
        Serial.println("Unit of weight does not matter to this program - if you enter in grams, output load will be in grams.");
        newCalNum = NOCAL;
        break;
      }*/
      
      cal[newCalNum + 2] = newCalWeight;
      Serial.print("Weight");
      Serial.print(newCalNum);
      Serial.print(" = ");
      Serial.println(newCalWeight);
      rawLoadRead = true;
      digitalWrite(LEDPIN, LOW);
      query = '+';
      validQuery = false;
      break;
      
    case 'P':
      // change pwm freq
      /*int temppwmFreq;
      temppwmFreq = readFloatSerial();
      Serial.print("Old PWM Freq: ");
      Serial.print(pwmFreq);
      changePWMfreq(temppwmFreq);
      Serial.print(", new PWM Freq: ");
      Serial.println(pwmFreq);*/
      query = 'x';
      validQuery = false;
      break;
      
    case 'W':
      // write all values to EEPROM
      Serial.println("writing values to EEPROM . . .");
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
  Serial.print(load);                                  // load in g
  Serial.println(",");
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
  Serial.print(cal[AREAD]);
  Serial.print(", aWeight = ");
  Serial.print(cal[AWEIGHT]);
  Serial.print(" g, bReading = ");
  Serial.print(cal[BREAD]);
  Serial.print(", bWeight = ");
  Serial.print(cal[BWEIGHT]);
  Serial.print(" g, gain: ");
  Serial.println(calGain);
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

void comma() {
  Serial.print(',');
}




