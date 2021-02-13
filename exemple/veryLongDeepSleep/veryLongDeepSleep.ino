/*************************
   veryLongDeepSleep  example with DeepSleepManger
   net234 04/01/2021

   Allow to make very long deep sleep ,

   A connection between D0 and RST with a 1K resistor or a DIODE must be done
   A user button to abort a very long deep sleep (here BP0)


*/

#include <Arduino.h>
#include <ESP8266WiFi.h>

#define APP_VERSION   "veryLongDeepSleep"

// GPIO2 on ESP8266
#define LED1        LED_BUILTIN
#define LED1_ON LOW
#define LED1_OFF (!LED1_ON)


// User button to unlock the deep Sleep (with press reset while BP0 down)
//BP0 (MOSI)   D7   GPIO13 is Used as BP0 status (pullup)

#define BP0 D7
#define BP0_DOWN LOW


// This lib need :
// A connection between D0 and RST with a 1K resistor or a DIODE must be done
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

    // here we start serial to show that we are awake with a 'regular' DEEP_SLEEP_AWAKE
    Serial.begin(115200);
    Serial.println(F("\n" APP_VERSION ));
    Serial.print("MyDeepSleepManager.getPowerOnTimestamp = ");
    Serial.println(niceDisplayTime(MyDeepSleepManager.getPowerOnTimestamp()));
    Serial.print("MyDeepSleepManager.getActualTimestamp  = ");
    Serial.println(niceDisplayTime(MyDeepSleepManager.getActualTimestamp()));
    Serial.print("MyDeepSleepManager.remainingTime       = ");
    Serial.println(niceDisplayTime(MyDeepSleepManager.getRemainingTime()));

    MyDeepSleepManager.continueDeepSleep();  // go back to deep sleep
  }
  // we are here because longDeepSleep is fully elapsed or a reset with BP0 append
  // if you need to use WiFi call MyDeepSleepManager.restoreWiFi() this will to a restart to unlock wifi
  if ( MyDeepSleepManager.WiFiLocked) {
    // "-->Restore WiFi"
    MyDeepSleepManager.WiFiUnlock();
    // !! restore WiFi will make a special reset so we never arrive here !!
  }


  digitalWrite(LED1, LED1_ON);

  Serial.begin(115200);
  Serial.println(F(APP_VERSION));


  Serial.print("MyDeepSleepManager.rstReason = ");
  Serial.print(MyDeepSleepManager.getTxtRstReason());
  Serial.print("MyDeepSleepManager.WiFiLocked = ");
  Serial.println(MyDeepSleepManager.WiFiLocked);
  Serial.print("MyDeepSleepManager.bootCounter = ");
  Serial.println(MyDeepSleepManager.getBootCounter());
  Serial.print("MyDeepSleepManager.getPowerOnTimestamp = ");
  Serial.println(niceDisplayTime(MyDeepSleepManager.getPowerOnTimestamp()));
  Serial.print("MyDeepSleepManager.getActualTimestamp  = ");
  Serial.println(niceDisplayTime(MyDeepSleepManager.getActualTimestamp()));
  Serial.print("MyDeepSleepManager.remainingTime       = ");
  Serial.println(niceDisplayTime(MyDeepSleepManager.getRemainingTime()));

  Serial.println(F( APP_VERSION ));


  //WiFi.forceSleepBegin();  // this do  a WiFiMode OFF  !!! save some power (Need

  bp0Status = digitalRead(BP0);
  Serial.print(("BP_0 = "));
  Serial.println(bp0Status);

  // This exemple supose a WiFi connection so if we are not WIFI_STA mode we force it
  if (WiFi.getMode() != WIFI_STA) {
    Serial.println(F("!!! Force WiFi to STA mode !!!  should be done only ONCE even if we power off "));
    WiFi.mode(WIFI_STA);
    //WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }



  Serial.println(F("Bonjour ..."));

  Serial.println(F("Type S for DeepSleep 24 Hours"));
  Serial.println(F("Type T for DeepSleep 1 Minute"));
  Serial.println(F("Type U for DeepSleep 5 Minute"));
  Serial.println(F("Type V for DeepSleep 1 Hour"));
  Serial.println(F("RESET only will just skip 1 increment in the deep sleep time"));
  Serial.println(F("press BP0  to full abort DeepSleep"));
  Serial.println(F("press only RST to skip 1 DeepSleep increment"));
  Serial.println(F(">"));

  bp0Status = !digitalRead(BP0);
}
bool wifiConnected = false;
//uint32_t lastnow = 0;
void loop() {
  // Save the time when it change so we can reboot with localtime almost acurate
  uint32_t lastnow = now();
  if ( lastnow != MyDeepSleepManager.getActualTimestamp() ) {
    MyDeepSleepManager.setActualTimestamp(lastnow);
    if (second() == 0) Serial.println(niceDisplayTime(lastnow)); // every minute
    // this will append every seconds

    // If we are not connected we warn the user every 30 seconds that we need to update credential
    if ( WiFi.status() == WL_CONNECTED ) {
      if ( !wifiConnected) {
        wifiConnected = true;
        Serial.print(F("Connected to Wifi : "));
        Serial.println(WiFi.SSID());
        setSyncProvider(getNtpTime);
        setSyncInterval(5 * 60);
      }
    } else {
      wifiConnected = false;
      // every 30 sec
      if ( now() % 15 == 10 ) {
        Serial.print(F("device not connected to local WiFi : "));
        Serial.println(WiFi.SSID());
        Serial.println(F("type 'W' to adjust WiFi credential"));
      }
    }
  }



  if (Serial.available()) {
    char aChar = (char)Serial.read();
   if (aChar == 'L') {
      Serial.println(F("-- start DeepSleep for 12 Hours"));
      Serial.println(F("   each press on RESET will skip 3 hours"));
      Serial.println(F("<-- GO"));
      MyDeepSleepManager.startDeepSleep(12 * 60 * 60); // start a deepSleepMode with 1 hours incremental
    }

    
    
    if (aChar == 'S') {
      Serial.println(F("-- start DeepSleep for 4 Hours"));
      Serial.println(F("   each press on RESET will skip 1 hours"));
      Serial.println(F("<-- GO"));
      MyDeepSleepManager.startDeepSleep(4 * 60 * 60, 60 * 60); // start a deepSleepMode with 1 hours incremental
    }

   if (aChar == 's') {
      Serial.println(F("-- start DeepSleep for 4 Hours max incremental"));
      Serial.println(F("   each press on RESET will skip 3 hours"));
      Serial.println(F("<-- GO"));
      MyDeepSleepManager.startDeepSleep(4 * 60 * 60); // start a deepSleepMode with 1 hours incremental
    }



    if (aChar == 'T') {
      Serial.println(F("-- start DeepSleep for 1 Minute with a 10 Second incremental"));
      Serial.println(F("<-- GO"));
      MyDeepSleepManager.startDeepSleep( 60 , 10 );
    }

    if (aChar == 't') {
      Serial.println(F("-- start DeepSleep for 1 Minute with no incremental"));
      Serial.println(F("<-- GO"));
      MyDeepSleepManager.startDeepSleep( 60 );
    }


   if (aChar == '1') {
      Serial.println(F("-- start DeepSleep for 10 Minute with no incremental"));
      Serial.println(F("<-- GO"));
      MyDeepSleepManager.startDeepSleep( 10 * 60 );
    }

   if (aChar == '2') {
      Serial.println(F("-- start DeepSleep fof 20 Minute with no incremental"));
      Serial.println(F("<-- GO"));
      MyDeepSleepManager.startDeepSleep( 20 * 60 );
    }
   if (aChar == '3') {
      Serial.println(F("-- start DeepSleep for 30 Minute with no incremental"));
      Serial.println(F("<-- GO"));
      MyDeepSleepManager.startDeepSleep( 30 * 60 );
    }


    if (aChar == 'U') {
      Serial.println(F("-- start DeepSleep for 5 Minutes with a 30 Seconds incremental"));
      Serial.println(F("<-- GO"));
      MyDeepSleepManager.startDeepSleep( 5 * 60 , 30 );
    }

    if (aChar == 'u') {
      Serial.println(F("-- start DeepSleep for 5 Minutes no incremental"));
      Serial.println(F("<-- GO"));
      MyDeepSleepManager.startDeepSleep( 5 * 60 );
    }


    if (aChar == 'V') {
      Serial.println(F("-- start DeepSleep for 1 Hour with a 1 Minute incremental"));
      Serial.println(F("<-- GO"));
      MyDeepSleepManager.startDeepSleep( 60 * 60, 60 ); // start a deepSleepMode with 15 sec
    }

   if (aChar == 'v') {
      Serial.println(F("-- start DeepSleep for 1 Hour with no incremental"));
      Serial.println(F("<-- GO"));
      MyDeepSleepManager.startDeepSleep( 60 * 60 ); // start a deepSleepMode with 15 sec
    }


    if (aChar == 'D') {
      Serial.println(F("-- set time to date of 01/01/2001 00:00:00"));
      setTime(0, 0, 0, 1, 1, 2001);
    }

    if (aChar == '.') {
      Serial.println(niceDisplayTime(now()));
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
    if (aChar == 'W') {
      Serial.println(F("SETUP WIFI : 'W WifiName,password"));
      if ( Serial.read() == ' ') {
        String ssid = Serial.readStringUntil(',');
        Serial.println(ssid);
        ssid.trim();
        if (ssid != "") {
          String pass = Serial.readStringUntil('\n');
          pass.replace("\r", "");
          pass.trim();
          Serial.println(pass);
          WiFi.begin(ssid, pass);
          Serial.println("Setup WiFi done");
          wifiConnected = false;
        }
      }
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


// display time a nice way
String str2digits(const uint8_t digits) {
  String txt;
  if (digits < 10)  txt = '0';
  txt += digits;
  return txt;
}



String niceDisplayTime(time_t time) {

  String txt;
  // we supose that time < NOT_A_DATE_YEAR is not a date
  if ( year(time) < NOT_A_DATE_YEAR ) {
    txt = "          ";
    txt += time / (24 * 3600);
    txt += ' ';
    txt = txt.substring(txt.length() - 10);
  } else {

    txt = str2digits(day(time));
    txt += '/';
    txt += str2digits(month(time));
    txt += '/';
    txt += year(time);
  }
  txt += " ";
  //  static String date;
  //  if (txt == date) {
  //    txt = "";
  //  } else {
  //    date = txt;
  //  }
  txt += str2digits(hour(time));
  txt += ':';
  txt += str2digits(minute(time));
  txt += ':';
  txt += str2digits(second(time));
  return txt;
}
