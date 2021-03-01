#include <DeviceRuntime.h>

#define DHT_PIN D3
#define DHTTYPE DHT11
#define MOTION_SENSOR_PIN D7

#define SDA D1
#define SCL D2

#define I2C_SLAVE_ADDRESS 8
#define ON_OFF_COMMAND 0
#define SET_COLOR_COMMAND 1
#define SET_BRIGHTNESS_COMMAND 2

#define FLASH_BUTTON_PIN 0

const int BUILD_NUMBER = 4;
const String DEVICE_ID = "5soti6";
const String DEVICE_PING_ID = "TRAjq2";
const String DEVICE_RESET_SWITCH_ID = "ogm8e8";

WiFiClient espClient;
DHT dht(DHT_PIN, DHTTYPE);

void setup() {
  // Serial.begin(9600);
  setupDevice(espClient, DEVICE_ID, BUILD_NUMBER, FLASH_BUTTON_PIN, DEVICE_PING_ID, DEVICE_RESET_SWITCH_ID,
              setupMqttSensorActors);
}

void setupMqttSensorActors() {
  MQTTMotionSensor *motionSensor =
      new MQTTMotionSensor(getMQTTDeviceInfo(), "ldOfO7", MOTION_SENSOR_PIN, 15, 20, 9);
  registerMQTTDevice(motionSensor);
  MQTTHumiditySensor *humiditySensor = new MQTTHumiditySensor(getMQTTDeviceInfo(), &dht, "ST17k2");
  registerMQTTDevice(humiditySensor);
  MQTTTemperatureSensor *temperatureSensor = new MQTTTemperatureSensor(getMQTTDeviceInfo(), &dht, "Fzwtrt");
  registerMQTTDevice(temperatureSensor);
  WirePinSet wireConfiguration = {I2C_SLAVE_ADDRESS, SDA, SCL};
  MQTTRgbLightI2CCommands connectionCommands = {ON_OFF_COMMAND, SET_COLOR_COMMAND, SET_BRIGHTNESS_COMMAND};
  MQTTI2CRgbLightConfiguration stripConfig = {connectionCommands, wireConfiguration};
  MQTTI2CRgbLight *rgbLedStrip = new MQTTI2CRgbLight(getMQTTDeviceInfo(), "26vFbD", stripConfig);
  registerMQTTDevice(rgbLedStrip);
}

void loop() { loopDevice(300); }
