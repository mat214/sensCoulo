# SensCoulo
 signalk coulomètre ESP32

SensCoulo est un projet de moniteur de batterie 12V pour serveur signalk à partir d'un ESP32. Il fonctionne avec un régulateur solaire MPPT victron. Le MPPT est connecté au serveur signalk en série. ESP32 initialise la charge compléte de la batterie quand le MPPT est en float.

Le projet a été réalisé avec [platformIO](https://platformio.org), le fichier platformio.ini reprend l'environnement de travail avec les librairies nécessaires.

Le projet est construit à partir du framework [sensESP](https://github.com/SignalK/SensESP).

## Matériel

- ESP32
- Mini 560 : module régulateur de tension DC-DC 12V-5V pour alimenter l'ESP32 depuis le circuit 12V de la batterie 
- Shunt : pour le projet est utilisé un shunt 75mv 30A [CG FL2C](http://www.cnchog.com/products/10a-50a-ID137.html)
- INA226 : il faudra dessouder la résistance shunt intégrée au module pour utiliser le module avec le shunt extérieur.
-  DS18B20 : capteur température One Wire 



## Configuration

SensCoulo se configure à partir de l'interface utilisateur : https://signalk.org/SensESP/pages/user_interface/

Paramètres à configurer dans UI :

- Temps de décharge donnée constructeur (100, 20, 10, 5)
- Capacité Batterie en Ah
- Coefficient de Peukert (valeur moyenne de 1.24 pour une batterie plomb/AGM)
- Efficience de charge de la batterie en % (valeur moyenne de 90% pour une batterie neuve)



## Objets signalK envoyés :



| Objet signalK                             | Unité | Description                 | Source                                                |
| ----------------------------------------- | ----- | --------------------------- | ----------------------------------------------------- |
| electrical.battery.voltage                | V     | Batterie voltage            | INA                                                   |
| electrical.battery.current                | A     | Batterie courant            | INA                                                   |
| electrical.battery.capacity.remaining     | Ah    | Batterie capacitée restante | INA + Peurk                                           |
| electrical.battery.capacity.stateOfCharge | %     | Pourcentage capacitée       |                                                       |
| electrical.current                        | A     | Circuit courant             | electrical.solar.current - electrical.battery.current |
| electrical.battery.temperature            | C     | Température batterie        | DS18B20                                               |


