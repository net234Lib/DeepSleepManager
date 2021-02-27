/*************************************************
 *************************************************
    DeepSleepManager  Allow long deep sleep (over 100 Year) and BP0 to be user push button and a awake form deep sleep buton while sleeping
    Copyright 2020  NET234   https://github.com/net234/DeepSleepManager

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



  V1.0  First release

  V1.0.1
  add permanentDeepSleep()
  V1.0.2
  removed #include <ESP8266WiFi.h> from .ccp
  V1.0.3 06/02/2021
  add restoreRTCData and saveRTCData
  add a crc8 to check RTC data

   TODO: auto adjust millisec lost in a RTC memory varibale for a better adjust of timestamps
   DONE V1.0.03: save a user struct in RTC memory
   DONE V1.0.03: add a checksum to control RTC ram data
   TOTO ameliorer getTxtRstReason(BP_0);


**********************************************************************************/
// trick to trace variables
//#define D_println(x) Serial.print(F(#x " => '")); Serial.print(x); Serial.println("'");


#include <Arduino.h>
#include <user_interface.h>
//#include <ESP8266WiFi.h>

#include <TimeLib.h>

// ESP standard restart reason keywords
//enum rst_reason {
// REASON_DEFAULT_RST = 0,              /* normal startup by power on */
// REASON_WDT_RST = 1,                  /* hardware watch dog reset */
// REASON_EXCEPTION_RST = 2,            /* exception reset, GPIO status won't change */
// REASON_SOFT_WDT_RST   = 3,           /* software watch dog reset, GPIO status won't change */
// REASON_SOFT_RESTART = 4,             /* software restart ,system_restart , GPIO status won't change */
// REASON_DEEP_SLEEP_AWAKE = 5,         /* wake up from standard deep-sleep */
// REASON_EXT_SYS_RST      = 6          /* external system reset */
// Specific DeepSleepManager restart reason
#define REASON_USER_BUTTON  10  /* user button detected */
#define REASON_DEEP_SLEEP_TERMINATED 11 /* long deep sleep cycle is terminated */
#define REASON_NOT_INITED   19          /* used internaly */

//time_t before 2000 will be considered as elapsed time they will be displayed as :' numberOfDay HH:MM:SS'
#define NOT_A_DATE_YEAR   2000


class DeepSleepManager {



  public:
    uint8_t  getRstReason(const int16_t buttonPin = -1 );          // return the reason of the deepsleep awake (adjusted reason)
    void     startDeepSleep(const uint32_t sleepTimeSeconds, const uint16_t increment = 0, const uint16_t offset = 0 ); // start a deepSleepMode for a long time with   default increment 6 hours
    void     deepSleepUntil(const uint8_t hour, const uint8_t minute, const uint8_t second,const uint16_t increment = 0, const uint16_t offset = 0);
    void     permanentDeepSleep();   // Activate DeepSleep Forever
    void     continueDeepSleep();    // Return in deepSleep to terminate an interupted deepsleep
    void     WiFiUnlock();           // arm a reset to restore WiFi back
    bool     WiFiLocked;            // true if wifi is locked (awake from a deep sleep)
    uint16_t getBootCounter();      // Number of reboot since power on
    uint32_t getRemainingTime();    // Number of second remaining to terminate long deep sleep
    time_t   getBootTimestamp();    // Timestamp of the last boot time
    time_t   getPowerOnTimestamp(); // Timestamp of the power on (set to 0 at power on)
    time_t   getActualTimestamp();  // Timestamp saved in RTC memory (set to 0 at power on)
    void     setActualTimestamp(time_t timestamp = 0);   // Save actual time stamp in case of reset and adjust PowerOn and Boot TimeStamp if needed
    // helper to save and restore RTC_DATA
    // this is ugly but we need this to get correct sizeof()
#define  RTC_DATA(x) (uint32_t*)&x,sizeof(x)
    bool     restoreRTCData( uint32_t* data, const uint16_t size);
    bool     saveRTCData( uint32_t* data, const uint16_t size);
    String   getTxtRstReason();
  private:
    bool setCrc8(const void* data, const uint16_t size, uint8_t &refCrc );
    bool saveRTCmemory();
    //const    uint8_t structSizeInt32 = (sizeof(savedRTCmemory) + 3) / 4;
    uint8_t  rstReason = REASON_NOT_INITED;    // reason of restart adjusted from ESP.getResetInfoPtr();
    time_t   bootTimestamp;
    
    //struct __attribute__((packed))
    struct  {
      // all these values are keep in RTC RAM
      uint8_t   crc8;                 // CRC for savedRTCmemory
      uint8_t   userDataCrc8;         // CRC for UserData
      uint16_t  userDataSize;         // size of UserData (max 450)
      uint16_t  bootCounter;          // Number of reboot since power on
      int16_t   increment;            // increment requested // -1 if it is a wifi restore (max 3H)
      uint32_t  remainingTime;        // Number of second remaining to terminate deep sleep  (over 100 year)
      time_t    actualTimestamp;      // time stamp restored on next end of deep sleep Should be update in the loop() with setActualTimestamp
      time_t    powerOnTimestamp;     // Timestamp of the power on (set to 0 at power on)
      int32_t   correction;           // microsec/sec correction
      uint32_t  startTimestamp;       // timeStamp of the last deepSleep
      uint32_t  sleepTime;            // duration of the last deepSleep
      int32_t  uncorrectedTime;      // cumulative time of deepSleep;
      } savedRTCmemory;


};
