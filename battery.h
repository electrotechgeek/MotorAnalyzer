/*
 * BatteryMonitor condensed for one battery
 */



struct BatteryData 
{
  byte   cells;         // Number of Cells (used for alarm/warning voltage)
  int16_t voltage;       // Current battery voltage (in 10mV:s)
  int16_t previousVoltage;
  int16_t minVoltage;    // Minimum voltage since reset
  
  int16_t current;          // Current battery current (in 10mA:s)
  int16_t previousCurrent;
  int16_t maxCurrent;       // Maximum current since reset
  int16_t usedCapacity;     // Capacity used since reset (in uAh)
  int16_t testStartUsedCapacity;
} batteryData;


// global battery data and alarms
extern struct BatteryData batteryData;       // BatteryMonitor config, !! MUST BE DEFINED BY MAIN SKETCH !!
extern int8_t            batteryAlarm;      // battery in alarm state
extern int8_t            batteryWarning;    // battery in warning state


// Function declarations
int8_t batteryAlarmCount, batteryWarningCount = 0;
void resetBattery(byte batteryNo);
void setBatteryCellVoltageThreshold(float alarmVoltage);

#define BM_WARNING_RATIO 1.1

byte    numberOfBatteries, buzzerState = 0; 
int8_t  batteryCellCount,battSamples = 0;
int8_t  batteryAlarm, batteryWarning = false;
float   batteryMonitorAlarmVoltage = 10.0;
int16_t batteryAlarmCellVoltage   = 333; // 9.9V on 3S
int16_t batteryWarningCellVoltage = 366; // 11.0V on 3S
//float aref;

/*void setBatteryCellVoltageThreshold (float alarmVoltage) {
  
  batteryAlarmCellVoltage   = alarmVoltage*100.0;
  batteryWarningCellVoltage = alarmVoltage*BM_WARNING_RATIO*100.0;
}*/

void resetBattery() 
{
  batteryData.voltage         = 1200;
  batteryData.previousVoltage = 1200;
  batteryData.minVoltage      = 9900;
  batteryData.current         = 0;
  batteryData.maxCurrent      = 0;
  batteryData.usedCapacity    = 0;
}


void batteryGetCellCount() 
{
  int16_t tempVoltage;
  tempVoltage = batteryData.voltage; 
  
  if (tempVoltage < 500) {
    batteryData.cells = 1;
  }
  else if (tempVoltage < 860) {
    batteryData.cells = 2;
  }
  else if (tempVoltage < 1300) {
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
  pinMode(VPin, INPUT);
  pinMode(CPin, INPUT);
  resetBattery();
  measureBattery(0);
  batteryGetCellCount();
}

int8_t batteryIsAlarm() 
{
  if (batteryData.voltage < batteryData.cells * batteryAlarmCellVoltage) {
    return true;
  }
  return false;
}

int8_t batteryIsWarning() 
{
  if (batteryData.voltage < batteryData.cells * batteryWarningCellVoltage) {
    return true;
  }
  return false;
}

void measureBattery(int8_t deltaTime) 
{
  batteryAlarm = false;
  batteryWarning = false;
  
  uint8_t tempVoltage = (long)analogRead(CPin) * VScale / (1L << ADC_NUMBER_OF_BITS) + VBias;
  if ((abs(tempVoltage - batteryData.previousVoltage) < 100) || (deltaTime == 0))  // stored in 10mV, 100*10mV = 1V change, probably a bad read
    batteryData.voltage = tempVoltage;
  
  batteryData.previousVoltage = tempVoltage;
  //batteryData.voltage = (long)analogRead(VPin);
  if (batteryData.voltage < batteryData.minVoltage) 
    batteryData.minVoltage = batteryData.voltage;
  
  
  int8_t tempCurrent = (long)analogRead(CPin) * CScale * 10 / (1L << ADC_NUMBER_OF_BITS) + CBias * 10;
  if (abs(tempCurrent - batteryData.previousCurrent) < 100)  // stored in 10mA, 100*10mA = 1A change, probably a bad read
    batteryData.current = tempCurrent;
  
  batteryData.previousCurrent = tempCurrent;
  if (batteryData.current > batteryData.maxCurrent) 
    batteryData.maxCurrent = batteryData.current;
  
  
  // current in 10mA , time in ms -> usedCapacity in uAh  // i.e. / 360 <=> * ( 91 / 32768 )
  batteryData.usedCapacity += (int16_t)batteryData.current * (int16_t)deltaTime * 91 / 32768;
  
  
  if (batteryIsAlarm()) 
  {
    batteryAlarmCount++;
    if (batteryAlarmCount >= 200) 
      batteryAlarm = true;
  } 
  else if (batteryAlarmCount > 0) 
    batteryAlarmCount--;
  
  if (batteryIsWarning()) 
  {
    batteryWarningCount++;
    if (batteryWarningCount >= 500) 
      batteryWarning = true;
    
    else if (batteryWarningCount > 0) 
      batteryWarningCount--;
  }
}


