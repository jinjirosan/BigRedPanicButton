/*
 Version 0.1.1

 This code is for the BigRedPanicButton. A home safety device which sends an alert over the Sigfox network when the button is pressed.
 The signal results in a callback to a specific email address or emergency SMS gateway.

 I developed this for my parents (who are of age) as a failsafe in-case someone breaks in or hwne there is an issue with pushy salespeople at the door. I will get notified immediately.
 
*/


#include <OneButton.h>
#include <SigFox.h>
#include <ArduinoLowPower.h>

#define PIN_INPUT 1
#define PIN_LED 3


// Setup a new OneButton on pin PIN_INPUT
// The 2. parameter activeLOW is true, because external wiring sets the button to LOW when pressed.
OneButton button(PIN_INPUT, true);

// In case the momentary button puts the input to HIGH when pressed:
// The 2. parameter activeLOW is false when the external wiring sets the button to HIGH when pressed.
// The 3. parameter can be used to disable the PullUp .
// OneButton button(PIN_INPUT, false, false);

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
    Serial.println("One Button Example with polling.");
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

  // enable the standard led on pin 13.
  pinMode(PIN_LED, OUTPUT); // sets the digital pin as output

  // enable the standard led on pin 13.
  digitalWrite(PIN_LED, ledState);

  // link the click function to be called on a panicclick event.
  button.attachClick(panicClick);

    // link the click function to be called on a panicclick event.
  button.attachDoubleClick(doubleClick);

  // Period of time in which to ignore additional level changes.
  button.setDebounceTicks(80);

  // Timeout used to distinguish single clicks from double clicks.
  button.setClickTicks(500);

  // Duration to hold a button to trigger a long press.
  button.setPressTicks(800);

} // setup


// main code here, to run repeatedly:
void loop()
{
  // keep watching the push button:
  button.tick();

  // You can implement other code in here or just wait a while
  delay(10);
} // loop


// this function will be called when the button was clicked once 
void panicClick()
{
  Serial.println("panicClick");

  digitalWrite(PIN_LED, HIGH);
} // panicClick

// this function will be called when the button was clicked once 
void doubleClick()
{
  Serial.println("x2");

  ledState = !ledState; // reverse the LED
  digitalWrite(PIN_LED, ledState);
} // panicClick

void reboot() {
  NVIC_SystemReset();
  while (1);
}

// Endi