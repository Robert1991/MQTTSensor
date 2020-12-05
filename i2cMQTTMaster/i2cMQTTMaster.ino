#include "credentials.h"
#include "IOTClients.h"
#include <wireUtils.h>
#include <MQTTDevices.h>

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

#define DHT_PIN D3
#define DHTTYPE DHT11
#define MOTION_SENSOR_PIN D4

#define SDA D1
#define SCL D2

char HUMIDITY_STATE_TOPIC[] = "kitchen/humidity";
char TEMPERATURE_STATE_TOPIC[] = "kitchen/temperature";
char MOTION_SENSOR_STATE_TOPIC[] = "kitchen/motion";
char LIGHT_INTENSITY_STATE_TOPIC[] = "kitchen/outside_light_intensity";

WiFiClient espClient;
PubSubClient client(espClient);
MQTTClient* mqttClient = new MQTTClient(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD);

MQTTMotionSensor* motionSensor = new MQTTMotionSensor(mqttClient, MOTION_SENSOR_STATE_TOPIC, MOTION_SENSOR_PIN);
DHT dht(DHT_PIN, DHTTYPE);
MQTTDhtSensor* dhtSensor = new MQTTDhtSensor(mqttClient, &dht, TEMPERATURE_STATE_TOPIC, HUMIDITY_STATE_TOPIC);

int lastLightSensorValue = -1;

bool areEqual(float value1, float value2, float minDifference = 0.001) {
  if (abs(value1 - value2) > minDifference) {
    return false;
  }
  return true;
}

void refreshI2CConnection() {
  if (checkI2CConnection(SDA, SCL)) {
    Serial.println("connection to slave got lost. trying to reestablish connection...");
    establishI2CConnectionTo(SDA, SCL);
  } else {
    while (Wire.available()) {
      Serial.println("flushing wire");
    }
  }
}

void sendI2CCommandWithParameter(byte command, byte parameter, int delayTime = 100) {
  refreshI2CConnection();
  Serial.print("Sending I2C command {"); Serial.print(command); Serial.print("} with parameter {"); Serial.print(parameter); Serial.println("}");
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
    Serial.print("Received single byte from slave: "); Serial.println(returnByte);
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

int fetchSensorValuesFromSlave() {
  int lightSensorValue = fetchIntegerFrom(READ_SENSOR_VALUES_COMMAND, LIGHT_SENSOR_VALUE_PARAMETER);
  Serial.println("Received sensor values: ");
  Serial.print("  light sensor value: "); Serial.println(lightSensorValue);
  return lightSensorValue;
}

bool lockSlaveDevice() {
  return unlockLockDevice(true);
}

bool unlockSlaveDevice() {
  return unlockLockDevice(false);
}

void setup() {
  Serial.begin(9600);
  establishI2CConnectionTo(SDA, SCL, true);
  setupWifiConnection(WIFI_SSID, WIFI_PASSWORD);

  motionSensor -> setupSensor();
  dhtSensor -> setupSensor();
  
  mqttClient -> setupClient(&client, MQTT_BROKER, MQTT_PORT);
  Serial.println("setup finished");

  StaticJsonDocument<200> doc;
  doc["state_topic"] = "bedroom/motion";
  doc["payload_on"] = "on";
  doc["payload_off"] = "off";
  doc["name"] = "Bedroom Motion Detector Auto Discovery";

  char[256] message;
  serializeJson(doc, message);
  mqttClient -> publishMessage("", message);
}

void loop() {
  checkWifiStatus(WIFI_SSID, WIFI_PASSWORD);
  if (mqttClient -> loopClient()) {
    motionSensor -> publishMeasurement();
    dhtSensor -> publishMeasurement();
  } else {
    dhtSensor -> reset();
    motionSensor -> reset();
  }
    
  if (lockSlaveDevice()) {
    int currentlightSensorValue = fetchSensorValuesFromSlave();
    unlockSlaveDevice();
    char result[8];
    if (lastLightSensorValue != currentlightSensorValue) {
      dtostrf(lastLightSensorValue, 6, 2, result);
      mqttClient -> publishMessage(LIGHT_INTENSITY_STATE_TOPIC, result);
      lastLightSensorValue = currentlightSensorValue;
    }
  }
  delay(50);
}
