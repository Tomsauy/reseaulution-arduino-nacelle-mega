#include <Arduino.h>
#include <bluetooth.h>
#include <varGlobal.h>
#include <bao.h>
#include <led.h>
#include <gimbal.h>

/******************************/
void checkBLE(Stream &serialPort)
{
  unsigned long startTime = millis();
  char curChar;
  boolean delimiterFound = false;
  boolean passcodeFound = false;
  while (serialPort.available() > 0 && (startTime - millis()) < 200)
  {
    curChar = serialPort.read();
    if (curChar == '\n')
    {
      delimiterFound = true;
      break;
    }
    blBuffer += curChar;
    if (blBuffer.startsWith("Passcode:"))
      passcodeFound = true;
  }
  if (delimiterFound)
  {
    Serial.println("BL Data available");
    Serial.println(blBuffer);

    if (blBuffer.startsWith("Connected"))
    {
      bleConnected = true;
    }
    else if (blBuffer.startsWith("Connection End"))
    {
      bleConnected = false;
      bleBonded = false;
      passcodeFound = false;
      serialPort.println("A");
    }
    else if (blBuffer.startsWith("Notify,") && blBuffer.endsWith(".\r"))
    {
      parseFrame(blBuffer.substring(7, 11), blBuffer.substring(12, blBuffer.length() - 2));
    }

    blBuffer = "";
  }
  else if (passcodeFound)
  {
    //debugSerial(serialBLE);
    Serial.println("PassCode");
    serialBLE.println(blePassCode);
    //debugSerial(serialBLE);
    if (attente_reponse(serialBLE, "Secured", "Bonded/Secured", 5000, 2000))
    {
      bleBonded = true;
    }

    //Activation des notify
    serialBLE.println("CUWC,91b1b2a5d4534a08b153c0902d2e9e88,1");
    if (!attente_reponse(serialBLE, "AOK", "En notif", 1000, 500))
    {
      reboot();
    }

    serialBLE.println("CUWC,f0b7f97fe92b440b82f1599c2f2e9fa2,1");
    if (!attente_reponse(serialBLE, "AOK", "En notif", 1000, 500))
    {
      reboot();
    }

    blBuffer = "";
  }
}
/******************************/

/******************************/
boolean initBLE()
{
  Serial.println("Init BLE...");
  pinMode(wkeBLE, OUTPUT);
  digitalWrite(wkeBLE, 0);
  delay(2000);
  digitalWrite(wkeBLE, 1);
  delay(2000);

  Serial2.begin(115200);

  serialBLE.println("SF,2");
  if (!attente_reponse(serialBLE, "AOK", "SF", 2000, 200))
    return false;

  serialBLE.println("R,1");
  if (!attente_reponse(serialBLE, "CMD", "Reboot", 4000, 200))
    return false;

  serialBLE.println("SR,00541000");
  if (!attente_reponse(serialBLE, "AOK", "SR", 1000, 200))
    return false;

  serialBLE.println("SN,NACELLER");
  if (!attente_reponse(serialBLE, "AOK", "SN", 1000, 200))
    return false;

  serialBLE.println("SS,00000000");
  if (!attente_reponse(serialBLE, "AOK", "SS", 1000, 200))
    return false;

  serialBLE.println("R,1");
  if (!attente_reponse(serialBLE, "CMD", "Reboot", 4000, 3000))
    return false;

  /*
    serialBLE.println("WC");
    if (!attente_reponse(serialBLE, "AOK", "Clear script", 1000, 200))
      return false;

    serialBLE.println("WW");
    if (!attente_reponse(serialBLE, "AOK", "Add script", 1000, 200))
      return false;

    serialBLE.print("@DISCON\n");
    serialBLE.print("A");
    serialBLE.write(0x1b);
    if (!attente_reponse(serialBLE, "END", "End script", 1000, 200))
      return false;
  */

  serialBLE.println("A");
  if (!attente_reponse(serialBLE, "AOK", "Advertising", 1000, 200))
    return false;

  // debugSerial(serialBLE);

  return true;
}
/******************************/

