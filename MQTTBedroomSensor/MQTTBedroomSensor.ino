#include <IOTClients.h>
#include <MQTTDevices.h>
#include "credentials.h"


#define DHT_PIN D4
#define DHTTYPE DHT22
#define PHOTO_SENSOR_PIN A0
#define MOTION_SENSOR_PIN D3
#define DOOR_SENSOR_PIN D2

WiFiClient espClient;
MQTTClient client(750);
MessageQueueClient* mqttClient = new MessageQueueClient(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD);

MQTTDeviceInfo deviceInfo = {"yPx7nV" ,"bedroom_sensor_1", "homeassistant", "Node MCU D1 Mini", "RoboTronix"};
MQTTDeviceResetSwitch* resetSwitch = new MQTTDeviceResetSwitch(deviceInfo, "xizwVB");
MQTTDevicePing* devicePing = new MQTTDevicePing(deviceInfo, "Q6YALb", 30000);

MQTTDoorSensor* doorSensor = new MQTTDoorSensor(deviceInfo, "14viy7", DOOR_SENSOR_PIN);
MQTTPhotoLightSensor* photoSensor = new MQTTPhotoLightSensor(deviceInfo, "lPYQso", PHOTO_SENSOR_PIN);
MQTTMotionSensor* motionSensor = new MQTTMotionSensor(deviceInfo, "ZcaJoY", MOTION_SENSOR_PIN, 10, 10, 4);

DHT dht(DHT_PIN, DHTTYPE);
MQTTHumiditySensor* humiditySensor = new MQTTHumiditySensor(deviceInfo, &dht, "uLE9b9");
MQTTTemperatureSensor* temperatureSensor = new MQTTTemperatureSensor(deviceInfo, &dht, "BOVhu3");

MQTTDeviceService* mqttDeviceService = new MQTTDeviceService(mqttClient, 8, 2);

void setup() {
  // Turn off onboard led
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  
  setupWifiConnection(WIFI_SSID, WIFI_PASSWORD); 
  
  client.begin(MQTT_BROKER, MQTT_PORT, espClient);
  client.onMessage(messageReceived);

  mqttClient -> setupClient(&client);
  
  mqttDeviceService -> setResetStateConsumer(resetSwitch);
  mqttDeviceService -> addPublisher(devicePing);
  mqttDeviceService -> addPublisher(doorSensor);
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
