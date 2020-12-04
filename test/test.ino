const int pinTrig = 2;

void setup() {
  Serial.begin(115200);
  pinMode(pinTrig, OUTPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
}

void loop() {
  digitalWrite(pinTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig, LOW);

  long T = pulseIn(3, HIGH);
  float L = T / 58.82;
  Serial.println(L);
  delay(500);
}