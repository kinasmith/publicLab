#include "LowPower.h"
#include <SPI.h>
#include <SdFat.h>

SdFat sd;
SdFile myFile;

const int led = 9;
const int bat_v_enable = 4; //enable pin for bat. voltage read
const int RTC_INT = 5; //RTC interrupt pin
const int sd_pwr_enable = 6; //enable pin for SD power
const int pinout_pwr_enable = 8; //enable pin for header power
const int chipSelect = 7;
const int MOSIpin = 11;
const int MISOpin = 12;
const int SSpin = 10;

int counter = 0;

void setup() {
  // pulling up the SPI lines at the start of Setup with 328p’s internal resistors

  pinMode(chipSelect, OUTPUT); digitalWrite(chipSelect, HIGH); //pullup SD CS pin
  pinMode(MOSIpin, OUTPUT); digitalWrite(MOSIpin, HIGH);//pullup the MOSI pin
  pinMode(MISOpin, INPUT); digitalWrite(MISOpin, HIGH); //pullup the MISO pin
  pinMode(SSpin, OUTPUT); digitalWrite(SSpin, HIGH);
  pinMode(led, OUTPUT);

  pinMode(bat_v_enable, OUTPUT); digitalWrite(bat_v_enable, HIGH);
  //enable sd power...power cycling is still weird. Don't do it.
  pinMode(sd_pwr_enable, OUTPUT); digitalWrite(sd_pwr_enable, LOW);

  delay(1);
  digitalWrite(led, HIGH);
  if (!sd.begin(chipSelect))  //init. card
    while (1);
  digitalWrite(led, LOW);
}

void loop() {
  writeToSd(counter++);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}

void writeToSd(int d) {
  digitalWrite(led, HIGH); //turn LED on
  if (!myFile.open("test.txt", O_RDWR | O_CREAT | O_AT_END))  //open file
    while (1);
  myFile.println(d);
  myFile.close();
  digitalWrite(led, LOW); //LED will stay on if something broke
}

