#include <Arduino.h>
#include <Wire.h>
#include <varGlobal.h>
#include <main.h>
#include <ultrason.h>


int getRange(int _srfAddr)
{ // This function gets a ranging from the SRF08

  int range = 0;

  Wire.beginTransmission(_srfAddr); // Start communticating with SRF08
  Wire.write(CMD);                  // Send Command Byte
  Wire.write(0x51);                 // Send 0x51 to start a ranging
  Wire.endTransmission();

  delay(100); // Wait for ranging to be complete

  Wire.beginTransmission(_srfAddr); // start communicating with SRFmodule
  Wire.write(RANGEBYTE);            // Call the register for start of ranging data
  Wire.endTransmission();

  Wire.requestFrom(_srfAddr, 2); // Request 2 bytes from SRF module
  while (Wire.available() < 2)
    ;                     // Wait for data to arrive
  highByte = Wire.read(); // Get high byte
  lowByte = Wire.read();  // Get low byte

  range = (highByte << 8) + lowByte; // Put them together

  return (range); // Returns Range
}

int getSoft(int _srfAddr)
{ // Function to get software revision

  Wire.beginTransmission(_srfAddr); // Begin communication with the SRF module
  Wire.write(CMD);                  // Sends the command bit, when this bit is read it returns the software revision
  Wire.endTransmission();

  Wire.requestFrom(_srfAddr, 1); // Request 1 byte
  while (Wire.available() < 0)
    ;                         // While byte available
  int software = Wire.read(); // Get byte

  return (software);
}

void checkUltrasons()
{
  if (streamUS)
  {
    if (millis() - lastSendUs > timingSendingUs)
    {
      lastSendUs = millis();
      int range1 = getRange(0x70); // Calls a function to get range
      Serial.print("Range 1 = ");
      Serial.print(range1, DEC); // Print rangeData to LCD03

      int range2 = getRange(0x71); // Calls a function to get range
      Serial.print(" - Range 2 = ");
      Serial.print(range2, DEC); // Print rangeData to LCD03

      int range3 = getRange(0x72); // Calls a function to get range
      Serial.print(" - Range 3 = ");
      Serial.println(range3, DEC); // Print rangeData to LCD03

      String frame = "CUWV," + idTrameUsLong + "," + intToHex(range1, 4) + intToHex(range2, 4) + intToHex(range3, 4);
      serialBLE.println(frame);

      Serial.println(frame);
    }
  }
}
