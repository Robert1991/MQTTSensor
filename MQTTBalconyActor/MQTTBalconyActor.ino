#include <IOTClients.h>
#include <MQTTDevices.h>
#include "credentials.h"

#define DHT_PIN D3
#define DHTTYPE DHT22

#define RELAIS_PIN_1 D1
#define RELAIS_PIN_2 D2

WiFiClient espClient;
MQTTClient client(750);
MessageQueueClient* mqttClient = new MessageQueueClient(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD);

MQTTDeviceInfo deviceInfo = {"NStlwd" ,"balcony_sensor_actor_1", "homeassistant", "Node MCU", "RoboTronix"};
MQTTDevicePing* devicePing = new MQTTDevicePing(deviceInfo, "wJvJZH", 30000);
MQTTDeviceResetSwitch* resetSwitch = new MQTTDeviceResetSwitch(deviceInfo, "pshrng");

DHT dht(DHT_PIN, DHTTYPE);
MQTTHumiditySensor* humiditySensor = new MQTTHumiditySensor(deviceInfo, &dht, "7CTb5m");
MQTTTemperatureSensor* temperatureSensor = new MQTTTemperatureSensor(deviceInfo, &dht, "8FiVDk");

MQTTSwitch* relaisLight1 = new MQTTSwitch(deviceInfo, "OqxzUZ", RELAIS_PIN_1, "relais_light_1");
MQTTSwitch* relaisLight2 = new MQTTSwitch(deviceInfo, "k9L2sA", RELAIS_PIN_2, "relais_light_2");

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
  mqttDeviceService -> addPublisher(humiditySensor);
  mqttDeviceService -> addPublisher(temperatureSensor);
  mqttDeviceService -> addStateConsumer(relaisLight1);
  mqttDeviceService -> addStateConsumer(relaisLight2);
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
