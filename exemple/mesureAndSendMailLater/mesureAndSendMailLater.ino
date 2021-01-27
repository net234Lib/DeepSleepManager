

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

#define D_println(x) Serial.print(F(#x " => '")); Serial.print(x); Serial.println("'");
//Le croquis utilise 320764 octets (30 % ) de l'espace de stockage de programmes. Le maximum est de 1044464 octets.
//Les variables globales utilisent 28860 octets (35%) de mémoire dynamique, ce qui laisse 53060 octets pour les variables locales. Le maximum est de 81920 octets.


#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// include PRIVATE_MAIL_ADRESSE  for test
#include "private.h"

#define APP_VERSION   "DeepSleep24Hours"
#define WIFI_SSID     PRIVATE_WIFI_SSID
#define WIFI_PASSWORD PRIVATE_WIFI_PASSWORD
#define SEND_TO       PRIVATE_SEND_TO      // replace with your test adresse
#define SMTP_SERVER   "smtp.free.fr"       // replace with your FAI smtp server
#define SEND_FROM     PRIVATE_SEND_FROM    // replace with your test adresse
#define DATA_FILENAME "/data.csv"



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
  //setTime(MyDeepSleepManager.getBootTimestamp());  // set the local system time

  if ( rstReason == REASON_DEEP_SLEEP_AWAKE  ) {



    // here we start serial to show that we are awake
    Serial.begin(115200);
    Serial.print(F("\n" APP_VERSION " - "));
    D_println(MyDeepSleepManager.getBootCounter());
    D_println(MyDeepSleepManager.getRemainingTime());


    // Init DHT11 and get Values
    MyDHT11.setup(D6, DHTesp::DHT11); // Connect DHT sensor to D6
    TempAndHumidity  dht11Values = MyDHT11.getTempAndHumidity();

    // Get local time
    time_t localTime = now();
    D_println(Ctime(localTime));

    // Init FS system and save Value and localTime on  local File:data.csv
    MyFS.begin();
    File f = MyFS.open(DATA_FILENAME, "a");
    if (!f) {
      Serial.println("!!file open failed!!");
    } else {
      f.print(localTime);
      f.print("\t");
      f.print(dht11Values.humidity, 1);
      f.print("\t");
      f.print(dht11Values.temperature, 1);
      f.print("\t");
      f.print(MyDHT11.getStatusString());
      f.print("\n");
      D_println(f.size());
      f.close();

    }
    MyFS.end();

    if ( rstReason == REASON_DEEP_SLEEP_AWAKE) {
      MyDeepSleepManager.continueDeepSleep();  // go back to deep sleep
    }
  }
  // we are here because longDeepSleep is fully elapsed or user pressed BP0


  digitalWrite(LED1, LED1_ON);

  Serial.begin(115200);
  Serial.println(F(APP_VERSION));

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
  Serial.print("MyDeepSleepManager.getBootCounter = ");
  Serial.println(MyDeepSleepManager.getBootCounter());
  Serial.print("MyDeepSleepManager.getRemainingTime = ");
  Serial.println(MyDeepSleepManager.getRemainingTime());
  Serial.print("MyDeepSleepManager.getBootTimeStamp = ");

  Serial.println(Ctime(MyDeepSleepManager.getBootTimestamp()));
  Serial.print("MyDeepSleepManager.getPowerOnTimeStamp = ");
  Serial.println(Ctime(MyDeepSleepManager.getPowerOnTimestamp()));



  // if you need to use WiFi call MyDeepSleepManager.restoreWiFi() this will to a restart to unlock wifi
  if ( MyDeepSleepManager.WiFiLocked) {
    Serial.println("-->Restore WiFi");
    MyDeepSleepManager.WiFiUnlock();
    // !! restore WiFi will make a special reset so we never arrive here !!
  }

  // Wifi is restored as it was and will soon connect to wifi
  D_println(WiFi.getMode());

  // This exemple supose a WiFi connection so if we are not WIFI_STA mode we force it
  if (WiFi.getMode() != WIFI_STA) {
    Serial.println(F("!!! Force WiFi to STA mode !!!  should be done only ONCE even if we power off "));
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }

  bp0Status = digitalRead(BP0);
  D_println(bp0Status);

  // init file system
  MyFS.begin();


  Serial.println(F("==== data.csv ====="));
  printDataCSV();
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

