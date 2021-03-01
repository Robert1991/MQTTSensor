#include <DeviceRuntime.h>

#define DHT_PIN D1
#define DHTTYPE DHT11
#define MOTION_SENSOR_PIN D7
#define LED_STRIP_RED_PIN D5
#define LED_STRIP_GREEN_PIN D2
#define LED_STRIP_BLUE_PIN D6

#define FLASH_BUTTON_PIN 0

const int BUILD_NUMBER = 4;
const String DEVICE_ID = "9EgmS4";
const String DEVICE_PING_ID = "lnwoRM";
const String DEVICE_RESET_SWITCH_ID = "JMhc8L";

DHT dht(DHT_PIN, DHTTYPE);
WiFiClient espClient;

void setup() {
  // Serial.begin(9600);
  setupDevice(espClient, DEVICE_ID, BUILD_NUMBER, FLASH_BUTTON_PIN, DEVICE_PING_ID, DEVICE_RESET_SWITCH_ID,
              setupMqttSensorActors);
}

void setupMqttSensorActors() {
  MQTTMotionSensor *motionSensor = new MQTTMotionSensor(deviceInfo, "VM5ts3", MOTION_SENSOR_PIN, 15, 20, 9);
  registerMQTTDevice(motionSensor);
  MQTTHumiditySensor *humiditySensor = new MQTTHumiditySensor(deviceInfo, &dht, "2KGqAK");
  registerMQTTDevice(humiditySensor);
  MQTTTemperatureSensor *temperatureSensor = new MQTTTemperatureSensor(deviceInfo, &dht, "VxgFPM");
  registerMQTTDevice(temperatureSensor);

  RGBPins stripPins = {LED_STRIP_RED_PIN, LED_STRIP_GREEN_PIN, LED_STRIP_BLUE_PIN};
  MQTTRgbLight *rgbLight = new MQTTRgbLight(deviceInfo, "L3Pc5A", stripPins);
  registerMQTTDevice(rgbLight);
}
void loop() { loopDevice(300); }
