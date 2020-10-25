#include <Wire.h>
#include <math.h>
#include "ArduinoJson.h"

#define LIGHT_SENSOR_PIN A0

#define BLOCK_SIZE 32
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


volatile bool _readLock = false;
volatile bool _lockSet = false;
const uint8_t OUT_BUFFER_SIZE = 256;

// *** Need volatile because we are updating this buffer outside the normal loop.
volatile byte _outboundBuffer[OUT_BUFFER_SIZE];

String _currentJsonMessage;
uint8_t _totalBlocks = 0;
uint8_t _stringLength = 0;

int readLightSensorAnalogValue() {
  int analogValue = analogRead(LIGHT_SENSOR_PIN);
  return analogValue;
}

void readLightSensorState(JsonArray sensorData) {
  JsonObject lightSensorData = sensorData.createNestedObject();
  lightSensorData["sensor"] = "lightSensor";
  JsonObject sensorDataEntry = lightSensorData.createNestedObject("data");
  
  int lightSensorAnalogValue = readLightSensorAnalogValue();
  sensorDataEntry["lightSensorRawValue"] = lightSensorAnalogValue;
  if (lightSensorAnalogValue < 10) {
    sensorDataEntry["lightSensorState"] = "Da";
  } else if (lightSensorAnalogValue < 200) {
    sensorDataEntry["lightSensorState"] = "Dim";
  } else if (lightSensorAnalogValue < 500) {
    sensorDataEntry["lightSensorState"] = "bright";
  } else if (lightSensorAnalogValue < 800) {
    sensorDataEntry["lightSensorState"] = "Light";
  } else {
    sensorDataEntry["lightSensorState"] = "Very bright";
  }
}

void setup() {
 Wire.begin(I2C_SLAVE_ADDRESS);                /* join i2c bus with address 8 */
 Wire.onReceive(receiveEvent); /* register receive event */
 Wire.onRequest(requestEvent); /* register request event */
 Serial.begin(9600);           /* start serial for debug */
}

int freeRam() {
  extern int __heap_start, *__brkval;
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

void loop() {
  Serial.print("Free ram: ");
  Serial.println(freeRam());
  Serial.println(_readLock);
  if (!_readLock) {
    Serial.println("Begin reading sensor");
    _lockSet = false;
    _currentJsonMessage = "";
    DynamicJsonDocument doc(200);
    JsonArray sensorData = doc.createNestedArray("sensorData");
    readLightSensorState(sensorData);
  
    serializeJson(doc, _currentJsonMessage);
    _stringLength = _currentJsonMessage.length();
    _totalBlocks = (uint8_t)(ceil((float)_stringLength / (float)BLOCK_SIZE));
    Serial.println(_stringLength);
    Serial.println(_totalBlocks);
    Serial.println(_currentJsonMessage);
  } else {
    _lockSet = true;
  }
  delay(2000);
}

byte _lastCommand = 0;
byte _lastParameter = 0;

// function that executes whenever data is received from master
void receiveEvent(int byteCount) {
  if (byteCount == 2) {
    _lastCommand = Wire.read();
    _lastParameter = Wire.read();
    Serial.print("Received new command and parameter: lastCommand = ");Serial.print(_lastCommand);Serial.print("  lastParameter = ");Serial.println(_lastParameter);
  } else {
    Serial.print("Received event with incompatible count of bytes: ");Serial.println(byteCount);
    Serial.println("Clearing buffer");
    while (0 <Wire.available()) {
      byte command = Wire.read();
      Serial.print(command); 
    }
  }
}

// request event functions

byte lockUnlockDevice(byte parameter) {
  if (parameter == LOCK_PARAMETER) {
    Serial.println("Locking data.");
    _readLock = true;
    return DEVICE_LOCKED;
  } else if (parameter == UNLOCK_PARAMETER) {
    Serial.println("Unlocking data.");
    _readLock = false;
    return DEVICE_UNLOCKED;
  } else if (parameter == LOCK_STATUS_PARAMETER) {
    Serial.println("Requesting lock status.");
    if (_lockSet) {
      return LOCK_SET_STATUS;
    }
    return 0;
  }
  Serial.print("unknown parameter: ");Serial.println(parameter);
  return 9;
}


void requestEvent() {
  switch(_lastCommand) {
    case LOCK_UNLOCK_COMMAND:
      Wire.write(lockUnlockDevice(_lastParameter));
      break;
    case MESSAGE_PARAMETERS_COMMAND:
      if (_lastParameter == MESSAGE_PARAMETERS_BLOCKSIZE) {
        Wire.write(BLOCK_SIZE);  
      } else if (_lastParameter == MESSAGE_PARAMETERS_TOTAL_BLOCKS) {
        Wire.write(_totalBlocks);
      } else if (_lastParameter == MESSAGE_PARAMETERS_STRING_LENGTH) {
        Wire.write(_stringLength);
      }
      
      break;
    case 3:
      Wire.write(_totalBlocks);
      break;
    default:
      Wire.write(255); 
  }
 
}
