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
    savedRTCmemory.estimatedSleepTime = 0;
    savedRTCmemory.lastSleepTime = 0;
    rstReason = REASON_DEFAULT_RST;  // most of time never seen cause a second rst cause a REASON_EXT_SYS_RST came ?
  } 
  savedRTCmemory.bootCounter++;
  // check for enable Wifi
  if (savedRTCmemory.lastSleepTime == -1) {
    rstReason = REASON_RESTORE_WIFI;
    savedRTCmemory.lastSleepTime = 0;
    WiFiLocked = false;
  } 
  if (rstReason == REASON_DEEP_SLEEP_AWAKE) savedRTCmemory.estimatedSleepTime += savedRTCmemory.lastSleepTime;

  estimatedSleepTime = savedRTCmemory.estimatedSleepTime;
  bootCounter = savedRTCmemory.bootCounter;
  if (rstReason == REASON_RESTORE_WIFI) savedRTCmemory.estimatedSleepTime = 0;
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


void DeepSleepManager::startDeepSleep(const uint16_t sleepTimeSeconds) {
  savedRTCmemory.lastSleepTime = sleepTimeSeconds;
  if (savedRTCmemory.lastSleepTime < 0) savedRTCmemory.lastSleepTime = 0;
  if (savedRTCmemory.lastSleepTime > 3 * 3600) savedRTCmemory.lastSleepTime = 3 * 3600;
  // Serial.println(F("-> PowerDown "));
  ESP.rtcUserMemoryWrite(0, (uint32_t*)&savedRTCmemory, sizeof(savedRTCmemory));
  ESP.deepSleep(savedRTCmemory.lastSleepTime * 1.004 * 1E6 - micros() - 149300, RF_DISABLED);  //2094
  while (true) delay(1);
}

void DeepSleepManager::WiFiUnlock() {
  savedRTCmemory.lastSleepTime = -1;
  ESP.rtcUserMemoryWrite(0, (uint32_t*)&savedRTCmemory, sizeof(savedRTCmemory));
  ESP.deepSleep(100L * 1000, RF_DEFAULT);   //reset in 100 ms to clear RF_DISABLED
  while (true) delay(1);
}
