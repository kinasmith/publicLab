/*
    For the Riffle Datalogger
    by Kina Smith
    kina.smith@gmail.com

    Sketch is get power consumption as low as possible while logging to the
    SD card.

*/
#include <Wire.h>
#include <SPI.h>
#include "SHT2x.h"    //  https://github.com/misenso/SHT2x-Arduino-Library
#include "SdFat.h"    //  https://github.com/greiman/SdFat
#include "EnableInterrupt.h"  //  https://github.com/GreyGnome/EnableInterrupt
#include "DS3231.h"   //  https://github.com/kinasmith/DS3231
#include "LowPower.h"   //  https://github.com/rocketscream/Low-Power

//#define DEBUG //power consumption is around 5mA w/Serial enabled. and 0.48mA w/o

DS3231 rtc; //initialize the Real Time Clock
SdFat sd;
SdFile myFile;

const int led = 9; //Feedback LED
const int bat_v_pin = A3;
const int bat_v_enable = 4; //enable pin for bat. voltage read
const int rtc_int = 5; //rtc interrupt pin
const int sd_pwr_enable = 6; //enable pin for SD power
const int hdr_pwr_enable = 8; //enable pin for header power
const int chipSelect = 7; //SPI Chip Select for SD Card
const int MOSIpin = 11;
const int MISOpin = 12;
const int SSpin = 10;

int counter = 0;

int interval_sec = 15; //Logging interval in seconds
float bat_v;

void setup() {
#ifdef DEBUG
  Serial.begin(9600);
#endif
  Wire.begin();
  rtc.begin();
  pinMode(rtc_int, INPUT_PULLUP); //rtc needs the interrupt line to be pulled up
  // rtc.adjust(DateTime((__DATE__), (__TIME__))); //Adjust automatically
  pinMode(led, OUTPUT);
  pinMode(chipSelect, OUTPUT); 
  pinMode(bat_v_enable, OUTPUT); 
  digitalWrite(bat_v_enable, HIGH); //Turn off Battery Reading
  pinMode(sd_pwr_enable, OUTPUT);
}

void loop() {
  DateTime now = rtc.now(); //get the current time
  DateTime nextAlarm = DateTime(now.unixtime() + interval_sec);
#ifdef DEBUG
  Serial.print("Now: ");
  Serial.print(now.unixtime());
  Serial.print(" Alarm Set for: ");
  Serial.println(nextAlarm.unixtime());
  Serial.flush();
#endif
  bat_v = getBat_v(bat_v_pin, bat_v_enable); //takes 20ms
#ifdef DEBUG
  Serial.print("Battery Voltage is: ");
  Serial.print(bat_v);
  Serial.println(" Volts.");
  Serial.flush();
#endif
  writeToSd(bat_v, now.unixtime(), counter++);
#ifdef DEBUG
  Serial.print("SD Card Written. Sleeping for ");
  Serial.print(interval_sec);
  Serial.print(" seconds.");
  Serial.println();
  Serial.println("---------------------------------");
  Serial.flush();
#endif
  enterSleep(nextAlarm);
}


void rtc_interrupt() {
  disableInterrupt(rtc_int); //first it Disables the interrupt so it doesn't get retriggered
}

void enterSleep(DateTime& dt) { //argument is Wake Time as a DateTime object
  delay(20); //Wait for file writing to finish. 10ms works somethings, 20 is more stable
  digitalWrite(sd_pwr_enable, HIGH); //Turn off power to SD Card
  delay(10); //wait for SD Card to power down
  rtc.clearAlarm(); //resets the alarm interrupt status on the rtc
  enableInterrupt(rtc_int, rtc_interrupt, FALLING); //enables the interrupt on Pin5
  rtc.enableAlarm(dt); //Sets the alarm on the rtc to the specified time (using the DateTime Object passed in)
  delay(1); //wait for a moment for everything to complete
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); //power down everything until the alarm fires
}

float getBat_v(int read, int en) {
  float v;
  digitalWrite(en, LOW); //write mosfet low to enable read
  delay(10); //wait for it to settle
  v = analogRead(bat_v_pin); //read voltage
  delay(10); //wait some more...for some reason
  digitalWrite(en, HIGH); //disable read circuit
  v = (v * (3.3 / 1024.0)) * 2.0; //calculate actual voltage
  return v;
}

void blink(byte PIN, int DELAY_MS) {
  digitalWrite(PIN, HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN, LOW);
  delay(DELAY_MS);
}

///********************************************************************
void writeToSd(int v, long t, int c) {
  digitalWrite(led, HIGH); //LED ON, write cycle start
  /**** POWER ON SD CARD ****/
  digitalWrite(sd_pwr_enable, LOW); //Turn power to SD Card On
  delay(20); //wait for power to stabilize (!!) 10ms works sometimes
  /**** INIT SD CARD ****/
#ifdef DEBUG
  Serial.print("SD Card Initializing...");
#endif
  if (!sd.begin(chipSelect)) {  //init. card
#ifdef DEBUG
    Serial.println("Failed!");
#endif
    while (1); //if card fails to init. the led will stay lit.
  }
#ifdef DEBUG
  Serial.println("Success");
#endif
  /**** OPEN FILE ****/
#ifdef DEBUG
  Serial.print("File Opening...");
#endif
  if (!myFile.open("data.csv", O_RDWR | O_CREAT | O_AT_END)) {  //open file
#ifdef DEBUG
    Serial.println("Failed!");
#endif
    while (1);
  }
#ifdef DEBUG
  Serial.println("Success");
#endif
  /**** WRITE TO FILE ****/
  myFile.print(v);
  myFile.print(",");
  myFile.print(t);
  myFile.print(",");
  myFile.print(c);
  myFile.println();
  myFile.close();
  digitalWrite(led, LOW); //LED will stay on if something broke

}
//*********************************************************************/
