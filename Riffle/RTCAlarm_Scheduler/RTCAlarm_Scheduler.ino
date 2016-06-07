/* ------------
	Uses the alarm on the DS3231 to wake the Riffle from deep sleep
	Libraries used:
	PinChange Interrupt: https://github.com/GreyGnome/EnableInterrupt
	My DS3231 Library, Forked from SeedStudio: https://github.com/kinasmith/DS3231
	LowPower: https://github.com/rocketscream/Low-Power

	Kina Smith
	kina.smith@gmail.com
	------------ */

#include <Wire.h>
#include <EnableInterrupt.h>
#include <DS3231.h>
#include <LowPower.h>

DS3231 rtc; //initialize the Real Time Clock

const int RTC_INT = 5; //This is the interrupt pin
int interval_sec = 20; //this is the interval which the RTC will wake the MCU (microcontroller)

int prevSecond = 0;

//This function is called by the interrupt when it is triggered by the RTC
void pin5_interrupt() {
  disableInterrupt(RTC_INT); //first it Disables the interrupt so it doesn't get retriggered
}

//Puts the MCU into power saving sleep mode and sets the wake time
void enterSleep(DateTime& dt) { //argument is Wake Time as a DateTime object
  rtc.clearAlarm(); //resets the alarm interrupt status on the RTC
  enableInterrupt(RTC_INT, pin5_interrupt, FALLING); //enables the interrupt on Pin5
  rtc.enableAlarm(dt); //Sets the alarm on the RTC to the specified time (using the DateTime Object passed in)
  delay(100); //wait for a moment for everything to complete
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); //power down everything until the alarm fires
}

void setup() {
  Serial.begin(9600);
  pinMode(RTC_INT, INPUT_PULLUP);
  Wire.begin();
  rtc.begin();
  //I personally like setting the time stamp to Greenwich Mean Time, that way the UnixTime code converts correctly for you time zone.
  //I do this by setting my time zone on my computer to GMT, then uncommenting the following line, which sets the clock, the commenting it again when done.
  //  rtc.adjust(DateTime((__DATE__), (__TIME__))); //this sets the RTC to the computer time. More documentation in other examples
}

void loop() {
  DateTime now = rtc.now(); //get the current time
  
  DateTime nextAlarm = DateTime(now.unixtime() + interval_sec);

  Serial.print("The Current Time is: ");
  Serial.print(now.unixtime());
  Serial.println();
  Serial.print("Sleeping for ");
  Serial.print(interval_sec);
  Serial.print(" seconds.");
  Serial.println();
  
  enterSleep(nextAlarm); //enter Sleep until alarm fires
}

