/*
 * BatteryMonitor condensed for one battery
 */



struct BatteryData 
{
  byte   cells;         // Number of Cells (used for alarm/warning voltage)
  int16_t voltage;       // Current battery voltage (in 10mV:s)
  int16_t voltageSum;
  int16_t previousVoltage;
  int16_t minVoltage;    // Minimum voltage since reset
  
  int16_t current;          // Current battery current (in 10mA:s)
  int16_t currentSum;
  int16_t previousCurrent;
  int16_t maxCurrent;       // Maximum current since reset
  int16_t usedCapacity;     // Capacity used since reset (in uAh)
  int16_t testStartUsedCapacity;
  
  int16_t samples;
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

void averageBattery(int8_t deltaTime)
{
  
  batteryAlarm = false;
  batteryWarning = false;
  int16_t tempVoltage, tempCurrent;
  
  tempVoltage = (batteryData.voltageSum / batteryData.samples) * VScale / (1L << ADC_NUMBER_OF_BITS) + VBias;
  if (abs(tempVoltage - batteryData.previousVoltage) < 100 || (deltaTime == 0))
    batteryData.voltage = tempVoltage;
  if (tempVoltage < batteryData.minVoltage)
    batteryData.minVoltage = tempVoltage;
    
  tempCurrent = (batteryData.currentSum / batteryData.samples) * CScale / (1L << ADC_NUMBER_OF_BITS) + CBias;
  if (abs(tempCurrent - batteryData.previousCurrent) < 100 || (deltaTime == 0))
    batteryData.current = tempCurrent;
  if (tempVoltage < batteryData.maxCurrent)
    batteryData.maxCurrent = tempCurrent;
    
  batteryData.usedCapacity += (int16_t)batteryData.current * (int16_t)deltaTime * 91 / 32768;  
  
  batteryData.previousVoltage = tempVoltage;
  batteryData.previousCurrent = tempCurrent;
  
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
  
  batteryData.samples = 0;
}



void measureBattery() 
{
  batteryData.voltageSum += (int16_t)analogRead(VPin);  
  batteryData.currentSum += (int16_t)analogRead(CPin);
  batteryData.samples++;
}


// TO DO: Fix oversampling here and in load cell

void initializeBatteryMonitor(float alarmVoltage) {
  // set ADC prescaler to 64, this ups samples per second to ~16KHz
  ADCSRA &= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)); // first clear bits
  ADCSRA |= ((1<<ADPS2)|(1<<ADPS1));            // set prescaler to 64, see datasheet pg
                                                  // shifting 1 into ADCSRA - ADPS1 and 2
  
  //setBatteryCellVoltageThreshold(alarmVoltage);
  //pinMode(VPin, INPUT);
  //pinMode(CPin, INPUT);
  resetBattery();
  measureBattery();
  averageBattery(0);
  batteryGetCellCount();
}



