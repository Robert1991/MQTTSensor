#include <DeviceRuntime.h>

#define RELAIS_PIN D1
#define LED_STRIP_RED_PIN D5
#define LED_STRIP_GREEN_PIN D2
#define LED_STRIP_BLUE_PIN D6
#define FLASH_BUTTON_PIN 0

const int BUILD_NUMBER = 0;
const String DEVICE_ID = "yUonzt";
const String DEVICE_PING_ID = "wIJTeY";
const String DEVICE_RESET_SWITCH_ID = "DqBPnU";

WiFiClient espClient;

void setup() {
  // Serial.begin(9600);
  setupDevice(espClient, DEVICE_ID, BUILD_NUMBER, FLASH_BUTTON_PIN, DEVICE_PING_ID, DEVICE_RESET_SWITCH_ID,
              setupMqttSensorActors);
}
void setupMqttSensorActors() {
  MQTTSwitch *relaisLight = new MQTTSwitch(getMQTTDeviceInfo(), "7fZczI", RELAIS_PIN);
  registerMQTTDevice(relaisLight);
  RGBPins stripPins = {LED_STRIP_RED_PIN, LED_STRIP_GREEN_PIN, LED_STRIP_BLUE_PIN};
  MQTTRgbLight *rgbLight = new MQTTRgbLight(getMQTTDeviceInfo(), "5n8s0e", stripPins);
  registerMQTTDevice(rgbLight);
}

void loop() { loopDevice(200); }
