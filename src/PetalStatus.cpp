#include <PetalStatus.h>

const int RED_PIN = 9;
const int GREEN_PIN = 10;
const int BLUE_PIN = 11;

const int LED_ON = 0xFF;
const int LED_OFF = 0;

double redDutyCycle = 0.0;
double greenDutyCycle = 0.0;
double blueDutyCycle = 0.0;

u_int32_t currentColor = 0;

unsigned long previousMillis = 0;        // will store last time LED was updated

const long interval = 1000;           // interval at which to blink (milliseconds)

bool isLedOn = false;

void PetalStatus::setup() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  clearLEDs();
}

void PetalStatus::process(bool isConnected) {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    updateStatusColor(isConnected);
  }
}

void PetalStatus::updateStatusColor(bool isConnected) {
//  sendRemoteLogging(appendInt("_isConnected: ", _isConnected) + appendInt("isLedOn: ", isLedOn) + appendLong("currentColor: ", currentColor) + "\n");

  if (isConnected) {
    isLedOn = true;
    if (currentColor != 0) {
      showCurrentColor();
    } else {
      showGreenLED();
    }
  } else {
    isLedOn = !isLedOn;
    if (isLedOn) {
      showYellowLED();
    } else {
      clearLEDs();
    }
  }
}

void PetalStatus::setCurrentColor(u_int32_t color) {
  currentColor = color;
}

void PetalStatus::showCurrentColor() {
  int rawRed = currentColor >> 16;
  int rawGreen = (currentColor & 0xffff) >> 8;
  int rawBlue = currentColor & 0xff;

  float redPercent = (float)rawRed / 255.0;
  float greenPercent = (float)rawGreen / 255.0;
  float bluePercent = (float)rawBlue / 255.0;

  int red = (int)(redPercent * 255.0);
  int green = (int)(greenPercent * 255.0);
  int blue = (int)(bluePercent * 255.0);

//  String redString = appendInt("ble showing current color red: ", red);
//  String greenString = appendInt(" green: ", green);
//  String blueString = appendInt(" blue: ", blue);
//  sendRemoteLogging(redString + greenString + blueString + "\n");

  setLEDColor(red, green, blue);
}

void PetalStatus::setLEDColor(byte red, byte green, byte blue) {
  analogWrite(RED_PIN, red);
  analogWrite(GREEN_PIN, green);
  analogWrite(BLUE_PIN, blue);
}

void PetalStatus::showYellowLED() {
//  sendRemoteLogging("Showing YELLOW COLOR\n");
  setLEDColor(0x7F, 0x7F, LED_OFF);
}

void PetalStatus::showGreenLED() {
//  sendRemoteLogging("Showing GREEN COLOR\n");
  setLEDColor(LED_OFF, LED_ON, LED_OFF);
}

void PetalStatus::showRedLED() {
//  sendRemoteLogging("Showing RED COLOR\n");
  setLEDColor(LED_ON, LED_OFF, LED_OFF);
}

void PetalStatus::showBlueLED() {
//  sendRemoteLogging("Showing BLUE COLOR\n");
// #00A1E5
  setLEDColor(LED_OFF, 0xA1, 0xE5);
}

void PetalStatus::clearLEDs() {
//  sendRemoteLogging("Showing CLEAR COLOR\n");
  setLEDColor(LED_OFF, LED_OFF, LED_OFF);
}