void parseFrame(String _frameId, String _frame)
{
  Serial.println("ID : " + _frameId + " - frame : " + _frame);

  if (_frameId == idTrameEnableShort) //Trame enables
  {
    char * endPtr;
    int str_len = _frame.substring(0, 2).length() + 1;
    char char_array[str_len];

    _frame.substring(0, 2).toCharArray(char_array, str_len);

    long  octet1 = strtol(char_array, &endPtr, 16);
    if ( endPtr == char_array ) {
      Serial.println("Erreur chaine");
      return;
    }

    char * endPtr2;
    int str_len2 = _frame.substring(2).length() + 1;;
    char char_array2[str_len2];
    _frame.substring(2).toCharArray(char_array2, str_len2);

    long  octet2 = strtol(char_array2, &endPtr2, 16);
    if ( endPtr2 == char_array2 ) {
      Serial.println("Erreur chaine");
      return;
    }

    Serial.println("Frame enables");
    Serial.print("Octet1 : "); Serial.println(octet1, BIN);
    Serial.print("Octet2 : "); Serial.println(octet2, BIN);

    //Send position angulaire
    if (bitRead(octet1, 7))
    {
      sendPosAng();
    }

    //Alerte lumineuse
    if (bitRead(octet1, 0))
    {
      if (!wLight)
      {
        Serial1.print("wLight=1\n");
        wLight = true;
      }
    }
    else if (wLight)
    {
      Serial1.print("wLight=0\n");
      wLight = false;
    }

    //Alerte sonore
    if (bitRead(octet1, 1))
    {
      if (!wSound)
      {
        Serial1.print("wSound=1\n");
        wSound = true;
      }
    }
    else if (wSound)
    {
      Serial1.print("wSound=0\n");
      wSound = false;
    }

    //Alim gimbal
    if (bitRead(octet1, 2))
    {
      Serial.println("octet alimGimbal ON");
      if (!alimGimbal)
      {
        Serial.println("set alimGimbal ON");
        digitalWrite(pinEnAlimGimbal, HIGH);
        alimGimbal = true;
      }
    } else {
      Serial.println("octet alimGimbal OFF");
      if (alimGimbal) {
        Serial.println("set alimGimbal OFF");
        digitalWrite(pinEnAlimGimbal, LOW);
        alimGimbal = false;
      }
    }

    //Alim Jetson
    if (bitRead(octet1, 3))
    {
      Serial.println("octet alimJetson ON");
      if (!alimJetson)
      {
        Serial.println("set alimJetson ON");
        digitalWrite(pinEnAlimjetson, HIGH);
        alimJetson = true;
      }
    } else {
      Serial.println("octet alimJetson OFF");
      if (alimJetson) {
        Serial.println("set alimJetson OFF");
        digitalWrite(pinEnAlimjetson, LOW);
        alimJetson = false;
      }
    }

    //Streaming US
    if (bitRead(octet1, 4))
    {
      streamUS = true;
    }
    else
    {
      streamUS = false;
    }

    //Streaming FDC
    if (bitRead(octet1, 5))
    {
      streamFDC = true;
    }
    else
    {
      streamFDC = false;
    }

    //Send BMS datas
    if (bitRead(octet1, 6))
    {
      bmsDataRequest = true;
    }

    //Eclairage LED
    int octetLed = octet2 & 0b00001111;
    Serial.println("OctetLed = " + String(octetLed));
    if (octetLed != prevPosLed)
    {
      if (octetLed == 9)
      {
        ledAuto = true;
        majPosLed();
      }
      else
      {
        ledAuto = false;
        Serial1.print("posLed=" + String(octetLed) + "\n");
      }

      prevPosLed = octetLed;
    }

    //Activation chargeur
    if (bitRead(octet2, 4))
    {
      digitalWrite(pinEnCharge, LOW);
    }
    else
    {
      digitalWrite(pinEnCharge, HIGH);
    }
  }
  else if (_frameId == idTrameGimbalShort) //Trame gimbal
  {
    char * endPtr;
    int str_len = _frame.substring(0, 2).length() + 1;
    char char_array[str_len];

    _frame.substring(0, 2).toCharArray(char_array, str_len);

    long  octet1 = strtol(char_array, &endPtr, 16);
    if ( endPtr == char_array ) {
      Serial.println("Erreur chaine");
      return;
    }
    Serial.print("Octet1 : "); Serial.println(octet1, BIN);

    char * endPtr2;
    int str_len2 = _frame.substring(2, 6).length() + 1;
    char char_array2[str_len2];

    _frame.substring(2, 6).toCharArray(char_array2, str_len2);

    long  octetsAngleX = strtol(char_array2, &endPtr2, 16);
    if (octetsAngleX > 0x7fffL)
      octetsAngleX -= 0x10000L;

    if ( endPtr2 == char_array2 ) {
      Serial.println("Erreur chaine");
      return;
    }
    Serial.print("long angle X : "); Serial.println(octetsAngleX);


    char * endPtr3;
    int str_len3 = _frame.substring(6, 10).length() + 1;
    char char_array3[str_len3];

    _frame.substring(6, 10).toCharArray(char_array3, str_len3);

    long  octetsAngleY = strtol(char_array3, &endPtr3, 16);
    if (octetsAngleY > 0x7fffL)
      octetsAngleY -= 0x10000L;

    if ( endPtr3 == char_array3 ) {
      Serial.println("Erreur chaine");
      return;
    }
    Serial.print("long angle Y : "); Serial.println(octetsAngleY);


    char * endPtr4;
    int str_len4 = _frame.substring(10, 14).length() + 1;
    char char_array4[str_len4];

    _frame.substring(10, 14).toCharArray(char_array4, str_len4);

    long  octetVitesseX = strtol(char_array4, &endPtr4, 16);
    if (octetVitesseX > 0x7fffL)
      octetVitesseX -= 0x10000L;

    if ( endPtr4 == char_array4 ) {
      Serial.println("Erreur chaine");
      return;
    }
    Serial.print("long vitesse X: "); Serial.println(octetVitesseX);


    char * endPtr5;
    int str_len5 = _frame.substring(14, 18).length() + 1;
    char char_array5[str_len5];

    _frame.substring(14, 18).toCharArray(char_array5, str_len5);

    long  octetVitesseY = strtol(char_array5, &endPtr5, 16);
    if (octetVitesseY > 0x7fffL)
      octetVitesseY -= 0x10000L;

    if ( endPtr5 == char_array5 ) {
      Serial.println("Erreur chaine");
      return;
    }
    Serial.print("long vitesse Y: "); Serial.println(octetVitesseY);


    //Standby mode
    if (bitRead(octet1, 0))
    {
      gStandBy = 1;
      gStandByRequest = true;
      Serial.println("Standby ON");
    }

    //WakeUp mode
    if (bitRead(octet1, 1))
    {
      gStandBy = 0;
      gStandByRequest = true;
      Serial.println("Standby OFF");
    }

    //Pan mode
    if (bitRead(octet1, 2))
    {
      gPanMode = 1;
      Serial.println("Mode Pan");
    }
    else
    {
      gPanMode = 0;
      Serial.println("Mode hold");
    }

    //Recenter
    if (bitRead(octet1, 3))
    {
      gRecenterRequest = true;
      Serial.println("Recenter");
    }

    //Demande infos
    if (bitRead(octet1, 4))
    {
      gInfoRequest = true;
      Serial.println("Ask info");
    }

    //Commande de position à traiter
    if (bitRead(octet1, 5))
    {
      //Type commande - 0 = positions - 1 = vitesse
      if (bitRead(octet1, 6))
      {
        Serial.println("Mode vitesse");
        gCommandMode = 1;
        gValX =  octetVitesseX;
        gValY =  octetVitesseY;
      }
      else
      {
        Serial.println("Mode Position angulaire");
        gCommandMode = 0;
        gPosX = octetsAngleX;
        gPosY = octetsAngleY;
      }
    }
  }
  else if (_frameId == idTrameGimbalConfigShort) //Trame config gimbal
  {

    char * endPtr;
    int str_len = _frame.substring(0, 4).length() + 1;
    char char_array[str_len];

    _frame.substring(0, 4).toCharArray(char_array, str_len);

    long  octetsYawMin = strtol(char_array, &endPtr, 16);
    if (octetsYawMin > 0x7fffL)
      octetsYawMin -= 0x10000L;

    if ( endPtr == char_array ) {
      Serial.println("Erreur chaine");
      return;
    }
    Serial.print("long Yaw Min: "); Serial.println(octetsYawMin);


    char * endPtr2;
    int str_len2 = _frame.substring(4, 8).length() + 1;
    char char_array2[str_len2];

    _frame.substring(4, 8).toCharArray(char_array2, str_len2);

    long  octetsYawMax = strtol(char_array2, &endPtr2, 16);
    if (octetsYawMax > 0x7fffL)
      octetsYawMax -= 0x10000L;

    if ( endPtr2 == char_array2 ) {
      Serial.println("Erreur chaine");
      return;
    }
    Serial.print("long Yaw Max: "); Serial.println(octetsYawMax);


    char * endPtr3;
    int str_len3 = _frame.substring(8, 12).length() + 1;
    char char_array3[str_len3];

    _frame.substring(8, 12).toCharArray(char_array3, str_len3);

    long  octetsPitchMin = strtol(char_array3, &endPtr3, 16);
    if (octetsPitchMin > 0x7fffL)
      octetsPitchMin -= 0x10000L;

    if ( endPtr3 == char_array3 ) {
      Serial.println("Erreur chaine");
      return;
    }
    Serial.print("long Pitch Min: "); Serial.println(octetsPitchMin);


    char * endPtr4;
    int str_len4 = _frame.substring(12, 16).length() + 1;
    char char_array4[str_len4];

    _frame.substring(12, 16).toCharArray(char_array4, str_len4);

    long  octetsPitchMax = strtol(char_array4, &endPtr4, 16);
    if (octetsPitchMax > 0x7fffL)
      octetsPitchMax -= 0x10000L;

    if ( endPtr4 == char_array4 ) {
      Serial.println("Erreur chaine");
      return;
    }
    Serial.print("long Pitch Max: "); Serial.println(octetsPitchMax);

    gYawMin = octetsYawMin; 
    gYawMax = octetsYawMax;
    gPitchMin = octetsPitchMin;
    gPitchMax = octetsPitchMax;

  } else
  {
    Serial.print("Frame ID non connu : ");
    Serial.println(_frameId);
  }
}
