/*************************
   mesureAndSendMailLater DeepSleepManger Exemple
   net234 25/01/2021

   Wake up on regular interval to read a standard DHT11 temperature and humidity
   Store data on a local file
   Then send a mail with the result
   B02
     add AUTH for mail
     

   // This exemple need :
  // A connection between D0 (GPIO16) and RST with a 1K resistor or a diode see documentation
  // A DHT11 (the temp/humidity sensor of your Arduino Kit) connected to D6 (GPIO12)
  // Eventualy a user button to abort a very long deep sleep (here BP0) connected to D7 (GPIO13)

 
  mesures : nodeMcu dev board CP2120 ESP8266-E12 3v3
  Wifi connected modem  =  24ma / 28ma / 77ma / 190ma
  Deep sleep            = 9,3ma 

  mesures : nodeMcu dev board CP2120 ESP8266-E12 3v3 without AMS117
  Wifi connected modem  =  25ma / 29ma / 80ma / 190ma
  Deep sleep            = 7,8ma 
  
  mesures : nodeMcu dev board CP2120 ESP8266-E12 5V usb or 5V Vin 
  Wifi connected modem  =  25ma / 29ma / 80ma / 190ma
  Deep sleep            = 10,2ma 

  mesures : nodeMcu dev board CH340 ESP8266-E12 3v3
  Wifi connected modem  =  14ma / 18ma / 70ma / 180ma
  Deep sleep            = 1,8ma 

  mesures : nodeMcu dev board CH340 ESP8266-E12 3v3 without AMS117
  Wifi connected modem  =  14ma / 18ma / 70ma / 180ma
  Deep sleep            = 0,15ma 

  mesures : nodeMcu dev board V3 CP2120 ESP8266-E12 5V (4,2V)
  Wifi AP              =  70ma 
  Deep sleep            = 1,54ma 

  
  mesures : nodeMcu dev board V3 CP2120 ESP8266-E12F 3v3 (3,6V)
  Wifi AP               =  70ma 
  Deep sleep            = 1,7ma 

  mesures : nodeMcu dev board V3 CP2120 ESP8266-E12F 3v3 without AMS117
  Wifi AP               =  70 ma 
  Deep sleep            = 0,03ma 
 
  mesures : ESP8266-E12S 3v3 Alone
  Wifi AP               =  70 ma
  Deep sleep            = 0,03ma 


  
  TODO: log if mail sended

*/

// trick to trace variables
#define D_println(x) Serial.print(F(#x " => '")); Serial.print(x); Serial.println("'");
//define D_println(x) while (false) {};



//Le croquis utilise 321212 octets (30%) de l'espace de stockage de programmes. Le maximum est de 1044464 octets.
//Les variables globales utilisent 28896 octets (35%) de mémoire dynamique, ce qui laisse 53024 octets pour les variables locales. Le maximum est de 81920 octets.
//

#include <Arduino.h>
#include <base64.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include "private.h"

#define APP_VERSION   "mesureAndSendMailLater B02  node00"
#define SEND_TO       PRIVATE_SEND_TO      // replace with your test adresse
#define HTTP_SERVER   "www.free.fr"        // for clock syncr replace with your FAI http server
#define SMTP_SERVER   PRIVATE_SMTP_SERVER  // replace with your FAI smtp server 
#define SMTP_LOGIN    PRIVATE_SMTP_LOGIN   // replace with your smtp login (base64) 
#define SMTP_PASS     PRIVATE_SMTP_PASS    // replace with your smtp pass  (base64) 
#define SEND_FROM     PRIVATE_SEND_FROM    // replace with your test adresse
#define DATA_FILENAME "/data.csv"

// to check batterie
ADC_MODE(ADC_VCC);

// GPIO2 on ESP32
//LED_1 D4(GPIO2)   LED_BUILTIN HERE
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
bool bp0Status;  // BP0 status
float  Vcc;      // last mesured Vcc
#define getVcc() ( float(ESP.getVcc())/1024 )  // to get Vcc in Volt

