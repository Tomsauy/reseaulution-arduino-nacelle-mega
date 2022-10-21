//
// Created by Thomas Meunier on 21/10/2022.
//
#include <Arduino.h>
#include <varGlobal.h>
#include <gimbal.h>
#include <main.h>
#include <STorM32_lib.h>

void sendPosAng()
{
    int nbValue = 0;
    float yprTemp[3] = {0, 0, 0};
    int nbAcq = 20;
    while (nbValue < nbAcq)
    {
        // Wait for MPU interrupt or extra packet(s) available
        while (!mpuInterrupt && fifoCount < packetSize) {
            // Do nothing...
        }

        // Reset interrupt flag and get INT_STATUS byte
        mpuInterrupt = false;
        mpuIntStatus = mpu.getIntStatus();

        // Get current FIFO count
        fifoCount = mpu.getFIFOCount();

        // Check for overflow (this should never happen unless our code is too slow)
        if ((mpuIntStatus & 0x10) || fifoCount == 1024)
        {
            // reset so we can continue cleanly
            mpu.resetFIFO();
            Serial.println(F("FIFO overflow!"));

            // Otherwise, check for DMP data ready interrupt (this should happen frequently)
        }
        else if (mpuIntStatus & 0x02)
        {
            // Wait for correct available data length, should be a VERY short wait
            while (fifoCount < packetSize) {
                fifoCount = mpu.getFIFOCount();
            }

            // Read a packet from FIFO
            mpu.getFIFOBytes(fifoBuffer, packetSize);

            // Track FIFO count here in case there is > 1 packet available
            // (this lets us immediately read more without waiting for an interrupt)
            fifoCount -= packetSize;

            // Convert Euler angles in degrees
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

            // Print angle values in degrees.
            //      Serial.print(ypr[YAW] * (180 / M_PI));
            //      Serial.print("\t");
            //      Serial.print(ypr[PITCH] * (180 / M_PI));
            //      Serial.print("\t");
            //      Serial.println(ypr[ROLL] * (180 / M_PI));
            nbValue++;

            yprTemp[YAW] += ypr[YAW];
            yprTemp[PITCH] += ypr[PITCH];
            yprTemp[ROLL] += ypr[ROLL];

            if (nbValue >= nbAcq)
            {
                yprTemp[YAW] /= nbValue;
                yprTemp[PITCH] /= nbValue;
                yprTemp[ROLL] /= nbValue;

                //String payload = "POSANG=" + String(yprTemp[PITCH] * (180 / M_PI)) + "/" + String(yprTemp[ROLL] * (180 / M_PI));


                String frame = "CUWV," + idTramePosAngLong + "," + intToHex((yprTemp[PITCH] * (180 / M_PI)) * 100, 4) + intToHex((yprTemp[ROLL] * (180 / M_PI) * 100), 4);
                serialBLE.println(frame);
                Serial.println("Pitch " + String(yprTemp[PITCH] * (180 / M_PI)));
                Serial.println("Roll " + String(yprTemp[ROLL] * (180 / M_PI)));
                Serial.println(frame);
            }
        }
    }
}

//deplacement relatif
void storm32Move(int valX, int valY) {
    //serialLog.println(String("valX : ")+valX+String(" valY : ")+valY+String("gPosX : ")+gPosX+String(" gPosY : ")+gPosY);
    if (valX >= gValMax) {
        valX = gValMax;
    }
    if (valX <= gValMin) {
        valX = gValMin;
    }
    if (valY >= gValMax) {
        valY = gValMax;
    }
    if (valY <= gValMin) {
        valY = gValMin;
    }
    gPosX = gPosX + valX;
    gPosY = gPosY + valY;
    storm32SetAngles();
}

//deplacement position fixe
void storm32SetAngles() {
    if (gPosX >= gYawMax) {
        gPosX = gYawMax;
    }
    if (gPosX <= gYawMin) {
        gPosX = gYawMin;
    }
    if (gPosY >= gPitchMax) {
        gPosY = gPitchMax;
    }
    if (gPosY <= gPitchMin) {
        gPosY = gPitchMin;
    }
    tSTorM32CmdSetAngles msgSetAngles;
    msgSetAngles.yaw = -gPosX;
    msgSetAngles.pitch = -gPosY;
    msgSetAngles.roll = 0;
    storm32_finalize_CmdSetAngles(&msgSetAngles);
    Serial3.write((unsigned char *)&msgSetAngles, sizeof(msgSetAngles));
    delay (5);
    while (Serial3.available() > 0) {
        serialGimbalReturn = Serial3.read();
    }
    //    if (serialGimbalReturn) {
    //        Serial.println(String("GIMBAL ERROR_POS ")+serialGimbalReturn);
    //    }
}

