#define MY_DEBUG
#define MY_RADIO_RF24
#define MY_NODE_ID 3

#include <MySensors.h>

// Wait times
#define LONG_WAIT 500
#define SHORT_WAIT 50

#define SKETCH_NAME "GarageDoor "
#define SKETCH_VERSION "v2.0"

#define ID_S_DOOR 0
#define ID_S_LOCK 1

#define SLEEP_TIME (6 * 60 * 60 * 1000ul) // Check battery every 6 hours

MyMessage msg_S_DOOR(ID_S_DOOR, V_TRIPPED);
MyMessage msg_S_LOCK(ID_S_LOCK, V_LOCK_STATUS);

#define LOCK_PIN 2
#define DOOR_PIN 3
#define BATT_PIN A0

#include <Vcc.h>
static uint8_t oldBatteryPcnt = 200;  // Initialize to 200 to assure first time value will be sent.
const float VccMin        = 1.8;      // Minimum expected Vcc level, in Volts: Brownout at 1.8V    -> 0%
const float VccMax        = 2.0*1.5;  // Maximum expected Vcc level, in Volts: 2xAA fresh Alkaline -> 100%
const float VccCorrection = 1.0;      // Measured Vcc by multimeter divided by reported Vcc
static Vcc vcc(VccCorrection); 

void setup()
{
  Serial.begin(9600);
  pinMode(LOCK_PIN, INPUT_PULLUP); // Lock
  pinMode(DOOR_PIN, INPUT_PULLUP); // Door

  sendSketchInfo(SKETCH_NAME, SKETCH_VERSION);
  
  present(ID_S_DOOR, S_DOOR, "Garage Door");
  wait(SHORT_WAIT);
  present(ID_S_LOCK, S_LOCK, "Garage Door Lock");
  wait(SHORT_WAIT);
  handleBatteryLevel();

}

void loop()
{
  int8_t wakeCause = sleep(digitalPinToInterrupt(LOCK_PIN), CHANGE, digitalPinToInterrupt(DOOR_PIN), CHANGE, SLEEP_TIME);
  switch (wakeCause) {
    case MY_WAKE_UP_BY_TIMER: {
        sendHeartbeat();
        handleBatteryLevel();
        break;
      }
    case digitalPinToInterrupt(LOCK_PIN): {
        bool isLocked = digitalRead(LOCK_PIN) == HIGH;
        Serial.println("Lock: " + String(isLocked));
        send(msg_S_LOCK.set(isLocked));
        handleBatteryLevel();

        break;
      }
    case digitalPinToInterrupt(DOOR_PIN): {
        bool tripped = digitalRead(DOOR_PIN) == HIGH;
        Serial.println("Door : " + String(tripped));
        send(msg_S_DOOR.set(tripped ? "1" : "0"));
        handleBatteryLevel();
        break;
      }
  }
}

void handleBatteryLevel() {
  const uint8_t batteryPcnt = static_cast<uint8_t>(0.5 + vcc.Read_Perc(VccMin, VccMax));

#ifdef MY_DEBUG
  Serial.print(F("Vbat "));
  Serial.print(vcc.Read_Volts());
  Serial.print(F("\tPerc "));
  Serial.println(batteryPcnt);
#endif

  // Battery readout should only go down. So report only when new value is smaller than previous one.
  if ( batteryPcnt < oldBatteryPcnt )
  {
      sendBatteryLevel(batteryPcnt);
      oldBatteryPcnt = batteryPcnt;
  }
}
