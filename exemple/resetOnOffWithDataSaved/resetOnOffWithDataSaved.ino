/*************************
   resetOnOffWithDataSaved DeepSleepManger
   net234 05/02/2021  https://github.com/net234/DeepSleepManager

   use reset button as on/off togle switch
   with the standard blinkWithoutDelay demo : (  http://www.arduino.cc/en/Tutorial/BlinkWithoutDelay )

   Data in the struct "MySavedData" are restored between each reset

 This file is part of DeepSleepManager.

    DeepSleepManager is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    DeepSleepManager is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with betaEvents.  If not, see <https://www.gnu.org/licenses/lglp.txt>.



*/

#include <Arduino.h>

// instance for  the DeepSleepManager
#include <DeepSleepManager.h>
DeepSleepManager MyDeepSleepManager;

// all variable stored in the structure MySavedData will be restored with a call to DeepSleepManager.restoreStruct(MySavedData)
// the variables must be basic variable as int, long, float, char[]
// object variables as String, pointers won't work
// the total size of the struc must be under 450 Bytes

const  uint8_t  max_name_size = 20;
struct {
  byte aNumber = 1;
  char aName[max_name_size + 1] = "anonymous Arduino";
}  MySavedData;

// classic code from "BlinkWithoutDelay.ino" examples 02.Digital

// constants won't change. Used here to set a pin number:
const int ledPin =  LED_BUILTIN;// the number of the LED pin

// Variables will change:
int ledState = LOW;             // ledState used to set the LED


// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change:
const long interval = 1000;           // interval at which to blink (milliseconds)



// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  
  // enable button RST On/Off mode see : exemple resetOnOff.ino
  if ( MyDeepSleepManager.getRstReason() == REASON_EXT_SYS_RST ) {
    Serial.println("\r\n" "Going to sleep :) ... Bye.");
    MyDeepSleepManager.permanentDeepSleep();  // go back to deep sleep
  }

  Serial.println("\r\n\n\n" "resetOnOff resetOnOffWithDataSaved");

  // restore the saved Data from the RTC memory
  MyDeepSleepManager.restoreRTCData(RTC_DATA(MySavedData));

  Serial.print("Hello my name is '"); Serial.print(MySavedData.aName); Serial.println("'.");
  Serial.print("This is the "); Serial.print(MySavedData.aNumber); Serial.println("' time that I wake up since I was powered on.");
  Serial.println("You can change my name if you type 'N mynewname'.");
  Serial.println("You can turn me OFF if you type 'S' or press RST button.");

  // Increment a counter  this counter will be restored after a reset
  MySavedData.aNumber++;
  // Save the counter in RTC ram
  MyDeepSleepManager.saveRTCData(RTC_DATA(MySavedData));
  
  Serial.println("I am ON my led BLINK");

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);


}


// the loop function runs over and over again forever
// but a reset here will switch off
void loop() {
  // here is where you'd put code that needs to be running all the time.

  // check to see if it's time to blink the LED; that is, if the difference
  // between the current time and last time you blinked the LED is bigger than
  // the interval at which you want to blink the LED.
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

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
  }

  if (Serial.available()) {
    Serial.setTimeout(100);

    char aChar = (char)Serial.read();
    if (aChar == 'N') {
      Serial.println("Change my name : 'N myNewName'");
      delay(10);
      if ( (char)Serial.read() == ' ') {
        String aName = Serial.readString();
        aName.trim();
        aName = aName.substring(0, max_name_size );
        Serial.print("My new name is '"); Serial.print(aName); Serial.println("'");
        strcpy(MySavedData.aName, aName.c_str());
        MyDeepSleepManager.saveRTCData(RTC_DATA(MySavedData));
      }
    }
    
    if (aChar == 'S') {
      Serial.println("S = Going to Sleep ..  press RST to wake up");
      MyDeepSleepManager.permanentDeepSleep();
    }

  }
}
