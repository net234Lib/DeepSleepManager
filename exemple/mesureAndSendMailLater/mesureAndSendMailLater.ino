

/*************************
   mesureAndSendMailLater DeepSleepManger Exemple
   net234 25/01/2021

   Wake up on regular interval to get some a standard DHT11 temperature and humidity
   Store data on a local file
   Then send a mail with the result

   // This exemple need :
  // A connection between D0 and RST with a 1K resistor or a diode see documentation
  // A DHT11 (the temp/humidity sensor of your Arduino Kit) connected to D6
  // Eventualy a user button to abort a very long deep sleep (here BP0) connected to D7


*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Initialize the client library
//WiFiClient client;

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


#include <Time.h>


// instance for  the DeepSleepManager
#include <DeepSleepManager.h>
DeepSleepManager MyDeepSleepManager;

// Instance for the DHT11 replace this by whathether you want to measure
#include <DHTesp.h>
DHTesp MyDHT11;

// Install Little FS file system
#include "LittleFS.h" // LittleFS is declared
#define MyFS LittleFS
// if you prefer SPIFFS
//#include "FS.h" // SPIFFS is declared
//#define MyFS SPIFFS

// status of push button connected on D7
bool bp0Status;




void setup() {
  // Setup BP0
  pinMode( BP0, INPUT_PULLUP);

  uint8_t rstReason = MyDeepSleepManager.getRstReason(BP0);
  if ( rstReason == REASON_DEEP_SLEEP_AWAKE  || rstReason == REASON_DEEP_SLEEP_TERMINATED ) {

    // here we start serial to show that we are awake
    Serial.begin(115200);
    Serial.print(F("\n" APP_VERSION " - "));
    Serial.print("MyDeepSleepManager.remainingTime = ");
    Serial.println(MyDeepSleepManager.remainingTime);

    Serial.print("time() = ");

    time_t now = time(nullptr);
    Serial.print(now);
    Serial.print(F(" - "));
    Serial.println(ctime(&now));

    // Init DHT11 and get Values
    MyDHT11.setup(D6, DHTesp::DHT11); // Connect DHT sensor to D6
    TempAndHumidity  dht11Values = MyDHT11.getTempAndHumidity();

    // Init FS system and save Value on  local File:data.csv
    MyFS.begin();
    File f = MyFS.open("/data.csv", "a");
    if (!f) {
      Serial.println("file open failed");
    } else {
      f.print(now);
      f.print("\t");
      f.print(MyDHT11.getStatusString());
      f.print("\t");
      f.print(dht11Values.humidity, 1);
      f.print("\t");
      f.print(dht11Values.temperature, 1);
      f.println();
      Serial.print(F("File Size = "));
      Serial.println(f.size());
      f.close();

    }
    MyFS.end();
    if ( rstReason == REASON_DEEP_SLEEP_AWAKE) MyDeepSleepManager.continueDeepSleep();  // go back to deep sleep
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
    case REASON_RESTORE_WIFI: Serial.println(F("->boot from a restore WiFI command")); break;
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


  // if you need to use WiFi call MyDeepSleepManager.restoreWiFi() this will to a restart to unlock wifi
  if ( MyDeepSleepManager.WiFiLocked) {
    Serial.println("-->Restore WiFi");
    MyDeepSleepManager.WiFiUnlock();
    // !! restore WiFi will make a special reset so we never arrive here !!
  }



  Serial.println(F( APP_VERSION ));




  Serial.print("Wifi Mode=");
  Serial.println(WiFi.getMode());

  if (WiFi.getMode() != WIFI_STA) {
    Serial.println(F("!!! FIRST WiFi init !!!"));
    WiFi.mode(WIFI_STA);
    WiFi.begin("mon_wifi", "ultrasecret");
    //    Serial.print("Wifi Mode=");
    //    Serial.println(WiFi.getMode());
  }



  bp0Status = digitalRead(BP0);
  Serial.print(("BP_0 = "));
  Serial.println(bp0Status);

  // init time
  MyFS.begin();

  Serial.println(F("==== data.csv ====="));
  File f = MyFS.open("/data.csv", "r");
  if (f) {
    while (f.available()) {
      Serial.write(f.read());
    }
    f.close();
  }
  Serial.println(F("==== eof datat.csv ="));



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
  static int oldWiFiStatus = 999;
  int WiFiStatus = WiFi.status();
  if (oldWiFiStatus != WiFiStatus) {
    oldWiFiStatus = WiFiStatus;
    Serial.print(F("WiFI = "));
    Serial.println(WiFiStatus);

    if (WiFiStatus == WL_CONNECTED) {
      Serial.println(F("WiFI Connected"));
      // connect to captive.apple.com
      //      captive.apple.com
      //      connectivitycheck.gstatic.com
      //      detectportal.firefox.com
      https://success.tanaza.com/s/article/How-Automatic-Detection-of-Captive-Portal-works
      #define CAPTIVE "www.msftncsi.com"
      
      Serial.println(F("connect to " CAPTIVE ));

      HTTPClient http;  //Declare an object of class HTTPClient

      http.begin("http://" CAPTIVE);  //Specify request destination
      const char * headerKeys[] = {"date"} ;
      const size_t numberOfHeaders = 1;
      http.collectHeaders(headerKeys, numberOfHeaders);

      int httpCode = http.GET();                                  //Send the request
      Serial.print(F("http.GET()="));
      Serial.println(httpCode);
      if (httpCode > 0) { //Check the returning code
        String headerDate = http.header(headerKeys[0]);
        Serial.println(headerDate);

        //Date: Mon, 25 Jan 2021 21:18:52 GMT
        String payload = http.getString();   //Get the request response payload
        Serial.println(payload);             //Print the response payload

      }

      http.end();   //Close connection


      //      if (client.connect("captive.apple.com", 80)) {
      //        Serial.println("connected");
      //        // Make a HTTP request:
      //        client.println("GET / HTTP/1.0");
      //        client.println("Host: captive.apple.com");
      //        client.println("User-Agent: ardui/1.0");
      //        client.println("Accept: */*");
      //        client.println();
      //        String line;
      //        do {
      //          line = client.readStringUntil('\r');
      //          Serial.print(line);
      //        } while (line != "");
      //        Serial.println("closing connection");
      //        client.stop();
      //      }

    }
  }

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


    if (aChar == 'E') {
      Serial.println(F("-- Erase data.csv"));
      MyFS.remove("/data.csv");
    }

    if (aChar == 'P') {
      Serial.println(F("==== data.csv ====="));
      File f = MyFS.open("/data.csv", "r");
      if (f) {
        while (f.available()) {
          Serial.write(f.read());
        }
        f.close();
      }
      Serial.println(F("==Eof data.csv =="));

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
