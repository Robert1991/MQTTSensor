#include <Adafruit_ADS1015.h>
#include <DeviceRuntime.h>

#define FLASH_BUTTON_PIN 0
#define PUMP_RELAIS_1_PIN D5
#define PUMP_RELAIS_2_PIN D6
#define DHT_PIN D3
#define DHTTYPE DHT22

const int BUILD_NUMBER = 4;
const String DEVICE_ID = "BXJpj7";
const String DEVICE_PING_ID = "BUXZN3";
const String DEVICE_RESET_SWITCH_ID = "g9GJjv";

WiFiClient client;
DHT dht(DHT_PIN, DHTTYPE);
Adafruit_ADS1115 ads;

void setup() {
  // Serial.begin(9600);
  ads.begin();
  setupDevice(client, DEVICE_ID, BUILD_NUMBER, FLASH_BUTTON_PIN, DEVICE_PING_ID, DEVICE_RESET_SWITCH_ID,
              setupMqttSensorActors);
}

void setupMqttSensorActors() {
  MQTTInputPullUpSensor *swimSwitchSensor =
      new MQTTInputPullUpSensor(getMQTTDeviceInfo(), "l5nMLf", D4, "moisture", "swim_switch");
  registerMQTTDevice(swimSwitchSensor);

  MQTTAnalogConverterSensor *moistureSensor1 =
      new MQTTAnalogConverterSensor(getMQTTDeviceInfo(), "sILEuT", &ads, 1, "moisture_1", 7000, 15000);
  registerMQTTDevice(moistureSensor1);

  MQTTAnalogConverterSensor *moistureSensor2 =
      new MQTTAnalogConverterSensor(getMQTTDeviceInfo(), "o0Zck4", &ads, 0, "moisture_2", 7000, 15000);
  registerMQTTDevice(moistureSensor2);

  MQTTSwitch *pumpRelais1 =
      new MQTTSwitch(getMQTTDeviceInfo(), "Y1JGud", PUMP_RELAIS_1_PIN, "pump_relais_1", "switch", true);
  registerMQTTDevice(pumpRelais1);

  MQTTSwitch *pumpRelais2 =
      new MQTTSwitch(getMQTTDeviceInfo(), "VDeOCs", PUMP_RELAIS_2_PIN, "pump_relais_2", "switch", true);
  registerMQTTDevice(pumpRelais2);

  MQTTHumiditySensor *humiditySensor = new MQTTHumiditySensor(getMQTTDeviceInfo(), &dht, "k2hGOM");
  registerMQTTDevice(humiditySensor);

  MQTTTemperatureSensor *temperatureSensor = new MQTTTemperatureSensor(getMQTTDeviceInfo(), &dht, "dLwNqe");
  registerMQTTDevice(temperatureSensor);
}

void loop() { loopDevice(400); }
