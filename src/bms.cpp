#include <Arduino.h>
#include <varGlobal.h>
#include <bms.h>
#include <main.h>

void sendBms()
{
  //SOC
  unsigned int soc = 0;
  Wire.beginTransmission(0x36); // transmit to device #8
  Wire.write(0x06);        // sends five bytes
  Wire.endTransmission(false);    // stop transmitting
  Wire.requestFrom(0x36, 2);    // request 6 bytes from slave device #8

  temp = Wire.read();
  // Serial.println(temp, HEX); // receive a byte as character

  soc = Wire.read();
  // Serial.println(soc, HEX); // receive a byte as character
  soc = soc << 8;

  soc |= temp;
  soc = soc * (1.0 / 256.0);
  Serial.println("SoC = " + String(soc) + "%");


  //TTF
  unsigned int TtF = 0;
  Wire.beginTransmission(0x36); // transmit to device #8
  Wire.write(0x20);        // sends five bytes
  Wire.endTransmission(false);    // stop transmitting
  Wire.requestFrom(0x36, 2);    // request 6 bytes from slave device #8

  temp = Wire.read();
  //Serial.println(temp, HEX); // receive a byte as character

  TtF = Wire.read();
  //Serial.println(TtF, HEX); // receive a byte as character
  TtF = TtF << 8;

  TtF |= temp;
  TtF = (TtF * 5.625) / 60.0;
  Serial.println("TtF = " + String(TtF) + " minutes");
  //Si FF FF = NC !


  //TTE
  unsigned int tte = 0;
  Wire.beginTransmission(0x36); // transmit to device #8
  Wire.write(0x11);        // sends five bytes
  Wire.endTransmission(false);    // stop transmitting
  Wire.requestFrom(0x36, 2);    // request 6 bytes from slave device #8

  temp = Wire.read();
  // Serial.println(temp, HEX); // receive a byte as character

  tte = Wire.read();
  // Serial.println(soc, HEX); // receive a byte as character
  tte = tte << 8;

  tte |= temp;
  tte = (tte * 5.625) / 60.0;
  Serial.println("TtE = " + String(tte) + " minutes");
  //Si FF FF = NC !


  //Pack Current
  int pCurrent = 0;
  Wire.beginTransmission(0x36); // transmit to device #8
  Wire.write(0x1c);        // sends five bytes
  Wire.endTransmission(false);    // stop transmitting
  Wire.requestFrom(0x36, 2);    // request 6 bytes from slave device #8

  temp = Wire.read();
  // Serial.println(temp, HEX); // receive a byte as character

  pCurrent = Wire.read();
  // Serial.println(soc, HEX); // receive a byte as character
  pCurrent = pCurrent << 8;

  pCurrent |= temp;
  pCurrent = (pCurrent * (1.5625 / 0.005) / 1000);
  Serial.println("Current = " + String(pCurrent) + "mA");


  //Cell 1 Avg Voltage -  Result x 0.078125 = mV
  unsigned int vcell1 = 0;
  Wire.beginTransmission(0x36); // transmit to device #8
  Wire.write(0xd4);        // sends five bytes
  Wire.endTransmission(false);    // stop transmitting
  Wire.requestFrom(0x36, 2);    // request 6 bytes from slave device #8

  temp = Wire.read();
  // Serial.println(temp, HEX); // receive a byte as character

  vcell1 = Wire.read();
  // Serial.println(soc, HEX); // receive a byte as character
  vcell1 = vcell1 << 8;

  vcell1 |= temp;
  vcell1 = vcell1 * 0.078125;
  Serial.println("Vcell1 = " + String(vcell1) + "mV");


  //Cell 2 Avg Voltage -  Result x 0.078125 = mV
  unsigned int vcell2 = 0;
  Wire.beginTransmission(0x36); // transmit to device #8
  Wire.write(0xd3);        // sends five bytes
  Wire.endTransmission(false);    // stop transmitting
  Wire.requestFrom(0x36, 2);    // request 6 bytes from slave device #8

  temp = Wire.read();
  // Serial.println(temp, HEX); // receive a byte as character

  vcell2 = Wire.read();
  // Serial.println(soc, HEX); // receive a byte as character
  vcell2 = vcell2 << 8;

  vcell2 |= temp;
  vcell2 = vcell2 * 0.078125;
  Serial.println("Vcell2 = " + String(vcell2) + "mV");



  //Cell 3 Avg Voltage -  Result x 0.078125 = mV
  unsigned int vcell3 = 0;
  Wire.beginTransmission(0x36); // transmit to device #8
  Wire.write(0xd2);        // sends five bytes
  Wire.endTransmission(false);    // stop transmitting
  Wire.requestFrom(0x36, 2);    // request 6 bytes from slave device #8

  temp = Wire.read();
  // Serial.println(temp, HEX); // receive a byte as character

  vcell3 = Wire.read();
  // Serial.println(soc, HEX); // receive a byte as character
  vcell3 = vcell3 << 8;

  vcell3 |= temp;
  vcell3 = vcell3 * 0.078125;
  Serial.println("Vcell3 = " + String(vcell3) + "mV");


  //Pack T°
  int packTemp = 0;
  Wire.beginTransmission(0x36); // transmit to device #8
  Wire.write(0x1b);        // sends five bytes
  Wire.endTransmission(false);    // stop transmitting
  Wire.requestFrom(0x36, 2);    // request 6 bytes from slave device #8

  temp = Wire.read();
  //Serial.println(ptemp, HEX); // receive a byte as character

  packTemp = Wire.read();
  packTemp = packTemp << 8;

  packTemp |= temp;
  packTemp = (packTemp * (1.0 / 256.0)) * 100;
  Serial.println("Pack Temp = " + String(packTemp / 100) + "°C");




  //Alert Status
  int protectionStatus = 0;
  Wire.beginTransmission(0x36); // transmit to device #8
  Wire.write(0xd9);        // sends five bytes
  Wire.endTransmission(false);    // stop transmitting
  Wire.requestFrom(0x36, 2);    // request 6 bytes from slave device #8

  temp = Wire.read();
  //Serial.println(ptemp, HEX); // receive a byte as character

  protectionStatus = Wire.read();
  protectionStatus = protectionStatus << 8;

  protectionStatus |= temp;

  Serial.print("Protection status = ");
  Serial.println(protectionStatus, BIN);
  if (bitRead(protectionStatus, 0))
  {
    Serial.println("> Ship");
  }
  if (bitRead(protectionStatus, 1))
  {
    Serial.println("> ResDFault");
  }
  if (bitRead(protectionStatus, 2))
  {
    Serial.println("> ODCP");
  }
  if (bitRead(protectionStatus, 3))
  {
    Serial.println("> UVP");
  }
  if (bitRead(protectionStatus, 4))
  {
    Serial.println("> TooHotD");
  }
  if (bitRead(protectionStatus, 5))
  {
    Serial.println("> DieHot");
  }
  if (bitRead(protectionStatus, 6))
  {
    Serial.println("> PermFail");
  }
  if (bitRead(protectionStatus, 7))
  {
    Serial.println("> Imbalance");
  }
  if (bitRead(protectionStatus, 8))
  {
    Serial.println("> PreqF");
  }
  if (bitRead(protectionStatus, 9))
  {
    Serial.println("> Qovflw");
  }
  if (bitRead(protectionStatus, 10))
  {
    Serial.println("> OCCP");
  }
  if (bitRead(protectionStatus, 11))
  {
    Serial.println("> OVP");
  }
  if (bitRead(protectionStatus, 12))
  {
    Serial.println("> TooColdC");
  }
  if (bitRead(protectionStatus, 13))
  {
    Serial.println("> Full");
  }
  if (bitRead(protectionStatus, 14))
  {
    Serial.println("> TooHotC");
  }
  if (bitRead(protectionStatus, 15))
  {
    Serial.println("> ChgWDT");
  }


  //Fet Status
  int fetStatus = 0;
  Wire.beginTransmission(0x36); // transmit to device #8
  Wire.write(0x1f1);        // sends five bytes
  Wire.endTransmission(false);    // stop transmitting
  Wire.requestFrom(0x36, 2);    // request 6 bytes from slave device #8

  temp = Wire.read();
  //Serial.println(ptemp, HEX); // receive a byte as character

  fetStatus = Wire.read();
  fetStatus = fetStatus << 8;

  fetStatus |= temp;

  Serial.print("FETs status = ");
  Serial.println(fetStatus, BIN);

  Serial.println("> Charge FET = " + String(bitRead(fetStatus, 0)));
  Serial.println("> Discharge FET = " + String(bitRead(fetStatus, 1)));

  int enCharge = 0;
  if (pCurrent > 50)
    enCharge = 1;

  unsigned long totalBatt = vcell1 + vcell2 + vcell3;

  String frame = "CUWV," + idTrameBmsLong + "," + intToHex(soc, 2) + intToHex(totalBatt, 4) + intToHex(tte, 4)  + intToHex(TtF, 4) + intToHex(enCharge, 2) + intToHex(packTemp, 4) + intToHex(pCurrent, 4);
  serialBLE.println(frame);
  Serial.println(frame);
  Serial.println("------------------------");
}
