#include "DeepSleepManager.h"
#include <ESP8266WiFi.h>

//My WiFi won't reconnect after deep sleep using ``WAKE_RF_DISABLED``
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//When you implement deep sleep using ``WAKE_RF_DISABLED``, this forces what
//appears to be a bare metal disabling of WiFi functionality, which is not
//restored using ``WiFi.forceSleepWake()`` or ``WiFi.mode(WIFI_STA)``. If you need
//to implement deep sleep with ``WAKE_RF_DISABLED`` and later connect to WiFi, you
//will need to implement an additional (short) deep sleep using
//``WAKE_RF_DEFAULT``.
//https://github.com/esp8266/Arduino/pull/7338/commits/ae0d8ffe84944284665facf13f847887e6459cfa

uint8_t DeepSleepManager::getRstReason(const int16_t buttonPin) {
  // rstReason is inited once
  if (rstReason != REASON_NOT_INITED) {
    return (rstReason);
  }
  // compute restReason
  bool bpStatus = HIGH;
  if ( buttonPin >= 0 ) {
    // pinMode(buttonPin, INPUT);  // not needed it should be in input mode by default
    bpStatus = digitalRead(buttonPin);
  }
  // get ESP reset reason
  rst_info* resetInfoPtr = ESP.getResetInfoPtr();
  rstReason = (resetInfoPtr->reason);
  WiFiLocked = false;
  // adjust ESP rstReason if bp is down or cold boot
  if ( rstReason == REASON_DEEP_SLEEP_AWAKE) {
    if (bpStatus == LOW ) rstReason = REASON_USER_BUTTON;
    WiFiLocked = true;
  }

  // adjust ESP rstReason if RTC memory not initialised
  //system_rtc_mem_read(64, &savedRTCmemory, sizeof(savedRTCmemory));
  ESP.rtcUserMemoryRead(0, (uint32_t*)&savedRTCmemory, sizeof(savedRTCmemory));
  // if RTCmemory not proprely inited it is a cold boot
  if (savedRTCmemory.checkPI != float(PI)) {
    //Serial.println("Power on boot");
    savedRTCmemory.checkPI = PI;
    savedRTCmemory.bootCounter = 0;
    savedRTCmemory.increment = 0;
    savedRTCmemory.remainingTime = 0;
    savedRTCmemory.actualTimestamp = 0;
    savedRTCmemory.powerOnTimestamp = 0;

    rstReason = REASON_DEFAULT_RST;  // most of time never seen cause a second rst cause a REASON_EXT_SYS_RST came ?
  }
  // little trick to leave timeStatus to timeNotSet
  // TODO: see with https://github.com/PaulStoffregen/Time to find a way to say timeNeedsSync
  adjustTime(savedRTCmemory.actualTimestamp);

//  if (rstReason == REASON_DEEP_SLEEP_AWAKE ) {
//    setTime(savedRTCmemory.actualTimestamp);
//  }
  savedRTCmemory.bootCounter++;
  // check for enable Wifi
  if (savedRTCmemory.increment == -1) {
    rstReason = REASON_RESTORE_WIFI;
    savedRTCmemory.increment = 0;
    WiFiLocked = false;
  }
  bootTimestamp = savedRTCmemory.actualTimestamp;
  if (rstReason == REASON_DEEP_SLEEP_AWAKE && savedRTCmemory.remainingTime == 0 )  rstReason = REASON_DEEP_SLEEP_TERMINATED;

  //  remainingTime = savedRTCmemory.remainingTime;
  //  bootCounter = savedRTCmemory.bootCounter;
  if (rstReason == REASON_RESTORE_WIFI) savedRTCmemory.increment = 0;
  ESP.rtcUserMemoryWrite(0, (uint32_t*)&savedRTCmemory, sizeof(savedRTCmemory));
  //system_rtc_mem_write(10, &savedRTCmemory, sizeof(savedRTCmemory));
  return (rstReason);
}

//=============== important note ==============================================
//ESP.deepSleep(microseconds, mode)`` will put the chip into deep sleep.
//``mode`` is one of ``WAKE_RF_DEFAULT``, ``WAKE_RFCAL``, ``WAKE_NO_RFCAL``, ``WAKE_RF_DISABLED``.
//(GPIO16 needs to be tied to RST to wake from deepSleep.)
//The chip can sleep for at most ``ESP.deepSleepMax()`` microseconds.
//If you implement deep sleep with ``WAKE_RF_DISABLED`` and require WiFi functionality on wake up,
//you will need to implement an additional ``WAKE_RF_DEFAULT`` before WiFi functionality is available.
//https://github.com/esp8266/Arduino/pull/7338/commits/ae0d8ffe84944284665facf13f847887e6459cfa


