/* Global variables
 *
 */

#define BAUD 115200
#define ADC_NUMBER_OF_BITS 10
#define LEDPIN 13

// main loop timing
#define TASK_100HZ 1
#define TASK_50HZ 2
#define TASK_25HZ 4
//#define TASK_10HZ 10
//#define TASK_1HZ 100

// global timing variables
float mainG_dt, G_dt, G_dt_100, G_dt_50, G_dt_25, G_dt_10, G_dt_5, G_dt_1 = 0.002;
int16_t previousMainTime, frameCounter, previousTime, currentTime, deltaTime = 0;
int16_t twentyfiveHZpreviousTime, fiftyHZpreviousTime, hundredHZpreviousTime = 0;
int16_t testStartTime, testEndTime = 0;

/////////////// SERIAL COMMUNICATION VARIABLES /////////////// 
void sendData(), printBatteryData(), printTestStart(), printTestTime(), printLoad(), printLoadCal();

uint8_t cliBusy, validQuery = false;
char query = 'x';

// function prototypes
float readFloatSerial();
long readIntegerSerial();






/////////////// BATTERY SETUP VARIABLES ///////////////
int8_t VPin, CPin = 0;
float VScale, VBias, CScale, CBias = 0;

// function prototypes
void initializeBatteryMonitor(float alarmVoltage);
void measureBattery(int8_t deltaTime);
int8_t batteryIsAlarm(), batteryIsWarning();






/////////////// MODE VARIABLES ///////////////
// global test execution variables and function prototypes
char mode = '0';
int8_t testRunning, firstIteration = false;

// motor based modes
#define CALIBRATE_ESC 'c'
#define PRIMARY_TEST 'p'
#define RESPONSE_TEST 'r'
#define HOVER_TEST 'h'
#define STOP 'X'           // shut down all activity

// other modes
#define TARE_LOAD 'L'       // taring load cell
#define IDLE '0'

// primary test variables
int8_t primaryUpTime, primaryDownTime, primaryMaxThrottleTime;

// function prototypes
void startTest();
void stopTest();







/////////////// MOTOR VARIABLES ///////////////
// predefined min/max throttle levels
#define MINCOMMAND 1000
#define MAXCOMMAND 2000

#define PWM_PRESCALER 8
int16_t pwmFreq;
float pwmCounterPeriod;

// signal sent to ESC - defined as period in uS
int16_t throttle = 1000;
int16_t minArmedThrottle;

// function prototypes
void writeMotor(int command);
void primaryTest();
void responseTest();
void hoverTest();