//recentrage de la camera
void storm32Recenter() {
    gPosX = 0;
    gPosY = 0;
    tSTorM32CmdSetPitchRollYaw msgRecenter;
    msgRecenter.pitch = 0;
    msgRecenter.roll = 0;
    msgRecenter.yaw = 0;
    storm32_finalize_CmdSetPitchRollYaw(&msgRecenter);
    Serial3.write((unsigned char *)&msgRecenter, sizeof(msgRecenter));
    delay(10);
    while (Serial3.available() > 0) {
        serialGimbalReturn = Serial3.read();
    }


    //    if (serialGimbalReturn) {
    //        Serial.println(String("GIMBAL ERROR_RECENTER ")+serialGimbalReturn);
    //    }
}

//gestion du mode Pan
void storm32PanMode(int panMode) {
    tSTorM32CmdActivePanModeSetting msgPanMode;
    if (panMode == 1) {
        msgPanMode.data = 1;
    } else if (panMode == 0 ) {
        msgPanMode.data = 2;
    }
    storm32_finalize_CmdActivePanModeSetting(&msgPanMode);
    Serial3.write((uint8_t *)&msgPanMode, sizeof(msgPanMode));
    delay (50);
    while (Serial3.available() > 0) {
        serialGimbalReturn = Serial3.read();
    }
    //    if (serialGimbalReturn) {
    //       Serial.println(String("GIMBAL ERROR_PAN_MODE ")+serialGimbalReturn);
    //    }
}

//Stanby de la camera
void storm32StandBy(int standBy) {
    if (standBy) {
        gPosX = 0;
        gPosY = -90;
        storm32SetAngles();
        delay(5000);
    } else {
        gPosX = 0;
        gPosY = 0;
    }
    tSTorM32CmdStandBySetting msgStandBy;
    msgStandBy.data = standBy;
    storm32_finalize_CmdStandBySetting(&msgStandBy);
    Serial3.write((uint8_t *)&msgStandBy, sizeof(msgStandBy));

    delay(50);
    while (Serial3.available() > 0) {
        serialGimbalReturn = Serial3.read();
    }
    //    if (serialGimbalReturn) {
    //       Serial.println(String("GIMBAL ERROR_STANDBY ")+serialGimbalReturn);
    //    }
}