void setup() {
  // Setup BP0
  pinMode( BP0, INPUT_PULLUP);

  Vcc = getVcc();
  uint8_t rstReason = MyDeepSleepManager.getRstReason(BP0);
  Serial.begin(115200);
  Serial.println(F("\n" APP_VERSION));
  D_println(Vcc);

  if ( rstReason == REASON_DEEP_SLEEP_AWAKE  ) {
    // here we start serial to show that we are awake
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
      f.print(MyDHT11.getStatusString());
      f.print("\t");
      f.print(dht11Values.humidity, 1);
      f.print("\t");
      f.print(dht11Values.temperature, 2);
      f.print("\t");
      f.print(Vcc, 2);
      f.print("\n");
      D_println(f.size());
      f.close();

    }
    MyFS.end();
    // go deep sleep reset in next increment
    MyDeepSleepManager.continueDeepSleep();  // go back to deep sleep
    //

  }
  // we are here because longDeepSleep is fully elapsed or user pressed BP0
  // init file system (needed to log)
  MyFS.begin();


  digitalWrite(LED1, LED1_ON);
  logDataCSV("Boot: " +  getTxtRestartReason());
  logDataCSV("Vcc: " +  String(Vcc));
  //  Serial.begin(115200);
  //  Serial.println(F(APP_VERSION));
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
    // "-->Restore WiFi"
    logDataCSV("WiFiUnlock.");
    MyFS.end();
    MyDeepSleepManager.WiFiUnlock();
    // !! restore WiFi will make a special reset so we never arrive here !!
  }

  // Wifi is restored as it was and will soon connect to wifi
  D_println(WiFi.getMode());

  // This exemple supose a WiFi connection so if we are not WIFI_STA mode we force it
  if (WiFi.getMode() != WIFI_STA) {
    Serial.println(F("!!! Force WiFi to STA mode !!!  should be done only ONCE even if we power off "));
    logDataCSV("Restore WiFI credential");
    WiFi.mode(WIFI_STA);
    //WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }

  bp0Status = digitalRead(BP0);
  D_println(bp0Status);

  // init file system
  MyFS.begin();


  Serial.println(F("==== data.csv ====="));
  printDataCSV();
  Serial.println(F("==== eof datat.csv ="));
  Serial.println(F(APP_VERSION));

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
time_t connectedTimeStamp  = 0;

void loop() {
  // Save the time when it change so we can reboot with localtime almost acurate
  if ( MyDeepSleepManager.getActualTimestamp() != now() ) {
    //        Serial.print(F("Save clock "));
    //        Serial.println( Ctime(now()) );
    MyDeepSleepManager.setActualTimestamp();

    // this will append every seconds

    // If we are not connected we warn the user every 30 seconds that we need to update credential
    if ( !connectedTimeStamp ) {
      // every 30 sec
      if ( (now() - MyDeepSleepManager.getBootTimestamp()) % 30 == 0 ) {
        Serial.println(F("device not connected to local WiFi"));
        D_println(WiFi.SSID());
        Serial.println(F("type 'W' to adjust WiFi credential"));
      }
    }
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

      connectedTimeStamp = now();

      Vcc = getVcc();
      logDataCSV( "WiFI Connected Vcc=" + String(Vcc) );
      if (!connectedToInternet()) {
        logDataCSV(F("No Internet connection"));
        break;
      }
      // dsisplay RTC status
      Vcc = getVcc();
      logDataCSV( "WWW Connected Vcc=" + String(Vcc)  );
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



      if ( sendDataCsvTo(SEND_TO) ) {

        Serial.println(F("Erase data.csv"));
        MyFS.remove(DATA_FILENAME);
        mailSended = true;
        logDataCSV(F("Mail sended"));
      } else {
        logDataCSV(F("tried an email"));
      }
      //logDataCSV(F("-- start DeepSleep again for 1 Min with a 10 sec incremental offset 2"));
      logDataCSV(F("-- start DeepSleep again for 3 Hour with a 10 Minutes incremental"));
      logDataCSV("Millsec was " + String(millis()));
      Serial.println(F("<-- GO"));
      //MyDeepSleepManager.startDeepSleep( 60, 10, 2 );
      MyDeepSleepManager.startDeepSleep( 3 * 60 * 60, 10 * 60, 10 );
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
      //Serial.println(F("-- start DeepSleep for 1 Minute with a 10 Second incremental"));
      logDataCSV(F("-- start a new DeepSleep  for 1 Min with a 10 sec incremental offset 2"));
      Serial.println(F("<-- GO"));
      MyDeepSleepManager.startDeepSleep( 1 * 60, 10, 2 );
    }

    if (aChar == 'U') {
      logDataCSV(F("-- start DeepSleep for 5 Minutes with a 30 Seconds incremental"));
      Serial.println(F("<-- GO"));
      MyDeepSleepManager.startDeepSleep( 5 * 60 , 30 );
    }

    if (aChar == 'V') {
      logDataCSV(F("-- start DeepSleep for 1 Hour with a 1 Minute incremental"));
      Serial.println(F("<-- GO"));
      MyDeepSleepManager.startDeepSleep( 60 * 60, 60, 10 );
    }
    if (aChar == 'M') {
      logDataCSV(F("-- manual send a mail to " SEND_TO));
      sendDataCsvTo(SEND_TO);
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
      logDataCSV(F("-- Hard reset with D0 -> LOW"));
      delay(2);
      pinMode(D0, OUTPUT);
      digitalWrite(D0, LOW);  //
      delay(1000);
      Serial.println(F("-- Soft Rest"));
      ESP.reset();
    }
    if (aChar == 'R') {
      logDataCSV(F("-- Soft reset"));
      ESP.reset();
    }
    if (aChar == 'N') {
      Serial.print(F(" now() = "));
      Serial.println(Ctime(now()));
      //      anow = time(nullptr);
      //      Serial.print(F(" time() = "));
      //      Serial.println(Ctime(&anow));

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
          bool result = WiFi.begin(ssid, pass);
          D_println(result);
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
    logDataCSV(F("BP0: DeepSleep 1 min inc 10 sec"));
    MyDeepSleepManager.startDeepSleep( 1 * 60, 10 );
  }
  delay(10);  // avoid rebounce of BP0 easy way :)
}




// check connectivity to internet via a captive portal and try to update the clock
bool connectedToInternet() {
  // connect to a captive portal
  //      captive.apple.com  timestamp around 4 second false
  //      connectivitycheck.gstatic.com
  //      detectportal.firefox.com
  //      www.msftncsi.com  timestamp about 1 second
  //  https://success.tanaza.com/s/article/How-Automatic-Detection-of-Captive-Portal-works
  // in fact any know hhtp web server redirect on https  so use your FAI web url
#define CAPTIVE HTTP_SERVER

  Serial.println(F("connect to " CAPTIVE " to get time and check connectivity to www"));

  WiFiClient client;
  HTTPClient http;  //Declare an object of class HTTPClient

  http.begin(client, "http://" CAPTIVE); //Specify request destination
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
  D_println(headerDate);
  if (headerDate.endsWith(" GMT") && headerDate.length() == 29) {
    tmElements_t dateStruct;
    // we got a date
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
    int delta = serverTS - nowTS;
    logDataCSV("delta time : " + String(delta));
    if (abs(delta) > 1) {
      D_println(timeStatus());
      logDataCSV("Settime(" + Ctime(serverTS) + ")");
      setTime(serverTS);
      MyDeepSleepManager.setActualTimestamp();
      D_println(timeStatus());
    }
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
  WiFiClient tcp;  //Declare an object of Wificlass Client to make a TCP connection
  String aLine;    // to get answer of SMTP
  // Try to find a valid smtp
  bool valideSmtp = false;
  do {

    Serial.println("connected with " SMTP_SERVER ":25" );
    aLine = tcp.readStringUntil('\n');
    if (!aLine.startsWith("220 "))  break;  //  not good answer
    Serial.println( "HELO arduino" );

    tcp.print("HELO arduino \r\n");
    aLine = tcp.readStringUntil('\n');
    if (!aLine.startsWith("250 "))  break;  //  not goog answer

    Serial.println( "MAIL FROM: " SEND_FROM );
    tcp.print("MAIL FROM: " SEND_FROM "\r\n");
    aLine = tcp.readStringUntil('\n');
    if (!aLine.startsWith("250 "))  break;  //  not goog answer

    Serial.println( "RCPT TO: " + sendto );
    tcp.print("RCPT TO: " + sendto + "\r\n");
    aLine = tcp.readStringUntil('\n');
    if (!aLine.startsWith("250 "))  break;  //  not goog answer

    Serial.println( "DATA" );
    tcp.print("DATA\r\n");
    aLine = tcp.readStringUntil('\n');
    if (!aLine.startsWith("354 "))  break;  //  not goog answer

    Serial.println( "Mail itself" );
    tcp.print("Subject: test mail from " APP_VERSION "\r\n");
    tcp.print("\r\n");  // end of header
    // body
    //    tcp.print("ceci est un mail de test\r\n");
    //    tcp.print("destine a valider la connection\r\n");
    //    tcp.print("au serveur SMTP\r\n");
    //    tcp.print("\r\n");
    tcp.print(F("===== data.csv ==\r\n"));
    File f = MyFS.open(DATA_FILENAME, "r");
    if (f) {
      while (f.available()) {
        String aTime = f.readStringUntil('\t');
        String aLine = f.readStringUntil('\n');
        tcp.print(Ctime(aTime.toInt()));
        tcp.print('\t');
        tcp.print(aTime);
        tcp.print('\t');
        tcp.print(aLine);
        tcp.print("\r\n");

      }
    }

    Serial.println(F("unable to connect with any smtp server"));
    break;
  } while (false);

  bool mailOk = false;
  if (valideSmtp) do {
      Serial.println(F("HELO arduino"));
      tcp.print(F("HELO arduino \r\n")); // EHLO send too much line
      aLine = tcp.readStringUntil('\n');
      if ( aLine.toInt() != 250 )  break;  //  not good answer
      // autentification
      if (SMTP_LOGIN != "") {
        Serial.println(F("AUTH LOGIN"));
        tcp.print(F("AUTH LOGIN \r\n"));
        aLine = tcp.readStringUntil('\n');
        if (aLine.toInt() != 334 )  break;  //  not good answer
        
        //Serial.println(F("SEND LOGIN"));
        tcp.print(F(SMTP_LOGIN "\r\n"));
        aLine = tcp.readStringUntil('\n');
        if (aLine.toInt() != 334 )  break;  //  not good answer
        
        //Serial.println(F("SEND PASS"));
        tcp.print(F(SMTP_PASS "\r\n"));
        aLine = tcp.readStringUntil('\n');
        if (aLine.toInt() != 235 )  break;  //  not good answer
      }


      Serial.println(F("MAIL FROM: " SEND_FROM));
      tcp.print(F("MAIL FROM: " SEND_FROM "\r\n"));
      aLine = tcp.readStringUntil('\n');
      if ( aLine.toInt() != 250 )  break;  //  not good answer

      Serial.println("RCPT TO: " + sendto);
      tcp.print("RCPT TO: " + sendto + "\r\n");
      aLine = tcp.readStringUntil('\n');
      if ( aLine.toInt() != 250 )  break;  //  not good answer

      Serial.println(F("DATA"));
      tcp.print(F("DATA\r\n"));
      aLine = tcp.readStringUntil('\n');
      if ( aLine.toInt() != 354 )  break;  //  not goog answer

      //Serial.println( "Mail itself" );
      tcp.print("Subject: test mail from " APP_VERSION "\r\n");
      tcp.print("\r\n");  // end of header
      // body
      //    tcp.print("ceci est un mail de test\r\n");
      //    tcp.print("destine a valider la connection\r\n");
      //    tcp.print("au serveur SMTP\r\n");
      //    tcp.print("\r\n");
      tcp.print(F(" == == = data.csv == \r\n"));
      File f = MyFS.open(DATA_FILENAME, "r");
      if (f) {
        while (f.available()) {
          String aTime = f.readStringUntil('\t');
          String aLine = f.readStringUntil('\n');
          tcp.print(Ctime(aTime.toInt()));
          tcp.print('\t');
          tcp.print(aTime);
          tcp.print('\t');
          tcp.print(aLine);
          tcp.print("\r\n");
        }
        f.close();
      }

      tcp.print(F(" == Eof data.csv == \r\n"));

      // end of body
      tcp.print("\r\n.\r\n");
      aLine = tcp.readStringUntil('\n');
      if ( aLine.toInt() != 250 )  break;  //  not goog answer

      mailOk = true;
      break;
    } while (false);
  D_println(mailOk);
  D_println(aLine);
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

void logDataCSV(const String atext) {
  File f = MyFS.open(DATA_FILENAME, "a");
  time_t aNow = now();
  if (!f) {
    Serial.println("!!file open failed!!");
  } else {
    f.print(aNow);
    f.print("\t");
    f.print(atext);
    f.print("\n");
    f.close();
  }
  Serial.print(F("Log: "));
  Serial.print(atext);
  Serial.print(F(" at "));
  Serial.println(Ctime(aNow));

}






String str2digits(const uint8_t digits) {
  String txt;
  if (digits < 10)  txt = '0';
  txt += digits;
  return txt;
}

//String Ctime(time_t time) {
//  return String(ctime(&time)).substring(0, 24);
//};


String Ctime(time_t time) {
  static String date;
  String txt = "";
  txt += str2digits(day(time));
  txt += '/';
  txt += str2digits(month(time));
  txt += '/';
  txt += year(time);
  if (txt == date) {
    txt = "";
  } else {
    date = txt;
    txt += " ";
  }
  txt += str2digits(hour(time));
  txt += ':';
  txt += str2digits(minute(time));
  txt += ':';
  txt += str2digits(second(time));
  return txt;
}


String getTxtRestartReason() {
  switch (MyDeepSleepManager.getRstReason()) {
    case REASON_DEFAULT_RST:  return (F("->Cold boot"));
    case REASON_EXT_SYS_RST:  return (F("->boot with BP Reset")); break;
    case REASON_DEEP_SLEEP_AWAKE:  return (F("->boot from a deep sleep pending")); break;
    case REASON_DEEP_SLEEP_TERMINATED: return (F("->boot from a deep sleep terminated")); break;
    case REASON_USER_BUTTON: return (F("->boot from a deep sleep aborted with BP User")); break;
    //   case REASON_RESTORE_WIFI: Serial.println(F("->boot from a restore WiFI command")); break;
    case REASON_SOFT_RESTART: return (F("->boot after a soft Reset")); break;
    default:
      return (String(F("->boot reason = ")) + MyDeepSleepManager.getRstReason());
  }
}
