//
// Created by Thomas Meunier on 21/10/2022.
//

#ifndef RESEAULUTION_ARDUINO_NACELLE_MEGA_BAO_H
#define RESEAULUTION_ARDUINO_NACELLE_MEGA_BAO_H

#include <Arduino.h>

boolean attente_reponse(Stream &serialPort, char strwait[], char strinfo[], unsigned int timeout, unsigned int attente);
void flushSerial(Stream &serialPort);
void debugSerial(Stream &serialPort);
void reboot();

#endif //RESEAULUTION_ARDUINO_NACELLE_MEGA_BAO_H
