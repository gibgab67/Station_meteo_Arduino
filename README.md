# Station_meteo_Arduino
Programme Arduinno de gestion d'une station météo connecté.

J'ai réalisé ce projet en un an et demi, en travaillant par intermittence. Je ne pensais pas le publier à l'origine, mais je me dis que cela pourra servir à d'autres !
Dans ce programme, tous les commentaires sont en français, rédigé à l'origine pour moi seul. Si le besoin se fait sentir, je pourrais envisager une traduction en anglais.

J'aimerais également préciser que c'est la première fois que je publie un de mes "bricolage", donc si vous avez des commentaires, des modifications ou des corrections, je suis tout ouïe.

## Objectif du projet
Le projet en tant que tel a été réalisé dans un but simple : faire une station météo pour moi et plus encore, me prouver que j'en était capable.
Ce n'est que tout récemment, alors que le projet touche à sa fin, et que la "mise en production" (dans mon jardin) approche que je me dit que ce serait intéressant de le partager.

Je tiens à préciser que ce projet ne s'inscrit dans aucun cadre scolaire ou professionnel. Cela vient d'un besoin personnel uniquement, et c'est la raison pour laquelle aucun suivit de projet sérieux n'a été réalisé, tout au plus une liste de tâches dans le programme. J'aimerais cependant à l'avenir partager d'autres projets, cette fois-ci plus détaillé et mieux documenté.
Ne voyez donc pas en ce travail une démonstration de mes compétences de gestion, mais plutôt un partage de mon envie de faire et un premier essai de communication autours.

## Materiel utilisé
Point de vue électronique, j'ai utilisé :
- Carte Arduino UNO
- Shield Ethernet compatible PoE DFR0850 (gère la connection Ethernet et l'alimentation en PoE)
- Shield météo DEV-13956 (interface les capteurs météo sur la carte Arduino)
- Un Jeu de capteurs météo (https://www.gotronic.fr/art-jeu-de-capteurs-meteo-33052.htm)
- Capteur de T° et d'humidité AM2301B

## Utilité du capteur AM2301B
Après test, il s'est avéré que les capteurs de température et d'humidité intégrés au shield météo donnaient une valeur erronée, souvent au-dessus de la valeur réelle. Je pense que la configuration "sandwich" de la carte Arduino et des deux shields nuit à la bonne prise de température et d'hygrométrie. J'ai donc installé un nouveau capteur déporté qui sera installé à l'extérieur du boîtier.

## Fonctionnement détaillé de la station
La station météo est composé de deux éléments principaux, la "tour", qui supporte les capteurs de vent et de pluie, et l'unité centrale, qui abrite les capteurs de température, hygrométrie, pression atmosphérique ainsi que les éléments de contrôle de la station météo. Les données sont ensuite renvoyées vers un serveur de domotique Jeedom, hébergé sur un raspberry Pi. 

### L'unité centrale
Composé de soucoupe à pot de fleurs percées puis assemblées (âme sensible s'abstenir) elle permet de maintenir à l'abri de l'humidité les capteurs et les éléments de contrôle.

![20241007_115622](https://github.com/user-attachments/assets/b98ad360-3eaa-49a6-ab39-b65277b7ba76)
Exemple de soucoupe constituant le corps de l'unité centrale.


![20241007_115703](https://github.com/user-attachments/assets/e46cbb4e-185f-48d3-bbf3-7dd5a1b83df6)
Le corps assemblé, sans l'Arduino, ventilateur visible en bas à gauche.


![20241007_115728](https://github.com/user-attachments/assets/776e8226-0970-4afc-a4c1-cf45f8fa08cf)
Vu du dessous de l'unité central, la pièce noire est le capteur de température et d'humidité.


![20241007_115439](https://github.com/user-attachments/assets/b10d2b18-46e2-47bf-929f-2e3678d6b1ba)
A l'intérieur de l'unité centrale, on retrouve la carte Arduino (en bas), qui contrôle la station, la carte Ethernet (au milieu) qui alimente le système et permet la connexion au serveur de domotique, et l'interface (en haut) qui permet la connexion aux capteurs.


### Connexion au serveur Jeedom
Par l'intermédiaire de la carte Ethernet, la carte Arduino communique les résultats des mesures à intervalles réguliers au serveur Jeedom via le protocole MQTT.

![Capture](https://github.com/user-attachments/assets/6fbafa16-a4f8-49a2-a3f4-7f26bd2a7481)
Interface du serveur Jeedom.
Oui, il y a un capteur de luminosité dans la station, oui, il est dans la boîte, dans le noir, moi non plus je sais pas à quoi il sert.


## Inspiration 
Il est important de dire que l'intégralité du code n'est pas de moi. Certaine parties très spécifiques aux composants viennent des programmes d'example fourni par le constructeur.
Par exemple, la grande cascade de "if" ligne 264 provient du programme de démonstration fourni par le constructeur du capteur. Par manque de temps, je n'ai pas souhaité réviser cette partie qui était déjà fonctionnelle.

Malgré tout, la grande majorité du programme et des solutions trouvé sont de mon cru.
