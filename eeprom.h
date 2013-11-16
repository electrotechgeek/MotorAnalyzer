/* 
 * Data Storage
 * 
 * Handles EEPROM value reading/saving
 */
 
 
void writeEEPROM();
void readEEPROM();
void initEEPROM();
float nvrReadFloat(int address); // defined in DataStorage.h
void nvrWriteFloat(float value, int address); // defined in DataStorage.h
long nvrReadLong(int address); // defined in DataStorage.h
void nvrWriteLong(long value, int address); // defined in DataStorage.h




typedef struct 
{ 
  float SOFTWARE_VERSION_ADR;
  float A_READ_ADR;
  float A_WEIGHT_ADR;
  float B_READ_ADR;
  float B_WEIGHT_ADR;
  float V_PIN_ADR;
  float V_SCALE_ADR;
  float V_BIAS_ADR;
  float C_PIN_ADR;
  float C_SCALE_ADR;
  float C_BIAS_ADR;
  float PRI_RAMP_UP_TIME_ADR;
  float PRI_RAMP_DOWN_TIME_ADR;
  float PRI_MAX_THROTTLE_TIME_ADR;
  float PWM_FREQ_ADR;
  float MIN_ARMED_THROTTLE_ADR;
} t_NVR_Data;


void initEEPROM() 
{
  loadCal[AREAD] = 68;
  loadCal[AWEIGHT] = 99.5;
  loadCal[BREAD] = 162;
  loadCal[BWEIGHT] = 344.9;
  VPin = 0;
  VScale = 78.5;
  VBias = 0;
  CPin = 1;
  CScale = 136.363636;
  CBias = 0;
  primaryUpTime = 15;
  primaryDownTime = 15;
  primaryMaxThrottleTime = 5;
  pwmFreq = 400;
}

#define GET_NVR_OFFSET(param) ((int)&(((t_NVR_Data*) 0)->param))
#define readFloat(addr) nvrReadFloat(GET_NVR_OFFSET(addr))
#define writeFloat(value, addr) nvrWriteFloat(value, GET_NVR_OFFSET(addr))
#define readLong(addr) nvrReadLong(GET_NVR_OFFSET(addr))
#define writeLong(value, addr) nvrWriteLong(value, GET_NVR_OFFSET(addr))

void readEEPROM() 
{ 
  loadCal[AREAD]   = readFloat(A_READ_ADR);
  loadCal[AWEIGHT] = readFloat(A_WEIGHT_ADR);
  loadCal[BREAD]   = readFloat(B_READ_ADR);
  loadCal[BWEIGHT] = readFloat(B_WEIGHT_ADR);
  VPin   = readFloat(V_PIN_ADR);
  VScale = readFloat(V_SCALE_ADR);
  VBias  = readFloat(V_BIAS_ADR);
  CPin   = readFloat(C_PIN_ADR);
  CScale = readFloat(C_SCALE_ADR);
  CBias  = readFloat(C_BIAS_ADR);
  primaryUpTime      = readFloat(PRI_RAMP_UP_TIME_ADR);
  primaryDownTime    = readFloat(PRI_RAMP_DOWN_TIME_ADR);
  primaryMaxThrottleTime = readFloat(PRI_MAX_THROTTLE_TIME_ADR);
  pwmFreq = readLong(PWM_FREQ_ADR);
  minArmedThrottle = readLong(MIN_ARMED_THROTTLE_ADR);
}

void writeEEPROM() 
{
  writeFloat(SOFTWARE_VERSION, SOFTWARE_VERSION_ADR);
  writeFloat(loadCal[AREAD],   A_READ_ADR);
  writeFloat(loadCal[AWEIGHT], A_WEIGHT_ADR);
  writeFloat(loadCal[BREAD],   B_READ_ADR);
  writeFloat(loadCal[BWEIGHT], B_WEIGHT_ADR);
  writeFloat(VPin,   V_PIN_ADR);
  writeFloat(VScale, V_SCALE_ADR);
  writeFloat(VBias,  V_BIAS_ADR);
  writeFloat(CPin,   C_PIN_ADR);
  writeFloat(CScale, C_SCALE_ADR);
  writeFloat(CBias,  C_BIAS_ADR);
  writeFloat(primaryUpTime,        PRI_RAMP_UP_TIME_ADR);
  writeFloat(primaryDownTime,      PRI_RAMP_DOWN_TIME_ADR);
  writeFloat(primaryMaxThrottleTime,   PRI_MAX_THROTTLE_TIME_ADR);
  writeLong(pwmFreq, PWM_FREQ_ADR);
  writeLong(minArmedThrottle, MIN_ARMED_THROTTLE_ADR);
}




//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////


float nvrReadFloat(int address) 
{
  union floatStore {
    byte floatByte[4];
    unsigned short floatUShort[2];
    float floatVal;
  } floatOut;

  for (int i = 0; i < 4; i++) 
    floatOut.floatByte[i] = EEPROM.read(address + i);

  return floatOut.floatVal;
}

void nvrWriteFloat(float value, int address) 
{
  union floatStore {
    byte floatByte[4];
    unsigned short floatUShort[2];
    float floatVal;
  } floatIn;

  floatIn.floatVal = value;

  for (int i = 0; i < 4; i++)
    EEPROM.write(address + i, floatIn.floatByte[i]);
}

long nvrReadLong(int address) 
{
  union longStore {
    byte longByte[4];
    unsigned short longUShort[2];
    long longVal;
  } longOut;  

  for (byte i = 0; i < 4; i++) 
    longOut.longByte[i] = EEPROM.read(address + i);
    
  return longOut.longVal;
}

void nvrWriteLong(long value, int address) 
{
  union longStore {
    byte longByte[4];
    unsigned short longUShort[2];
    long longVal;
  } longIn;  

  longIn.longVal = value;
  
  for (int i = 0; i < 4; i++) 
    EEPROM.write(address + i, longIn.longByte[i]);
}

