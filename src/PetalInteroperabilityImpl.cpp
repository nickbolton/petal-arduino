#include "PetalInteroperabilityImpl.h"
#include "PetalUtils.h"
#include "PetalStatus.h"
#include <ArduinoBLE.h>
#include <MIDI.h>

const int MAX_PROGRAM_SIZE = 10000;

static const char *const SERVICE_UUID        = "03b80e5a-ede8-4b33-a751-6ce34ec4c700";
static const char *const CHARACTERISTIC_UUID = "7772e5db-3868-4112-a1a9-f2669d106bf3";

BLEService midiService(SERVICE_UUID); // BLE Service
BLECharacteristic midiCharacteristic(CHARACTERISTIC_UUID, BLEWrite | BLERead | BLENotify, MAX_PROGRAM_SIZE);

MIDI_CREATE_INSTANCE(HardwareSerial,Serial1, MIDI_HARDWARE); 

const byte BANK_STATUS = 0x10;
const byte PC_STATUS = 0xc0;
const byte CC_STATUS = 0xb0;

PetalInteroperabilityImpl *current = nullptr;

int bleStatus = 0;
bool isBLEConnected = false;

// void onMidiSysEx(byte* data, unsigned length) {
//   PETAL_LOGI("RX SYSEX!");
//   if (!current || !current->bridge() || !current->isConnected()) { return; }
//   current->bridge()->receiveSysExMessage(data, length);
// }

// void errorCallback(int8_t error) {
//   PETAL_LOGE("Received error from MIDI: %d", error);
// }

// void onControlChange(unsigned char channel, unsigned char number, unsigned char value) {
//   if (!current || !current->bridge() || !current->isConnected()) { return; }
//   PETAL_LOGD("RX CC channel %2d number %3d value %3d", channel, number, value);
// }

void handleControlChange(byte channel, byte number, byte value) {
  if (!current || !current->bridge() || !current->isConnected()) { 
    PETAL_LOGI("Received (but not connected) CC %d on Arduino channel %d with value 0x%x", number, channel, value);
    return;
  }
  PETAL_LOGI("Received (and processing) CC %d on Arduino channel %d with value 0x%x", number, channel, value);
  current->bridge()->receiveControlChange(channel, number, value);
}

PetalInteroperabilityImpl::PetalInteroperabilityImpl() {
  current = this;
}

PetalInteroperabilityImpl::~PetalInteroperabilityImpl() { 
  _bridge = nullptr; 
  current = nullptr;
}

void PetalInteroperabilityImpl::setup(PetalMidiBridge *bridge) {
  _bridge = bridge;
  PETAL_LOGI("Setting up BLE MIDI…");

  PETAL_LOGI("Setting up hardware MIDI…");
  MIDI_HARDWARE.begin(MIDI_CHANNEL_OMNI);
  MIDI_HARDWARE.turnThruOff();
  MIDI_HARDWARE.setHandleControlChange(handleControlChange);

  bleStatus = BLE.begin();
  if (bleStatus) {
    midiService.addCharacteristic(midiCharacteristic);

    BLE.setLocalName("PetalMIDIBridge");
    BLE.addService(midiService);
    BLE.setAdvertisedService(midiService);
    BLE.advertise();
    PETAL_LOGI("BLE started successfull");
  } else {
    PETAL_LOGE("starting BLE failed!");
  }
}

void PetalInteroperabilityImpl::handleBLEIncomingConnections() {
  logConnectionChange();
  if (!isConnected()) { return; }
  readBLECharacteristic();
}

