/*************************
   resetOnOff DeepSleepManger
   net234 04/02/2021 https://github.com/net234/DeepSleepManager

   use reset button as on/off togle switch
   with the standard blink demo :)

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


*/

#include <Arduino.h>


// instance for  the DeepSleepManager
#include <DeepSleepManager.h>
DeepSleepManager MyDeepSleepManager;


// the setup function runs once when you press reset or power the board
void setup() {


  ////////////////////////////////////////////////
  // in your sketch just add these 3 lines on top of setup () to do the same
  //
  //  if ( MyDeepSleepManager.getRstReason() == REASON_EXT_SYS_RST ) {
  //    MyDeepSleepManager.permanentDeepSleep();  // go back to deep sleep
  //  }
  //
  ////////////////////////////////////////////////

  // here we start serial just to show that append
  Serial.begin(115200);
  Serial.println("\r\n\n\n" "resetOnOff DeepSleepManger");
  if ( MyDeepSleepManager.getRstReason() == REASON_EXT_SYS_RST ) {
    Serial.println("This is a button RST reset so we go to sleep undefinitly with all function OFF (including WiFi) so very low power needed");
    Serial.println("ESP is OFF the led is OFF");
    Serial.println("Power supply is up to 10ma with USB   2ma with 3,3v ");

    MyDeepSleepManager.permanentDeepSleep();  // go back to deep sleep
    // after a deep sleep ANY reset (execpt power on startup) will be reported as a REASON_DEEP_SLEEP_AWAKE
    //   so the next time we press RST button we wont do  permanentDeepSleep()
  }
  // we are here because we restart from a deep sleep
  Serial.println("This is a DeepSleep reset (or any other reset than a button RST reset) so we do the loop");
  Serial.println("WiFi is full restored as it was before including credential to WiFi (if it was already programed in another sketch)");
  Serial.println("ESP is ON the led BLINK");
  Serial.println("Power supply is around 80ma with wifi on  (170ma max)");




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
