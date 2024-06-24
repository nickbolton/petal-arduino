#include <Arduino.h>
#include "PetalUtils.h"
#include "PetalMidiBridge.h"
#include "PetalStatus.h"
#include "PetalInteroperabilityImpl.h"

PetalInteroperabilityImpl *interop = new PetalInteroperabilityImpl;
PetalMidiBridge *bridge = new PetalMidiBridge(interop);

unsigned long lastTime = 0l;

void setup() {
  Serial.begin(115200);
  delay(2000);
  PetalUtils::setup();
  PetalStatus::setup();
  PETAL_LOGI("Startup up…");
  bridge->setup();
  interop->setup(bridge);
}

void printLoop() {
  unsigned long now = millis();
  if (now - lastTime >= 1000) {
    PETAL_LOGI("LOOP!!");
    lastTime = now;
  }
}

void loop() {
  bridge->process();
  interop->process();
  PetalStatus::process(interop->isConnected());
  // printLoop();
}