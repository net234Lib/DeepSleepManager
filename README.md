# DeepSleepManager Library for arduino
Small lib for an easy use of deepSleep

  
DeepSleepManager is a library that provides deepsleep functionality for ESP8266 Arduino.
this library use the Arduino library TimeLib.h  (PaulStoffregen / Time) for extended functionality with time. 


A primary goal was to switch on/off an ESP8266 with minimum harware and keep some data in memory without using flash memory.

Example sketches are provide to 
 - switch On/Off an arduino just with reset
 - switch ON the arduino with a external Standard user button who may be read in the loop()
 - activate deepSleep for long period (over 6 hours) with a regular wake up in low power mode to make some mesures.
## Functionality

The functions available in the library include

getRstReason(); 								// return the reason of the deepsleep awake 
startDeepSleep(sleepTimeSeconds); 				// start a deepSleepMode for a long time (over 1 Year) with default increment of 6 hours
permanentDeepSleep();   						// Activate DeepSleep Forever
getBootCounter();      							// Number of reboot since power on

Advanced functions
getRstReason(buttonPin); 						// return the reason of the deepsleep awake (optionnal Button to abort long deep sleep)
continueDeepSleep();    						// Return in deepSleep to terminate an interupted deepsleep 
WiFiUnlock();           						// arm a reset to restore WiFi back
WiFiLocked;         							// true if wifi is locked (awake from a deep sleep)



## Examples

The DeepSleepManager directory contains the DeepSleepManager library and some example sketches
illustrating how the library can be used in different way:

- `resetONOff` allow to swith On/Off with RST button and no external hardware.
	you may also just connect RST pin to a Standart press button
	when on the led Blink when off the Arduino is in minimal power mode.
	
- `resetOnOffWithDataSaved` same as above with some data saved in low power memory.

- `deepSleep24Hours` allow to manage very long deep sleep.
	you can arm a long deep sleep ( up to 100 Year )- with minimal harware
	the Arduino will sleep and wake on regular interval to check if the long deep sleep is elapsed.
	an extra input pin is needed to abort a pending  long deep sleep with a push button
	
  'mesureAndSendMailLater' make mesure on a DHT11 every minute and send a mail every day
	you can adjust the time for mesure up to 6 hours
	an extra input pin is needed to abort a pending  long deep sleep with a push button

## Technical notes:

	very long deep sleep are made with repetitive deepSleep with noWifi mode
	the elapsed time is keep in RTC memory (no flash used)
	when ESP8266 awake the WiFi is 'locked' in this case a WiFiUnlock() must be done.
	the last deep sleep restore the WiFi

