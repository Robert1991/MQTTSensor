#ifndef wireUtils_h
#define wireUtils_h

#include <Wire.h>
#include "hardwareSerial.h"
#include "Arduino.h"

// see http://www.forward.com.au/pfod/ArduinoProgramming/I2C_ClearBus/index.html
int clearI2CBus(int sdaPin, int sclPin);

bool checkI2CConnection();

void establishI2CConnectionTo(int sdaPort, int sclPort);

#endif
