/*
 * MotorAnalyzer Serial communications
 * Commands: 
 * x - stop everything
 * 1 - begin slowMotorRamp
 * 2 - 
 * b - print current battery data
 * t - print last test start and end times (if no test run yet, returns 0,0)
 * B - set new amount of cells ( B1 sets cells to 1, B2 to 2, etc.)
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
int calNum = 0;

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
      opMode = '1';
      if (!testRunning) {
        firstIteration = true;
      }
      sendData();
      break;
    
    case '2':
      // begin response data collection
      opMode = '2';
      firstIteration = true;
      break;
    
    case 'b':
      // print current battery levels, total current usage, and current used during last test
      printBatteryData();
      break;
    
    case 'c':
      // calibrate ESC
      opMode = 'c';
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
      tareLoad = true;
      queryType = 'x';
      break;
      
    
      
    case 'L':
      //change load cell calibration
      calNum = (int)readFloatSerial();
      if (calNum == 0) {
        cal[1] = readFloatSerial(); // aLoad
        Serial.print("weight0 = ");
        Serial.println(cal[1]);
        rawLoadRead = true;
        calibrateLoad = true;
        digitalWrite(LEDPIN, LOW);
      } else if (calNum == 1) {
        cal[3] = readFloatSerial(); // bLoad
        Serial.print("weight1 = ");
        Serial.println(cal[3]);
        rawLoadRead = true;
        calibrateLoad2 = true;
        digitalWrite(LEDPIN, LOW);
      }
      tare = 0;
      queryType = '0';
      break;
    
    case 'x':
      // stop errythang
      if (opMode == '1') {
        stopTest();
      }
      writeMotor(MINCOMMAND);
      opMode = '0';
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
  Serial.print(throttle);
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
  //Serial.println(analogRead(LOADPIN));
  Serial.println(load);
}

void printLoadCal() {
  Serial.print("aReading = ");
  Serial.print(cal[0]);
  Serial.print(", aWeight = ");
  Serial.print(cal[1]);
  Serial.print(" g, bReading = ");
  Serial.print(cal[2]);
  Serial.print(", bWeight = ");
  Serial.print(cal[3]);
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




