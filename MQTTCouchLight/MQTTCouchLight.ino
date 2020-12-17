#include <IOTClients.h>
#include <MQTTDevices.h>
#include "credentials.h"

#define SDA D1
#define SCL D2

#define I2C_SLAVE_ADDRESS 8
#define ON_OFF_COMMAND 0
#define SET_COLOR_COMMAND 1
#define SET_BRIGHTNESS_COMMAND 2

WiFiClient espClient;
MQTTClient client(750);
MessageQueueClient* mqttClient = new MessageQueueClient(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD);

MQTTDeviceInfo deviceInfo = {"xl2H1Q" ,"couch_rgb_i2c_platine", "homeassistant", "Node MCU", "RoboTronix"};
MQTTDevicePing* devicePing = new MQTTDevicePing(deviceInfo,"Du6v7I", 30000);
MQTTDeviceResetSwitch* resetSwitch = new MQTTDeviceResetSwitch(deviceInfo, "xqyNMW");  

WirePinSet wireConfiguration = {I2C_SLAVE_ADDRESS, SDA, SCL};
MQTTRgbLightI2CCommands connectionCommands = {ON_OFF_COMMAND, SET_COLOR_COMMAND, SET_BRIGHTNESS_COMMAND};
MQTTI2CRgbLightConfiguration stripConfig = {connectionCommands, wireConfiguration};
MQTTI2CRgbLight* rgbLedStrip = new MQTTI2CRgbLight(deviceInfo, "L5xVUX", stripConfig);

MQTTDeviceService* mqttDeviceService = new MQTTDeviceService(mqttClient, 4, 3);

void setup() {
  Serial.begin(9600);
  setupWifiConnection(WIFI_SSID, WIFI_PASSWORD); 
  client.begin(MQTT_BROKER, MQTT_PORT, espClient);
  client.onMessage(messageReceived);
  
  mqttClient -> setVerbose(true);
  rgbLedStrip -> setVerbose(true);
  mqttClient -> setupClient(&client);

  mqttDeviceService -> setResetStateConsumer(resetSwitch);
  
  mqttDeviceService -> addPublisher(devicePing);
  mqttDeviceService -> addStateConsumer(rgbLedStrip);
  
  mqttDeviceService -> setupMQTTDevices();
}

void loop() {
  checkWifiStatus(WIFI_SSID, WIFI_PASSWORD);
  mqttDeviceService -> executeLoop();
  delay(50);
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  mqttDeviceService -> handleMessage(topic, payload);
}
