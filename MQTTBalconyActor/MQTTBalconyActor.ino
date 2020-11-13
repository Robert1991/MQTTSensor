#include <IOTClients.h>
#include <MQTTDevices.h>
#include "credentials.h"

#define DHT_PIN D3
#define DHTTYPE DHT11

#define RELAIS_PIN_1 D1
#define RELAIS_PIN_2 D2

char HUMIDITY_STATE_TOPIC[] = "balcony/humidity";
char TEMPERATURE_STATE_TOPIC[] = "balcony/temperature";

char RELAIS_LIGHT_SWITCH_SUBSCRIPTION_TOPIC_1[] = "balcony/relais_light_1/switch";
char RELAIS_LIGHT_SWITCH_STATE_TOPIC_1[] = "balcony/relais_light_1/status";

char RELAIS_LIGHT_SWITCH_SUBSCRIPTION_TOPIC_2[] = "balcony/relais_light_2/switch";
char RELAIS_LIGHT_SWITCH_STATE_TOPIC_2[] = "balcony/relais_light_2/status";


MQTTSwitchConfiguration RELAIS_SWITCH_CONFIGURATION_1 = {RELAIS_LIGHT_SWITCH_SUBSCRIPTION_TOPIC_1, RELAIS_LIGHT_SWITCH_STATE_TOPIC_1, RELAIS_PIN_1};
MQTTSwitch* relaisSwitch_1 = new MQTTSwitch(RELAIS_SWITCH_CONFIGURATION_1);

MQTTSwitchConfiguration RELAIS_SWITCH_CONFIGURATION_2 = {RELAIS_LIGHT_SWITCH_SUBSCRIPTION_TOPIC_2, RELAIS_LIGHT_SWITCH_STATE_TOPIC_2, RELAIS_PIN_2};
MQTTSwitch* relaisSwitch_2 = new MQTTSwitch(RELAIS_SWITCH_CONFIGURATION_2);

WiFiClient espClient;
PubSubClient client(espClient);
MQTTClient* mqttClient = new MQTTClient(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD);

DHT dht(DHT_PIN, DHTTYPE);
MQTTDhtSensor* dhtSensor = new MQTTDhtSensor(mqttClient, &dht, TEMPERATURE_STATE_TOPIC, HUMIDITY_STATE_TOPIC);

void setup() {
  Serial.begin(9600);
  setupWifiConnection(WIFI_SSID, WIFI_PASSWORD);

  relaisSwitch_1 -> setupActor(mqttClient);
  relaisSwitch_1 -> setVerbose(true);

  relaisSwitch_2 -> setupActor(mqttClient);
  relaisSwitch_2 -> setVerbose(true);

  dhtSensor -> setupSensor();
  
  mqttClient -> setVerbose(true);
  mqttClient -> setupClient(&client, MQTT_BROKER, MQTT_PORT);
  client.setCallback(callback);
}

void loop() {
  checkWifiStatus(WIFI_SSID, WIFI_PASSWORD);
 if (mqttClient -> loopClient()) {
    dhtSensor -> publishMeasurement();
    relaisSwitch_1 -> applySwitchStatus();
    relaisSwitch_2 -> applySwitchStatus();
  } else {
    dhtSensor -> reset();
  }
  delay(150);
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
  relaisSwitch_1 -> consumeMessage(mqttClient, topicTemp, messageTemp);
  relaisSwitch_2 -> consumeMessage(mqttClient, topicTemp, messageTemp);
}
