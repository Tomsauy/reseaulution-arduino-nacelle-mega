//
// Created by Thomas Meunier on 21/10/2022.
//

#ifndef RESEAULUTION_ARDUINO_NACELLE_MEGA_BLUETOOTH_H
#define RESEAULUTION_ARDUINO_NACELLE_MEGA_BLUETOOTH_H

#include <Arduino.h>

void checkBLE(Stream &serialPort);
boolean initBLE();
void parseFrame(String _frameId, String _frame);

#endif //RESEAULUTION_ARDUINO_NACELLE_MEGA_BLUETOOTH_H
