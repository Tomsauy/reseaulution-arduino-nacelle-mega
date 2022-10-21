//
// Created by Thomas Meunier on 21/10/2022.
//

#ifndef RESEAULUTION_ARDUINO_NACELLE_MEGA_GIMBAL_H
#define RESEAULUTION_ARDUINO_NACELLE_MEGA_GIMBAL_H

void sendPosAng();
void storm32Move(int valX, int valY);
void storm32SetAngles();
void storm32Recenter();
void storm32PanMode(int panMode);
void storm32StandBy(int standBy);
void storm32ReadInfo();
uint16_t receiveSerialUint16();

#endif //RESEAULUTION_ARDUINO_NACELLE_MEGA_GIMBAL_H
