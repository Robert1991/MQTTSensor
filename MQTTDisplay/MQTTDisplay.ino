#include "LiquidCrystal.h"
#include <Adafruit_ADS1015.h>
#include <DeviceRuntime.h>

#define FLASH_BUTTON_PIN 0

const int BUILD_NUMBER = 17;
const String DEVICE_ID = "ZXwamE";
const String DEVICE_PING_ID = "FOM4be";
const String DEVICE_RESET_STATE_SWITCH_ID = "gcGaX6";

const int RS = D2, EN = D3, d4 = D5, d5 = D6, d6 = D7, d7 = D8;
LiquidCrystal lcd(RS, EN, d4, d5, d6, d7);

WiFiClient client;

// LCD Display set topic homeassistant/display/mqtt_lcd_display_show/show

void setup() {
  Serial.begin(9600);
  setupDevice(client, DEVICE_ID, BUILD_NUMBER, FLASH_BUTTON_PIN, DEVICE_PING_ID, DEVICE_RESET_STATE_SWITCH_ID,
              setupMqttSensorActors);
}

void setupMqttSensorActors() {
  MQTTLcdDisplay *lcdDisplay = new MQTTLcdDisplay(getMQTTDeviceInfo(), "cVG63O", &lcd);
  registerMQTTDevice(lcdDisplay);
}

void loop() { loopDevice(250); }
