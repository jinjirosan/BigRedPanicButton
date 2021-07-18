/*
 Version 0.1.7 - code rewrite for sigfox notification - edited sigfox.cpp !!!

 This code is for the BigRedPanicButton. A home safety device which sends an alert over the Sigfox network when the button is pressed.
 The signal results in a callback to a specific email address or emergency SMS gateway.

 I developed this for my parents (who are of age) as a failsafe in-case someone breaks in or hwne there is an issue with pushy salespeople at the door. I will get notified immediately.

Sigfox callback contents:

-- EMERGENCY AT B192 --

Call mom/dad

Status : {customData#status}
Sequence : {seqNumber}
Sensor : {customData#sensor}
Device : {device} - {deviceTypeId}
Battery percentage: {customData#percentage}

George the BigRedPanicButton

*/

//#include <OneButton.h>
#include <SigFox.h>
#include <ArduinoLowPower.h>

#define PIN_INPUT 1
#define PIN_LED 3

// current LED state, staring with LOW (0)
int ledState = LOW;

// Set debug to false to enable continuous mode
// and disable serial prints
int debug = false;

volatile int alarm_source = 0;

// Variables for battery SoC calculation
float voltage = 0;
int sensorValue = 0;
uint8_t battery_percentage = 0;

// setup code here, to run once:
void setup()
{

    if (debug == true) {

    // We are using Serial1 instead than Serial because we are going in standby
    // and the USB port could get confused during wakeup. To read the debug prints,
    // connect pins 13-14 (TX-RX) to a 3.3V USB-to-serial converter

    Serial1.begin(115200);
    while (!Serial1) {}
    Serial.println("Debug is true, Starting setup");
  }

  if (!SigFox.begin()) {
    //something is really wrong, try rebooting
    reboot();
  }

  //Send module to standby until we need to send a message
  SigFox.end();

  if (debug == true) {
    // Enable debug prints and LED indication if we are testing
    SigFox.debug();
  }

  // attach pin 1 to a switch and enable the interrupt on voltage rising event
  pinMode(1, INPUT_PULLUP);
  LowPower.attachInterruptWakeup(1, alarmEvent1, RISING);

  // enable the LED output on the defined pin_led
  pinMode(PIN_LED, OUTPUT); // sets the digital pin as output

  // set the LED output to the value in ledState. Mainly used fo the doubleclick action to reverse the LED state.
  digitalWrite(PIN_LED, ledState);

} // setup

void alarmEvent1() {
  alarm_source = 1;

  Serial.println("Click button");
  digitalWrite(PIN_LED, HIGH);
  // battery SoC calculation
  analogReadResolution(10);
  analogReference(AR_INTERNAL1V0);

  sensorValue = analogRead(ADC_BATTERY);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 4.3V):
  // if you're using a 3.7 V Lithium battery pack - adjust the 3 to 3.7 in the following formulas
  voltage = sensorValue * (3 / 1023.0);

  //battery percentage calculation
  // 2.2 is the cutoff voltage so adjust it if you're using a 3.7 V battery pack

  battery_percentage = ((voltage - 2.2) / (3 - 2.2)) * 100;

  analogReference(AR_DEFAULT);
}

// main code here, to run repeatedly:
void loop()
{
  // Sleep until an event is recognized
  LowPower.sleep();

  // if we get here it means that an event was received
  SigFox.begin();

  if (debug == true) {
    Serial1.println("Alarm event on sensor " + String(alarm_source));
  }
  delay(100);

// 3 bytes (ALM) + 8 bytes (ID as String) + 1 byte (source) < 12 bytes
// 414c4d = ALM + 313030 = 100 + 1 or 2 for the alarm_source so example 414c4d3130301
  String to_be_sent = "ALM" + String(battery_percentage) +  String(alarm_source);

  // sending the payload to Sigfox server
  SigFox.beginPacket();
  SigFox.print(to_be_sent);
  int ret = SigFox.endPacket();

  // shut down module, back to standby
  digitalWrite(PIN_LED, LOW);
  SigFox.end();

  if (debug == true) {
    if (ret > 0) {
      Serial1.println("No transmission");
    } else {
      Serial1.println("Transmission ok");
    }

    Serial1.println(SigFox.status(SIGFOX));
    Serial1.println(SigFox.status(ATMEL));

  }

} // loop

void reboot() {
  NVIC_SystemReset();
  while (1);
}

// End