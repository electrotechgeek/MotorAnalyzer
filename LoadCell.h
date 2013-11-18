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

// analog pins start at 14, so A0 = 14, A1 = 15, etc.

int8_t loadPin = 2;

#define AREAD 0
#define AWEIGHT 1 
#define BREAD 2
#define BWEIGHT 3
#define NOCAL 999


float loadCal[4];
int16_t newLoadCalNum = NOCAL;
float newLoadCalWeight;

uint16_t rawLoad, load, loadAvg = 0;
uint8_t  loadSamples         = 0;
int8_t   newLoadAvgAvail = false;

uint16_t loadTare, loadTareSum, loadTareStartFrameCounter = 0;
uint8_t  loadTareCounter = 0;

uint16_t loadCalSum = 0;
uint8_t  loadCalCounter = 0;
float    loadCalGain = 0;
int16_t  loadNoGain = 0;
int8_t   rawLoadRead = false;

void calculateLoadGain();

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////




void averageLoadCell() 
{
  loadAvg = rawLoad / loadSamples;
  loadNoGain = loadAvg - loadTare;
  load = loadNoGain * loadCalGain;
  
  loadSamples = 0;
  rawLoad = 0;
  newLoadAvgAvail = true;
}

void readLoadCell() 
{
  rawLoad += (int16_t)analogRead(loadPin+14);
  loadSamples++;
  
  //if (frameCounter % TASK_25HZ == 0)
  //  averageLoadCell();
}

void calculateLoadGain() 
{
  loadCalGain = (loadCal[BWEIGHT] - loadCal[AWEIGHT])/(loadCal[BREAD] - loadCal[AREAD]);
}

/* calibrateLoadCell() 
 * 
 * sum up readings over time and average them
 * store the new average and report the findings
 */
void calibrateLoadCell() 
{
  if (newLoadCalNum != NOCAL) 
  {
    if (newLoadAvgAvail) 
    {
      loadCalSum += loadAvg;
      loadCalCounter++;
      newLoadAvgAvail = false;
    }
  
    if (loadCalCounter >= 20) 
    {
      loadCal[newLoadCalNum] = loadCalSum / loadCalCounter;
      loadCalSum = 0.0;
      loadCalCounter = 0;
      rawLoadRead = false;
      mode = '0';
      query = 'x';
      Serial.print("weight: ");
      Serial.print(loadCal[newLoadCalNum + 2]);
      Serial.print("g, raw reading: ");
      Serial.println(loadCal[newLoadCalNum]);
      digitalWrite(LEDPIN, HIGH);
      calculateLoadGain();
      newLoadCalNum = NOCAL;
      loadTare = 0;
    }
  }
}

void startLoadTare() 
{
  firstIteration = false;
  loadTareCounter = 0;
  loadTareSum = 0;
  digitalWrite(LEDPIN, LOW);
  Serial.print("Beginning tare, current tare = ");
  Serial.println(loadTare);
  Serial.println("new tare measurements:");
  
}

/* Tare/Zero out load cell
 * methodology:
 *     - User commands tare through serial, this sets the mode to TARE_LOAD and firstIteration to TRUE
 *       - CLI goes back to default waiting mode
 *     - TARE_LOAD  checks for first iteration, if so, calls startTareLoad() seen above
 *       - startTareLoad() prints some preliminary info, sets tare counter and sum to 0
 *       - once startTareLoad() has been called, first iteration is set to false
 */

void tareLoadCell() 
{
  if (newLoadAvgAvail) 
  {
    loadTareSum += loadAvg;
    loadTareCounter++;
    newLoadAvgAvail = false;
    if ((loadTareCounter % 5) == 0) 
      Serial.print("tare sample: ");
      Serial.println(loadTareSum / loadTareCounter);
  }
  
  if (loadTareCounter >= 20) 
  {
    loadTare = loadTareSum / loadTareCounter;
    digitalWrite(LEDPIN, HIGH);
    Serial.print("new tare = ");
    Serial.println(loadTare);
    
    loadTareSum = 0;
    loadTareCounter = 0;
    mode = STOP;
  }
}

void initializeLoadCell() 
{
  //DDRC |= B0000100;        // enable PC2 - TODO: Fix this and battery monitor initialization
  pinMode(loadPin+14, INPUT);
  calculateLoadGain();
}

/* processLoadCell()
 *
 * called at 100Hz
 */
void processLoadCell() 
{  
  readLoadCell();
  calibrateLoadCell();
}