void DeepSleepManager::startDeepSleep(const uint32_t sleepTimeSeconds, const uint16_t increment ) { // start a deepSleepMode with   default increment 2 hours
  savedRTCmemory.remainingTime = sleepTimeSeconds;
  savedRTCmemory.actualTimestamp = now();
  uint16_t nextIncrement = increment;
  if (nextIncrement == 0) nextIncrement = 3 * 60 * 60;
  savedRTCmemory.increment = nextIncrement;
  if (savedRTCmemory.remainingTime <= nextIncrement) {
    nextIncrement = savedRTCmemory.remainingTime;
  }
  savedRTCmemory.remainingTime -= nextIncrement;
  savedRTCmemory.actualTimestamp += nextIncrement;
  ESP.rtcUserMemoryWrite(0, (uint32_t*)&savedRTCmemory, sizeof(savedRTCmemory));
  //Serial.print("sizeof savedRTCmemory=");
  //Serial.println(sizeof savedRTCmemory);  //20
  if (nextIncrement > 0) ESP.deepSleep(nextIncrement * 1.004 * 1E6 - 149300, RF_DISABLED);  //2094
}



void DeepSleepManager::continueDeepSleep() {
  int16_t nextIncrement = savedRTCmemory.increment;
  if (savedRTCmemory.remainingTime <= nextIncrement) {
    nextIncrement = savedRTCmemory.remainingTime;
  }
  savedRTCmemory.remainingTime -= nextIncrement;
  savedRTCmemory.actualTimestamp += nextIncrement + millis() / 1000;
  ESP.rtcUserMemoryWrite(0, (uint32_t*)&savedRTCmemory, sizeof(savedRTCmemory));

  if (nextIncrement > 0) ESP.deepSleep(nextIncrement * 1.004 * 1E6 - micros() - 149300 , RF_DISABLED);  //2094
}


//void DeepSleepManager::startDeepSleep(const uint16_t sleepTimeSeconds) {
//  savedRTCmemory.lastSleepTime = sleepTimeSeconds;
//  if (savedRTCmemory.lastSleepTime < 0) savedRTCmemory.lastSleepTime = 0;
//  if (savedRTCmemory.lastSleepTime > 3 * 3600) savedRTCmemory.lastSleepTime = 3 * 3600;
//  // Serial.println(F("-> PowerDown "));
//  ESP.rtcUserMemoryWrite(0, (uint32_t*)&savedRTCmemory, sizeof(savedRTCmemory));
//  ESP.deepSleep(savedRTCmemory.lastSleepTime * 1.004 * 1E6 - micros() - 149300, RF_DISABLED);  //2094
//  while (true) delay(1);
//}

void DeepSleepManager::WiFiUnlock() {
  savedRTCmemory.increment = -1;
  ESP.rtcUserMemoryWrite(0, (uint32_t*)&savedRTCmemory, sizeof(savedRTCmemory));
  ESP.deepSleep(100L * 1000, RF_DEFAULT);   //reset in 100 ms to clear RF_DISABLED
  while (true) delay(1);
}

uint16_t DeepSleepManager::getBootCounter() {      // Number of reboot since power on
  return savedRTCmemory.bootCounter;
}

uint32_t DeepSleepManager::getRemainingTime() {    // Number of second remaining to terminate deep sleep
  return savedRTCmemory.remainingTime;
}

time_t   DeepSleepManager::getBootTimestamp() {    // Timestamp of the last boot time 
  return bootTimestamp;
}

time_t   DeepSleepManager::getPowerOnTimestamp() { // Timestamp of the power on (set to 0 at power on)
  return savedRTCmemory.powerOnTimestamp;
}

time_t   DeepSleepManager::getActualTimestamp() { // Timestamp saved in RTC
  return savedRTCmemory.actualTimestamp;
}


void     DeepSleepManager::setActualTimestamp(time_t timestamp) {   // init the power on timestamp
  if (timestamp == 0) timestamp = now();
  if (savedRTCmemory.powerOnTimestamp == 0 ) {
    savedRTCmemory.powerOnTimestamp = timestamp - millis() / 1000;
  }
  if (bootTimestamp == 0 ) {
    bootTimestamp = timestamp - millis() / 1000;
  }
  savedRTCmemory.actualTimestamp = timestamp;
  ESP.rtcUserMemoryWrite(0, (uint32_t*)&savedRTCmemory, sizeof(savedRTCmemory));
}
