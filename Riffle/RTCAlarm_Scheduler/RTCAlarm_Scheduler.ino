/* ------------
	Uses the alarm on the DS3231 to wake the Riffle from deep sleep
	Libraries used:
	PinChange Interrupt: https://github.com/GreyGnome/EnableInterrupt
	DS3231 Library from SeedStudio: https://github.com/bpg/DS3231
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
int interval_min = 45; //this is the interval which the RTC will wake the MCU (microcontroller)

//This function is called by the interrupt when it is triggered by the RTC
void pin5_interrupt() {
	disableInterrupt(RTC_INT); //first it Disables the interrupt so it doesn't get retriggered
}

//Puts the MCU into power saving sleep mode and sets the wake time
void enterSleep(int h, int m, int s) { //we give it an arguement for when we want it to wake up in Hour, Minute, Second
	rtc.clearINTStatus(); //resets the alarm interrupt status on the RTC
	enableInterrupt(RTC_INT, pin5_interrupt, FALLING); //Sets the interrupt on Pin5
	rtc.enableInterrupts(h, m, s); //Sets the alarm on the RTC to the specified time
	delay(100); //wait for a moment for everything to complete
	LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); //power down everything until the alarm fires
}

void setup() {
	Serial.begin(9600);
	pinMode(RTC_INT, INPUT_PULLUP);
	Wire.begin();
	rtc.begin();
	rtc.adjust(DateTime((__DATE__), (__TIME__))); //this sets the RTC to the computer time. More documentation in other examples
}

void loop() {
	DateTime now = rtc.now(); //get the current time
	if (now.second() == 0) { //at the top of the minute....
		//calculate the next alarm time
		int nextHour = now.hour();
		int nextMinute = now.minute() + interval_min;
		if (nextMinute >= 60) {
			//assumes an interval time of less than 60 min. 
			//There is a more elegant way of doing this, I'm sure. 
			nextMinute -= 60;
			nextHour += 1;       
		}
		//if hour passes over 24 (ie. Midnight), set Hour to 00. 
		if(nextHour >= 24) {
			nextHour -= 24;
		}
		Serial.print("The Current Time is: ");
		Serial.print(now.hour());
		Serial.print(":");
		Serial.print(now.minute());
		Serial.println();
		Serial.print("Sleeping until: ");
		Serial.print(nextHour);
		Serial.print(":");
		Serial.print(nextMinute);
		Serial.println();
		enterSleep(nextHour, nextMinute, 0); //enter Sleep until alarm fires
	}
}
