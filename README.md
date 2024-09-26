# Station_meteo_Arduino
Programme Arduinno de gestion d'une station météo connecté.

J'ai réalisé ce projet en un an et demi, en travaillant par intermittence. Je ne pensais pas le publier à l'origine, mais je me dis que cela pourra servir à d'autres !
Dans ce programme, tous les commentaires sont en français, rédigé à l'origine pour moi seul. Si le besoin se fait sentir, je pourrais envisager une traduction en anglais.

J'aimerais également préciser que c'est la première fois que je publie un de mes "bricolage", donc si vous avez des commentaires, des modifications ou des corrections, je suis tout ouïe.

Point de vue électronique, j'ai utilisé :
- Carte Arduino UNO
- Shield Ethernet compatible PoE DFR0850 (gère la connection et l'alimentation en PoE)
- Shield météo DEV-13956 (interface les capteurs météo sur la carte Arduino)
- Un Jeu de capteurs météo (https://www.gotronic.fr/art-jeu-de-capteurs-meteo-33052.htm)
- Capteur de T° et d'humidité AM2301B

## Utilité du capteur AM2301B
Après test, il s'est avéré que les capteurs de température et d'humidité intégrés au shield météo donnaient une valeur erronée, souvent au-dessus de la valeur réelle. Je pense que la configuration "sandwich" de la carte Arduino et des deux shields nuit à la bonne prise de température et d'hygrométrie. J'ai donc installé un nouveau capteur déporté qui sera installé à l'extérieur du boîtier.
