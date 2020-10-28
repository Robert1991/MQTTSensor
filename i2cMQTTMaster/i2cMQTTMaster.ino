#include <Wire.h>
#include "wireUtils.h"
#include "credentials.h"
#include "IOTClients.h"

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

#define SDA D1
#define SCL D2
#define RELAIS_PIN D3

struct SensorValues {
  int lightSensorValue;
  float tempSensorValue;
  float humiditySensorValue;
  byte motionDetectedSensorValue;
};

WiFiClient espClient;
PubSubClient client(espClient);
MQTTClient* mqttClient = new MQTTClient(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD);

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

void sendI2CCommandWithParameter(byte command, byte parameter, int delayTime = 100) {
  refreshI2CConnection();
  Serial.print("Sending I2C command {");Serial.print(command);Serial.print("} with parameter {");Serial.print(parameter);Serial.println("}");
  Wire.beginTransmission(I2C_SLAVE_ADDRESS); 
  Wire.write(command);
  Wire.write(parameter);
  Wire.endTransmission(); 
  delay(delayTime);
}

byte fetchSingleByte(byte command, byte parameter, int delayTime = 100) {
  sendI2CCommandWithParameter(command, parameter, delayTime);
  Wire.requestFrom(I2C_SLAVE_ADDRESS, 1);
  if (Wire.available()) {
    byte returnByte = Wire.read();
    Serial.print("Received single byte from slave: ");Serial.println(returnByte);
    return returnByte;
  }
  return 255;
}

float fetchFloatFrom(byte command, byte parameter, int delayTime = 100) {
  sendI2CCommandWithParameter(command, parameter, delayTime);
  String dataString = "";
  Wire.requestFrom(I2C_SLAVE_ADDRESS, 7);
  while (Wire.available()) {
    char c = Wire.read();
    dataString = dataString + c;
  }
  return dataString.toFloat();
}

int fetchIntegerFrom(byte command, byte parameter, int delayTime = 100) {
  sendI2CCommandWithParameter(command, parameter, delayTime);
  Wire.requestFrom(I2C_SLAVE_ADDRESS, 2);
  if (Wire.available()) {
    byte hByte = Wire.read();
    byte lByte = Wire.read();
    return (int(hByte << 8) + int(lByte));
  }
  return -1;
}

bool unlockLockDevice(bool lock, int delayTime = 150, int confirmLockStatusTries = 5) {
  if (lock) {
    Serial.println("Locking slave device");
    if (fetchSingleByte(LOCK_UNLOCK_COMMAND, LOCK_PARAMETER, delayTime) == DEVICE_LOCKED) {
      Serial.println("retained lock command");
      for (int currentTry = 0; currentTry < confirmLockStatusTries; currentTry++) {
        if (fetchSingleByte(LOCK_UNLOCK_COMMAND, LOCK_STATUS_PARAMETER, delayTime) == LOCK_SET_STATUS) {
          Serial.println("Successfully locked slave device");
          return true;
        }
      }
      
      return false;
    }
  } else {
    Serial.println("Unlocking slave device");
    if (fetchSingleByte(LOCK_UNLOCK_COMMAND, UNLOCK_PARAMETER) == DEVICE_UNLOCKED) {
      for (int currentTry = 0; currentTry < confirmLockStatusTries; currentTry++) {
        if (fetchSingleByte(LOCK_UNLOCK_COMMAND, LOCK_STATUS_PARAMETER, delayTime) != LOCK_SET_STATUS) {
          Serial.println("Successfully unlocked slave device");
          return true;
        }
      }
      return true;
    }
  }
  return false;
}

SensorValues fetchSensorValuesFromSlave() {
  int lightSensorValue = fetchIntegerFrom(READ_SENSOR_VALUES_COMMAND, LIGHT_SENSOR_VALUE_PARAMETER);
  float tempSensorValue = fetchFloatFrom(READ_SENSOR_VALUES_COMMAND, TEMP_SENSOR_VALUE_PARAMETER);
  float humiditySensorValue = fetchFloatFrom(READ_SENSOR_VALUES_COMMAND, HUMIDITY_SENSOR_VALUE_PARAMETER);
  byte motionDetectedSensorValue = fetchSingleByte(READ_SENSOR_VALUES_COMMAND, MOTION_SENSOR_VALUE_PARAMETER);
  
  Serial.println("Received sensor values: ");
  Serial.print("  light sensor value: ");Serial.println(lightSensorValue);
  Serial.print("  temp sensor value: ");Serial.println(tempSensorValue);
  Serial.print("  humidity sensor value: ");Serial.println(humiditySensorValue);
  Serial.print("  motion detector sensor value: ");Serial.println(motionDetectedSensorValue);
  
  SensorValues sensorValues = {lightSensorValue, tempSensorValue, humiditySensorValue, motionDetectedSensorValue};
  return sensorValues;
}

bool lockSlaveDevice() {
  return unlockLockDevice(true);
}

bool unlockSlaveDevice() {
  return unlockLockDevice(false);
}

void setup() {
  pinMode(RELAIS_PIN , OUTPUT);
  Serial.begin(9600);
  establishI2CConnectionTo(SDA, SCL, true);
  setupWifiConnection(WIFI_SSID, WIFI_PASSWORD);
  mqttClient -> setupClient(&client, MQTT_BROKER, MQTT_PORT);
  Serial.println("setup finished");
}

void loop() {
  if (lockSlaveDevice()) {
    SensorValues currentSensorValues = fetchSensorValuesFromSlave();
    unlockSlaveDevice();

    char result[8];
    dtostrf(currentSensorValues.tempSensorValue, 6, 2, result);
    mqttClient -> publishMessage("living_room/temperature", result);
    dtostrf(currentSensorValues.humiditySensorValue, 6, 2, result);
    mqttClient -> publishMessage("living_room/humidity", result);
    dtostrf(currentSensorValues.lightSensorValue, 6, 2, result);
    mqttClient -> publishMessage("living_room/light", result);
    char byteResult[1];
    snprintf(byteResult, 1, "%d",(int)currentSensorValues.motionDetectedSensorValue);

    if (currentSensorValues.motionDetectedSensorValue == 1) {
      mqttClient -> publishMessage("living_room/motion", "on");  
    } else {
      mqttClient -> publishMessage("living_room/motion", "off");  
    }
  }
  delay(300);
}
