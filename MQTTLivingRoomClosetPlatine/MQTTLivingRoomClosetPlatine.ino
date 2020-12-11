#include <IOTClients.h>
#include <MQTTDevices.h>
#include "credentials.h"
#include "DHT.h"

#define DHT_PIN D1
#define DHTTYPE DHT11
#define MOTION_SENSOR_PIN D7
#define LED_STRIP_RED_PIN D5
#define LED_STRIP_GREEN_PIN D2
#define LED_STRIP_BLUE_PIN D6

WiFiClient espClient;
MQTTClient client(750);

MessageQueueClient* mqttClient = new MessageQueueClient(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD);

MQTTDeviceInfo deviceInfo = {"9EgmS4" ,"living_room_sensor_actor_1", "homeassistant", "Node MCU", "RoboTronix"};
MQTTDevicePing* devicePing = new MQTTDevicePing(deviceInfo, "lnwoRM", 30000);
MQTTDeviceResetSwitch* resetSwitch = new MQTTDeviceResetSwitch(deviceInfo, "JMhc8L");

MQTTMotionSensor* motionSensor = new MQTTMotionSensor(deviceInfo, "VM5ts3", MOTION_SENSOR_PIN, 15, 20, 9);

DHT dht(DHT_PIN, DHTTYPE);
MQTTHumiditySensor* humiditySensor = new MQTTHumiditySensor(deviceInfo, &dht, "2KGqAK");
MQTTTemperatureSensor* temperatureSensor = new MQTTTemperatureSensor(deviceInfo, &dht, "VxgFPM");

RGBPins stripPins = {LED_STRIP_RED_PIN, LED_STRIP_GREEN_PIN, LED_STRIP_BLUE_PIN};
MQTTRgbLight* rgbLight = new MQTTRgbLight(deviceInfo, "L3Pc5A", stripPins);

MQTTDeviceService* mqttDeviceService = new MQTTDeviceService(mqttClient, 6, 2);

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
  mqttDeviceService -> addPublisher(temperatureSensor);
  mqttDeviceService -> addStateConsumer(rgbLight);
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
