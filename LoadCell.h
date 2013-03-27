/*
 *  Measuring load cell
 *  Using load cell from JSV Dually - has 1000g capacity.
 *
 */

#define LOADPIN A2 // port PC2

unsigned int rawLoad = 0;;
unsigned int samples = 0;
float load = 0;



/*float aReading = 29;  // 0, raw analog reading
float aLoad = 0;     // 1, weight of object a in g
float bReading = 127; // 2, raw analog reading
float bLoad = 268;   // 3, weight of object b in g*/

float tareLoads[20] = {0};
float tare = 0;
unsigned int tareCounter = 0;
unsigned long tareStartFrameCounter = 0;
boolean tareLoad = true;

float calLoads[20] = {0};
// calibration stored as aReading, aWeight, bReading, bWeight
//float cal[4];// = {68, 99.5, 162, 344.9};
unsigned int calCounter = 0;
boolean calibrateLoad = false;
boolean calibrateLoad2 = false;
boolean rawLoadRead = false;

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

void reorderCalibration() {
  if (cal[0] < cal[2]) {
    float tempCal = cal[2];
    cal[2] = cal[0];
    cal[0] = tempCal;
    tempCal = cal[3];
    cal[3] = cal[1];
    cal[1] = tempCal;
  }
}

void initializeLoadCell() {
  //DDRC |= B0000100;        // enable PC2
  pinMode(LOADPIN, INPUT);
}


void averageLoadCell() {
  if (!rawLoadRead) {
    load = ((cal[3] - cal[1])/(cal[2] - cal[0])) * ((float)rawLoad / samples - cal[0]) + cal[1] - tare;
  }
  else {
    load = (float)rawLoad / samples;
  }
  //load = rawLoad / samples;
  samples = 0;
  rawLoad = 0;
}

void readLoadCell() {
  rawLoad += analogRead(LOADPIN);
  samples++;
  
  if (samples >= 25) {
    averageLoadCell();
    
    if (tareLoad) {
      if (tareCounter == 0) {
        digitalWrite(LEDPIN, LOW);
        Serial.print("Beginning tare, tare = ");
        Serial.print(tare);
        Serial.print(". . .");
      }
      tareLoads[tareCounter] = load;
      tareCounter++;
      
      if (tareCounter >= 20) 
      {
        float tareLoadSum = 0;
        for (int i = 0; i < 20; i++) {
          tareLoadSum += tareLoads[i];
          tareLoads[i] = 0;
        }
        tare = tareLoadSum / 20;
        digitalWrite(LEDPIN, HIGH);
        Serial.print(" done, tare = ");
        Serial.println(tare);
        
        tareLoad = false;
        tareLoadSum = 0.0;
        tareCounter = 0;
        
      } // if (tareCounter >= 20)
    } // if (tareLoad)
  } // if (frameCounter ...)
}


void calibrateLoadCell() {
  if (samples == 0) {
    if (calibrateLoad) {
      calLoads[calCounter] = load;
      calCounter++;
    
      if (calCounter >= 20) {
        float calSum = 0;
        for (calCounter = 0; calCounter >= 19; calCounter++) {
          calSum += calCounter;
          calLoads[calCounter] = 0;
        }
        cal[0] = calSum / 20; // cal[0] corresponds to aReading
        calibrateLoad = false;
        rawLoadRead = false;
        opMode = '0';
        queryType = 'x';
        Serial.print("weight0: ");
        Serial.print(cal[1]);
        Serial.print("g, raw reading: ");
        Serial.println(cal[0]);
        digitalWrite(LEDPIN, HIGH);
        reorderCalibration();
        writeEEPROM();
      }
    }
    if (calibrateLoad2) {
      calLoads[calCounter] = load;
      calCounter++;
      
      if (calCounter >= 20) {
        float calSum = 0;
        for (calCounter = 0; calCounter >= 19; calCounter++) {
          calSum += calCounter;
          calLoads[calCounter] = 0;
        }
        cal[2] = calSum / 20; // cal[2] corresponds to bReading
        calibrateLoad2 = false;
        rawLoadRead = false;
        tareLoad = true;
        opMode = '0';
        queryType = 'x';
        Serial.print("weight1: ");
        Serial.print(cal[3]);
        Serial.print("g, raw reading: ");
        Serial.println(cal[2]);
        digitalWrite(LEDPIN, HIGH);
        reorderCalibration();
        writeEEPROM();
      }
    }
  }
}





/* processLoadCell()
 *
 * called at 100Hz
 */
void processLoadCell() {  
  readLoadCell();
  calibrateLoadCell();
}
