/*************************************************
 *************************************************
    DeepSleepManager  Allow BP0 to be user push button and a awake form deep sleep buton while sleeping
    Copyright 2020  NET234

    B01  11/01/2021
    from testpowerdown (.h only)



**********************************************************************************/
#include <Arduino.h>


// restart reason keywords
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
#define REASON_USER_BUTTON  10
//( (rst_reason)10)
#define REASON_RESTORE_WIFI 11
#define REASON_NOT_INITED   12



class DeepSleepManager {
  public:
    uint8_t  getRstReason(const int16_t buttonPin = -1 );          // return the reason of the deepsleep awake (adjusted reason)
    void     startDeepSleep(const uint16_t sleepTimeSeconds = 60 * 60 ); // start a deepSleepMode with   default 1 Hour  (max 3 Hours)
    void     restoreWiFi();                                        // arm a reset to restore WiFi back
    bool     WiFiActive;          // true if wifi is active
    uint16_t bootCounter;         // Number of reboot since power on
    uint32_t estimatedSleepTime;  // Number of second since deepsleep statrted


  private:
    uint8_t  rstReason = REASON_NOT_INITED;    // reason of restart adjusted from ESP.getResetInfoPtr();
    struct  {
      float     checkPI;              // initialised to PI value to check POWER_ON Boot
      uint16_t  bootCounter;          // Number of reboot since power on
      int16_t   lastSleepTime;         // last sleep requested duration in second // -1 if it is a wifi restore (max = 3H)
      uint32_t  estimatedSleepTime;   // Number of second since deepsleep started (estimation)
    } savedRTCmemory;


};
