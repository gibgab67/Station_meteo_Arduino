#define WSPEED 3
#define nbinterruptAnemo 5

int tempsEntreTour [nbinterruptAnemo];
byte indiceTab = 0;//indice du tableau
unsigned long dernierinterruptAnemo = 0; //variable de l'heure de la dernière intéruption

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(WSPEED, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(WSPEED), interruptAnemo, FALLING);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("Vitesse du vent : ");
  Serial.print(getWSPEED());
  Serial.println(" km/H");
  delay(1000);
}

float getWSPEED() {
  
  if((millis() - dernierinterruptAnemo) > 3000){// si pas de tour depuis + de 3 seconde
    memset(tempsEntreTour, 0, sizeof(tempsEntreTour));//vide le tableau de temps
    return 0;
  }
  
  float sommeTemps = 0;// variable de la somme des temps mesurés
  byte nbTemps = 0; //nombres de temps enregistré (zéro ignorés)
  for(byte i = 0; i < sizeof(tempsEntreTour); i++){//pour chaque temps du tableau
    if(tempsEntreTour[i] != 0){// si temps différent de zéro
      sommeTemps += tempsEntreTour[i];//ajout à sommeTemps
      nbTemps++;
    }
  }

  if(sommeTemps == 0) {
    return 0;
  }else {
    float moyenneTempsSeconde = (sommeTemps / nbTemps) / 1000;//calcule de la moyenne des temps et conversion en seconde
    Serial.println(moyenneTempsSeconde);


    float vitesseVent = (2,4 / moyenneTempsSeconde); //Calcule de la vitesse du vent (si 1 = 2,4) 2.4 / moyenneDesTemps
    
    return (vitesseVent);
  }
}

void interruptAnemo() {//chargé de stocker le temps des tours à chaque intéruption de l'anémomètre
  
  unsigned long temps = millis();
  unsigned long diff = millis() - dernierinterruptAnemo;

  if(diff > 100){//on évite les double interuptions (rebonds)
    if(diff > 0 && diff < 3000){// SI le temps est supèrieur à la dernière interruption ( !!! remise à 0 de millis à 50 jours) et si durée infèrieur à 3 seconde
      tempsEntreTour[indiceTab] = (temps - dernierinterruptAnemo); //stocke le temps d'un tour
      indiceTab = (indiceTab + 1) % nbinterruptAnemo; //ajoute 1 puis calcule le reste de la division par nbinterruptAnemo (si indiceTab vaut nbinterruptAnemo -> remise à 0)
    } else {
      indiceTab = 0;//remise à zéro de l'indice de tableau
    }
  }
  dernierinterruptAnemo = temps;
}
