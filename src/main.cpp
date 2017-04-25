/*********************************************************************************************************************
 ** 2017-04-24, RDU: Built for Sonoff "Switch" based on Homie sample sketch
 **                  https://www.itead.cc/sonoff-wifi-wireless-switch.html
 **                  https://github.com/marvinroger/homie-esp8266/blob/develop/examples/IteadSonoffButton/IteadSonoffButton.ino
 *********************************************************************************************************************/
#include <Homie.h>


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Globals
  // Software specifications
    #define FW_NAME    "Sonoff-Switch"
    #define FW_VERSION "0.17.4.24"


  // Hardware specs
    const int PIN_RELAY = 12;
    const int PIN_LED = 13;
    const int PIN_BUTTON = 0;

 // Button state
    unsigned long buttonDownTime = 0;
    byte lastButtonState = 1;
    byte buttonPressHandled = 0;

  // Sensor consts/vars
    HomieNode switchNode("switch", "switch");


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Homie handler for Sonoff switch over MQTT
bool switchOnHandler(const HomieRange& range, const String& value) {
  if (value != "true" && value != "false") return false;

  bool on = (value == "true");
  digitalWrite(PIN_RELAY, on ? HIGH : LOW);
  switchNode.setProperty("on").send(value);
  Homie.getLogger() << "Switch is " << (on ? "on" : "off") << " (MQTT)" << endl;

  return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Homie handler for Sonoff hardware switch : Reverse relay state
void toggleRelay() {
  bool on = digitalRead(PIN_RELAY) == HIGH;
  digitalWrite(PIN_RELAY, on ? LOW : HIGH);
  switchNode.setProperty("on").send(on ? "false" : "true");
  Homie.getLogger() << "Switch is " << (on ? "off" : "on") << " (button)" << endl;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Homie Setup Handler
void setupHandler() {
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Homie loop Handler : Checks button state to have hardware button to set relay state (on/off)
void loopHandler() {
  byte buttonState = digitalRead(PIN_BUTTON);

  if ( buttonState != lastButtonState ) {
    if (buttonState == LOW) {
      buttonDownTime     = millis();
      buttonPressHandled = 0;
    } else {
      unsigned long dt = millis() - buttonDownTime;
      if ( dt >= 90 && dt <= 900 && buttonPressHandled == 0 ) {
        toggleRelay();
        buttonPressHandled = 1;
      }
    }

    lastButtonState = buttonState;
  }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arduino Setup Handler
void setup() {
  // Serial setup
  Serial.begin(115200); // Required to enable serial output
  Serial << endl << endl;

  // Firmware setup
  Homie_setFirmware(FW_NAME, FW_VERSION);
  Homie.setSetupFunction(setupHandler);

  // Device setup
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_BUTTON, INPUT);
  digitalWrite(PIN_RELAY, LOW);
  Homie.setLedPin(PIN_LED, LOW).setResetTrigger(PIN_BUTTON, LOW, 5000);

  switchNode.advertise("on").settable(switchOnHandler);

  Homie.setLoopFunction(loopHandler); // Move after setupHandler definition?
  Homie.setup();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arduino Loop Handler
void loop() {
  Homie.loop();
}
