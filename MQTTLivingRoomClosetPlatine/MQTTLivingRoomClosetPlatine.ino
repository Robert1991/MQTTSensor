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

char HUMIDITY_STATE_TOPIC[] = "living_room/closet_humidity";
char TEMPERATURE_STATE_TOPIC[] = "living_room/closet_temperature";
char MOTION_SENSOR_STATE_TOPIC[] = "living_room/closet_motion";

char RGB_LIGHT_STATE_TOPIC[] = "living_room/closet_light/status";
char RGB_LIGHT_SWITCH_SUBSCRIPTION[] = "living_room/closet_light/switch";
char RGB_LIGHT_BRIGHTNESS_SUBSCRIPTION[] = "living_room/closet_light/brightness/set";
char RGB_LIGHT_COLOR_SET_SUBSCRIPTION[] = "living_room/closet_light/color/set";
char RGB_LIGHT_COLOR_SET_STATE_TOPIC[] = "living_room/closet_light/color/status";
char RGB_LIGHT_BRIGHTNESS_STATE_TOPIC[] = "living_room/closet_light/brightness/status";
RGBPins stripPins = {LED_STRIP_RED_PIN, LED_STRIP_GREEN_PIN, LED_STRIP_BLUE_PIN};
MQTTRgbLightConfiguration stripConfig = {stripPins, RGB_LIGHT_STATE_TOPIC, RGB_LIGHT_SWITCH_SUBSCRIPTION, RGB_LIGHT_BRIGHTNESS_SUBSCRIPTION, RGB_LIGHT_BRIGHTNESS_STATE_TOPIC,
  RGB_LIGHT_COLOR_SET_SUBSCRIPTION, RGB_LIGHT_COLOR_SET_STATE_TOPIC};
MQTTRgbLight* rgbLedStrip = new MQTTRgbLight(stripConfig);

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHT_PIN, DHTTYPE);
MQTTClient* mqttClient = new MQTTClient(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD);

MQTTMotionSensor* motionSensor = new MQTTMotionSensor(mqttClient, MOTION_SENSOR_STATE_TOPIC, MOTION_SENSOR_PIN);
MQTTDhtSensor* dhtSensor = new MQTTDhtSensor(mqttClient, &dht, TEMPERATURE_STATE_TOPIC, HUMIDITY_STATE_TOPIC);

void setup() {
  Serial.begin(9600);
  motionSensor -> setupSensor();
  dhtSensor -> setupSensor();
  
  mqttClient -> setVerbose(true);  
  setupWifiConnection(WIFI_SSID, WIFI_PASSWORD);
  rgbLedStrip -> setupActor(mqttClient);
  
  mqttClient -> setupClient(&client, MQTT_BROKER, MQTT_PORT);
  client.setCallback(callback);
}

void loop() {
  motionSensor -> publishMeasurement();
  dhtSensor -> publishMeasurement();
  mqttClient -> loopClient();
  rgbLedStrip -> applyChoosenColorToLeds();
  delay(200);
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
  rgbLedStrip -> consumeMessage(mqttClient, topicTemp, messageTemp);
}
