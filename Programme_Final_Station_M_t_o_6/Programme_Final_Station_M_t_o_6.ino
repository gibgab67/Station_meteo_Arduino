/*
 ▄████  ██▓ ▄▄▄▄     ▄████  ▄▄▄       ▄▄▄▄   
 ██▒ ▀█▒▓██▒▓█████▄  ██▒ ▀█▒▒████▄    ▓█████▄ 
▒██░▄▄▄░▒██▒▒██▒ ▄██▒██░▄▄▄░▒██  ▀█▄  ▒██▒ ▄██
░▓█  ██▓░██░▒██░█▀  ░▓█  ██▓░██▄▄▄▄██ ▒██░█▀  
░▒▓███▀▒░██░░▓█  ▀█▓░▒▓███▀▒ ▓█   ▓██▒░▓█  ▀█▓
 ░▒   ▒ ░▓  ░▒▓███▀▒ ░▒   ▒  ▒▒   ▓▒█░░▒▓███▀▒
  ░   ░  ▒ ░▒░▒   ░   ░   ░   ▒   ▒▒ ░▒░▒   ░ 
░ ░   ░  ▒ ░ ░    ░ ░ ░   ░   ░   ▒    ░    ░ 
      ░  ░   ░            ░       ░  ░ ░      
                  ░                         ░
*/
/*
 * Programme final projet Station météo
 * 12/02/2023 20h15
 * ***Let's start !***
 * Version 6 -> corection d'un bug d'initialisation dût à une commande manquante
 */
/*
 * A FAIRE :
 * Import Ethernet : OK
 * Import Anémomètre : OK
 * Import I2C : OK
 * Import luminosité : OK
 * Import Pluviomètre : OK
 * Import Girouette : OK
 * Relevé des données : OK
 * Envoi info MQTT : OK
 * Nouveau capteur hygro + temp : OK
 */
 
//Section importations
#include <SPI.h>//Connexion avec la carte Ethernet
#include <Ethernet.h>//Gestion de la connexion
#include <MQTT.h>//pour message MQTT
#include <Wire.h> //Librairie pour la communication I2C
#include "SparkFunMPL3115A2.h" //Librairie du capteur "SparkFun MPL3115" (baromètre)
//#include "SparkFun_Si7021_Breakout_Library.h" //Librairie du capteur "SparkFun Si7021"(humidité) (non-utilisé)
#include <Adafruit_AHTX0.h> //librairie capteur hygrometrie + température

//Configuration Ethernet / MQTT
byte mac[] = {0x90, 0xA2, 0xDA, 0x11, 0x1D, 0x82};
IPAddress ip(192,168,0,64);// Adresse statique utilisé si pas de DHCP
IPAddress server(192, 168, 0, 61);

EthernetClient ethClient;
MQTTClient client;
const char ClientID = "Station_meteo";

//Configuration capteurs I2C (baromètre + hygromètre)
MPL3115A2 myPressure; //Instance du capteur de pression
//SI7021 humidityTemp;//Instance du capteur d'humidité
Adafruit_AHTX0 aht; //instance capteur humidité / température
Adafruit_Sensor *aht_humidity, *aht_temp; //séparation hygromètre / thermomètre
#define STAT_BLUE 7//led bleue
#define STAT_GREEN 8//led verte
#define REFERENCE_3V3 A3//3,3V tention de référence créé par la carte de capteurs
#define LIGHT A1//capteur de luminosité
//const byte BATT = A2;//Non utilisé, pas de batterie

//Configuration anémomètre
#define WSPEED 3
#define nbinterruptAnemo 5
byte nbWindInterrupt = 0;//nombre d'interuption par unité de temps
float windSpeed = 0.0;//variable de vitesse du vent
unsigned long lastWindInterrupt = 0;//dernière interruption

//configuration Girouette
#define WDIR A0

//Configuration pluviomètre
#define RAIN 2