//lecture infos carte Storm32
void storm32ReadInfo() {

    uint16_t uint16Received;
    uint16_t uint16State;
    uint16_t uint16Status;
    uint16_t uint16Status2;
    uint16_t uint16Status3;
    uint16_t uint16Voltage;
    uint16_t uint16Performance;
    uint16_t uint16Errors;

    String strState = "";
    String strStatus = "";
    String strPanMode = "";
    String strError = "";

    while (Serial3.available() > 0) {
        Serial3.read();
    }
    Serial3.write("s");
    delay (100);

    //state
    uint16State = receiveSerialUint16();
    //status //39024 -> batterie déconnectée //39032 -> voltage is low //39016 -> batterie connectée
    uint16Status = receiveSerialUint16();
    //status2 //32768 en standBy et 32824 en normal
    uint16Status2 = receiveSerialUint16();
    //status3
    uint16Status3 = receiveSerialUint16();
    //voltage
    uint16Voltage = receiveSerialUint16();
    //performance //46845
    uint16Performance = receiveSerialUint16();
    //errors //65391
    uint16Errors = receiveSerialUint16();

    //Serial.print(uint16State+String("-")+uint16Status+String("-")+uint16Status2+String("-")+uint16Status3+String("-")+uint16Voltage+String("-")+uint16Performance+String("-")+uint16Errors+String("\n"));

    // traduction de state
    //  STORM32STATE_STARTUP_MOTORS               = 0
    //  STORM32STATE_STARTUP_SETTLE               = 1
    //  STORM32STATE_STARTUP_CALIBRATE            = 2
    //  STORM32STATE_STARTUP_LEVEL                = 3
    //  STORM32STATE_STARTUP_MOTORDIRDETECT,
    //  STORM32STATE_STARTUP_RELEVEL              = 5
    //  STORM32STATE_NORMAL                       = 6
    //  STORM32STATE_STARTUP_FASTLEVEL,
    //  STORM32STATE_STANDBY                      = 99
    switch (uint16State) {
        case STORM32STATE_STARTUP_MOTORS:
            strState = "startup_motors";
            break;
        case STORM32STATE_STARTUP_SETTLE:
            strState = "startup_settle";
            break;
        case STORM32STATE_STARTUP_CALIBRATE:
            strState = "startup_calibrate";
            break;
        case STORM32STATE_STARTUP_LEVEL:
            strState = "startup_level";
            break;
        case STORM32STATE_STARTUP_RELEVEL:
            strState = "startup_relevel";
            break;
        case STORM32STATE_NORMAL:
            if (uint16Status2 == 32824) {
                strState = "activated";
            } else if (uint16Status2 == 32768) {
                strState = "off";
            } else
            {
                strState = "unknown_status2";
            }
            break;
        case STORM32STATE_STANDBY:
            strState = "standby";
            break;
        default:
            strState = "unknown";
            break;
    }


    // traduction de status -> etat batterie   39024 -> batterie déconnectée //39032 -> voltage is low //39016 -> batterie connectée
    switch (uint16Status) {
        case 39024:
            strStatus = "disconnected";
            break;
        case 39032:
            strStatus = "low voltage";
            break;
        case 39016:
            strStatus = "connected";
            break;
        default:
            strStatus = "unknown";
            break;
    }

    // retour pan mode
    if (gPanMode) {
        strPanMode = "pan";
    } else {
        strPanMode = "hold";
    }


    // traduction des erreurs
    //0 = SERIALRCCMD_ACK_OK
    //1 = SERIALRCCMD_ACK_ERR_FAIL
    //2 = SERIALRCCMD_ACK_ERR_ACCESS_DENIED
    //3 = SERIALRCCMD_ACK_ERR_NOT_SUPPORTED
    //150 = SERIALRCCMD_ACK_ERR_TIMEOUT
    //151 = SERIALRCCMD_ACK_ERR_CRC
    //152 = SERIALRCCMD_ACK_ERR_PAYLOADLEN
    switch (uint16Errors) {
        case 0:
            strError = "ACK_OK";
            break;
        case 1:
            strError = "ACK_ERR_FAIL";
            break;
        case 2:
            strError = "ACK_ERR_ACCESS_DENIED";
            break;
        case 3:
            strError = "ACK_ERR_NOT_SUPPORTED";
            break;
        case 150:
            strError = "ACK_ERR_TIMEOUT";
            break;
        case 151:
            strError = "ACK_ERR_CRC";
            break;
        case 152:
            strError = "ACK_ERR_PAYLOADLEN";
            break;
        default:
            strError = "unknown";
            break;
    }

    // Serial.print(String("{ \"recipient\":\"WEB-CLIENT\",\n\"command\":\"GIMBAL_INFO\",\n\"sender\":\"ROBOT\",\n\"options\":{\"\nstate\":\"") + strState + String("\",\"\npanMode\":\"") + strPanMode + String("\",\"\nbattery\":\"") + strStatus + String("\",\"\nvoltage\":\"") + (uint16Voltage / 1000) + String(".") + (uint16Voltage % 1000) + String("\n\",\"X\":\"") + gPosX + String("\",\"Y\":\"") + gPosY + String("\" }}\n"));
    Serial.println("----");
    Serial.println("state: " + String(strState) + "\npanMode: " + String(strPanMode) + "\nbattery: " + String(strStatus) + "\nX: " + String(gPosX) + " Y: " + String(gPosY));

    while (Serial3.available() > 0) {
        Serial3.read();
    }

    String frame = "CUWV," + idTrameGimbalUpLong + "," + intToHex(uint16State, 2) + intToHex(gPanMode, 2) + intToHex(uint16Status, 4) + intToHex(gPosX, 4) + intToHex(gPosY, 4);
    serialBLE.println(frame);
    Serial.println(frame);
}

//lecture d'un uint16
uint16_t receiveSerialUint16() {

    // Read low byte into rxnum
    uint16_t rxnum = Serial3.read();

    // Read high byte into rxnum
    rxnum += Serial3.read() << 8;
    return rxnum;
}
