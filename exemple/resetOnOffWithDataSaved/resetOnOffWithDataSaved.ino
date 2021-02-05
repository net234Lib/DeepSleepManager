/*************************
   resetOnOffWithDataSaved DeepSleepManger
   net234 05/02/2021

   use reset button as on/off togle switch
   with the standard blinkWithoutDelay demo : (  http://www.arduino.cc/en/Tutorial/BlinkWithoutDelay )

   Data in the struct "MySavedData" are restored between each reset

*/

#include <Arduino.h>
// trick to trace variables
#define D_println(x) Serial.print(F(#x " => '")); Serial.print(x); Serial.println("'");

// instance for  the DeepSleepManager
#include <DeepSleepManager.h>
DeepSleepManager MyDeepSleepManager;

// all variable stored in the structure MySavedData will be restored with a call to DeepSleepManager.restoreStruct(MySavedData)
// the variables must be basic variable as int, long, float, char[]
// object variables as String, pointers won't work
// the total size of the struc must be under 400 Bytes



//struct __attribute__((packed)) myDSMSavedData {
struct  myDSMSavedData {
  byte aNumber = 1;
  char aName[32] = "anonymous Arduino";
};

myDSMSavedData MySavedData;

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

  // restore the saved Data from the RTC memory

  Serial.println("\r\n\n\n" "resetOnOff resetOnOffWithDataSaved");
  MyDeepSleepManager.restoreRTCStruct(MySavedData);

  D_println(sizeof(MySavedData));



  Serial.print("Hello my name is '"); Serial.print(MySavedData.aName); Serial.println("'");
  Serial.print("This is the "); Serial.print(MySavedData.aNumber); Serial.println("' time that I wake up since I was powered on.");
  Serial.println("You can change my name if you type 'N mynewname'");
  MySavedData.aNumber++;
  MyDeepSleepManager.saveRTCStruct(MySavedData);
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
    char aChar = (char)Serial.read();
    if (aChar == 'N') {
      Serial.println("Change my name : 'N myNewName'");
      if ( (char)Serial.read() == ' ') {
        String aName = Serial.readString();
        Serial.println(aName);
        aName.trim();
        aName = aName.substring(0,30);
        Serial.print("My new name is '");Serial.print(aName);Serial.println("'");
        strcpy(MySavedData.aName,aName.c_str());
        MyDeepSleepManager.saveRTCStruct(MySavedData);
        
      }
    }
  }
}
