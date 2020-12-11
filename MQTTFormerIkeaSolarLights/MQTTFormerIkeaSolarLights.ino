#include <IOTClients.h>
#include <MQTTDevices.h>
#include "credentials.h"

#define PHOTO_SENSOR_PIN A0

WiFiClient espClient;
MQTTClient client(750);
MessageQueueClient* mqttClient = new MessageQueueClient(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD);

MQTTDeviceInfo deviceInfo = {"XT9Ohl" ,"former_ikea_light", "homeassistant", "Node MCU", "RoboTronix"};
MQTTDevicePing* devicePing = new MQTTDevicePing(deviceInfo,"UzBJqg", 30000);
MQTTDeviceResetSwitch* resetSwitch = new MQTTDeviceResetSwitch(deviceInfo, "k6lnfO");
MQTTSwitch* transistorLight = new MQTTSwitch(deviceInfo, "ODQ7Jt", D3);
MQTTPhotoLightSensor* photoSensor = new MQTTPhotoLightSensor(deviceInfo,"zMvxqy", PHOTO_SENSOR_PIN);

MQTTDeviceService* mqttDeviceService = new MQTTDeviceService(mqttClient, 4, 2);

void setup() {
  //Serial.begin(9600);
  setupWifiConnection(WIFI_SSID, WIFI_PASSWORD); 
  
  client.begin(MQTT_BROKER, MQTT_PORT, espClient);
  client.onMessage(messageReceived);

  mqttClient -> setupClient(&client);
  //mqttClient -> setVerbose(true);
  mqttDeviceService -> setResetStateConsumer(resetSwitch);
  mqttDeviceService -> addPublisher(photoSensor);
  mqttDeviceService -> addPublisher(devicePing);
  mqttDeviceService -> addStateConsumer(transistorLight);
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
