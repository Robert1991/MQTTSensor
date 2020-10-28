#include <Wire.h>
#include <math.h>
#include "DHT.h"

#define LIGHT_SENSOR_PIN A0

#define DHTPIN 2
#define DHTTYPE DHT11
#define MOTION_SENSOR_PIN 4

#define BLOCK_SIZE 32
#define I2C_SLAVE_ADDRESS 0x26

#define LOCK_UNLOCK_COMMAND 1
#define LOCK_SET_STATUS 1
#define DEVICE_LOCKED 1
#define DEVICE_UNLOCKED 2
#define UNLOCK_PARAMETER 0
#define LOCK_PARAMETER 1
#define LOCK_STATUS_PARAMETER 2

#define READ_SENSOR_VALUES_COMMAND 2
#define LIGHT_SENSOR_VALUE_PARAMETER 0
#define TEMP_SENSOR_VALUE_PARAMETER 1
#define HUMIDITY_SENSOR_VALUE_PARAMETER 2
#define MOTION_SENSOR_VALUE_PARAMETER 3

volatile bool _readLock = false;
volatile bool _lockSet = false;

volatile int _lightSensorRawValue;
volatile bool _motionDetectedValue = false;
volatile float _tempSensorValue;
volatile float _humiditySensorValue;

byte _lastCommand = 0;
byte _lastParameter = 0;

DHT dht(DHTPIN, DHTTYPE);

int readLightSensorAnalogValue() {
  int analogValue = analogRead(LIGHT_SENSOR_PIN);
  return analogValue;
}

void readDHTValues() {
  float currentHumiditySensorValue = dht.readHumidity();
  float currentTempSensorValue = dht.readTemperature(); 
  if (!isnan(currentHumiditySensorValue)) {       
    _humiditySensorValue = currentHumiditySensorValue;
  }
  if (!isnan(currentTempSensorValue)) {       
    _tempSensorValue = currentTempSensorValue;
  }
  Serial.print("Humidity: ");
  Serial.print(_humiditySensorValue);
  Serial.print("%\t");
  Serial.print("Temperatur: ");
  Serial.print(_tempSensorValue);
  Serial.write('°');
  Serial.println("C");
}

void readLightSensorState() {
  _lightSensorRawValue = readLightSensorAnalogValue();
  Serial.print("Current light sensor state: ");Serial.println(_lightSensorRawValue);
}

void readMotionDetectorState() {
  int state = digitalRead(MOTION_SENSOR_PIN); 
  
  if (state == HIGH) { 
    Serial.println("Motion detected");
    _motionDetectedValue = true;
  } else {
    Serial.println("No motion detected");
    _motionDetectedValue = false;
  }
}

void setup() {
  pinMode(MOTION_SENSOR_PIN, INPUT); 
  dht.begin();
  Wire.begin(I2C_SLAVE_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  Serial.begin(9600);
}

void loop() {
  if (!_readLock) {
    Serial.println("Begin reading sensor");
    _lockSet = false;
    readLightSensorState();
    readDHTValues();
    readMotionDetectorState();
  } else {
    Serial.println("Device locked");
    _lockSet = true;
  }
  delay(100);
}

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

void sendFloatValueToMaster(float value) {
  char valueBuffer[7];
  dtostrf(value, 7, 2, valueBuffer);
  Wire.write(valueBuffer, 7);
}

void requestEvent() {
  switch(_lastCommand) {
    case LOCK_UNLOCK_COMMAND:
      Wire.write(lockUnlockDevice(_lastParameter));
      break;
    case READ_SENSOR_VALUES_COMMAND:
      if (_lastParameter == LIGHT_SENSOR_VALUE_PARAMETER) {
        Wire.write(highByte(_lightSensorRawValue));  
        Wire.write(lowByte(_lightSensorRawValue));  
      } else if (_lastParameter == TEMP_SENSOR_VALUE_PARAMETER) {
        sendFloatValueToMaster(_tempSensorValue);
      } else if (_lastParameter == HUMIDITY_SENSOR_VALUE_PARAMETER) {
        sendFloatValueToMaster(_humiditySensorValue);
      } else if (_lastParameter == MOTION_SENSOR_VALUE_PARAMETER) {
        Wire.write(_motionDetectedValue);  
      } else {
        Wire.write(0);
      }
      break;
    default:
      Wire.write(255); 
  }
}
