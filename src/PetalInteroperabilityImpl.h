#include "PetalInteroperability.h"
#include "PetalMidiBridge.h"

class PetalInteroperabilityImpl : public PetalInteroperability {
protected:
bool _isConnected = false;
private:
PetalMidiBridge *_bridge = nullptr;

public:
  PetalInteroperabilityImpl();
  ~PetalInteroperabilityImpl();

  bool isConnected();
  void handleBLEIncomingConnections();
  void sendSysExMessage(const byte * message, unsigned length);
  void sendProgramChange(byte channel, byte number);
  void sendControlChange(byte channel, byte number, byte value);
  void setCurrentColor(u_int32_t color);
  void logConnectionChange();
  void readBLECharacteristic();

  PetalMidiBridge *bridge() { return _bridge; };
  void setup(PetalMidiBridge *bridge);
  void process();
  void processPacket(unsigned long data);
 };