#include <Arduino.h>
#include <MPU6050_6Axis_MotionApps20.h>
#include <bluetooth.h>
#include <bao.h>
#include <ultrason.h>
#include <fdc.h>
#include <bms.h>
#include <gimbal.h>
#include <led.h>

#define DEBUG

// ---------------------------------------------------------------------------
#define YAW 0
#define PITCH 1
#define ROLL 2
// --------------------- MPU650 variables ------------------------------------
MPU6050 mpu;
// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// Orientation/motion vars
Quaternion q;        // [w, x, y, z]         quaternion container
VectorFloat gravity; // [x, y, z]            gravity vector
float ypr[3];        // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

volatile bool mpuInterrupt = false; // Indicates whether MPU interrupt pin has gone high
// ---------------------------------------------------------------------------

//BLE
unsigned long blePassCode = 38243845;
Stream &serialBLE = Serial2;
unsigned int wkeBLE = 6;
boolean bleConnected = false;
boolean bleBonded = false;
boolean ultrasonsActive = true;
unsigned long lastSendUs = 0;
unsigned long timingSendingUs = 1000;
String blBuffer = "";

bool wLight = false;
bool wSound = false;
bool alimGimbal = false;
bool alimJetson = false;
bool streamUS = false;
bool streamFDC = false;
bool bmsDataRequest = false;
unsigned int prevPosLed = 9;


//Trame enables
String idTrameEnableShort = "0013";
String idTrameEnalbeLong = "91b1b2a5d4534a08b153c0902d2e9e88";
unsigned int empEnLight = 0;
unsigned int empEnSound = 1;
unsigned int empEnAlimGimbal = 2;
unsigned int empEnAlimJetson = 3;
unsigned int empEnUsStream = 4;
unsigned int empEnFdcStream = 5;
unsigned int empSendBmsDatas = 6;
unsigned int empSendPosAng = 7;
unsigned int empPosLed = 8;

unsigned int maskPosLed = 0b1111 << empPosLed;

//Trame gimbal
String idTrameGimbalShort = "0016";
String idTrameGimbalLong = "f0b7f97fe92b440b82f1599c2f2e9fa2";
unsigned int empStdbyMode = 0;
unsigned int empPanMode = 1;
unsigned int empRecenter = 2;
unsigned int empInfo = 3;
unsigned int empTypeCommande = 4;
unsigned int empAngleX = 5;
unsigned int empAngleY = 21;
unsigned int empVitX = 37;
unsigned int empVitY = 45;

unsigned int maskAngleX = 0xffff << empAngleX;
unsigned int maskAngleY = 0xffff << empAngleY;
unsigned int maskVitX = 0xff << empVitX;
unsigned int maskVitY = 0xff << empVitY;


//Trames nacelle vers base
//Trame BMS
String idTrameBmsLong = "57b44761a46a4c4396432e9c1d8c3478";

//Trame FDC
String idTrameFdcLong = "ed5873c822d4495a86be61479435c806";

//Trame US
String idTrameUsLong = "9f27001b94bf4685ac5124f65aa3b5c3";

//Trame Gimbal
String idTrameGimbalUpLong = "e7908e861ba740088bee704cd7052c04";

//Trame PosAng
String idTramePosAngLong = "e50aef2aef8211eb9a030242ac130003";

//Trame config gimbal
String idTrameGimbalConfigShort = ""; //Fred : a completer
String idTrameGimbalConfigLong = "57cd9aed39b24b1494652d5dfcbfe987";
unsigned int empYawMin = 0;
unsigned int empYawMax = 16;
unsigned int empPitchMin = 32;
unsigned int empPitchMax = 48;

unsigned int maskYawMin = 0xffff << empYawMin;
unsigned int maskYawMax = 0xffff << empYawMax;
unsigned int maskPitchMin = 0xffff << empPitchMin;
unsigned int maskPitchMax = 0xffff << empPitchMax;

// SRF08 - Ultrasons
byte highByte = 0x00; // Stores high byte from ranging
byte lowByte = 0x00;  // Stored low byte from ranging
#define CMD (byte)0x00
#define RANGEBYTE 0x02 // Byte for start of ranging data
int seuilDetection = 100;

//Alim Jetson
unsigned int pinEnAlimjetson = 12;

//Alim Gimbal
unsigned int pinEnAlimGimbal = 30;

//FDC
unsigned int pinFdcMilieu = 46;
unsigned int pinFdcGauche = 45;
unsigned int pinFdcDroite = 44;
unsigned long lastSendFdc = 0;
unsigned long timingSendingFdc = 150;

//BMS - Chargeur
unsigned int temp = 0;
unsigned int pinEnCharge = 11;

//LED automatique
boolean ledAuto = false; // Si true = pilotage led selon position gimbal

// --------------------- STorM32 (Gimbal variables) ------------------------------------
int gPosX = 0;
int gPosY = 0;
int gYawMin = -180; // Fred : conversin de const int � int
int gYawMax = 180; // Fred : conversion de const int � int
int gPitchMin = -90; // Fred : conversin de const int � int
int gPitchMax = 45; // Fred : conversin de const int � int
int gValMin = -9;
int gValMax = 9;
int  gValX = 0;
int  gValY = 0;
// const int gSpeedMax = 9; // Fred : a priori pas utilis�
// const int gSpeedMin = -9; // Fred : a priori pas utilis�
const long gimbalLongDelayEventTime = 2000;
unsigned long gimbalLongDelayPreviousTime = 0;
const long gimbalShortDelayEventTime = 200;
unsigned long gimbalShortDelayPreviousTime = 0;
int gPanMode = 0;
int gStandByRequest = 0;
int gInfoRequest = 0;
int gStandBy = 0;
int gCommandMode = 0;
int gRecenterRequest = 0;
int serialGimbalReturn;
String gimbalInfo = "";

