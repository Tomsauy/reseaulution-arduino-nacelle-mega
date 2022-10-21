#include <Arduino.h>
#include <varGlobal.h>
#include <fdc.h>
#include <main.h>

void streamFdc()
{
  if (streamFDC)
  {
    if (millis() - lastSendFdc > timingSendingFdc)
    {
      lastSendFdc = millis();
      int octet = 0;
      if (digitalRead(pinFdcGauche))
        bitSet(octet, 2);

      if (digitalRead(pinFdcMilieu))
        bitSet(octet, 1);

      if (digitalRead(pinFdcDroite))
        bitSet(octet, 0);

      //TODO Rajouter envoie en BLE
      Serial.print("FDC = ");
      Serial.println(octet, BIN);

      String frame = "CUWV," + idTrameFdcLong + "," + intToHex(octet, 2);
      serialBLE.println(frame);

      Serial.println(frame);
    }
  }
}
