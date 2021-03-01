#include <DeviceRuntime.h>

#define RELAIS_PIN_1 D1
#define RELAIS_PIN_2 D2

#define DHTTYPE DHT22
#define DHT_PIN D3

#define FLASH_BUTTON_PIN 0

const int BUILD_NUMBER = 4;
const String DEVICE_ID = "NStlwd";
const String DEVICE_PING_ID = "wJvJZH";
const String DEVICE_RESET_SWITCH_ID = "pshrng";

DHT dht(DHT_PIN, DHTTYPE);
WiFiClient espClient;

void setup() {
  setupDevice(espClient, DEVICE_ID, BUILD_NUMBER, FLASH_BUTTON_PIN, DEVICE_PING_ID, DEVICE_RESET_SWITCH_ID,
              setupMqttSensorActors);
}

void loop() { loopDevice(300); }

void setupMqttSensorActors() {
  MQTTSwitch *relaisLight1 = new MQTTSwitch(getMQTTDeviceInfo(), "OqxzUZ", RELAIS_PIN_1, "relais_light_1");
  registerMQTTDevice(relaisLight1);
  MQTTSwitch *relaisLight2 = new MQTTSwitch(getMQTTDeviceInfo(), "k9L2sA", RELAIS_PIN_2, "relais_light_2");
  registerMQTTDevice(relaisLight2);
  MQTTHumiditySensor *humiditySensor = new MQTTHumiditySensor(getMQTTDeviceInfo(), &dht, "7CTb5m");
  registerMQTTDevice(humiditySensor);
  MQTTTemperatureSensor *temperatureSensor = new MQTTTemperatureSensor(getMQTTDeviceInfo(), &dht, "8FiVDk");
  registerMQTTDevice(temperatureSensor);
}
