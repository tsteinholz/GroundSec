//----------------------------------------------------------------------------//
//                   ______________________________________                   //
//         ________|                                      |_______            //
//         \       |       This file is a part of         |      /            //
//          \      |              GroundSec               |     /             //
//          /      |______________________________________|     \             //
//         /__________)                                (_________\            //
//                                                                            //
//                             GroundSec(c) 2016                              //
//                                                                            //
//                        GNU GENERAL PUBLIC LICENSE                          //
//                         Version 3, 29 June 2007                            //
//                        													                          //
//     Copyright (C) 2007 Free Software Foundation, Inc. <http://fsf.org/>    //
//       Everyone is permitted to copy and distribute verbatim copies	        //
//         of this license document, but changing it is not allowed.          //
//                                                                            //
//----------------------------------------------------------------------------//

const int ledPin =  6;// the number of the LED pin

// Variables will change :
int ledState = LOW;             // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change :
const long interval = 1000;           // interval at which to blink (milliseconds)

void setup() {
  Serial.begin(9600);
  // set the digital pin as output:
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // here is where you'd put code that needs to be running all the time.

  // check to see if it's time to blink the LED; that is, if the
  // difference between the current time and last time you blinked
  // the LED is bigger than the interval at which you want to
  // blink the LED.
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    Serial.print("blinking led!\n");

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
  }
}

