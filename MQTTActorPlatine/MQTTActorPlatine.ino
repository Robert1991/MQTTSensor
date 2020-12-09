#include <IOTClients.h>
#include <MQTTDevices.h>
#include "credentials.h"

#define RELAIS_PIN D1
#define LED_STRIP_RED_PIN D5
#define LED_STRIP_GREEN_PIN D2
#define LED_STRIP_BLUE_PIN D6
//#define DOOR_SENSOR_PIN D1

WiFiClient espClient;
MQTTClient client(750);
MessageQueueClient* mqttClient = new MessageQueueClient(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD);

MQTTDeviceInfo deviceInfo = {"yUonzt" ,"kitchen_actor_1", "homeassistant", "Node MCU", "RoboTronix"};
MQTTDevicePing* devicePing = new MQTTDevicePing(deviceInfo,"wIJTeY", 30000);
MQTTDeviceResetSwitch* resetSwitch = new MQTTDeviceResetSwitch(deviceInfo, "DqBPnU");  

MQTTSwitch* relaisLight = new MQTTSwitch(deviceInfo, "7fZczI", RELAIS_PIN);

RGBPins stripPins = {LED_STRIP_RED_PIN, LED_STRIP_GREEN_PIN, LED_STRIP_BLUE_PIN};
MQTTRgbLight* rgbLight = new MQTTRgbLight(deviceInfo, "5n8s0e", stripPins);

MQTTDeviceService* mqttDeviceService = new MQTTDeviceService(mqttClient, 4, 3);

void setup() {
  //Serial.begin(9600);
  //pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);
  
  setupWifiConnection(WIFI_SSID, WIFI_PASSWORD); 
  client.begin(MQTT_BROKER, MQTT_PORT, espClient);
  client.onMessage(messageReceived);
  
  //mqttClient -> setVerbose(true);
  mqttClient -> setupClient(&client);

  mqttDeviceService -> setResetStateConsumer(resetSwitch);
  
  mqttDeviceService -> addPublisher(devicePing);
  mqttDeviceService -> addStateConsumer(relaisLight);
  mqttDeviceService -> addStateConsumer(rgbLight);
  
  mqttDeviceService -> setupMQTTDevices();
}

void loop() {
  checkWifiStatus(WIFI_SSID, WIFI_PASSWORD);
  mqttDeviceService -> executeLoop();
  
  //int doorState = digitalRead(DOOR_SENSOR_PIN);
  //if(doorState == HIGH) {
//    Serial.println("Open");
//  } else {
    //Serial.println("Closed");
//  }

  delay(50);
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  mqttDeviceService -> handleMessage(topic, payload);
}
