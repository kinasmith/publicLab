//#include <jeelib-sleepy.h>
#include "LowPower.h"

//ISR(WDT_vect) { Sleepy::watchdogEvent(); }

const int led = 9; 
//const int led = 13;
void setup() {
  
  pinMode(led, OUTPUT);
}


void loop() {
  digitalWrite(led, HIGH); 
//  delay(500);  
  LowPower.powerDown(SLEEP_500MS, ADC_OFF, BOD_OFF);              
//  Sleepy::loseSomeTime(500);
  digitalWrite(led, LOW);  
//  delay(500);              
  LowPower.powerDown(SLEEP_500MS, ADC_OFF, BOD_OFF);  
//  Sleepy::loseSomeTime(500);
}
