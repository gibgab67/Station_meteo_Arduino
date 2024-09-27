int pin = A1;
int value;
void setup() {
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  value = analogRead(pin);
  Serial.println(value);
  delay(500);
}