bool mailSended = false;


void loop() {
  // Save the time when it change so we can reboot with localtime almost acurate
  if ( MyDeepSleepManager.getActualTimestamp() != now() ) {
    //    Serial.print(F("Save clock "));
    //    Serial.println( Ctime(now()) );
    MyDeepSleepManager.setActualTimestamp(now());
  }
  // 30 seconde au bout d'un deep sleep on relance
  static uint32_t oldMillis = millis();
  if ( millis() - oldMillis > 30  && MyDeepSleepManager.getRstReason() != REASON_DEEP_SLEEP_TERMINATED ) {

  }


  // check for connection to local WiFi
  static int oldWiFiStatus = 999;
  int WiFiStatus = WiFi.status();
  if (oldWiFiStatus != WiFiStatus) {
    oldWiFiStatus = WiFiStatus;
    Serial.print(F("WiFI = "));
    Serial.println(WiFiStatus);

    // on connection to WiFI we try to setup clock and send mail
    do {
      if (WiFiStatus != WL_CONNECTED)  break;

      Serial.println(F("WiFI Connected"));
      if (!connectedToInternet()) {
        Serial.println(F("Pas de liaison internet Internet"));
        break;
      }
      // dsisplay RTC status
      Serial.print("now() = ");
      Serial.println(Ctime(now()));
      Serial.print("bootTime = ");
      Serial.println( Ctime(MyDeepSleepManager.getBootTimestamp()) );
      Serial.print("powerTime = ");
      Serial.println( Ctime(MyDeepSleepManager.getPowerOnTimestamp()) );


      // do we restart from a end of deep sleep ?
      if ( MyDeepSleepManager.getRstReason() != REASON_DEEP_SLEEP_TERMINATED) {
        Serial.println(F("No job to when restart is not Deep Sleep Terminated"));
        break;
      }
      // some data to send
      if (!MyFS.exists(DATA_FILENAME)) {
        Serial.println(F("No data to send"));
        break;
      }


      delay(1000);
      if ( false && sendDataCsvTo(SEND_TO) ) {
        Serial.println(F("Mail sended"));
        Serial.println(F("Erase data.csv"));
        MyFS.remove(DATA_FILENAME);
        mailSended = true;
      } else {

        File f = MyFS.open(DATA_FILENAME, "a");
        if (!f) {
          Serial.println("!!file open failed!!");
        } else {
          f.print(now());
          f.print("\t");
          f.print("tried an email");
          f.print("\n");
          f.close();

        }

      }
      Serial.println(F("-- start DeepSleep for 1 Minute with a 15 Second incremental"));
      Serial.println(F("<-- GO"));
      //MyDeepSleepManager.GMTBootTime = now();
      D_println(millis());
      MyDeepSleepManager.startDeepSleep( 60 * 60, 60, 10 );
    } while (false);

  }

  if (Serial.available()) {
    char aChar = (char)Serial.read();
    if (aChar == 'S') {
      Serial.println(F("-- start DeepSleep for 24 Hours"));
      Serial.println(F("   each press on RESET will skip 1 hours"));
      Serial.println(F("<-- GO"));
      MyDeepSleepManager.startDeepSleep(24 * 60 * 60, 60 * 60 ); // start a deepSleepMode with 1 hours incremental
    }

    if (aChar == 'T') {
      Serial.println(F("-- start DeepSleep for 1 Minute with a 15 Second incremental"));
      Serial.println(F("<-- GO"));
      //MyDeepSleepManager.GMTBootTime = now();
      MyDeepSleepManager.startDeepSleep( 1 * 60, 30, 5 );
    }

    if (aChar == 'U') {
      Serial.println(F("-- start DeepSleep for 5 Minutes with a 30 Seconds incremental"));
      Serial.println(F("<-- GO"));
      //MyDeepSleepManager.GMTBootTime = now();
      MyDeepSleepManager.startDeepSleep( 5 * 60 , 30 );
    }

    if (aChar == 'V') {
      Serial.println(F("-- start DeepSleep for 1 Hour with a 1 Minute incremental"));
      Serial.println(F("<-- GO"));
      //MyDeepSleepManager.GMTBootTime = now();
      MyDeepSleepManager.startDeepSleep( 60 * 60, 60, 10 );
    }


    if (aChar == 'E') {
      Serial.println(F("-- Erase data.csv"));
      MyFS.remove(DATA_FILENAME);
    }

    if (aChar == 'P') {
      Serial.println(F("==== data.csv ====="));
      printDataCSV();
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
    if (aChar == 'N') {
      Serial.print(F(" now() = "));
      Serial.println(Ctime(now()));
      //      anow = time(nullptr);
      //      Serial.print(F(" time() = "));
      //      Serial.println(Ctime(&anow));

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


String Ctime(time_t time) {
  return String(ctime(&time)).substring(0, 24);
};



// check connectivity to internet via a captive portal and try to update the clock
bool connectedToInternet() {
  // connect to a captive portal
  //      captive.apple.com  timestamp around 4 second false
  //      connectivitycheck.gstatic.com
  //      detectportal.firefox.com
  //      www.msftncsi.com  timestamp about 1 second
  //  https://success.tanaza.com/s/article/How-Automatic-Detection-of-Captive-Portal-works
#define CAPTIVE "www.msftncsi.com"

  Serial.println(F("connect to " CAPTIVE " to get time and check connectivity to www"));

  HTTPClient http;  //Declare an object of class HTTPClient

  http.begin("http://" CAPTIVE);  //Specify request destination
  // we need date to setup clock
  const char * headerKeys[] = {"date"} ;
  const size_t numberOfHeaders = 1;
  http.collectHeaders(headerKeys, numberOfHeaders);

  int httpCode = http.GET();                                  //Send the request
  if (httpCode < 0) {
    Serial.print(F("cant get an answer http.GET()="));
    Serial.println(httpCode);
    http.end();   //Close connection
    return (false);
  }

  // we got an answer
  String headerDate = http.header(headerKeys[0]);
  // try to setup Clock
  Serial.println(headerDate);
  if (headerDate.endsWith(" GMT") && headerDate.length() == 29) {
    tmElements_t dateStruct;
    // we got a date
    Serial.println(F("Analyse date serveur"));
    dateStruct.Second  = headerDate.substring(23, 25).toInt();
    dateStruct.Minute  = headerDate.substring(20, 22).toInt();
    dateStruct.Hour = headerDate.substring(17, 19).toInt();
    dateStruct.Year = headerDate.substring(12, 16).toInt() - 1970;
    const String monthName = F("JanFebMarAprJunJulAugSepOctNovDec");
    dateStruct.Month = monthName.indexOf(headerDate.substring(8, 11)) / 3 + 1;
    dateStruct.Day = headerDate.substring(5, 7).toInt();

    //          Serial.print(dateStruct.tm_hour); Serial.print(":");
    //          Serial.print(dateStruct.tm_min); Serial.print(":");
    //          Serial.print(dateStruct.tm_sec); Serial.print(" ");
    //          Serial.print(dateStruct.tm_mday); Serial.print("/");
    //          Serial.print(dateStruct.tm_mon); Serial.print("/");
    //          Serial.print(dateStruct.tm_year); Serial.println(" ");
    time_t serverTS = makeTime(dateStruct) + 3600;

    Serial.print("Server time = ");
    Serial.println(Ctime(serverTS));
    Serial.print("Local  time = ");
    time_t nowTS = now();
    Serial.println(Ctime(nowTS));

    Serial.print("timeStatus()=");
    Serial.println(timeStatus());
    setTime(serverTS);
    Serial.print("timeStatus()=");
    Serial.println(timeStatus());
  }

  //Date: Mon, 25 Jan 2021 21:18:52 GMT
  String payload = http.getString();   //Get the request response payload
  //Serial.println(payload);             //Print the response payload
  http.end();   //Close connection
  return true;
}

// Just standard SMTP mail
//https://fr.wikipedia.org/wiki/Simple_Mail_Transfer_Protocol
//telnet smtp.xxxx.xxxx 25
//Connected to smtp.xxxx.xxxx.
//220 smtp.xxxx.xxxx SMTP Ready
//HELO client
//250-smtp.xxxx.xxxx
//250-PIPELINING
//250 8BITMIME
//MAIL FROM: <auteur@yyyy.yyyy>
//250 Sender ok
//RCPT TO: <destinataire@xxxx.xxxx>
//250 Recipient ok.
//DATA
//354 Enter mail, end with "." on a line by itself
//Subject: Test
//
//Corps du texte
//.
//250 Ok
//QUIT
//221 Closing connection
//Connection closed by foreign host.

bool sendDataCsvTo(const String sendto)  {
  Serial.print("Send data.csv to ");
  Serial.println(sendto);
  WiFiClient tcp;  //Declare an object of class Client to make a TCP connection
  String aLine;    // to get answer of SMTP
  if ( !tcp.connect(SMTP_SERVER, 25) ) {
    Serial.println("unable to connect with " SMTP_SERVER ":25" );
    return false;
  }
  bool mailOk = false;
  do {
    Serial.println("connected with " SMTP_SERVER ":25" );
    aLine = tcp.readStringUntil('\n');
    D_println(aLine);
    if (!aLine.startsWith("220 "))  break;  //  not goog answer

    Serial.println( "HELO arduino" );
    tcp.print("HELO arduino \r\n");
    aLine = tcp.readStringUntil('\n');
    D_println(aLine);
    if (!aLine.startsWith("250 "))  break;  //  not goog answer

    Serial.println( "MAIL FROM: " SEND_FROM );
    tcp.print("MAIL FROM: " SEND_FROM "\r\n");
    aLine = tcp.readStringUntil('\n');
    D_println(aLine);
    if (!aLine.startsWith("250 "))  break;  //  not goog answer

    Serial.println( "RCPT TO: " + sendto );
    tcp.print("RCPT TO: " + sendto + "\r\n");
    aLine = tcp.readStringUntil('\n');
    D_println(aLine);
    if (!aLine.startsWith("250 "))  break;  //  not goog answer

    Serial.println( "DATA" );
    tcp.print("DATA\r\n");
    aLine = tcp.readStringUntil('\n');
    D_println(aLine);
    if (!aLine.startsWith("354 "))  break;  //  not goog answer

    Serial.println( "Mail itself" );
    tcp.print("Subject: test mail from arduino\r\n");
    //    tcp.print("ceci est un mail de test\r\n");
    //    tcp.print("destine a valider la connection\r\n");
    //    tcp.print("au serveur SMTP\r\n");
    //    tcp.print("\r\n");
    tcp.println(F("===== data.csv =="));
    File f = MyFS.open(DATA_FILENAME, "r");
    if (f) {
      while (f.available()) {
        time_t aTime = f.readStringUntil('\t').toInt();
        String aLine = f.readStringUntil('\n');
        tcp.print(Ctime(aTime));
        tcp.print('\t');
        tcp.println(aLine);
      }
      f.close();
    }

    tcp.println(F("==Eof data.csv =="));


    tcp.print("\r\n.\r\n");
    aLine = tcp.readStringUntil('\n');
    D_println(aLine);

    if (!aLine.startsWith("250 "))  break;  //  not goog answer

    mailOk = true;
    break;
  } while (false);
  D_println(mailOk);

  Serial.println( "quit" );
  tcp.print("QUIT\r\n");
  aLine = tcp.readStringUntil('\n');
  D_println(aLine);

  Serial.println( "Stop TCP connection" );
  tcp.stop();
  return mailOk;
}


void printDataCSV() {
  File f = MyFS.open(DATA_FILENAME, "r");
  if (f) {
    while (f.available()) {
      time_t aTime = f.readStringUntil('\t').toInt();
      String aLine = f.readStringUntil('\n');
      Serial.print(Ctime(aTime));
      Serial.print('\t');
      Serial.println(aLine);
    }
    f.close();
  }
}
