#include <DeviceRuntime.h>

#define FLASH_BUTTON_PIN 0
#define PHOTO_SENSOR_PIN A0
#define LIGHT_PIN D3

const int BUILD_NUMBER = 5;
const String DEVICE_ID = "XT9Ohl";
const String DEVICE_PING_ID = "UzBJqg";
const String DEVICE_RESET_SWITCH_ID = "k6lnfO";

WiFiClient espClient;
MQTTClient client(750);

void setup() {
  // Serial.begin(9600);
  setupDevice(espClient, DEVICE_ID, BUILD_NUMBER, FLASH_BUTTON_PIN, DEVICE_PING_ID, DEVICE_RESET_SWITCH_ID,
              setupMqttSensorActors);
}

void setupMqttSensorActors() {
  MQTTSwitch *transistorLight = new MQTTSwitch(getMQTTDeviceInfo(), "ODQ7Jt", LIGHT_PIN);
  registerMQTTDevice(transistorLight);
  MQTTPhotoLightSensor *photoSensor =
      new MQTTPhotoLightSensor(getMQTTDeviceInfo(), "zMvxqy", PHOTO_SENSOR_PIN);
  registerMQTTDevice(photoSensor);
}

void loop() { loopDevice(300); }
