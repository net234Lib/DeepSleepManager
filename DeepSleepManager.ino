/*************************
   Test DeepSleepManger
   net234 04/01/2021

   0001 simple test ESP.deepSleep(time_in_us)
    Memory dont survive a deep sleep (including section (".noinit") )
    after a deepsleep external system reset (REASON_EXT_SYS_RST) reset are seen as REASON_DEEP_SLEEP_AWAKE it logical but not obvious

*/

#include <Arduino.h>
#include <ESP8266WiFi.h>

#define D_println(x) Serial.print(F(#x " => '")); Serial.print(x); Serial.println("'");


#define APP_VERSION   "TestDeepSleepManager"

// GPIO2 on ESP32
//LED_1 D4(GPIO2)   LED_BUILTIN HERE
//LED_2 D0(GPIO16)
#define LED1        LED_BUILTIN
#define LED1_ON LOW
#define LED1_OFF (!LED1_ON)


// User button
//BP0 (MOSI)   D7   GPIO13 is Used as BP0 status (pullup)
//BP0 (RXD0)    RX0 GPIO3
#define BP0 D7  
#define BP0_DOWN LOW

#define POWER_ENABLE  16
#define PE_OFF LOW


// This lib need :
// A connection between D0 and RST with a 1K resistor or a DIODE see documentation
// A user button so we can abort a very long deep sleep (here BP0)
#include "DeepSleepManager.h"

//// restart reason keywords
////enum rst_reason {
//// REASON_DEFAULT_RST = 0, /* normal startup by power on */
//// REASON_WDT_RST = 1, /* hardware watch dog reset */
//// REASON_EXCEPTION_RST = 2, /* exception reset, GPIO status won't change */
//// REASON_SOFT_WDT_RST   = 3, /* software watch dog reset, GPIO status won't change */
//// REASON_SOFT_RESTART = 4, /* software restart ,system_restart , GPIO status won't change */
//// REASON_DEEP_SLEEP_AWAKE = 5, /* wake up from deep-sleep */
//// REASON_EXT_SYS_RST      = 6 /* external system reset */
////};
//// specific reason to
//#define REASON_USER_BUTTON  10
////( (rst_reason)10)
//#define REASON_RESTORE_WIFI 11
//#define REASON_NOT_INITED   12

DeepSleepManager MyDeepSleepManager;
# define TIMER_DEEPSLEEP 60  // Laps for the DeepSleep in seconds

bool bp0Status;
bool bpD0Status;


