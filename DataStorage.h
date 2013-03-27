



float nvrReadFloat(int address) {
  union floatStore {
    byte floatByte[4];
    unsigned short floatUShort[2];
    float floatVal;
  } floatOut;

  for (int i = 0; i < 4; i++) {
    floatOut.floatByte[i] = EEPROM.read(address + i);
  }

  return floatOut.floatVal;
}

void nvrWriteFloat(float value, int address) {
  union floatStore {
    byte floatByte[4];
    unsigned short floatUShort[2];
    float floatVal;
  } floatIn;

  floatIn.floatVal = value;

  for (int i = 0; i < 4; i++) {
    EEPROM.write(address + i, floatIn.floatByte[i]);
  }
}

long nvrReadLong(int address) {
  union longStore {
    byte longByte[4];
    unsigned short longUShort[2];
    long longVal;
  } longOut;  

  for (byte i = 0; i < 4; i++) {
    longOut.longByte[i] = EEPROM.read(address + i);
  }
    
  return longOut.longVal;
}

void nvrWriteLong(long value, int address) {
  union longStore {
    byte longByte[4];
    unsigned short longUShort[2];
    long longVal;
  } longIn;  

  longIn.longVal = value;
  
  for (int i = 0; i < 4; i++) {
    EEPROM.write(address + i, longIn.longByte[i]);
  }
}

typedef struct { 
  float SOFTWARE_VERSION_ADR;
  float A_READ_ADR;
  float A_WEIGHT_ADR;
  float B_READ_ADR;
  float B_WEIGHT_ADR;
  //float V_SENSE_ADR;
  //float I_SENSE_ADR;
} t_NVR_Data;


void initEEPROM() {
  cal[0] = 68;
  cal[1] = 99.5;
  cal[2] = 162;
  cal[3] = 344.9;
}

#define GET_NVR_OFFSET(param) ((int)&(((t_NVR_Data*) 0)->param))
#define readFloat(addr) nvrReadFloat(GET_NVR_OFFSET(addr))
#define writeFloat(value, addr) nvrWriteFloat(value, GET_NVR_OFFSET(addr))
#define readLong(addr) nvrReadLong(GET_NVR_OFFSET(addr))
#define writeLong(value, addr) nvrWriteLong(value, GET_NVR_OFFSET(addr))

void readEEPROM() { 
  cal[0] = readFloat(A_READ_ADR);
  cal[1] = readFloat(A_WEIGHT_ADR);
  cal[2] = readFloat(B_READ_ADR);
  cal[3] = readFloat(B_WEIGHT_ADR);
  
}

void writeEEPROM() {
  writeFloat(cal[0], A_READ_ADR);
  writeFloat(cal[1], A_WEIGHT_ADR);
  writeFloat(cal[2], B_READ_ADR);
  writeFloat(cal[3], B_WEIGHT_ADR);
  writeFloat(SOFTWARE_VERSION, SOFTWARE_VERSION_ADR);
}


