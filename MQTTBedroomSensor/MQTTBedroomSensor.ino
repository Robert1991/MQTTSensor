#include <IOTClients.h>
#include <MQTTDevices.h>
#include "credentials.h"

#define DHT_PIN 2
#define DHTTYPE DHT11
#define PHOTO_SENSOR_PIN A0
#define MOTION_SENSOR_PIN D3

WiFiClient espClient;
MQTTClient client(750);
MessageQueueClient* mqttClient = new MessageQueueClient(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD);

MQTTDeviceInfo deviceInfo = {"yPx7nV" ,"bedroom_sensor_1", "homeassistant", "Node MCU D1 Mini", "RoboTronix"};
MQTTDevicePing* devicePing = new MQTTDevicePing(deviceInfo, "Q6YALb", 30000);
MQTTPhotoLightSensor* photoSensor = new MQTTPhotoLightSensor(deviceInfo, "lPYQso", PHOTO_SENSOR_PIN);
MQTTMotionSensor* motionSensor = new MQTTMotionSensor(deviceInfo, "ZcaJoY", MOTION_SENSOR_PIN, 10, 10, 4);

DHT dht(DHT_PIN, DHTTYPE);
MQTTHumiditySensor* humiditySensor = new MQTTHumiditySensor(deviceInfo, &dht, "uLE9b9");
MQTTTemperatureSensor* temperatureSensor = new MQTTTemperatureSensor(deviceInfo, &dht, "BOVhu3");

MQTTDeviceService* mqttDeviceService = new MQTTDeviceService(mqttClient, 6, 1);

void setup() {
  setupWifiConnection(WIFI_SSID, WIFI_PASSWORD); 
  
  client.begin(MQTT_BROKER, MQTT_PORT, espClient);
  client.onMessage(messageReceived);

  mqttClient -> setupClient(&client);
  
  mqttDeviceService -> addPublisher(devicePing);
  mqttDeviceService -> addPublisher(photoSensor);
  mqttDeviceService -> addPublisher(motionSensor);
  mqttDeviceService -> addPublisher(humiditySensor);
  mqttDeviceService -> addPublisher(temperatureSensor);
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