void dmpDataReady()
{
    mpuInterrupt = true;
}

/**********************************/
String intToHex(int _input, int _nbChar = 2)
{
    // %[flags][width][.precision][length]specifier
    // flags : 0  Left-pads the number with zeroes (0) instead of spaces, where padding is specified (see width sub-specifier).
    //width : (number)  Minimum number of characters to be printed. If the value to be printed is shorter than this number, the result is padded with blank spaces. The value is not truncated even if the result is larger.
    //length : l The argument is interpreted as a long int or unsigned long int for integer specifiers (i, d, o, u, x and X), and as a wide character or wide character string for specifiers c and s.
    //specifier : X Unsigned hexadecimal integer (capital letters)
    //sprintf(str_temp, " % 04X", _input);
    //sprintf(str_temp, " % 08lX", (long)lat_gps);
    char tempChar[7] = "";

    switch (_nbChar)
    {
        case 2:
            sprintf(tempChar, "%02X", _input);
            break;

        case 4:
            sprintf(tempChar, "%04X", _input);
            break;
    }
    return String(tempChar);
}
/**********************************/

void setup()
{

    Serial.begin(115200);
    Serial.println("Starting...");
    //Port Serie comm Arduino Pro mini
    Serial1.begin(57600);
    //Port  série comm gimbal
    Serial3.begin(57600);

    //Alim Jetson
    pinMode(pinEnAlimjetson, OUTPUT);
    digitalWrite(pinEnAlimjetson, LOW);

    //Alim Gimbal
    pinMode(pinEnAlimGimbal, OUTPUT);
    digitalWrite(pinEnAlimGimbal, LOW);

    //Chargeur
    pinMode(pinEnCharge, OUTPUT);
    digitalWrite(pinEnCharge, LOW); //Activation du chargeur

    //FDC
    pinMode(pinFdcMilieu, INPUT);
    pinMode(pinFdcGauche, INPUT);
    pinMode(pinFdcDroite, INPUT);


    //BLE
    if (!initBLE())
    {
        Serial.println("Erreur init BLE - Rebooting");
        reboot();
    }


    //MPU6050
    Wire.begin();
    TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)

    Serial.println(F("Initializing I2C devices..."));
    mpu.initialize();

    // Verify connection
    Serial.println(F("Testing device connections..."));
    Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

    // Load and configure the DMP
    Serial.println(F("Initializing DMP..."));
    devStatus = mpu.dmpInitialize();

    // MPU calibration: set YOUR offsets here.
    mpu.setXAccelOffset(-1616);
    mpu.setYAccelOffset(-3606);
    mpu.setZAccelOffset(1468);
    mpu.setXGyroOffset(37);
    mpu.setYGyroOffset(-34);
    mpu.setZGyroOffset(-21);

    // Returns 0 if it worked
    if (devStatus == 0)
    {
        // Turn on the DMP, now that it's ready
        Serial.println(F("Enabling DMP..."));
        mpu.setDMPEnabled(true);

        // Enable Arduino interrupt detection
        Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0 : #pin2)..."));
        attachInterrupt(0, dmpDataReady, RISING);
        mpuIntStatus = mpu.getIntStatus();

        // Set our DMP Ready flag so the main loop() function knows it's okay to use it
        Serial.println(F("DMP ready! Waiting for first interrupt..."));
        dmpReady = true;

        // Get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    }
    else
    {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
        reboot();
    }

    // If programming failed, don't try to do anything
    if (!dmpReady)
    {
        reboot();
    }
}

void loop()
{
    while (!bleBonded)
    {
        checkBLE(serialBLE);
    }

    checkBLE(serialBLE);


    //Fonction utrasons
    checkUltrasons();

    //Streaming FDC
    streamFdc();


    if (bmsDataRequest)
    {
        bmsDataRequest = false;
        sendBms();
    }

    // Gestion de la Gimbal
    if ( millis() - gimbalLongDelayPreviousTime >= gimbalLongDelayEventTime) {
        //gestion du mode Pan
        Serial.println("Value pan mode = " + String(gPanMode));
        storm32PanMode(gPanMode);

        //gestion du standBy
        if (gStandByRequest) {
            Serial.println("exec Standby gimball = " + String(gStandBy));
            storm32StandBy(gStandBy);
            Serial.println("Standby gimball executed");
            gStandByRequest = 0;
        }

        //gestion du recenter
        if (gRecenterRequest) {
            storm32Recenter();
            gRecenterRequest = 0;
        }

        //Lecture des infos du Storm32
        if (gInfoRequest) {
            storm32ReadInfo();
            gInfoRequest = 0;
        }  // storm32ReadInfo();

        gimbalLongDelayPreviousTime = millis();
    }

    if ( millis() - gimbalShortDelayPreviousTime >= gimbalShortDelayEventTime) {
        if (gCommandMode == 1) {
            //Mode position relative
            storm32Move(gValX, gValY);
        }
        else {
            // Mode position fixe
            storm32SetAngles();
        }
        if (ledAuto)
        {
            majPosLed();
        }
        gimbalShortDelayPreviousTime = millis();
    }
}
