/*
 * Measuring load cell
 * 
 * Default calibration is with INA125, RG = 1ohm
 * Load Cell is from 600g JSV Dually (says 1000g on the cell...)
 * weights calibrated with: 99.5g and 344.9g
 * (measured two random objects with several digital scales)
 *
 * AREAD | BREAD | AWEIGHT | BWEIGHT
 *    68 |   162 |   99.5g |  344.9g
 *
 * to do: reconfigure gain of INA125 to correspond to max
 *        input level of microprocessor (PCB)
 */

#define LOADPIN A2 // port PC2

unsigned long rawLoad   = 0;
uint8_t samples         = 0;
int16_t load            = 0;
int16_t loadAvg         = 0;
boolean newLoadAvgAvail = false;

uint16_t tareSum = 0;
int16_t  tare = 0;
uint8_t tareCounter = 0;
unsigned long tareStartFrameCounter = 0;
boolean tareLoad = true;

uint16_t calSum = 0;
uint8_t calCounter = 0;
float calGain = 0;
int16_t loadNoGain = 0;
boolean rawLoadRead = false;

void calculateLoadGain();

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void initializeLoadCell() 
{
  //DDRC |= B0000100;        // enable PC2 - TODO: Fix this and battery monitor initialization
  pinMode(LOADPIN, INPUT);
  calculateLoadGain();
}


void averageLoadCell() 
{
  loadAvg = rawLoad / samples;
  samples = 0;
  rawLoad = 0;
  loadNoGain = loadAvg - tare;
  load = loadNoGain * calGain;
  newLoadAvgAvail = true;
}

void readLoadCell() 
{
  rawLoad += (long)analogRead(LOADPIN);
  samples++;
  
  if (frameCounter % TASK_25HZ == 0)
    averageLoadCell();
}

void calculateLoadGain() 
{
  calGain = (cal[BWEIGHT] - cal[AWEIGHT])/(cal[BREAD] - cal[AREAD]);
}

/* calibrateLoadCell() 
 * 
 * sum up readings over time and average them
 * store the new average and report the findings
 */
void calibrateLoadCell() 
{
  if (newCalNum != NOCAL) {
    if (newLoadAvgAvail) {
      calSum += loadAvg;
      calCounter++;
      newLoadAvgAvail = false;
    }
  
    if (calCounter >= 20) {
      cal[newCalNum] = calSum / calCounter;
      calSum = 0.0;
      calCounter = 0;
      rawLoadRead = false;
      mode = '0';
      query = 'x';
      Serial.print("weight: ");
      Serial.print(cal[newCalNum + 2]);
      Serial.print("g, raw reading: ");
      Serial.println(cal[newCalNum]);
      digitalWrite(LEDPIN, HIGH);
      calculateLoadGain();
      newCalNum = NOCAL;
      tare = 0;
    }
  }
}

void startTare() 
{
  tareLoad = true;
  tareCounter = 0;
  tareSum = 0;
  digitalWrite(LEDPIN, LOW);
  Serial.print("Beginning tare, current tare = ");
  Serial.println(tare);
  Serial.println("new tare measurements");
  
}

void tareLoadCell() 
{
  if (tareLoad) 
  {
    if (newLoadAvgAvail) 
    {
      tareSum += loadAvg;
      tareCounter++;
      newLoadAvgAvail = false;
      if ((tareCounter % 5) == 0) 
        Serial.println(tareSum / tareCounter);
    }
    
    if (tareCounter >= 20) {
      tare = tareSum / tareCounter;
      digitalWrite(LEDPIN, HIGH);
      Serial.print("new tare = ");
      Serial.println(tare);
      
      tareLoad = false;
      tareSum = 0;
      tareCounter = 0;
    }
  }
}


/* processLoadCell()
 *
 * called at 100Hz
 */
void processLoadCell() 
{  
  readLoadCell();
  tareLoadCell();
  calibrateLoadCell();
}
