#include <IOTClients.h>
#include <MQTTDevices.h>
#include "credentials.h"

#define RELAIS_PIN 0

WiFiClient espClient;
MQTTClient client(750);
MessageQueueClient* mqttClient = new MessageQueueClient(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD);

MQTTDeviceInfo deviceInfo = {"JNDHbc" ,"kitchen_ceiling_light_1", "homeassistant", "ESP-01S", "RoboTronix"};
MQTTDevicePing* devicePing = new MQTTDevicePing(deviceInfo, "Fz3npn", 30000);
MQTTDeviceResetSwitch* resetSwitch = new MQTTDeviceResetSwitch(deviceInfo, "DvnRhr");

MQTTSwitch* relaisLight = new MQTTSwitch(deviceInfo, "T1VMmW", RELAIS_PIN, "relais_light_1");

MQTTDeviceService* mqttDeviceService = new MQTTDeviceService(mqttClient, 6, 3);

void setup() {
  //Serial.begin(9600);
  setupWifiConnection(WIFI_SSID, WIFI_PASSWORD); 
  
  client.begin(MQTT_BROKER, MQTT_PORT, espClient);
  client.onMessage(messageReceived);

  mqttClient -> setupClient(&client);
  //mqttClient -> setVerbose(true);
  
  mqttDeviceService -> setResetStateConsumer(resetSwitch);
  mqttDeviceService -> addPublisher(devicePing);
  mqttDeviceService -> addStateConsumer(relaisLight);
  mqttDeviceService -> setupMQTTDevices();
}

void loop() {
  checkWifiStatus(WIFI_SSID, WIFI_PASSWORD);
  mqttDeviceService -> executeLoop();
  delay(100);
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  mqttDeviceService -> handleMessage(topic, payload);
}
