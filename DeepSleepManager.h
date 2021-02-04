/*************************************************
 *************************************************
    DeepSleepManager  Allow BP0 to be user push button and a awake form deep sleep buton while sleeping
    Copyright 2020  NET234 https://github.com/net234/DeepSleepManager

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

  

V1.0  First realease

V1.0.1
  add permanentDeepSleep() 

   TODO: grab millisec lost in a RTC memory varibale for a better adjust of timestamps

**********************************************************************************/
#include <Arduino.h>
#include <user_interface.h>
//#include <ESP8266WiFi.h>

//#include <Time.h>
#include <TimeLib.h>

// ESP standard restart reason keywords
//enum rst_reason {
// REASON_DEFAULT_RST = 0, /* normal startup by power on */
// REASON_WDT_RST = 1, /* hardware watch dog reset */
// REASON_EXCEPTION_RST = 2, /* exception reset, GPIO status won't change */
// REASON_SOFT_WDT_RST   = 3, /* software watch dog reset, GPIO status won't change */
// REASON_SOFT_RESTART = 4, /* software restart ,system_restart , GPIO status won't change */
// REASON_DEEP_SLEEP_AWAKE = 5, /* wake up from deep-sleep */
// REASON_EXT_SYS_RST      = 6 /* external system reset */
//};
// specific reason to

// Specific restart reason
#define REASON_USER_BUTTON  10
//#define REASON_RESTORE_WIFI 11
#define REASON_DEEP_SLEEP_TERMINATED 11
#define REASON_NOT_INITED   19



class DeepSleepManager {
  public:
    uint8_t  getRstReason(const int16_t buttonPin = -1 );          // return the reason of the deepsleep awake (adjusted reason)
    void     startDeepSleep(const uint32_t sleepTimeSeconds, const uint16_t increment = 0, const uint16_t offset = 0 ); // start a deepSleepMode with   default increment 3 hours
    void     permanentDeepSleep();
    void     continueDeepSleep();
    void     WiFiUnlock();                                        // arm a reset to restore WiFi back
    bool     WiFiLocked;            // true if wifi is locked (awake from a deep sleep)
    uint16_t getBootCounter();      // Number of reboot since power on
    uint32_t getRemainingTime();    // Number of second remaining to terminate deep sleep
    time_t   getBootTimestamp();    // Timestamp of the last boot time 
    time_t   getPowerOnTimestamp(); // Timestamp of the power on (set to 0 at power on)
    time_t   getActualTimestamp();  // Timestamp saved in RTC memory (set to 0 at power on)
    void     setActualTimestamp(time_t timestamp = 0);   // Save actual time stamp in case of reset and adjust PowerOn and Boot TimeStamp if needed

  private:
    uint8_t  rstReason = REASON_NOT_INITED;    // reason of restart adjusted from ESP.getResetInfoPtr();
    time_t   bootTimestamp; 
    struct  {
      // all these values are keep in RTC RAM
      float     checkPI;              // initialised to PI value to check POWER_ON Boot
      uint16_t  bootCounter;          // Number of reboot since power on
      int16_t   increment;            // increment requested // -1 if it is a wifi restore (max 9H)
      uint32_t  remainingTime;        // Number of second remaining to terminate deep sleep  (Over 100 year)
      time_t    actualTimestamp;      // time stamp restored on next end of deep sleep Should be update in the loop() with setActualTimestamp
      time_t    powerOnTimestamp;     // Timestamp of the power on (set to 0 at power on)
    } savedRTCmemory;


};
