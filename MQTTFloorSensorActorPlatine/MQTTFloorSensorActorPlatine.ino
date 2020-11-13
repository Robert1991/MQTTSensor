#include <MQTTDevices.h>
#include <wireUtils.h>
#include "credentials.h"
#include "DHT.h"

#define DHT_PIN D3
#define DHTTYPE DHT11
#define MOTION_SENSOR_PIN D7
#define LED_STRIP_RED_PIN D5
#define LED_STRIP_GREEN_PIN D1
#define LED_STRIP_BLUE_PIN D6

extern "C" {
#include "user_interface.h"
}

char HUMIDITY_STATE_TOPIC[] = "floor/humidity";
char TEMPERATURE_STATE_TOPIC[] = "floor/temperature";
char MOTION_SENSOR_STATE_TOPIC[] = "floor/motion";

char RGB_LIGHT_STATE_TOPIC[] = "floor/light_strip/status";
char RGB_LIGHT_SWITCH_SUBSCRIPTION[] = "floor/light_strip/switch";
char RGB_LIGHT_BRIGHTNESS_SUBSCRIPTION[] = "floor/light_strip/brightness/set";
char RGB_LIGHT_COLOR_SET_SUBSCRIPTION[] = "floor/light_strip/color/set";
char RGB_LIGHT_COLOR_SET_STATE_TOPIC[] = "floor/light_strip/color/status";
char RGB_LIGHT_BRIGHTNESS_STATE_TOPIC[] = "floor/light_strip/brightness/status";

#define I2C_SLAVE_ADDRESS 8
#define ON_OFF_COMMAND 0
#define SET_COLOR_COMMAND 1
#define SET_BRIGHTNESS_COMMAND 2

#define SDA D1
#define SCL D2

WirePinSet wireConfiguration = {I2C_SLAVE_ADDRESS, SDA, SCL};
MQTTRgbLightI2CCommands connectionCommands = {ON_OFF_COMMAND, SET_COLOR_COMMAND, SET_BRIGHTNESS_COMMAND};

MQTTI2CRgbLightConfiguration stripConfig = {connectionCommands, wireConfiguration, RGB_LIGHT_STATE_TOPIC, RGB_LIGHT_SWITCH_SUBSCRIPTION, RGB_LIGHT_BRIGHTNESS_SUBSCRIPTION, RGB_LIGHT_BRIGHTNESS_STATE_TOPIC,
  RGB_LIGHT_COLOR_SET_SUBSCRIPTION, RGB_LIGHT_COLOR_SET_STATE_TOPIC};
MQTTI2CRgbLight* rgbLedStrip = new MQTTI2CRgbLight(stripConfig);

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHT_PIN, DHTTYPE);
MQTTClient* mqttClient = new MQTTClient(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD);

MQTTMotionSensor* motionSensor = new MQTTMotionSensor(mqttClient, MOTION_SENSOR_STATE_TOPIC, MOTION_SENSOR_PIN);
MQTTDhtSensor* dhtSensor = new MQTTDhtSensor(mqttClient, &dht, TEMPERATURE_STATE_TOPIC, HUMIDITY_STATE_TOPIC);

void setup() {
  Serial.begin(9600);

  mqttClient -> setVerbose(true);
  setupWifiConnection(WIFI_SSID, WIFI_PASSWORD); 
  
  rgbLedStrip -> setupActor(mqttClient);
  rgbLedStrip -> setVerbose(false);
  motionSensor -> setupSensor();
  motionSensor -> setVerbose(true);
  dhtSensor -> setupSensor();
  dhtSensor -> setVerbose(true);
  
  mqttClient -> setupClient(&client, MQTT_BROKER, MQTT_PORT);
  client.setCallback(callback);
}

void loop() {
  checkWifiStatus(WIFI_SSID, WIFI_PASSWORD);
  if (mqttClient -> loopClient()) {
    motionSensor -> publishMeasurement();
    dhtSensor -> publishMeasurement();  
  } else {
    dhtSensor -> reset();
    motionSensor -> reset();
  }
  
  delay(100);
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
