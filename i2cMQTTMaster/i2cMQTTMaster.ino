#include "credentials.h"
#include "IOTClients.h"
#include <MQTTDevices.h>

#define PHOTO_SENSOR_PIN A0
#define DOOR_SENSOR_PIN D1
#define MOTION_SENSOR_PIN D2
#define DHT_PIN D3
#define DHTTYPE DHT11

WiFiClient espClient;
MQTTClient client(750);
MessageQueueClient* mqttClient = new MessageQueueClient(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD);

MQTTDeviceInfo deviceInfo = {"2VgWWW" ,"kitchen_sensor_1", "homeassistant", "Node MCU", "RoboTronix"};
MQTTDevicePing* devicePing = new MQTTDevicePing(deviceInfo,"urT3UA", 30000);
MQTTDeviceResetSwitch* resetSwitch = new MQTTDeviceResetSwitch(deviceInfo, "SJGZoR");

MQTTDoorSensor* doorSensor = new MQTTDoorSensor(deviceInfo, "lEqFzg", DOOR_SENSOR_PIN);
MQTTMotionSensor* motionSensor = new MQTTMotionSensor(deviceInfo, "ywseEo", MOTION_SENSOR_PIN, 15, 20, 9);
MQTTPhotoLightSensor* photoSensor = new MQTTPhotoLightSensor(deviceInfo, "ulHFRc", PHOTO_SENSOR_PIN);
DHT dht(DHT_PIN, DHTTYPE);
MQTTHumiditySensor* humiditySensor = new MQTTHumiditySensor(deviceInfo, &dht, "p1uDD4");
MQTTTemperatureSensor* temperatureSensor = new MQTTTemperatureSensor(deviceInfo, &dht, "czt75Q");

MQTTDeviceService* mqttDeviceService = new MQTTDeviceService(mqttClient, 8, 1);

void setup() {
  //Serial.begin(9600);
  setupWifiConnection(WIFI_SSID, WIFI_PASSWORD); 
  client.begin(MQTT_BROKER, MQTT_PORT, espClient);
  client.onMessage(messageReceived);
  
  //mqttClient -> setVerbose(true);
  mqttClient -> setupClient(&client);

  mqttDeviceService -> setResetStateConsumer(resetSwitch);
  mqttDeviceService -> addPublisher(devicePing);
  mqttDeviceService -> addPublisher(motionSensor);
  mqttDeviceService -> addPublisher(humiditySensor);
  mqttDeviceService -> addPublisher(photoSensor);
  mqttDeviceService -> addPublisher(temperatureSensor);  
  mqttDeviceService -> addPublisher(doorSensor);
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
