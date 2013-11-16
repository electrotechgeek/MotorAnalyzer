/*
    Motor Analyzer
    written by Chip Wood
    Project started March 2013
    
    https://www.github.com/imchipwood/MotorAnalyzer/
    
    !!! SAFETY WARNING !!!
      This program is capable of making a motor go full throttle! If you're
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
             

 */
 
#define SOFTWARE_VERSION 0.2
 

#include <stdlib.h>
#include <math.h>
#include "Arduino.h"
#include "pins_arduino.h"
#include <EEPROM.h>

#include "globals.h"

// battery monitoring setup setup
#include "battery.h"

// load cell
#include "loadCell.h"

// control of motor via ESC
#include "motor.h"

// different data collection modes
#include "modes.h"

// EEPROM
#include "eeprom.h"

// Serial Communication
#include "CLI.h"

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
  
  //Serial.print("\n");
  Serial.print("MotorAnalyzer v");
  Serial.print(swver);
  Serial.print(" by wooden\n");
  
  startLoadTare();
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



