#pragma once

#include <Arduino.h>

struct PetalStatus {
  private:
  static void updateStatusColor(bool isConnected);
  static void showCurrentColor();
  static void showYellowLED();
  static void showGreenLED();
  static void showRedLED();
  static void showBlueLED();
  static void clearLEDs();
  static void setLEDColor(byte red, byte green, byte blue);

  public:
  static void setup();
  static void process(bool isConnected);
  static void setCurrentColor(u_int32_t color);
};