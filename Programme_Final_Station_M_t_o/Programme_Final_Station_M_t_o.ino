/*
 * Programme final projet Station météo
 * 12/02/2023 20h15
 * ***Let's start !***
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
 * Ecran + bouton : NO
 */
 
//Section importations
#include <SPI.h>//Connexion avec la carte Ethernet
#include <Ethernet.h>//Gestion de la connexion
#include <PubSubClient.h>//pour message MQTT
#include <Wire.h> //Librairie pour la communication I2C
#include "SparkFunMPL3115A2.h" //Librairie du capteur "SparkFun MPL3115" (baromètre)
#include "SparkFun_Si7021_Breakout_Library.h" //Librairie du capteur "SparkFun Si7021"(humidité)
#include "timer.h" //Librarie du timer pour le déclenchement des actions (j'ai la flemme de toucher aux registres)
#include "timerManager.h" //Librairie facilitant la gestion des deux timers (première à gicler si manque de place)

//Configuration Ethernet / MQTT
byte mac[] = {0x90, 0xA2, 0xDA, 0x11, 0x1D, 0x82};
IPAddress ip(192,168,0,64);// Adresse statique utilisé si pas de DHCP
IPAddress server(192, 168, 0, 61);

EthernetClient ethClient;
PubSubClient client(ethClient);
char ClientID = "Station_meteo";

//Configuration capteurs I2C (baromètre + hygromètre)
MPL3115A2 myPressure; //Instance du capteur de pression
Weather humidityTemp;//Instance du capteur d'humidité
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
float rainQuantity = 0.0;

//Création des timers
Timer timer2sec;
Timer timer5sec;
Timer timer1min;

void setup() {
  Serial.begin(9600);// PROVISOIRE ?
  //Serial.println("Hello World !");

  //Setup MQTT
  client.setServer(server, 1883);
  //Serial.println("MQTT : OK");

  //Setup Ethernet
  #if defined(WIZ550io_WITH_MACADDRESS)
  Ethernet.begin(ip);
  #else
  Ethernet.begin(mac, ip);
  #endif
  //if(!Ethernet.begin(mac))Ethernet.begin(mac, ip);//Si DHCP injoignable, utiliser adresse statique
  //Serial.println(Ethernet.localIP());



  //Setup baromètre
  myPressure.begin(); // On allume le capteur
  myPressure.setModeBarometer(); //Configuration en mode "baromètre"
  myPressure.setOversampleRate(7); // Taux d'échantillonage définit au maximum (7 = 128) /!\ attendre 512 ms entre chaque mesures
  myPressure.enableEventFlags(); // activation des évenements

  //Setup hygromètre + thermomètre
  humidityTemp.begin();

  //Setup Anémomètre
  pinMode(WSPEED, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(WSPEED), interruptAnemo, FALLING);

  //Setup Girouette
  pinMode(WDIR, INPUT);

  //Setup Pluviomètre
  attachInterrupt(digitalPinToInterrupt(RAIN), interruptPluvio, FALLING);
  pinMode(RAIN, INPUT_PULLUP);

  //Setup Led
  pinMode(STAT_BLUE, OUTPUT);
  pinMode(STAT_GREEN, OUTPUT);

  //Setup Luminausité
  pinMode(LIGHT, INPUT);

  //Setup Timers
  timer2sec.setInterval(2000);
  timer5sec.setInterval(5000);
  timer1min.setInterval(30000);
  timer2sec.setCallback(actual2sec);
  timer5sec.setCallback(actual5sec);
  timer1min.setCallback(actual1min);
  TimerManager::instance().start();
}

//variable d'historique
int Hgirouette = -1;
float Hhumidite = NULL;
float Htemperature = NULL;
float HpressionAtm = NULL;
float HvitesseVent = NULL;
float Hluminosite = NULL;
float HrainQuantity = NULL;

