#include <IOTClients.h>
#include <MQTTDevices.h>
#include "credentials.h"

#define RELAIS_PIN D1
#define LED_STRIP_RED_PIN D5
#define LED_STRIP_GREEN_PIN D1
#define LED_STRIP_BLUE_PIN D6

char RELAIS_LIGHT_SWITCH_SUBSCRIPTION_TOPIC[] = "kitchen/relais_light/set";
char RELAIS_LIGHT_SWITCH_STATE_TOPIC[] = "kitchen/relais_light/status";

MQTTSwitchConfiguration RELAIS_SWITCH_CONFIGURATION = {RELAIS_LIGHT_SWITCH_SUBSCRIPTION_TOPIC, RELAIS_LIGHT_SWITCH_STATE_TOPIC, RELAIS_PIN};
MQTTSwitch* relaisSwitch = new MQTTSwitch(RELAIS_SWITCH_CONFIGURATION);

char RGB_LIGHT_STATE_TOPIC[] = "kitchen/light1/status";
char RGB_LIGHT_SWITCH_SUBSCRIPTION[] = "kitchen/light1/switch";
char RGB_LIGHT_BRIGHTNESS_SUBSCRIPTION[] = "kitchen/light1/brightness/set";
char RGB_LIGHT_COLOR_SET_SUBSCRIPTION[] = "kitchen/light1/color/set";
char RGB_LIGHT_COLOR_SET_STATE_TOPIC[] = "kitchen/light1/color/status";
char RGB_LIGHT_BRIGHTNESS_STATE_TOPIC[] = "kitchen/light1/brightness/status";

RGBPins stripPins = {LED_STRIP_RED_PIN, LED_STRIP_GREEN_PIN, LED_STRIP_BLUE_PIN};
MQTTRgbLightConfiguration stripConfig = {stripPins, RGB_LIGHT_STATE_TOPIC, RGB_LIGHT_SWITCH_SUBSCRIPTION, RGB_LIGHT_BRIGHTNESS_SUBSCRIPTION, RGB_LIGHT_BRIGHTNESS_STATE_TOPIC,
  RGB_LIGHT_COLOR_SET_SUBSCRIPTION, RGB_LIGHT_COLOR_SET_STATE_TOPIC};
MQTTRgbLight* rgbLedStrip = new MQTTRgbLight(stripConfig);

WiFiClient espClient;
PubSubClient client(espClient);
MQTTClient* mqttClient = new MQTTClient(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD);

void setup() {
  Serial.begin(9600);
  setupWifiConnection(WIFI_SSID, WIFI_PASSWORD);

  relaisSwitch -> setupActor(mqttClient);
  rgbLedStrip -> setupActor(mqttClient);
  
  mqttClient -> setupClient(&client, MQTT_BROKER, MQTT_PORT);
  client.setCallback(callback);

  rgbLedStrip ->executeDefaultAction(mqttClient);
}

void loop() {
  checkWifiStatus(WIFI_SSID, WIFI_PASSWORD);
  mqttClient -> loopClient();
  delay(50);
  relaisSwitch -> applySwitchStatus();
  rgbLedStrip -> applyChoosenColorToLeds();
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
  rgbLedStrip -> consumeMessage(mqttClient, topicTemp, messageTemp);
}
