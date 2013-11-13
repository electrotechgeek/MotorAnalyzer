/*
 * BatteryMonitor condensed for one battery
 */

struct BatteryData {
  byte  vPin;          // A/D pin for voltage sensor
  byte  cells;         // Number of Cells (used for alarm/warning voltage)
  int8_t vScale,vBias;  // voltage polynom V = vbias + AnalogIn(vpin)*vscale
  int16_t voltage;       // Current battery voltage (in 10mV:s)
  int16_t previousVoltage;
  int16_t minVoltage;    // Minimum voltage since reset
  byte  cPin;             // A/D pin for current sensor (255 = BM_NOPIN <=> no sensor)
  int8_t cScale,cBias;     // current polynom C = cbias + AnalogIn(cpin)*cscale
  int16_t current;          // Current battery current (in 10mA:s)
  int16_t previousCurrent;
  int16_t maxCurrent;       // Maximum current since reset
  int16_t  usedCapacity;     // Capacity used since reset (in uAh)
  int16_t  testStartUsedCapacity;
};

// global battery data and alarms
extern struct BatteryData batteryData;       // BatteryMonitor config, !! MUST BE DEFINED BY MAIN SKETCH !!
extern int8_t            batteryAlarm;      // battery in alarm state
extern int8_t            batteryWarning;    // battery in warning state

// Helper macro to make battery definitions cleaner
#define DEFINE_BATTERY(CELLS,VPIN,VSCALE,VBIAS,CPIN,CSCALE,CBIAS) {(VPIN),(CELLS),(short)((VSCALE)*100.0),(short)((VBIAS)*100.0),0,0,(CPIN),(short)((CSCALE)*10.0),(short)((CBIAS)*10.0),0,0,0}

// Function declarations
int8_t batteryIsAlarm();
int8_t batteryAlarmCount = 0;
int8_t batteryIsWarning();
int8_t batteryWarningCount = 0;
void resetBattery(byte batteryNo);
void initializeBatteryMonitor(byte numberOfMonitoredBatteries, float alarmVoltage);
void setBatteryCellVoltageThreshold(float alarmVoltage);
void measureBattery(int8_t deltaTime);

#define BM_WARNING_RATIO 1.1

byte    numberOfBatteries = 0; 
int8_t     batteryCellCount;
int8_t batteryAlarm      = false;
int8_t batteryWarning    = false;
byte    buzzerState       = 0;
float batteryMonitorAlarmVoltage = 10.0;
int16_t batteryAlarmCellVoltage   = 333; // 9.9V on 3S
int16_t batteryWarningCellVoltage = 366; // 11.0V on 3S
int8_t battSamples = 0;
//float aref;

/*void setBatteryCellVoltageThreshold (float alarmVoltage) {
  
  batteryAlarmCellVoltage   = alarmVoltage*100.0;
  batteryWarningCellVoltage = alarmVoltage*BM_WARNING_RATIO*100.0;
}*/

void resetBattery() {
  batteryData.voltage         = 1200;
  batteryData.previousVoltage = 1200;
  batteryData.minVoltage      = 9900;
  batteryData.current         = 0;
  batteryData.maxCurrent      = 0;
  batteryData.usedCapacity    = 0;
}


void batteryGetCellCount() {
  /*if (batteryData.cells) {
    return batteryData.cells;
  }
  else*/ if (batteryData.voltage < 500) {
    batteryData.cells = 1;
  }
  else if (batteryData.voltage < 860) {
    batteryData.cells = 2;
  }
  else if (batteryData.voltage < 1300) {
    batteryData.cells = 3;
  } 
  else {
    batteryData.cells = 4;
  }
}

// TO DO: Fix oversampling here and in load cell

void initializeBatteryMonitor(float alarmVoltage) {
  // set ADC prescaler to 64, this ups samples per second to ~16KHz
  //ADCSRA &= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)); // first clear bits
  //ADCSRA |= ((1<<ADPS2)|(1<<ADPS1));            // set prescaler to 64, see datasheet pg
  
  //setBatteryCellVoltageThreshold(alarmVoltage);
  resetBattery();
  measureBattery(0);
  batteryGetCellCount();
}

int8_t batteryIsAlarm() {
  if (batteryData.voltage < batteryData.cells * batteryAlarmCellVoltage) {
    return true;
  }
  return false;
}

int8_t batteryIsWarning() {
  if (batteryData.voltage < batteryData.cells * batteryWarningCellVoltage) {
    return true;
  }
  return false;
}

void measureBattery(int8_t deltaTime) 
{
  batteryAlarm = false;
  batteryWarning = false;
  
  uint8_t tempVoltage = (long)analogRead(batteryData.vPin) * batteryData.vScale / (1L << ADC_NUMBER_OF_BITS) + batteryData.vBias;
  if ((abs(tempVoltage - batteryData.previousVoltage) < 100) || (deltaTime == 0)) { // stored in 10mV, 100*10mV = 1V change, probably a bad read
    batteryData.voltage = tempVoltage;
  }
  batteryData.previousVoltage = tempVoltage;
  //batteryData.voltage = (long)analogRead(batteryData.vPin);
  if (batteryData.voltage < batteryData.minVoltage) {
    batteryData.minVoltage = batteryData.voltage;
  }
  
  int8_t tempCurrent = (long)analogRead(batteryData.cPin) * batteryData.cScale * 10 / (1L << ADC_NUMBER_OF_BITS) + batteryData.cBias * 10;
  if (abs(tempCurrent - batteryData.previousCurrent) < 100) { // stored in 10mA, 100*10mA = 1A change, probably a bad read
    batteryData.current = tempCurrent;
  }
  batteryData.previousCurrent = tempCurrent;
  if (batteryData.current > batteryData.maxCurrent) {
    batteryData.maxCurrent = batteryData.current;
  }
  
  // current in 10mA , time in ms -> usedCapacity in uAh  // i.e. / 360 <=> * ( 91 / 32768 )
  batteryData.usedCapacity += (int16_t)batteryData.current * (int16_t)deltaTime * 91 / 32768;
  
  
  if (batteryIsAlarm()) {
    batteryAlarmCount++;
    if (batteryAlarmCount >= 200) {
      batteryAlarm = true;
    }
  } else if (batteryAlarmCount > 0) {
    batteryAlarmCount--;
  }
  if (batteryIsWarning()) {
    batteryWarningCount++;
    if (batteryWarningCount >= 500) {
      batteryWarning = true;
    } 
  } else if (batteryWarningCount > 0) {
    batteryWarningCount--;
  }
}


