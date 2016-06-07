
const int led = 9;
const int bat_v_pin = A3;
const int bat_v_enable = 4;

void setup() {
  Serial.begin(9600);
  pinMode(bat_v_enable, OUTPUT);
}

void loop() {
  digitalWrite(bat_v_enable, LOW);
  delay(10);
  float bat_v = analogRead(bat_v_pin);
  delay(10);
  digitalWrite(bat_v_enable, HIGH);
  bat_v = (bat_v * (3.3/1024.0)) * 2.0;
  Serial.println(bat_v);
  delay(1000);
}
