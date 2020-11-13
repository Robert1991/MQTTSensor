#include <Wire.h>

#define ON_OFF_COMMAND 0
#define SET_COLOR_COMMAND 1
#define SET_BRIGHTNESS_COMMAND 2

#define RED_LED_PIN 3
#define GREEN_LED_PIN 6
#define BLUE_LED_PIN 5

#define ON_OFF_COMMAND_READ_SIZE 1
#define SET_COLOR_COMMAND_READ_SIZE 3
#define SET_BRIGHTNESS_COMMAND_READ_SIZE 1
#define SECONDARY_ADDRESS 8

bool stripIsOn = false;
byte currentBrightness = 0;
byte currentRedPart = 0;
byte currentGreenPart = 0;
byte currentBluePart = 0;

void setup() {
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  
  Wire.begin(SECONDARY_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  Serial.begin(9600);
}

void loop() {
 delay(100);
}

void applyChoosenColorToLeds() {
  if (stripIsOn) {
    analogWrite(RED_LED_PIN, (int)(currentRedPart * (currentBrightness / 255.0)));
    analogWrite(GREEN_LED_PIN, (int)(currentGreenPart * (currentBrightness / 255.0)));
    analogWrite(BLUE_LED_PIN, (int)(currentBluePart * (currentBrightness / 255.0)));
  } else {
    analogWrite(RED_LED_PIN, LOW);
    analogWrite(GREEN_LED_PIN, LOW);
    analogWrite(BLUE_LED_PIN, LOW);
  }
}

void receiveEvent(int byteCount) {
  Serial.print("Received message with byte count: ");Serial.println(byteCount);

  if (byteCount >= 2 && byteCount <=4) {
    byte commandByte = Wire.read();
    if (commandByte == ON_OFF_COMMAND && byteCount == ON_OFF_COMMAND_READ_SIZE + 1) {
      byte onOffParameterByte = Wire.read();
      if (onOffParameterByte) {
        Serial.println("turn on");
        stripIsOn = true;
      } else {
        Serial.println("turn off");
        stripIsOn = false;
      }
      Serial.print("Received on/off command ");Serial.print(" with parameter ");Serial.println(onOffParameterByte);
    } else if (commandByte == SET_COLOR_COMMAND && SET_COLOR_COMMAND_READ_SIZE + 1) {
      currentRedPart = Wire.read();
      currentGreenPart = Wire.read();
      currentBluePart = Wire.read();
      Serial.print("Received set color command ");Serial.print(" with values: r=");Serial.print(currentRedPart);Serial.print(" g=");Serial.print(currentGreenPart);Serial.print(" b=");Serial.println(currentBluePart);
    } else if (commandByte == SET_BRIGHTNESS_COMMAND && SET_BRIGHTNESS_COMMAND_READ_SIZE + 1) {
      currentBrightness = Wire.read();
      Serial.print("Received set brightness command ");Serial.print(" with value: ");Serial.print(currentBrightness);
    }
    applyChoosenColorToLeds();
  } else {
    while (0 <Wire.available()) {
      char c = Wire.read();
      Serial.print(c);
    }
    Serial.println();
  }
}

void requestEvent() {
 Wire.write("Hello NodeMCU"); 
}
