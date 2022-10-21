#include <Arduino.h>
#include <varGlobal.h>
#include <led.h>

void majPosLed()
{
  int ledByte = 0;
  if (gPosX >= -15 && gPosX < 15)
    ledByte = 0b10000011;
  else if (gPosX >= 15 && gPosX < 30)
    ledByte = 0b00000011;
  else if (gPosX >= 30 && gPosX < 60)
    ledByte = 0b00000111;
  else if (gPosX >= 60 && gPosX < 75)
    ledByte = 0b00000110;
  else if (gPosX >= 75 && gPosX < 105)
    ledByte = 0b00001110;
  else if (gPosX >= 105 && gPosX < 120)
    ledByte = 0b00001100;
  else if (gPosX >= 120 && gPosX < 150)
    ledByte = 0b00011100;
  else if (gPosX >= 150 && gPosX < 165)
    ledByte = 0b00011000;
  else if (gPosX >= 165 || gPosX < -165)
    ledByte = 0b00111000;
  else if (gPosX >= -165 && gPosX < -150)
    ledByte = 0b00110000;
  else if (gPosX >= -150 && gPosX < -120)
    ledByte = 0b01110000;
  else if (gPosX >= -120 && gPosX < -105)
    ledByte = 0b01100000;
  else if (gPosX >= -105 && gPosX < -75)
    ledByte = 0b11100000;
  else if (gPosX >= -75 && gPosX < -60)
    ledByte = 0b11000000;
  else if (gPosX >= -60 && gPosX < -30)
    ledByte = 0b11000001;
  else if (gPosX >= -30 && gPosX < -15)
    ledByte = 0b10000001;

  Serial1.print("controlLed=" + String(ledByte) + "\n");
}
