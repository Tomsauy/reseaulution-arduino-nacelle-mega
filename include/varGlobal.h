//
// Created by Thomas Meunier on 21/10/2022.
//
#include <Arduino.h>
#include "MPU6050_6Axis_MotionApps20.h"

#ifndef RESEAULUTION_ARDUINO_NACELLE_MEGA_VARGLOBAL_H
#define RESEAULUTION_ARDUINO_NACELLE_MEGA_VARGLOBAL_H

// ---------------------------------------------------------------------------
#define YAW 0
#define PITCH 1
#define ROLL 2
// --------------------- MPU650 variables ------------------------------------

extern MPU6050 mpu;
// MPU control/status vars
extern bool dmpReady;  // set true if DMP init was successful
extern uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
extern uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
extern uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
extern uint16_t fifoCount;     // count of all bytes currently in FIFO
extern uint8_t fifoBuffer[64]; // FIFO storage buffer

// Orientation/motion vars
extern Quaternion q;        // [w, x, y, z]         quaternion container
extern VectorFloat gravity; // [x, y, z]            gravity vector
extern float ypr[3];        // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

extern volatile bool mpuInterrupt; // Indicates whether MPU interrupt pin has gone high
// ---------------------------------------------------------------------------

//BLE
extern unsigned long blePassCode;
extern Stream &serialBLE;
extern unsigned int wkeBLE;
extern boolean bleConnected;
extern boolean bleBonded;
extern boolean ultrasonsActive;
extern unsigned long lastSendUs;
extern unsigned long timingSendingUs;
extern String blBuffer;

extern bool wLight;
extern bool wSound;
extern bool alimGimbal;
extern bool alimJetson;
extern bool streamUS;
extern bool streamFDC;
extern bool bmsDataRequest;
extern unsigned int prevPosLed;


//Trame enables
extern String idTrameEnableShort;
extern String idTrameEnalbeLong;
extern unsigned int empEnLight;
extern unsigned int empEnSound;
extern unsigned int empEnAlimGimbal;
extern unsigned int empEnAlimJetson;
extern unsigned int empEnUsStream;
extern unsigned int empEnFdcStream;
extern unsigned int empSendBmsDatas;
extern unsigned int empSendPosAng;
extern unsigned int empPosLed;

extern unsigned int maskPosLed;

//Trame gimbal
extern String idTrameGimbalShort;
extern String idTrameGimbalLong;
extern unsigned int empStdbyMode;
extern unsigned int empPanMode;
extern unsigned int empRecenter;
extern unsigned int empInfo;
extern unsigned int empTypeCommande;
extern unsigned int empAngleX;
extern unsigned int empAngleY;
extern unsigned int empVitX;
extern unsigned int empVitY;

extern unsigned int maskAngleX;
extern unsigned int maskAngleY;
extern unsigned int maskVitX;
extern unsigned int maskVitY;


//Trames nacelle vers base
//Trame BMS
extern String idTrameBmsLong;

//Trame FDC
extern String idTrameFdcLong;

//Trame US
extern String idTrameUsLong;

//Trame Gimbal
extern String idTrameGimbalUpLong;

//Trame PosAng
extern String idTramePosAngLong;

//Trame config gimbal
extern String idTrameGimbalConfigShort;
extern String idTrameGimbalConfigLong;
extern unsigned int empYawMin;
extern unsigned int empYawMax;
extern unsigned int empPitchMin;
extern unsigned int empPitchMax;

extern unsigned int maskYawMin;
extern unsigned int maskYawMax;
extern unsigned int maskPitchMin;
extern unsigned int maskPitchMax;

// SRF08 - Ultrasons
extern byte highByte; // Stores high byte from ranging
extern byte lowByte;  // Stored low byte from ranging
#define CMD (byte)0x00
#define RANGEBYTE 0x02 // Byte for start of ranging data
extern int seuilDetection;

//Alim Jetson
extern unsigned int pinEnAlimjetson;

//Alim Gimbal
extern unsigned int pinEnAlimGimbal;

//FDC
extern unsigned int pinFdcMilieu;
extern unsigned int pinFdcGauche;
extern unsigned int pinFdcDroite;
extern unsigned long lastSendFdc;
extern unsigned long timingSendingFdc;

//BMS - Chargeur
extern unsigned int temp;
extern unsigned int pinEnCharge;

//LED automatique
extern boolean ledAuto; // Si true = pilotage led selon position gimbal

// --------------------- STorM32 (Gimbal variables) ------------------------------------
extern int gPosX;
extern int gPosY;
extern int gYawMin; // Fred : conversin de const int � int
extern int gYawMax; // Fred : conversion de const int � int
extern int gPitchMin; // Fred : conversin de const int � int
extern int gPitchMax; // Fred : conversin de const int � int
extern int gValMin;
extern int gValMax;
extern int  gValX;
extern int  gValY;
// const int gSpeedMax = 9; // Fred : a priori pas utilis�
// const int gSpeedMin = -9; // Fred : a priori pas utilis�
extern const long gimbalLongDelayEventTime;
extern unsigned long gimbalLongDelayPreviousTime;
extern const long gimbalShortDelayEventTime;
extern unsigned long gimbalShortDelayPreviousTime;
extern int gPanMode;
extern int gStandByRequest;
extern int gInfoRequest;
extern int gStandBy;
extern int gCommandMode;
extern int gRecenterRequest;
extern int serialGimbalReturn;
extern String gimbalInfo;

#endif //RESEAULUTION_ARDUINO_NACELLE_MEGA_VARGLOBAL_H
