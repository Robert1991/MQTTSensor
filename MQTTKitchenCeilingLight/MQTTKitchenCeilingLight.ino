#include <IOTClients.h>
#include <MQTTDevices.h>
#include "credentials.h"

#define RELAIS_PIN 0

char RELAIS_LIGHT_SWITCH_SUBSCRIPTION_TOPIC[] = "kitchen/ceiling_light/set";
char RELAIS_LIGHT_SWITCH_STATE_TOPIC[] = "kitchen/ceiling_light/status";

MQTTSwitchConfiguration RELAIS_SWITCH_CONFIGURATION = {RELAIS_LIGHT_SWITCH_SUBSCRIPTION_TOPIC, RELAIS_LIGHT_SWITCH_STATE_TOPIC, RELAIS_PIN};
MQTTSwitch* relaisSwitch = new MQTTSwitch(RELAIS_SWITCH_CONFIGURATION);

WiFiClient espClient;
PubSubClient client(espClient);
MQTTClient* mqttClient = new MQTTClient(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD);

void setup() {
  Serial.begin(9600);
  setupWifiConnection(WIFI_SSID, WIFI_PASSWORD);
  relaisSwitch -> setupActor(mqttClient);
  mqttClient -> setupClient(&client, MQTT_BROKER, MQTT_PORT);
  client.setCallback(callback);

  relaisSwitch -> executeDefaultAction(mqttClient);
}

void loop() {
  checkWifiStatus(WIFI_SSID, WIFI_PASSWORD);
  mqttClient -> loopClient();
  relaisSwitch -> applySwitchStatus();
  delay(50);
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  String topicTemp(topic);
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  relaisSwitch -> consumeMessage(mqttClient, topicTemp, messageTemp);
}