unsigned long timer24h = 0;
unsigned long timer2S = 0;
unsigned long timer30S = 0;

void setup() {
  //test
  Serial.begin(9600);

  //Setup Led
  pinMode(STAT_BLUE, OUTPUT);
  pinMode(STAT_GREEN, OUTPUT);
  digitalWrite(STAT_BLUE, HIGH);
  digitalWrite(STAT_GREEN, HIGH);
  delay(20);

  //Setup hygromètre + thermomètre
  /* Périphérique natif non-utilisé
  humidityTemp.begin();
  */
  if (!aht.begin()) {
    Serial.println("Failed to find AHT10/AHT20 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("AHT10/AHT20 Found!");
  aht_temp = aht.getTemperatureSensor();//thermomètre
  aht_temp->printSensorDetails();
  aht_humidity = aht.getHumiditySensor();//hygromètre
  aht_humidity->printSensorDetails();
  digitalWrite(STAT_GREEN, LOW);

  //Setup baromètre
  myPressure.begin(); // On allume le capteur
  myPressure.setModeBarometer(); //Configuration en mode "baromètre"
  myPressure.setOversampleRate(7); // Taux d'échantillonage définit au maximum (7 = 128) /!\ attendre 512 ms entre chaque mesures
  myPressure.enableEventFlags(); // activation des évenements

  //Setup Anémomètre
  pinMode(WSPEED, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(WSPEED), interruptAnemo, FALLING);

  //Setup Girouette
  pinMode(WDIR, INPUT);

  //Setup Pluviomètre
  pinMode(RAIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RAIN), interruptPluvio, FALLING);

  //Setup Luminosité
  pinMode(REFERENCE_3V3, INPUT);
  pinMode(LIGHT, INPUT);

  //corection de l'érreur d'appel lors de la déclaration
  pluvioReset();

  //Setup Ethernet
  Serial.println("setup ethernet");
  Ethernet.begin(mac, ip);

  //Setup MQTT
  client.begin(server, ethClient);

  //fin setup
  digitalWrite(STAT_BLUE, LOW);
  
}

float tmpHumidity = 0;
float tmpHTemp = 0;
float tmpPressure = 0;
float tmpLum = 0;
float tmpWdir = 0;
float tmpWSpeed = 0;
float rainQuantity = 0;

void loop() {
  //mise à jour MQTT
  client.loop();
    
  if(abs(millis() - timer24h) >= 86400000){// toute les 24h
    timer24h = millis();
    pluvioReset();
  }
  if(abs(millis() - timer2S) >= 2000){// toute les 2 secondes
    timer2S = millis();
    majWspeed();

    tmpWdir = get_wind_direction();//lecture girouette
    tmpWSpeed = getWSPEED();//lecture anémometre

    publish();//envoi des données
  } 
  if(abs(millis() - timer30S) >= 30000){// toute les 30 secondes
    timer30S = millis();

    tmpHumidity = getHumidity(); //lecture humidité
    tmpHTemp = getHTemp(); //lecture température
    tmpPressure = getPressure(); //lecture pression atmo
    tmpLum = getLum(); //lecture luminosité 

    digitalWrite(STAT_GREEN, HIGH);
    delay(20);//pour voir le voyant
    digitalWrite(STAT_GREEN, LOW);
  }

  delay(500);// tempo
}

void majWspeed(){
  windSpeed = (nbWindInterrupt * 1.2);//Reff: 2.4 km/h si 1 contact par seconde (donc 1,2 km/h sur 2 secondes)
  nbWindInterrupt = 0;
  
}

//Fonctions MQTT
////Publication d'une nouvelle donnée (2 possibilités)
void pub(char topic[64], char mess){
  client.publish(topic, mess);
}
void pub(char topic[64], float mess){

  char result[8];
  dtostrf(mess, 6, 2, result);

  client.publish(topic, result);
}
void publish() {
  digitalWrite(SS, LOW);
  for(int i = 0; i<3; i++){
    if(client.connected()){
      digitalWrite(STAT_BLUE, HIGH);

      pub("stationMeteo/humidity", tmpHumidity);
      pub("stationMeteo/temperature", tmpHTemp);
      pub("stationMeteo/pressure", tmpPressure);
      pub("stationMeteo/luminosité", tmpLum);
      pub("stationMeteo/rainQuantity", rainQuantity);
      pub("stationMeteo/windDirection", tmpWdir);
      pub("stationMeteo/windSpeed", tmpWSpeed);

      digitalWrite(STAT_BLUE, LOW);

      break;
    } else {
      client.connect("Station_meteo");
    }
    delay(500);
    
  }
  digitalWrite(SS, HIGH);
}

//Fonctions Capteur I2C
////Fonction de lecture de l'humidité ambiante
float getHumidity() {
  //return humidityTemp.getRH(); obsolète
  sensors_event_t humidity;
  aht_humidity->getEvent(&humidity);
  return humidity.relative_humidity;
}
////Fonction de lecture de la température sur l'hygromètre
float getHTemp(){
  //return humidityTemp.getTemp(); obsolète
  sensors_event_t temp;
  aht_temp->getEvent(&temp);
  return temp.temperature;
}
////Fonction de lecture de la pression atmosphérique
float getPressure() {
  return myPressure.readPressure();
}

//Fonctions Anémomètre
////Fonction d'interuption
void interruptAnemo() {//chargé de compter les tours de l'anémomètre
  if((millis() - lastWindInterrupt) > 50){//évite les doubles impultions
    nbWindInterrupt++;//ajout de 1 tour
    lastWindInterrupt = millis();
  } 
}
////Fonction de retour de la vitesse du vent
float getWSPEED() {
  return windSpeed;
}

//Fonction capteur de luminosité
float getLum(){
  float operatingVoltage = analogRead(REFERENCE_3V3);//voltage de réfference
  float lightSensor = analogRead(LIGHT);//lecture de la valeur analogique
  operatingVoltage = 3.3 / operatingVoltage; //calcul du rapport entre la valeur analogique et numérique
  lightSensor = operatingVoltage * lightSensor;//obtention de la luminosité en Volt
  return (lightSensor);
}

//Fonctions Girouette
//// Fonction de lecture de direction du vent
float get_wind_direction()
// read the wind direction sensor, return heading in degrees
{
  unsigned int adc;
  adc = averageAnalogRead(WDIR); // get the current reading from the sensor
  // The following table is ADC readings for the wind direction sensor output, sorted from low to high.
  // Each threshold is the midpoint between adjacent headings. The output is degrees for that ADC reading.
  // Note that these are not in compass degree order! See Weather Meters datasheet for more information.
  if (adc < 380) return (113);
  if (adc < 393) return (68);
  if (adc < 414) return (90);
  if (adc < 456) return (158);
  if (adc < 508) return (135);
  if (adc < 551) return (203);
  if (adc < 615) return (180);
  if (adc < 680) return (23);
  if (adc < 746) return (45);
  if (adc < 801) return (248);
  if (adc < 833) return (225);
  if (adc < 878) return (338);
  if (adc < 913) return (0);
  if (adc < 940) return (293);
  if (adc < 967) return (315);
  if (adc < 990) return (270);
  return (-1); // error, disconnected?
}
////Fonction de calcul de la résistance moyenne de la girouette
int averageAnalogRead(int pinToRead)
{
  byte numberOfReadings = 8;
  unsigned int runningValue = 0;
  for(int x = 0 ; x < numberOfReadings ; x++)
    runningValue += analogRead(pinToRead);
  runningValue /= numberOfReadings;
  return(runningValue);
}

//Fonctions Pluviomètre
void interruptPluvio (){
  rainQuantity += 0.2794;//on aditionne une mesure de pluie
}
void pluvioReset() {
  rainQuantity = 0; //reset du compteur de pluie
}