void PetalInteroperabilityImpl::readBLECharacteristic() {
  // PETAL_LOGI("written: %d", midiCharacteristic.written());
  // PETAL_LOGI("current: %x", current);
  // if (current) {
  //   PETAL_LOGI("bridge: %x", current->bridge());
  // }
  if (!midiCharacteristic.written() || !current || !current->bridge()) { return; }
  PETAL_LOGI("valueLength: %d", midiCharacteristic.valueLength());
  if (midiCharacteristic.valueLength() <= 0) { PETAL_LOGI("HELLO2"); return; }
  PETAL_LOGI("HELLO3"); 
  byte *data = (byte *)midiCharacteristic.value();
  unsigned int length = midiCharacteristic.valueLength();
  unsigned int startIndex = PetalUtils::findIndex(data, length, 0xf0);
  unsigned int endIndex = length - 1;
  if (startIndex < 0) {
    PetalUtils::logBuffer("Dropping SYSEX", data, length, false); 
    return;
  }
  if (data[endIndex - 1] == 0x83) {
    endIndex--;
    data[endIndex] = 0xf7;
  }
  byte *trimmedData = data+startIndex;
  unsigned int trimmedLength = length - startIndex - (length - 1 - endIndex);
  PetalUtils::logBuffer("RX SYSEX", trimmedData, trimmedLength, false);
  current->bridge()->receiveSysExMessage(trimmedData, trimmedLength);
}

void PetalInteroperabilityImpl::logConnectionChange() {
  BLEDevice central = BLE.central();
  if (central && central.connected()) {
    if (!isBLEConnected) {
      PETAL_LOGI("Connected to BLE central");
    }
    isBLEConnected = true;
  } else {
    if (isBLEConnected) {
      PETAL_LOGI("Disconnected from BLE central");
    }
    isBLEConnected = false;
  }
}

void PetalInteroperabilityImpl::process() {
  MIDI_HARDWARE.read();
  if (bleStatus) {
    handleBLEIncomingConnections();
  }
}

bool PetalInteroperabilityImpl::isConnected() {
  BLEDevice central = BLE.central();
  return central && central.connected(); 
}

void PetalInteroperabilityImpl::sendSysExMessage(const byte * message, unsigned length) {
  byte preamble[] = { 0x82, 0x83, 0xf0 }; 
  unsigned int preambleLength = sizeof(preamble);
  unsigned int totalLength = preambleLength + length + 1; // +1 for trailing f7
  byte wrappedMessage[totalLength]; 
  memcpy(wrappedMessage, preamble, preambleLength); 
  memcpy(wrappedMessage + preambleLength, message, length);
  wrappedMessage[preambleLength+length] = 0xf7;
  PetalUtils::logSysExMessage("TX", wrappedMessage, totalLength, false);
  midiCharacteristic.writeValue(wrappedMessage, totalLength);
}

void PetalInteroperabilityImpl::sendProgramChange(byte channel, byte number) {
  PETAL_LOGI("Sending pc %d on channel %d", number, channel);
  MIDI_HARDWARE.sendProgramChange(number, channel);
}

void PetalInteroperabilityImpl::sendControlChange(byte channel, byte number, byte value) {
  PETAL_LOGI("Sending cc %d 0x%x on channel %d", number, value, channel);
  MIDI_HARDWARE.sendControlChange(number, value, channel);
}

// void PetalInteroperabilityImpl::processPacket(unsigned long data) {
//   byte status = data >> 24;
//   byte channel = (data & 0xffffff) >> 16;
//   byte number =  (data & 0xffff) >> 8;
//   byte value = data & 0xff;
//   if (status == BANK_STATUS) {
//     sendControlChange(channel, 0, 0);
//     sendControlChange(channel, 0x20, number);
//     sendProgramChange(channel, value);
//   } else if (status == PC_STATUS) {
//     sendProgramChange(channel, number);
//   } else if (status == CC_STATUS) {
//     sendControlChange(channel, number, value);
//   } else {
//      PETAL_LOGI("Invalid event status 0x%x, channel %d, number: %d, value: 0x%x", status, channel, number, value);
//   }
// }

void PetalInteroperabilityImpl::setCurrentColor(u_int32_t color) {
  PetalStatus::setCurrentColor(color);
}