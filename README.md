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

## Inspiration 
Il est important de dire que l'intégralité du code n'est pas de moi. Certaine parties très spécifiques aux composants viennent des programmes d'example fourni par le constructeur.
Par exemple, la grande cascade de "if" ligne 264 provient du programme de démonstration fourni par le constructeur du capteur. Par manque de temps, je n'ai pas souhaité réviser cette partie qui était déjà fonctionnelle.

Malgré tout, la grande majorité du programme et des solutions trouvé sont de mon cru.
