/*************************
   DeepSleep24Hours DeepSleepManger
   net234 04/01/2021

   0001 simple test ESP.deepSleep(time_in_us)
    Memory dont survive a deep sleep (including section (".noinit") )
    after a deepsleep external system reset (REASON_EXT_SYS_RST) reset are seen as REASON_DEEP_SLEEP_AWAKE it logical but not obvious

*/

#include <Arduino.h>
#include <ESP8266WiFi.h>

#define APP_VERSION   "DeepSleep24Hours"

// GPIO2 on ESP32
//LED_1 D4(GPIO2)   LED_BUILTIN HERE
//LED_2 D0(GPIO16)
#define LED1        LED_BUILTIN
#define LED1_ON LOW
#define LED1_OFF (!LED1_ON)


// User button to unlock the deep Sleep (with press reset while BP0 down)
//BP0 (MOSI)   D7   GPIO13 is Used as BP0 status (pullup)

#define BP0 D7
#define BP0_DOWN LOW


// This lib need :
// A connection between D0 and RST with a 1K resistor or a DIODE see documentation
// A user button so we can abort a very long deep sleep (here BP0)
#include "DeepSleepManager.h"

// instance for  the DeepSleepManager
DeepSleepManager MyDeepSleepManager;

// status of push button connected on D7
bool bp0Status;


void setup() {
  // Setup BP0  
  pinMode( BP0, INPUT_PULLUP);

  if ( MyDeepSleepManager.getRstReason(BP0) == REASON_DEEP_SLEEP_AWAKE ) {

    // here we start serial to show that we are awake     
    Serial.begin(115200);
    Serial.print(F("\n" APP_VERSION " - "));
    Serial.print("MyDeepSleepManager.remainingTime = ");
    Serial.println(MyDeepSleepManager.remainingTime);
    

    MyDeepSleepManager.continueDeepSleep();  // go back to deep sleep
  }
  // we are here because longDeepSleep is fully elapsed


  digitalWrite(LED1, LED1_ON);

  Serial.begin(115200);
  Serial.println(F(APP_VERSION));


  Serial.print("MyDeepSleepManager.rstReason = ");
  Serial.print(MyDeepSleepManager.getRstReason());
  switch (MyDeepSleepManager.getRstReason()) {
    case REASON_DEFAULT_RST:  Serial.println(F("->Cold boot")); break;
    case REASON_EXT_SYS_RST:  Serial.println(F("->boot with BP Reset")); break;
    case REASON_DEEP_SLEEP_AWAKE:  Serial.println(F("->boot from a deep sleep pending")); break;
    case REASON_DEEP_SLEEP_TERMINATED: Serial.println(F("->boot from a deep sleep terminated")); break;
    case REASON_USER_BUTTON: Serial.println(F("->boot from a deep sleep aborted with BP User")); break;
    case REASON_SOFT_RESTART: Serial.println(F("->boot after a soft Reset")); break;
    default:
      Serial.print(F("->boot reason = "));
      Serial.println(MyDeepSleepManager.getRstReason());
  }
  Serial.print("MyDeepSleepManager.WiFiLocked = ");
  Serial.println(MyDeepSleepManager.WiFiLocked);
  Serial.print("MyDeepSleepManager.bootCounter = ");
  Serial.println(MyDeepSleepManager.bootCounter);
  Serial.print("MyDeepSleepManager.remainingTime = ");
  Serial.println(MyDeepSleepManager.remainingTime);




  Serial.println(F( APP_VERSION ));


  WiFi.forceSleepBegin();  // this do  a WiFiMode OFF  !!! 21ma
  // return to sleep


  bp0Status = digitalRead(BP0);
  Serial.print(("BP_0 = "));
  Serial.println(bp0Status);

  Serial.println(F("Bonjour ..."));

  Serial.println(F("Type S for DeepSleep 24 Hours"));
  Serial.println(F("Type T for DeepSleep 1 Minute"));
  Serial.println(F("Type U for DeepSleep 5 Minute"));
  Serial.println(F("Type V for DeepSleep 1 Hour"));
  Serial.println(F("RESET only will just skip 1 increment in the deep sleep time"));
  Serial.println(F("To full abort DeepSleep press and hold BP0 and press RESET"));
  Serial.println(F("press BP0 4 seconds to start DeepSleep 15 seconds"));
  Serial.println(F(">"));

  bp0Status = !digitalRead(BP0);
}

void loop() {
  if (Serial.available()) {
    char aChar = (char)Serial.read();
    if (aChar == 'S') {
      Serial.println(F("-- start DeepSleep for 24 Hours"));
      Serial.println(F("   each press on RESET will skip 1 hours"));
      Serial.println(F("<-- GO"));
      MyDeepSleepManager.startDeepSleep(24 * 60 * 60, 60 * 60); // start a deepSleepMode with 1 hours incremental
    }

    if (aChar == 'T') {
      Serial.println(F("-- start DeepSleep for 1 Minute with a 10 Second incremental"));
      Serial.println(F("<-- GO"));
      MyDeepSleepManager.startDeepSleep( 60 , 10 );
    }

    if (aChar == 'U') {
      Serial.println(F("-- start DeepSleep for 5 Minutes with a 30 Seconds incremental"));
      Serial.println(F("<-- GO"));
      MyDeepSleepManager.startDeepSleep( 5 * 60 , 30 );
    }

    if (aChar == 'V') {
      Serial.println(F("-- start DeepSleep for 1 Hour with a 1 Minute incremental"));
      Serial.println(F("<-- GO"));
      MyDeepSleepManager.startDeepSleep( 60 * 60, 60 ); // start a deepSleepMode with 15 sec
    }




    if (aChar == 'H') {
      Serial.println(F("-- Hard reset with D0 -> LOW"));
      delay(10);
      pinMode(D0, OUTPUT);
      digitalWrite(D0, LOW);  //
      delay(1000);
      Serial.println(F("-- Soft Rest"));
      ESP.reset();
    }
    if (aChar == 'R') {
      Serial.print(F("-- Soft reset"));
      ESP.reset();
    }
  }
  static uint32_t lastDown = millis();
  if ( bp0Status != digitalRead(BP0) ) {
    bp0Status = !bp0Status;
    Serial.print(F("BP0 = "));
    Serial.println(bp0Status);

    digitalWrite( LED1 , LED1_OFF );
    delay(100);
    digitalWrite( LED1 , LED1_ON );
    if (bp0Status == BP0_DOWN) {
      lastDown = millis();
    }
  }

  // if you want to start deep sleep without terminale connected
  // start a deepsleep 15 sec with a long press BP0
  if (bp0Status == BP0_DOWN && millis() - lastDown  > 3000 ) {
    Serial.println(F("DeepSleep 15 sec"));
    MyDeepSleepManager.startDeepSleep(15);
  }
  delay(10);  // avoid rebounce of BP0 easy way :)
}
