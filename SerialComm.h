/*
 * MotorAnalyzer Serial communications
 * Commands: 
 * 0 - not used (literally does nothing, used to continue doing current task)
 * 1 - begin slowMotorRamp
 * 2 - begin hover test (50% throttle for 15 secs)
 * b - print current battery data (continuous stream)
 * c - calibrate ESC (requires removing power from ESC and applying after a moment)
 * g - print main time loop delta
 * k - print load cell calibration values
 * l - print load cell values
 * p - pulse motor 3 times
 * t - print last test start and end times (if no test run yet, returns 0,0)
 * z - zero out or TARE load cell
 * L - new Load Cell calibration - usage: Lweight where weight is the weight of the object. ex: L50.3
 * x - stop everything
 */




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

void processSerial() {
  if (Serial.available()) {
    queryType = Serial.read();
  }
  switch (queryType) {
    case '0':
      // continue doing whatever is currently happening
      break;
    
    case '1':
      // begin primary data collection
      mode = '1';
      if (!testRunning) {
        firstIteration = true;
      }
      sendData();
      break;
    
    case '2':
      // begin response data collection
      mode = '2';
      firstIteration = true;
      break;
    
    case 'b':
      // print current battery levels, total current usage, and current used during last test
      printBatteryData();
      break;
    
    case 'c':
      // calibrate ESC
      mode = 'c';
      break;
      
    case 'g':
      // print delta time G_dt
      Serial.print("delta time G_dt: ");
      Serial.println(mainG_dt);
      queryType = 'x';
      break;
      
    case 'k':
      // print load cell calibration
      printLoadCal();
      queryType = 'x';
      break;
      
    case 'l':
      // print load cell values
      printLoad();
      break;
      
    case 'p':
      // pulse motor
      pulseMotor(3);
      queryType = 'x';
      break;
      
    case 't':    // send test start and end time
      printTestTime();
      queryType = 'x';
      break;
      
    case 'z':
      // zero out, or TARE, load cell
      startTare();
      queryType = 'x';
      break;
      
    
      
    case 'L':
      //change load cell calibration
      mode = '0';
      
      newCalWeight = readFloatSerial();
      
      // catch if we already have this calibration
      // or if we're obviously replacing low or high cal
      if (newCalWeight <= cal[AWEIGHT]) {
        newCalNum = AREAD;
        Serial.println("Replacing low calibration");
      } else if (newCalWeight >= cal[BWEIGHT]) {
        newCalNum = BREAD;
        Serial.println("Replacing high calibration");
      }
      else if ((newCalWeight > cal[AWEIGHT]) && (newCalWeight < cal[BWEIGHT])){
        // new calibration weight is in between previous readings
        // find which it's closer to, replace that one
        int tempDeltaCal0 = abs(newCalWeight - cal[AWEIGHT]);
        int tempDeltaCal1 = abs(newCalWeight - cal[BWEIGHT]);
        if (tempDeltaCal0 > tempDeltaCal1) {
          newCalNum = BREAD;
        } else {
          newCalNum = AREAD;
        }
      } else { // timed out - user didn't enter a number
        Serial.println("To calibrate the load cell, two measurements are taken with known weights.");
        Serial.println("Place a known weight on the scale and use the \"L\" command to initiate the calibration");
        Serial.println("Command \"L\" usage example: \"L50.3\" - 50.3 means the object weighs 50.3.");
        Serial.println("Unit of weight does not matter to this program - if you enter in grams, output load will be in grams.");
        newCalNum = NOCAL;
        break;
      }
      
      cal[newCalNum + 2] = newCalWeight;
      Serial.print("Weight");
      Serial.print(newCalNum);
      Serial.print(" = ");
      Serial.println(newCalWeight);
      rawLoadRead = true;
      digitalWrite(LEDPIN, LOW);
      queryType = '0';
      break;
    
    case 'x':
      // stop everything
      if (mode == '1') {
        stopTest();
      }
      mode = '0';
      firstIteration = false;
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


void sendData() {
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

void printBatteryData() {
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

void printTestStart() {
  Serial.println("Beginning test");
  Serial.println("elapsedTime (ms),throttle,V,I (amps),used I (mAh),thrust (g)");
}

void printTestTime() {
  float testDuration = (float)(testEndTime - testStartTime) / 1000000;
  Serial.print("Start: ");
  Serial.print((float)testStartTime / 1000000); // u
  Serial.print(" sec, End: ");
  Serial.print((float)testEndTime / 1000000);
  Serial.print(" sec, Duration: ");
  Serial.print(testDuration);
  Serial.println(" sec");
}

void printLoad() {
  Serial.print("rawLoad = ");
  Serial.print(rawLoad/samples);
  Serial.print(", load = ");
  Serial.println(load);
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
  Serial.println(" g");
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




