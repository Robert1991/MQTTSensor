#include <MQTTDevices.h>
#include <wireUtils.h>
#include "credentials.h"
#include "DHT.h"

#define DHT_PIN D3
#define DHTTYPE DHT11
#define MOTION_SENSOR_PIN D7

#define SDA D1
#define SCL D2

#define I2C_SLAVE_ADDRESS 8
#define ON_OFF_COMMAND 0
#define SET_COLOR_COMMAND 1
#define SET_BRIGHTNESS_COMMAND 2

WiFiClient espClient;
MQTTClient client(750);

MessageQueueClient* mqttClient = new MessageQueueClient(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD);

MQTTDeviceInfo deviceInfo = {"5soti6" ,"floor_sensor_actor_1", "homeassistant", "Node MCU", "RoboTronix"};
MQTTDevicePing* devicePing = new MQTTDevicePing(deviceInfo, "TRAjq2", 30000);
MQTTDeviceResetSwitch* resetSwitch = new MQTTDeviceResetSwitch(deviceInfo, "ogm8e8");
MQTTMotionSensor* motionSensor = new MQTTMotionSensor(deviceInfo, "ldOfO7", MOTION_SENSOR_PIN, 15, 20, 9);

DHT dht(DHT_PIN, DHTTYPE);
MQTTHumiditySensor* humiditySensor = new MQTTHumiditySensor(deviceInfo, &dht, "ST17k2");
MQTTTemperatureSensor* temperatureSensor = new MQTTTemperatureSensor(deviceInfo, &dht, "Fzwtrt");

WirePinSet wireConfiguration = {I2C_SLAVE_ADDRESS, SDA, SCL};
MQTTRgbLightI2CCommands connectionCommands = {ON_OFF_COMMAND, SET_COLOR_COMMAND, SET_BRIGHTNESS_COMMAND};
MQTTI2CRgbLightConfiguration stripConfig = {connectionCommands, wireConfiguration};
MQTTI2CRgbLight* rgbLedStrip = new MQTTI2CRgbLight(deviceInfo, "26vFbD", stripConfig);

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
