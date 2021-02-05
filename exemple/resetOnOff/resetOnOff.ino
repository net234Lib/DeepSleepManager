/*************************
   resetOnOff DeepSleepManger
   net234 04/02/2021

   use reset button as on/off togle switch
   with the standard blink demo :)

*/

#include <Arduino.h>


// instance for  the DeepSleepManager
#include <DeepSleepManager.h>
DeepSleepManager MyDeepSleepManager;
// ESP standard restart reason keywords
//enum rst_reason {
// REASON_DEFAULT_RST = 0,              /* normal startup by power on */
// REASON_WDT_RST = 1,                  /* hardware watch dog reset */
// REASON_EXCEPTION_RST = 2,            /* exception reset, GPIO status won't change */
// REASON_SOFT_WDT_RST   = 3,           /* software watch dog reset, GPIO status won't change */
// REASON_SOFT_RESTART = 4,             /* software restart ,system_restart , GPIO status won't change */
// REASON_DEEP_SLEEP_AWAKE = 5,         /* wake up from deep-sleep */
// REASON_EXT_SYS_RST      = 6          /* external system reset */
// Specific DeepSleepManager restart reason
// REASON_USER_BUTTON  10               /* user button detected */
// REASON_DEEP_SLEEP_TERMINATED 11      /* full deep sleep cycle is terminated */

// the setup function runs once when you press reset or power the board
void setup() {

  // here we start serial just to show that append
  ////////////////////////////////////////////////
  // in your sketch just add these 3 lines on top of setup () to do the same
  //
  //  if ( MyDeepSleepManager.getRstReason() == REASON_EXT_SYS_RST ) {
  //    MyDeepSleepManager.permanentDeepSleep();  // go back to deep sleep
  //  }
  //
  ////////////////////////////////////////////////


  Serial.begin(115200);
  Serial.println("\r\n\n\n" "resetOnOff DeepSleepManger");
  if ( MyDeepSleepManager.getRstReason() == REASON_EXT_SYS_RST ) {
    Serial.println("This is a button RST reset so we go to sleep undefinitly with all function OFF (including WiFi) so very low power needed");
    Serial.println("ESP is OFF the led is OFF");
    MyDeepSleepManager.permanentDeepSleep();  // go back to deep sleep
    // after a deep sleep ANY reset (execpt power on startup) will be reported as a REASON_DEEP_SLEEP_AWAKE 
    //   so the next time we press RST button we wont do  permanentDeepSleep()
  }
  // we are here because we restart from a deep sleep
  Serial.println("This is a DeepSleep reset (or any other reset than a button RST reset) so we do the loop");
  Serial.println("WiFi is full restored as it was before including credential to WiFi (if it was already programed in another sketch)");
  Serial.println("ESP is ON the led BLINK");



  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);


}


// the loop function runs over and over again forever
// but a reset here will switch off
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}
