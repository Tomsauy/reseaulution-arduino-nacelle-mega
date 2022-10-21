#include <Arduino.h>
#include <avr/wdt.h>
#include <bao.h>

/**********************************/
boolean attente_reponse(Stream &serialPort, char strwait[], char strinfo[], unsigned int timeout, unsigned int attente)
{
  unsigned long actual_time = millis();
  while ((unsigned long)(millis() - actual_time) <= timeout)
  {
#ifdef DEBUG
    Serial.print(F("waiting "));
    Serial.println(strinfo);
#endif

    if (serialPort.find(strwait))
    {
#ifdef DEBUG
      Serial.print(strwait);
      Serial.println(F(" found !"));
#endif
      delay(attente);
      return true;
    }
  }
#ifdef DEBUG
  Serial.print(F("Timeout : "));
  Serial.println(strwait);
#endif
  flushSerial(serialPort);
  return false; //timeout dÃ©passÃ©
}
/**********************************/

/**********************************/
void flushSerial(Stream &serialPort)
{
  while (serialPort.available() > 0)
  {
    char t = serialPort.read();
  }
}
/**********************************/

/**********************************/
void debugSerial(Stream &serialPort)
{

  while (1)
  {
    while (serialPort.available() > 0)
    {
      Serial.write(serialPort.read());
    }

    while (Serial.available() > 0)
    {
      serialPort.write(Serial.read());
    }
  }
}
/**********************************/

/**********************************/
void reboot() {
  Serial.println("Rebooting...");
  delay(500);
  wdt_disable();
  wdt_enable(WDTO_15MS);
  while (1) {}
}
/**********************************/
