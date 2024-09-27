#define WSPEED 3
#define nbinterruptAnemo 5

int nbWindInterrupt = 0;//nombre d'interuption par unité de temps
unsigned long lastWindInterrupt = 0;//dernière interruption

void setup() {
  Serial.begin(9600);
  pinMode(WSPEED, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(WSPEED), interruptAnemo, FALLING);
}

void loop() {
  delay(2000);//programme test uniquement
  Serial.println(nbWindInterrupt * 1.2);
  nbWindInterrupt = 0;
}

void interruptAnemo() {
  if((millis() - lastWindInterrupt) > 50){//évite les doubles impultions
    nbWindInterrupt++;//ajout de 1 tour
    lastWindInterrupt = millis();s
  } 
}