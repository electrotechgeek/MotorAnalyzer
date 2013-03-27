/*
 * Measuring load cell
 * 
 * Default calibration is with INA125, RG = 1ohm
 * Load Cell is from 600g JSV Dually (says 1000g on the cell...)
 * weights calibrated with: 99.5g and 344.9g
 * (measured two random objects with several digital scales)
 *
 * aReading, bReading, aWeight, bWeight
 *    68   ,   162   ,  99.5g ,  344.9g
 *
 * to do: reconfigure gain of INA125 to correspond to max
 *        input level of microprocessor
 */

#define LOADPIN A2 // port PC2

unsigned int rawLoad = 0;;
unsigned int samples = 0;
float load = 0;
boolean newLoadAvgAvail = false;

float tareSum = 0;
float tare = 0;
unsigned int tareCounter = 0;
unsigned long tareStartFrameCounter = 0;
boolean tareLoad = true;

unsigned int calSum = 0;
unsigned int calCounter = 0;
boolean rawLoadRead = false;

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void initializeLoadCell() {
  //DDRC |= B0000100;        // enable PC2 - TODO: Fix this and battery monitor initialization
  pinMode(LOADPIN, INPUT);
}


void averageLoadCell() {
  if (!rawLoadRead) {
    load = ((cal[BWEIGHT] - cal[AWEIGHT])/(cal[BREAD] - cal[AREAD])) * ((float)rawLoad / samples - cal[AREAD]) + cal[AWEIGHT] - tare;
  }
  else {
    load = (float)rawLoad / samples;
  }
  newLoadAvgAvail = true;
}

void readLoadCell() {
  rawLoad += analogRead(LOADPIN);
  samples++;
  
  if (samples >= 20) {
    averageLoadCell();
    samples = 0;
    rawLoad = 0;
  }
}

/* calibrateLoadCell() 
 * 
 * sum up readings over time and average them
 * store the new average and report the findings
 */
void calibrateLoadCell() {
  if (newCalNum != NOCAL) {
    if (newLoadAvgAvail) {
      calSum += (int)load;
      calCounter++;
      newLoadAvgAvail = false;
    }
  
    if (calCounter >= 20) {
      cal[newCalNum] = (float)calSum / calCounter;
      calSum = 0.0;
      calCounter = 0;
      rawLoadRead = false;
      mode = '0';
      queryType = 'x';
      Serial.print("weight: ");
      Serial.print(cal[newCalNum + 2]);
      Serial.print("g, raw reading: ");
      Serial.println(cal[newCalNum]);
      digitalWrite(LEDPIN, HIGH);
      writeEEPROM();
      newCalNum = NOCAL;
      tare = 0;
    }
  }
}

void startTare() {
  tareLoad = true;
  tareCounter = 0;
  tareSum = 0;
  digitalWrite(LEDPIN, LOW);
  Serial.print("Beginning tare, current tare = ");
  Serial.print(tare);
  Serial.print(". . .");
}

void tareLoadCell() {
  if (tareLoad) {
    if (newLoadAvgAvail) {
      tareSum += load;
      tareCounter++;
      newLoadAvgAvail = false;
    }
    
    if (tareCounter >= 20) {
      tare = tareSum / tareCounter;
      digitalWrite(LEDPIN, HIGH);
      Serial.print(" done, new tare = ");
      Serial.println(tare);
      
      tareLoad = false;
      tareSum = 0.0;
      tareCounter = 0;
    }
  }
}


/* processLoadCell()
 *
 * called at 100Hz
 */
void processLoadCell() {  
  readLoadCell();
  tareLoadCell();
  calibrateLoadCell();
}
