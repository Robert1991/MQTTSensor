#include <Wire.h>
#include "wireUtils.h"

#define I2C_SLAVE_ADDRESS 0x26

#define LOCK_UNLOCK_COMMAND 1
#define LOCK_SET_STATUS 1
#define DEVICE_LOCKED 1
#define DEVICE_UNLOCKED 2
#define UNLOCK_PARAMETER 0
#define LOCK_PARAMETER 1
#define LOCK_STATUS_PARAMETER 2

#define MESSAGE_PARAMETERS_COMMAND 2
#define MESSAGE_PARAMETERS_BLOCKSIZE 1
#define MESSAGE_PARAMETERS_TOTAL_BLOCKS 2
#define MESSAGE_PARAMETERS_STRING_LENGTH 3

#define TRANSFER_JSON_MESSAGE_COMMAND 3

#define SDA D1
#define SCL D2

struct SensorMessageParameters {
  byte blockSize;
  byte totalBlocks;
  byte stringLength;
};

void refreshI2CConnection() {
  if (checkI2CConnection()) {
    Serial.println("connection to slave got lost. trying to reestablish connection...");
    establishI2CConnectionTo(SDA, SCL);
  } else {
    while(Wire.available()) {
      Serial.println("flushing wire");
    }
  }
}

void sendI2CCommandWithParameter(byte command, byte parameter, int delayTime = 500) {
  refreshI2CConnection();
  Serial.print("Sending I2C command {");Serial.print(command);Serial.print("} with parameter {");Serial.print(parameter);Serial.println("}");
  Wire.beginTransmission(I2C_SLAVE_ADDRESS); 
  Wire.write(command);
  Wire.write(parameter);
  Wire.endTransmission(); 
  delay(delayTime);
}

byte fetchSingleByte(byte command, byte parameter, int delayTime = 500) {
  sendI2CCommandWithParameter(command, parameter, delayTime);
  Wire.requestFrom(I2C_SLAVE_ADDRESS, 1);
  if (Wire.available()) {
    byte returnByte = Wire.read();
    Serial.print("Received single byte from slave: ");Serial.println(returnByte);
    return returnByte;
  }
  return 255;
}

bool unlockLockDevice(bool lock, int delayTime = 750, int lockHoldDelay = 1000, int confirmLockStatusTries = 5) {
  if (lock) {
    Serial.println("Locking slave device");
    if (fetchSingleByte(LOCK_UNLOCK_COMMAND, LOCK_PARAMETER, delayTime) == DEVICE_LOCKED) {
      delay(lockHoldDelay);
      for (int currentTry = 0; currentTry < confirmLockStatusTries; currentTry++) {
        if (fetchSingleByte(LOCK_UNLOCK_COMMAND, LOCK_STATUS_PARAMETER) == LOCK_SET_STATUS) {
          Serial.println("Successfully locked slave device");
          return true;
        }
      }
      
      return false;
    }
  } else {
    Serial.println("Unlocking slave device");
    if (fetchSingleByte(LOCK_UNLOCK_COMMAND, UNLOCK_PARAMETER) == DEVICE_UNLOCKED) {
      delay(lockHoldDelay);
      for (int currentTry = 0; currentTry < confirmLockStatusTries; currentTry++) {
        if (fetchSingleByte(LOCK_UNLOCK_COMMAND, LOCK_STATUS_PARAMETER) != LOCK_SET_STATUS) {
          Serial.println("Successfully unlocked slave device");
          return true;
        }
      }
      return true;
    }
  }
  return false;
}

bool lockSlaveDevice() {
  return unlockLockDevice(true);
}

bool unlockSlaveDevice() {
  return unlockLockDevice(false);
}

SensorMessageParameters fetchMessageParametersFromSlave() {
  byte blockSize = fetchSingleByte(MESSAGE_PARAMETERS_COMMAND, MESSAGE_PARAMETERS_BLOCKSIZE);
  delay(500);
  byte totalBlocks = fetchSingleByte(MESSAGE_PARAMETERS_COMMAND, MESSAGE_PARAMETERS_TOTAL_BLOCKS);
  delay(500);
  byte stringLength = fetchSingleByte(MESSAGE_PARAMETERS_COMMAND, MESSAGE_PARAMETERS_STRING_LENGTH);
  delay(500);
  
  Serial.println("Received message parameters: ");
  Serial.print("  block size: ");Serial.println(blockSize);
  Serial.print("  total blocks: ");Serial.println(totalBlocks);
  Serial.print("  string length: ");Serial.println(stringLength);
  SensorMessageParameters messageParameters = {blockSize, totalBlocks, stringLength};
  return messageParameters;
}

int fetchSensorDataFromSlave(SensorMessageParameters messageParameters) {
  for(int currentBlock = 0; currentBlock < messageParameters.totalBlocks; currentBlock++) {
    sendI2CCommandWithParameter(TRANSFER_JSON_MESSAGE_COMMAND, currentBlock);
    
    if (currentBlock == (messageParameters.totalBlocks -1)) {
      int lastBlockSize = messageParameters.stringLength - (messageParameters.totalBlocks -1)*messageParameters.blockSize;
      Wire.requestFrom(I2C_SLAVE_ADDRESS, (uint8_t)lastBlockSize);
    } else {
      Wire.requestFrom(I2C_SLAVE_ADDRESS, (uint8_t)messageParameters.blockSize);
    }
    delay(5000);
    while(Wire.available()) {
      Serial.print((char)Wire.read());
      delay(1000);
    }
  }
  Serial.println();
}

void setup() {
  Serial.begin(9600);
  establishI2CConnectionTo(SDA, SCL);
  Serial.println("setup finished");
}

void loop() {
  if (lockSlaveDevice()) {
    fetchMessageParametersFromSlave();
  
    delay(10000);
  }

  unlockSlaveDevice();
  
  delay(10000);
}
