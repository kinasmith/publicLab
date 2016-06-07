#include <Wire.h>
#include <SHT2x.h>
#include <SD.h>
#include <SPI.h>
#include <EnableInterrupt.h>
#include <DS3231.h>
#include <LowPower.h>

DS3231 rtc; //initialize the Real Time Clock

const int led = 9;
const int bat_v_pin = A3;
const int bat_v_enable = 4;
const int sd_pwr_enable = 6;
const int chipSelect = 7;
const int RTC_INT = 5; //This is the interrupt pin

int interval_min = 1; //this is the interval which the RTC will wake the MCU (microcontroller)

float bat_v;
float temp;
float humidity;

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

float getBat_v(int read_p, int en_p) {
  float v;
  digitalWrite(en_p, LOW);
  delay(10);
  v = analogRead(read_p);
  delay(10);
  digitalWrite(en_p, HIGH);
  v = (v * (3.3 / 1024.0)) * 2.0;
  return v;
}

void writeDataToCard(float t, float h, float v, int t_mo, int t_d, int t_h, int t_m, int t_s) {
  File dataFile = SD.open("LOG.CSV", FILE_WRITE);
  if (dataFile) {
    dataFile.print(t);
    dataFile.print(",");
    dataFile.print(h);
    dataFile.print(",");
    dataFile.print(v);
    dataFile.print(",");
    dataFile.print(t_mo);
    dataFile.print(",");
    dataFile.print(t_d);
    dataFile.print(",");
    dataFile.print(t_h);
    dataFile.print(",");
    dataFile.print(t_m);
    dataFile.print(",");
    dataFile.print(t_s);
    dataFile.println();
    dataFile.close();
  }
}

void Blink(byte PIN, int DELAY_MS) {
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN, HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN, LOW);
  delay(DELAY_MS);
}

void setup() {
  Wire.begin();
  Serial.begin(9600);
  pinMode(bat_v_enable, OUTPUT);
  pinMode(sd_pwr_enable, OUTPUT);
  pinMode(RTC_INT, INPUT_PULLUP);

  rtc.begin();
  rtc.adjust(DateTime((__DATE__), (__TIME__))); //this sets the RTC to the computer time. More documentation in other examples

  digitalWrite(sd_pwr_enable, LOW);
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1) {
      digitalWrite(led, HIGH);
      delay(200);
      digitalWrite(led, LOW);
      delay(200);
    }
  }
}

void loop() {
  DateTime now = rtc.now(); //get the current time
  if (now.second() == 0) { //at the top of the minute....
    //take readings
    bat_v = getBat_v(bat_v_pin, bat_v_enable); //takes 20ms
    temp = SHT2x.GetTemperature();
    humidity = SHT2x.GetHumidity();
    
    //write data
    writeDataToCard(temp, humidity, bat_v, now.month(), now.date(), now.hour(), now.minute(), now.second());
    Blink(led, 100);
    //calculate the next alarm time
    int nextHour = now.hour();
    int nextMinute = now.minute() + interval_min;
    if (nextMinute >= 60) {
      nextMinute -= 60;
      //this bit of code assumes an interval time of less than 60 min. There is a more elegant way of doing this, I'm sure.
      nextHour += 1;
    }
    Serial.println(temp);
    Serial.println(humidity);
    Serial.println(bat_v);
    enterSleep(nextHour, nextMinute, 0); //enter Sleep until alarm fires
  }
}
