#include <DeviceRuntime.h>

#define RELAIS_PIN 0

const int BUILD_NUMBER = 4;
const String DEVICE_ID = "JNDHbc";
const String DEVICE_PING_ID = "Fz3npn";
const String DEVICE_RESET_SWITCH_ID = "T1VMmW";

WiFiClient espClient;

void setup() {
  // Serial.begin(9600);
  setupDevice(espClient, DEVICE_ID, BUILD_NUMBER, 1, DEVICE_PING_ID, DEVICE_RESET_SWITCH_ID,
              setupMqttSensorActors);
}

void loop() { loopDevice(300); }

void setupMqttSensorActors() {
  MQTTSwitch *relaisLight =
      new MQTTSwitch(deviceInfo, "T1VMmW", RELAIS_PIN, "relais_light_1", "light", false);
  registerMQTTDevice(relaisLight);
}