void setup() {
  // Setup BP0
  pinMode( BP0, INPUT_PULLUP);

  // init Serial

  if ( MyDeepSleepManager.getRstReason(BP0) == REASON_DEEP_SLEEP_AWAKE ) {
    Serial.begin(115200);
    Serial.println(F("\r" APP_VERSION));



//
//    Serial.print("MyDeepSleepManager.rstReason = ");
//    Serial.println(MyDeepSleepManager.getRstReason());
//    Serial.print("MyDeepSleepManager.WiFiLocked = ");
//    Serial.println(MyDeepSleepManager.WiFiLocked);
//    Serial.print("MyDeepSleepManager.bootCounter = ");
//    Serial.println(MyDeepSleepManager.bootCounter);
//    Serial.print("MyDeepSleepManager.estimatedSleepTime = ");
//    Serial.println(MyDeepSleepManager.estimatedSleepTime);
//
//    // put here the code you want to do with minimal power (not with WiFi)
//    // remember that we just rebooted so you cant read back data from standard variables (use RTCmemory)
//    // if you dont have anything more to do go back to sleep with a MyDeepSleepManager.startDeepSleep(time_in_seconds)
//    // you will be back here in "time_in_seconds" and this number can be more than one year (1 year = 356 * 24 * 60 * 60)
//
//    Serial.println("-->Go backto sleep");
    //Serial.println(micros());
    MyDeepSleepManager.startDeepSleep(TIMER_DEEPSLEEP);  // go back to deep sleep
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
    case REASON_DEEP_SLEEP_AWAKE:  Serial.println(F("->boot from a deep sleep")); break;
    case REASON_USER_BUTTON: Serial.println(F("->boot from a deep sleep aborted with BP User")); break;
    case REASON_SOFT_RESTART: Serial.println(F("->boot after a soft Reset")); break;
    default:
      Serial.print(F("->boot reason = "));
      Serial.println(MyDeepSleepManager.getRstReason());
  }
  Serial.print("MyDeepSleepManager.WiFiLocked = ");
  Serial.println(MyDeepSleepManager.WiFiLocked);
  Serial.print("MyDeepSleepManager.bootCounter = ");
  Serial.println(MyDeepSleepManager.getBootCounter());
  Serial.print("MyDeepSleepManager.remainingTime = ");
  Serial.println(MyDeepSleepManager.getRemainingTime());




// if you need to use WiFi call MyDeepSleepManager.restoreWiFi() this will to a restart to unlock wifi
  if ( MyDeepSleepManager.WiFiLocked) {
    Serial.println("-->Restore WiFi");
    MyDeepSleepManager.WiFiUnlock();
    // !! restore WiFi will make a special reset so we never arrive here !!
  }

  //  Serial.print("Wifi Mode=");
  //  Serial.println(WiFi.getMode());



  Serial.println(F( APP_VERSION ));


  WiFi.forceSleepBegin();  // this do  a WiFiMode OFF  !!! 21ma
  // return to sleep


  bp0Status = digitalRead(BP0);
  Serial.print(("BP_0 = "));
  Serial.println(bp0Status);
  bpD0Status = digitalRead(D0);
  Serial.print(("BP_D0 = "));
  Serial.println(bpD0Status);

  Serial.println(F("Bonjour ..."));

  Serial.print(F("compteur = "));
  Serial.println(MyDeepSleepManager.getBootCounter());
  Serial.println(F("Type S for DeepSleep"));
  //  Serial.print(F("RTC Time="));
  //  Serial.println(system_get_rtc_time());
  bp0Status = !digitalRead(BP0);
}

void loop() {
  if (Serial.available()) {
    char aChar = (char)Serial.read();
    if (aChar == 'S') {
      Serial.print(F("PowerDown 5 sec"));
      ESP.deepSleep(5 * 1E6, RF_DISABLED);

    }
    if (aChar == 'L') {
      Serial.print(F("PowerDown 1 Hour"));
      ESP.deepSleep(60 * 60 * 1E6, RF_DISABLED);
    }
    if (aChar == 'D') {
      Serial.print(F("DeepSleep 15 sec"));
      MyDeepSleepManager.startDeepSleep(15); // start a deepSleepMode with 15 sec

    }


    if (aChar == 'H') {
      Serial.println(F("Hard reset"));
      delay(10);
      pinMode(POWER_ENABLE, OUTPUT);
      digitalWrite(POWER_ENABLE, PE_OFF);  //
      delay(1000);
      Serial.println(F("Soft Rest"));
      ESP.reset();
    }
    if (aChar == 'R') {
      Serial.print(F("Soft reset"));
      ESP.reset();
    }

  }
  static uint32_t lastDown = millis();
  if ( bp0Status != digitalRead(BP0) ) {
    bp0Status = !bp0Status;
    Serial.print(F("BP0 = "));
    Serial.println(bp0Status);
        Serial.print(F("D0 = "));
    Serial.println(digitalRead(D0));

    digitalWrite( LED1 , LED1_OFF );
    delay(100);
    digitalWrite( LED1 , LED1_ON );
    if (bp0Status == BP0_DOWN) {
      lastDown = millis();
    } else {
      if ( millis() - lastDown  > 3000 ) {
        Serial.print(F("PowerDown 1 Hour"));
        MyDeepSleepManager.startDeepSleep(10); // start a deepSleepMode with 15 sec

      }
    }
  }

  delay(10);
}
