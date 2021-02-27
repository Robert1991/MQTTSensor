#include <DeviceRuntime.h>

#define DHT_PIN D4
#define DHTTYPE DHT22
#define PHOTO_SENSOR_PIN A0
#define MOTION_SENSOR_PIN D3
#define DOOR_SENSOR_PIN D2
#define FLASH_BUTTON_PIN 0

const int BUILD_NUMBER = 0;
const String DEVICE_ID = "EuZQUg";
const String DEVICE_PING_ID = "6fp4mh";
const String DEVICE_RESET_SWITCH_ID = "CMOph5";

DHT dht(DHT_PIN, DHTTYPE);
WiFiClient client;

void setup() {
  // Turn off onboard led
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  setupDevice(client, DEVICE_ID, BUILD_NUMBER, FLASH_BUTTON_PIN, DEVICE_PING_ID, DEVICE_RESET_SWITCH_ID,
              setupMqttSensorActors);
}

void loop() { loopDevice(200); }

void setupMqttSensorActors() {
  MQTTInputPullUpSensor *doorSensor =
      new MQTTInputPullUpSensor(getMQTTDeviceInfo(), "UWJtLa", DOOR_SENSOR_PIN, "door", "door_open");
  registerMQTTDevice(doorSensor);
  MQTTPhotoLightSensor *photoSensor =
      new MQTTPhotoLightSensor(getMQTTDeviceInfo(), "PuCh8f", PHOTO_SENSOR_PIN);
  registerMQTTDevice(photoSensor);
  MQTTMotionSensor *motionSensor =
      new MQTTMotionSensor(getMQTTDeviceInfo(), "uzyYNO", MOTION_SENSOR_PIN, 10, 10, 4);
  registerMQTTDevice(motionSensor);
  MQTTHumiditySensor *humiditySensor = new MQTTHumiditySensor(getMQTTDeviceInfo(), &dht, "7xNFxb");
  registerMQTTDevice(humiditySensor);
  MQTTTemperatureSensor *temperatureSensor = new MQTTTemperatureSensor(getMQTTDeviceInfo(), &dht, "s3ZRns");
  registerMQTTDevice(temperatureSensor);
}
