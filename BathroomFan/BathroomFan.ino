#include <DeviceRuntime.h>

#define RELAIS_PIN D2
#define FLASH_BUTTON_PIN 0

const String DEVICE_ID = "PpP9Dr";
const String DEVICE_PING_ID = "KXTfZ4";
const String DEVICE_RESET_SWITCH_ID = "aTbxnT";

const String DEVICE_FAN_SWITCH_ID = "aTbxnT";
const String DEVICE_BATTERY_SENSOR_ID = "AolIP2";

const int BUILD_NUMBER = 0;

WiFiClient espClient;

void setup() {
  Serial.begin(9600);
  setupDevice(espClient, DEVICE_ID, BUILD_NUMBER, FLASH_BUTTON_PIN, DEVICE_PING_ID, DEVICE_RESET_SWITCH_ID,
              setupMqttSensorActors);
}

void loop() { loopDevice(750); }

void setupMqttSensorActors() {
  MQTTSwitch *fanSwitch = new MQTTSwitch(getMQTTDeviceInfo(), DEVICE_FAN_SWITCH_ID, RELAIS_PIN, "switch");
  registerMQTTDevice(fanSwitch);

  MQTTBatterySensor *batterySensor =
      new MQTTBatterySensor(getMQTTDeviceInfo(), DEVICE_BATTERY_SENSOR_ID, A0, 12.0, 10.4, 12.1);
  registerMQTTDevice(batterySensor);
}
