/*
 * For the Riffle Datalogger
 * Sketch logs the Temperature and Humidity from a SHT21 sensor
 * at an interval.
 * It uses the RTC as a scheduler by using an Alarm and Interrupt
 * pin to wake up the ATmega at an interval.
 * It logs to the SD card and outputs time and values in the Serial
 * Monitor
 * 
 * This Sketch uses the SdFat.h library instead of the SD.h library that is 
 * is included in the IDE due to a bug which doesn't allow the SD card to sleep.
 * 
 * Also, disabling the Serial writes will save some power. Once everything is
 * tested and working set the debug variable to 0 to disable Serial.
 * 
 * Kina Smith
 * kina.smith@gmail.com
 */

 
#include <Wire.h>
#include <SPI.h>
#include <SHT2x.h>		//  https://github.com/misenso/SHT2x-Arduino-Library
#include <SdFat.h> 		//  https://github.com/greiman/SdFat
#include <EnableInterrupt.h>	//  https://github.com/GreyGnome/EnableInterrupt
#include <DS3231.h> 	//  https://github.com/kinasmith/DS3231
#include <LowPower.h> 	//  https://github.com/rocketscream/Low-Power

#define debug 1

DS3231 rtc; //initialize the Real Time Clock

SdFat sd; //SD card
SdFile myFile; //File to save to



const int led = 9; //led pin
const int bat_v_pin = A3; //battery voltage pin (1/2 of actual voltage)
const int bat_v_enable = 4; //enable pin for bat. voltage read
const int sd_pwr_enable = 6; //enable pin for SD power
const int chipSelect = 7; //chipSelect for SD card
const int RTC_INT = 5; //RTC interrupt pin
const int pinout_pwr_enable = 8; //enable pin for header power

int interval_sec = 10; //Logging interval in seconds

//sensor values
float bat_v;
float temp;
float humidity;

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

float getBat_v(int read_p, int en_p) {
	float v;
	digitalWrite(en_p, LOW); //write mosfet low to enable read
	delay(10); //wait for it to settle
	v = analogRead(read_p); //read voltage
	delay(10); //wait some more...for some reason
	digitalWrite(en_p, HIGH); //disable read circuit
	v = (v * (3.3 / 1024.0)) * 2.0; //calculate actual voltage
	return v; 
}

void writeDataToCard(float t, float h, float v, long utc) {
	//open and write files to SD card
	if (!myFile.open("data.csv", O_RDWR | O_CREAT | O_AT_END))  //open file
		while (1) Blink(led, 200); //if fails to open, blink forever
	myFile.print(t);
	myFile.print(",");
	myFile.print(h);
	myFile.print(",");
	myFile.print(v);
	myFile.print(",");
	myFile.print(utc);
	myFile.println();
	myFile.close();
}

void Blink(byte PIN, int DELAY_MS) {
	//Blink an LED
	pinMode(PIN, OUTPUT);
	digitalWrite(PIN, HIGH);
	delay(DELAY_MS);
	digitalWrite(PIN, LOW);
	delay(DELAY_MS);
}

void setup() {
	Wire.begin();
	if(debug) Serial.begin(9600);
	pinMode(bat_v_enable, OUTPUT);
	pinMode(sd_pwr_enable, OUTPUT);
	pinMode(RTC_INT, INPUT_PULLUP); //RTC interrupt line requires a pullup
	pinMode(pinout_pwr_enable, OUTPUT);
	digitalWrite(pinout_pwr_enable, HIGH);
	rtc.begin(); //start RTC
	if(debug) rtc.adjust(DateTime((__DATE__), (__TIME__))); //sets the RTC to the computer time.

	digitalWrite(sd_pwr_enable, LOW); //Enable power for SD card  
	delay(1); //wait for pin to settle
	if (!sd.begin(chipSelect)) {
		if(debug) Serial.println("Card failed, or not present");
		while (1) {
			Blink(led, 200);
		}
	}
	if(debug) Serial.println("SD Card Initialized");
}

void loop() {
	DateTime now = rtc.now(); //get the current time
	DateTime nextAlarm = DateTime(now.unixtime() + interval_sec);
	if(debug) {
		Serial.print("The Current Time is: ");
		Serial.print(now.unixtime());
		Serial.println();
	}
	//take readings
	bat_v = getBat_v(bat_v_pin, bat_v_enable); //takes 20ms
	temp = SHT2x.GetTemperature();
	humidity = SHT2x.GetHumidity();
	if(debug) {	
		Serial.print("Temp: ");
		Serial.print(temp);
		Serial.print(", ");
		Serial.print("Humidity: ");
		Serial.print(humidity);
		Serial.print(", ");
		Serial.print("Batt V: ");
		Serial.print(bat_v);
		Serial.println();
	}
	//write data
	writeDataToCard(temp, humidity, bat_v, now.unixtime());
	Blink(led, 100);
	if(debug) {
		Serial.print("Sleeping for ");
		Serial.print(interval_sec);
		Serial.print(" seconds.");
		Serial.println();
	}
	enterSleep(nextAlarm); //enter Sleep until alarm fires
}

