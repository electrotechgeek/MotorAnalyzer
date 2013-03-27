/*
    Motor Analyzer
    written by Chip Wood
    March 27, 2013
    
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
             that can easily be configured to work with different ranges of voltages anc current.
             The defaults for the first iteration are 18V and 40A. Exceeding these values
             will damage the microcontroller.             
 */
 

#include <stdlib.h>
#include <math.h>
#include "Arduino.h"
#include "pins_arduino.h"
#include <EEPROM.h>

#define SOFTWARE_VERSION 0.1
#define BAUD 115200
#define ADC_NUMBER_OF_BITS 10
#define LEDPIN 13

// timing
#define TASK_100HZ 1
#define TASK_50HZ 2
#define TASK_25HZ 4
//#define TASK_10HZ 10
//#define TASK_1HZ 100
#define THROTTLE_ADJUST_TASK_SPEED TASK_50HZ

float G_dt = 0.002;
float mainG_dt = 0.002;
unsigned long frameCounter = 0;
unsigned long previousTime = 0;
unsigned long currentTime = 0;
unsigned long deltaTime = 0;
//unsigned long oneHZpreviousTime = 0;
//unsigned long tenHZpreviousTime = 0;
unsigned long twentyfiveHZpreviousTime = 0;
unsigned long fiftyHZpreviousTime = 0;
unsigned long hundredHZpreviousTime = 0;
unsigned long testStartTime = 0;
unsigned long testEndTime = 0;
boolean firstIteration = false;
boolean testRunning = false;
void startTest();
void stopTest();

char queryType = 'x';

unsigned int throttle = 1000;

void writeEEPROM();
void readEEPROM();
float nvrReadFloat(int address); // defined in DataStorage.h
void nvrWriteFloat(float value, int address); // defined in DataStorage.h
long nvrReadLong(int address); // defined in DataStorage.h
void nvrWriteLong(long value, int address); // defined in DataStorage.h

// AQ BatteryMonitor setup
#include "BatteryMonitor.h"
// Usage: #define BattCustomConfig DEFINE_BATTERY(#cells, vpin, vscale, vbias, cpin, cscale, cbias)
// breadboarded version w/90A AttoPilot using 5V/16MHz Arduino Pro Mini
//#define BattCustomConfig DEFINE_BATTERY(3,A0,78.5,0,A1,136.363636,0) // voltage on a0, current on a1
// v0.1 first prototype from DorkBot:
#define BattCustomConfig DEFINE_BATTERY(3,A0,18.666,0,A1,40.26,0) // voltage on a0, current on a1
struct BatteryData batteryData = BattCustomConfig;

// load cell
char mode = '0';
float cal[4];
int newCalNum = 0;
#define AREAD 0
#define BREAD 1
#define AWEIGHT 2
#define BWEIGHT 3
#define NOCAL -1
#include "LoadCell.h"

// control of motor
#define MINCOMMAND 1000
#define MAXCOMMAND 2000
void slowMotorRamp();
#include "MotorsPWMTimer.h"


// serial transmission
#include "SerialComm.h"

// different data collection modes
#include "Modes.h"

// EEPROM
#include "DataStorage.h"



///////////////////////////////////////////////////////////////
//////////////////////// Loop Functions ///////////////////////
///////////////////////////////////////////////////////////////
void process100() {
  G_dt = (currentTime - hundredHZpreviousTime) / 1000000.0;
  hundredHZpreviousTime = currentTime;
  
  processLoadCell();
  processMotor();
}

void process50() {
  G_dt = (currentTime - fiftyHZpreviousTime) / 1000000.0;
  fiftyHZpreviousTime = currentTime;
}

void process25() {
  G_dt = (currentTime - twentyfiveHZpreviousTime) / 1000000.0;
  twentyfiveHZpreviousTime = currentTime;
  
  processSerial(); // actually prints at 25Hz, need to try to optimize code to get data at 50Hz
                   // once done, check if data at 50Hz actually improves calculations/graphing
}

/*void process10() {
  G_dt = (currentTime - tenHZpreviousTime) / 1000000.0;
  tenHZpreviousTime = currentTime;
}

void process1() {
  G_dt = (currentTime - oneHZpreviousTime) / 1000000.0;
  oneHZpreviousTime = currentTime;
}*/


///////////////////////////////////////////////////////////////
///////////////////////////// SETUP ///////////////////////////
///////////////////////////////////////////////////////////////
void setup() {
  readEEPROM();
  Serial.begin(115200);
  float swver = readFloat(SOFTWARE_VERSION_ADR);
  if (swver != SOFTWARE_VERSION) {
    initEEPROM();
    writeEEPROM();
  }
  
  initializeBatteryMonitor(batteryMonitorAlarmVoltage);
  initializeMotor();
  tareLoad = true;
  initializeLoadCell();
  
  //Serial.println();
  Serial.print("MotorAnalyzer v");
  Serial.print(swver);
  Serial.println(" by wooden");
}

///////////////////////////////////////////////////////////////
////////////////////////// MAIN EXECUTION /////////////////////
///////////////////////////////////////////////////////////////

void loop() {
  currentTime = micros();
  deltaTime = currentTime - previousTime;
  G_dt = (deltaTime) / 1000000.0;
  mainG_dt = G_dt;
  
  measureBattery(G_dt*1000.0);
  
  if (deltaTime >= 10000) {
    frameCounter++;
    process100();
    
    if (frameCounter % TASK_50HZ == 0 ) {
      process50();
    }
    if (frameCounter % TASK_25HZ == 0 ) {
      process25();
    }
    // no functions in process10 or 1 currently, don't bother running
    /*if (frameCounter % TASK_10HZ == 0 ) {
      process10();
    }
    if (frameCounter % TASK_1HZ == 0 ) {
      process1();
    }*/
    previousTime = currentTime;
  }
  
  if (frameCounter >= 10000) {
    frameCounter = 0;
  }
}



