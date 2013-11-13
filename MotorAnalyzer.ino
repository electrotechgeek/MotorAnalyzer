/*
    Motor Analyzer
    written by Chip Wood
    March 28, 2013
    
    !!! SAFETY WARNING !!!
      This program will cause a motor to go from no throttle to full throttle! If you're
      doing what it was designed for, this means props are on and it can lift off or 
      damage anything in the path of the props! Ensure the motor is strapped down well
      and cannot move no matter how hard the motor pulls or pushes!
    
    Purpose: The motor analyzer is designed to collect data on the efficiency and throttle
             characteristics of a given motor and ESC combination. 
             To get an accurate measurement of the performance, it will collect current,
             voltage, thrust, and RPM data and output it to the console via serial for
             easy collection and parsing for analysis.
             
   Software: This software was initially designed to work with an Arduino Pro Mini 5V/16MHz,
             AttoPilot current/voltage sensor, and a load cell amplified by an INA125.
             There is a PCB in the works (first prototypes have been received but not tested)
             that can easily be configured to work with different ranges of voltages and current.
             The defaults for the first iteration are 18V and 40A. Exceeding these values
             will damage the microcontroller.
             
             to do:
             * allow user to change primaryTest length, store in EEPROM
             * fix analog port definitions: increase speed
 */
 

#include <stdlib.h>
#include <math.h>
#include "Arduino.h"
#include "pins_arduino.h"
#include <EEPROM.h>

#define SOFTWARE_VERSION 0.2
#define BAUD 115200
#define ADC_NUMBER_OF_BITS 10
#define LEDPIN 13

// timing
#define TASK_100HZ 1
#define TASK_50HZ 2
#define TASK_25HZ 4
//#define TASK_10HZ 10
//#define TASK_1HZ 100

float mainG_dt, G_dt, G_dt_100, G_dt_50, G_dt_25, G_dt_10, G_dt_5, G_dt_1 = 0.002;
int16_t previousMainTime, frameCounter, previousTime, currentTime, deltaTime = 0;
/*uint_16t frameCounter = 0;
uint_16t previousTime = 0;
uint_16t currentTime = 0;
uint_16t deltaTime = 0;*/
//uint_16t oneHZpreviousTime = 0;
//uint_16t tenHZpreviousTime = 0;
int16_t twentyfiveHZpreviousTime, fiftyHZpreviousTime, hundredHZpreviousTime = 0;
/*int_16t fiftyHZpreviousTime = 0;
int_16t hundredHZpreviousTime = 0;*/
int16_t testStartTime, testEndTime = 0;
//int_16t testEndTime = 0;
int8_t testRunning, firstIteration = false;
//int8_t testRunning = false;
void startTest();
void stopTest();

char query = 'x';

int16_t throttle = 1000;

void writeEEPROM();
void readEEPROM();
void initEEPROM();
float nvrReadFloat(int address); // defined in DataStorage.h
void nvrWriteFloat(float value, int address); // defined in DataStorage.h
long nvrReadLong(int address); // defined in DataStorage.h
void nvrWriteLong(long value, int address); // defined in DataStorage.h

// AQ BatteryMonitor setup
#include "BatteryMonitor.h"
// Usage: #define BattCustomConfig DEFINE_BATTERY(#cells, vpin, vscale, vbias, cpin, cscale, cbias)
// breadboarded version w/90A AttoPilot using 5V/16MHz Arduino Pro Mini
#define BattCustomConfig DEFINE_BATTERY(3,A0,78.5,0,A1,136.363636,0) // voltage on a0, current on a1
// v0.1 first prototype from DorkBot:
//#define BattCustomConfig DEFINE_BATTERY(3,A0,18.666,0,A1,40.26,0) // voltage on a0, current on a1
struct BatteryData batteryData = BattCustomConfig;

// load cell
#define AREAD 0
#define BREAD 1
#define AWEIGHT 2
#define BWEIGHT 3
#define NOCAL 999
char mode = '0';
float cal[4];
int16_t newCalNum = NOCAL;
#include "LoadCell.h"

// control of motor
#define MINCOMMAND 1000
#define MAXCOMMAND 2000
void primaryTest();
#include "MotorsPWMTimer.h"


// different data collection modes
#include "Modes.h"

// serial transmission
//#include "SerialComm.h"
#include "CLI.h"
#include "CLImotor.h"

// EEPROM
#include "DataStorage.h"



///////////////////////////////////////////////////////////////
//////////////////////// Loop Functions ///////////////////////
///////////////////////////////////////////////////////////////
void process100() {
  G_dt = (currentTime - hundredHZpreviousTime) / 1000000.0;
  G_dt_100 = G_dt;
  hundredHZpreviousTime = currentTime;
  
  modeHandler();
  processLoadCell();
  processMotor();
}

void process50() {
  G_dt = (currentTime - fiftyHZpreviousTime) / 1000000.0;
  G_dt_50 = G_dt;
  fiftyHZpreviousTime = currentTime;
}

void process25() {
  G_dt = (currentTime - twentyfiveHZpreviousTime) / 1000000.0;
  G_dt_25 = G_dt;
  twentyfiveHZpreviousTime = currentTime;
  
  processSerial(); // actually prints at 25Hz, need to try to optimize code to get data at 50Hz
                   // once done, check if data at 50Hz actually improves calculations/graphing
}

/*void process10() {
  G_dt = (currentTime - tenHZpreviousTime) / 1000000.0;
  G_dt_10 = G_dt;
  tenHZpreviousTime = currentTime;
}

void process1() {
  G_dt = (currentTime - oneHZpreviousTime) / 1000000.0;
  G_dt_1 = G_dt;
  oneHZpreviousTime = currentTime;
}*/


///////////////////////////////////////////////////////////////
///////////////////////////// SETUP ///////////////////////////
///////////////////////////////////////////////////////////////
void setup() 
{
  readEEPROM();
  Serial.begin(115200);
  float swver = readFloat(SOFTWARE_VERSION_ADR);
  if (swver != SOFTWARE_VERSION) 
  {
    initEEPROM();
    writeEEPROM();
  }
  
  initializeBatteryMonitor(batteryMonitorAlarmVoltage);
  initializeMotor();
  initializeLoadCell();
  
  //Serial.println();
  Serial.print("MotorAnalyzer v");
  Serial.print(swver);
  Serial.println(" by wooden");
  
  startTare();
}

///////////////////////////////////////////////////////////////
////////////////////////// MAIN EXECUTION /////////////////////
///////////////////////////////////////////////////////////////

void loop() {
  currentTime = micros();
  deltaTime = currentTime - previousTime;
  G_dt = (deltaTime) / 1000000.0; 
  mainG_dt = currentTime - previousMainTime;
  
  measureBattery(mainG_dt*1000.0);
  
  if (deltaTime >= 10000) 
  {
    frameCounter++;
    process100();
    
    if (frameCounter % TASK_50HZ == 0 ) 
      process50();
    
    if (frameCounter % TASK_25HZ == 0 ) 
      process25();
    
    // no functions in process10 or 1 currently, don't bother running
    /*if (frameCounter % TASK_10HZ == 0 ) 
      process10();
    
    if (frameCounter % TASK_1HZ == 0 ) 
      process1();*/
    
    previousTime = currentTime;
  }
  previousMainTime = currentTime;
  
  if (frameCounter >= 10000)
    frameCounter = 0;
}



