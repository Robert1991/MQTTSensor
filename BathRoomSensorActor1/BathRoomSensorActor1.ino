#include <DeviceRuntime.h>

#define RELAIS_PIN D1

#define LED_STRIP_RED_PIN D5
#define LED_STRIP_GREEN_PIN D2
#define LED_STRIP_BLUE_PIN D6
#define FLASH_BUTTON_PIN 0

#define DHT_PIN D8
#define DHTTYPE DHT22

const int BUILD_NUMBER = 0;
const String DEVICE_ID = "bb69Mc";
const String DEVICE_PING_ID = "drkmge";
const String DEVICE_RESET_SWITCH_ID = "0Em8Zf";

WiFiClient espClient;
DHT dht(DHT_PIN, DHTTYPE);

void setup() {
  // Serial.begin(9600);
  setupDevice(espClient, DEVICE_ID, BUILD_NUMBER, FLASH_BUTTON_PIN, DEVICE_PING_ID, DEVICE_RESET_SWITCH_ID,
              setupMqttSensorActors);
}

void setupMqttSensorActors() {
  MQTTSwitch *relaisLight = new MQTTSwitch(getMQTTDeviceInfo(), "6DdPuF", RELAIS_PIN);
  registerMQTTDevice(relaisLight);
  RGBPins stripPins = {LED_STRIP_RED_PIN, LED_STRIP_GREEN_PIN, LED_STRIP_BLUE_PIN};
  MQTTRgbLight *rgbLight = new MQTTRgbLight(getMQTTDeviceInfo(), "xd4WYG", stripPins);
  registerMQTTDevice(rgbLight);
  MQTTHumiditySensor *humiditySensor = new MQTTHumiditySensor(deviceInfo, &dht, "2qYldn");
  registerMQTTDevice(humiditySensor);
  MQTTTemperatureSensor *temperatureSensor = new MQTTTemperatureSensor(deviceInfo, &dht, "V5qgll");
  registerMQTTDevice(temperatureSensor);
}

void loop() { loopDevice(200); }
