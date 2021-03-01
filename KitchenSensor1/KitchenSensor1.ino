#include <DeviceRuntime.h>

#define PHOTO_SENSOR_PIN A0
#define DOOR_SENSOR_PIN D1
#define MOTION_SENSOR_PIN D2
#define DHT_PIN D3
#define DHTTYPE DHT11
#define FLASH_BUTTON_PIN 0

const int BUILD_NUMBER = 4;
const String DEVICE_ID = "2VgWWW";
const String DEVICE_PING_ID = "urT3UA";
const String DEVICE_RESET_SWITCH_ID = "SJGZoR";

WiFiClient espClient;
DHT dht(DHT_PIN, DHTTYPE);

void setup() {
  // Serial.begin(9600);
  setupDevice(espClient, DEVICE_ID, BUILD_NUMBER, FLASH_BUTTON_PIN, DEVICE_PING_ID, DEVICE_RESET_SWITCH_ID,
              setupMqttSensorActors);
}
void setupMqttSensorActors() {
  MQTTInputPullUpSensor *doorSensor =
      new MQTTInputPullUpSensor(getMQTTDeviceInfo(), "lEqFzg", DOOR_SENSOR_PIN, "door", "door_open");
  registerMQTTDevice(doorSensor);
  MQTTMotionSensor *motionSensor =
      new MQTTMotionSensor(getMQTTDeviceInfo(), "ywseEo", MOTION_SENSOR_PIN, 15, 20, 9);
  registerMQTTDevice(motionSensor);
  MQTTPhotoLightSensor *photoSensor =
      new MQTTPhotoLightSensor(getMQTTDeviceInfo(), "ulHFRc", PHOTO_SENSOR_PIN);
  registerMQTTDevice(photoSensor);
  MQTTHumiditySensor *humiditySensor = new MQTTHumiditySensor(getMQTTDeviceInfo(), &dht, "p1uDD4");
  registerMQTTDevice(humiditySensor);
  MQTTTemperatureSensor *temperatureSensor = new MQTTTemperatureSensor(getMQTTDeviceInfo(), &dht, "czt75Q");
  registerMQTTDevice(temperatureSensor);
}

void loop() { loopDevice(300); }
