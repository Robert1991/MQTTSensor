#include <DeviceRuntime.h>

#define SDA D1
#define SCL D2

#define I2C_SLAVE_ADDRESS 8
#define ON_OFF_COMMAND 0
#define SET_COLOR_COMMAND 1
#define SET_BRIGHTNESS_COMMAND 2

#define FLASH_BUTTON_PIN 0

const int BUILD_NUMBER = 11;
const String DEVICE_ID = "xl2H1Q";
const String DEVICE_PING_ID = "Du6v7I";
const String DEVICE_RESET_SWITCH_ID = "xqyNMW";

WiFiClient espClient;

void setup() {
  // Serial.begin(9600);
  setupDevice(espClient, DEVICE_ID, BUILD_NUMBER, FLASH_BUTTON_PIN, DEVICE_PING_ID, DEVICE_RESET_SWITCH_ID,
              setupMqttSensorActors);
}

void setupMqttSensorActors() {
  WirePinSet wireConfiguration = {I2C_SLAVE_ADDRESS, SDA, SCL};
  MQTTRgbLightI2CCommands connectionCommands = {ON_OFF_COMMAND, SET_COLOR_COMMAND, SET_BRIGHTNESS_COMMAND};
  MQTTI2CRgbLightConfiguration stripConfig = {connectionCommands, wireConfiguration};
  MQTTI2CRgbLight *rgbLedStrip = new MQTTI2CRgbLight(getMQTTDeviceInfo(), "L5xVUX", stripConfig);
  registerMQTTDevice(rgbLedStrip);
}

void loop() { loopDevice(300); }