float tmpHumidity = 0;
float tmpHTemp = 0;
float tmpPressure = 0;
float tmpLum = 0;
int tmpWdir = 0;
float tmpWSpeed = 0;

void loop() {
  //Serial.println("loop");
  //mise à jour MQTT
  client.loop();

  //mise à jour timers
  TimerManager::instance().update();

  //reset led
  digitalWrite(STAT_GREEN, LOW);
  digitalWrite(STAT_BLUE, LOW);
}


//Fonctions Timers
////Toute les minutes
void actual1min() {
  digitalWrite(STAT_GREEN, HIGH);
    
  tmpHumidity = getHumidity(); //lecture humidité
  tmpHTemp = getHTemp(); //lecture température
  tmpPressure = getPressure(); //lecture pression atmo
  tmpLum = getLum(); //lecture luminosité 
}
////Actualisation courte
void actual5sec(){
  //Serial.println("5sec");
  digitalWrite(STAT_BLUE, HIGH);
    
  tmpWdir = get_wind_direction();
  tmpWSpeed = getWSPEED();

  publish();
}
////Actualisation 2 secondes
void actual2sec(){
  //Serial.println("2sec");
  windSpeed = (nbWindInterrupt * 1.2);
  nbWindInterrupt = 0;
}

//Fonctions MQTT
////Publication d'une nouvelle donnée (2 possibilité)
void pub(char topic[64], char mess){
  client.publish(topic, mess);
}
void pub(char topic[64], float mess){
  Serial.println("start pub float");

  char result[8];
  dtostrf(mess, 6, 2, result);
  Serial.println("end math");

  client.publish(topic, result);
}
void pub(char topic[64], int mess){
  Serial.println("start pub int");

  char buffer[64];
  itoa ( mess, buffer, 10);
  Serial.println("end math");

  client.publish(topic, buffer);
}
void publish() {
  TimerManager::instance().pause();
  Serial.println("start publish");

  if(!client.connected()){
    reconnect();
  }

  if(tmpHumidity != Hhumidite){
    pub("stationMeteo/data/humidity", tmpHumidity);
    Hhumidite = tmpHumidity;
    //Serial.println("update Humidity");
  }
  if(tmpHTemp != Htemperature){//si temperature différente
    pub("stationMeteo/data/temperature", tmpHTemp);
    Htemperature = tmpHTemp;
    //Serial.println("update Temp");
  }
  if(tmpPressure != HpressionAtm){//si pression atmosphérique différente
    pub("stationMeteo/data/pressure", tmpPressure);
    HpressionAtm = tmpPressure;
    //Serial.println("update Press");
  }
  if(tmpLum != Hluminosite){//si luminosité différente
    pub("stationMeteo/data/luminosité", tmpLum);
    Hluminosite = tmpLum;
    //Serial.println(tmpLum);
  }
  if(rainQuantity != HrainQuantity){//si il a plu
    pub("stationMeteo/data/rainQuantity", rainQuantity);
    HrainQuantity = rainQuantity;
    //Serial.println("update rain");
  }
  if(tmpWdir != Hgirouette){//si la direction du vent à changé
    pub("stationMeteo/data/windDirection", tmpWdir);
    Hgirouette = tmpWdir;
    //Serial.println("update girouette");
  }
  if(tmpWSpeed != HvitesseVent) {
    pub("stationMeteo/data/windSpeed", tmpWSpeed);
    HvitesseVent = tmpWSpeed;
    //Serial.println("update wSpeed");
  }

  TimerManager::instance().start();
  Serial.println("end publish");
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    //Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("meteoArduino")) {
      //Serial.println("connected");
    } else {
      //Serial.print("failed, rc=");
      //Serial.print(client.state());
      //Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//Fonctions Capteur I2C
////Fonction de lecture de l'humidité ambiante
float getHumidity() {
  return humidityTemp.getRH();
}
////Fonction de lecture de la température sur l'hygromètre
float getHTemp(){
  return humidityTemp.getTemp();
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
int get_wind_direction()
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
