# SensCoulo
 signalk coulomètre ESP32

SensCoulo est un projet de moniteur de batterie 12V pour serveur signalk à partir d'un ESP32. Il fonctionne avec un régulateur solaire MPPT victron avec lequel l'ESP32 communique en liaison série à l'aide [du protocole Ve.direct.](https://www.victronenergy.com/upload/documents/VE.Direct-Protocol-3.32.pdf) 

L'ESP32 renvoie les données issues du MPPT au serveur signalK et initialise la charge compléte de la batterie quand le MPPT est en float.

Le projet a été réalisé avec [platformIO](https://platformio.org), le fichier platformio.ini reprend l'environnement de travail avec les librairies nécessaires.

Le projet est construit à partir du framework [sensESP](https://github.com/SignalK/SensESP) , complété de sa librairie [VEDirect](https://github.com/SensESP/VEDirect) et de la librairie [INA226Lib](https://github.com/peterus/INA226Lib)

## Matériel

- ESP32
- Mini 560 : module régulateur de tension DC-DC 12V-5V pour alimenter l'ESP32 depuis le circuit 12V de la batterie 
- Shunt : pour le projet est utilisé un shunt 75mv 30A [CG FL2C](http://www.cnchog.com/products/10a-50a-ID137.html)
- INA226 : il faudra dessouder la résistance shunt intégrée au module pour utiliser le module avec le shunt extérieur.
- ADUM1201 : isolateur magnétique pour isoler la connexion série entre l'ESP32 et le MPPTvictron. **Attention les GPIO de l'ESP32 sont en 3V et le MPPT en 5V** en plus d'isoler l'ADUM1201 effectue la conversion des signaux 5V-3V
-  DS18B20 : capteur température One Wire 

![testmat](img/testmat.jpg)

![box](img/IMG20230709160223.jpg)

## Configuration

SensCoulo se configure à partir de l'interface utilisateur : https://signalk.org/SensESP/pages/user_interface/

Paramètres à configurer dans UI :

- Temps de décharge donnée constructeur (100, 20, 10, 5)
- Capacité Batterie en Ah
- Coefficient de Peukert (valeur moyenne de 1.24 pour une batterie plomb/AGM)
- Efficience de charge de la batterie en % (valeur moyenne de 90% pour une batterie neuve)



## Objets signalK envoyés :



| Objet signalK                               | Unité | Description                 | Source                                                    |
| ------------------------------------------- | ----- | --------------------------- | --------------------------------------------------------- |
| electrical.battery.1.voltage                | V     | Batterie voltage            | MPPT                                                      |
| electrical.solar.1.current                  | mA    | Chargeur courant            | MPPT                                                      |
| electrical.solar.1.panelCurrent             | mA    | Panneau courant             | MPPT                                                      |
| electrical.solar.1.panelVoltage             | V     | Panneau Voltage             | MPPT                                                      |
| electrical.solar.1.panelPower               | W     | Panneau puissance           | MPPT                                                      |
| electrical.solar.1.yieldToday               | Wh    | Panneau Wh jour             | MPPT                                                      |
| electrical.solar.1.maxPowerToday            | W     | Panneau max puissance       | MPPT                                                      |
| electrical.solar.1.chargingMode             |       | Mode de charge              | MPPT                                                      |
| electrical.battery.1.current                | A     | Batterie courant            | INA                                                       |
| electrical.battery.1.capacity.remaining     | Ah    | Batterie capacitée restante | INA + Peurk                                               |
| electrical.battery.1.capacity.stateOfCharge | %     | Pourcentage capacitée       |                                                           |
| electrical.current                          | A     | Circuit courant             | electrical.solar.1.current - electrical.battery.1.current |
| electrical.battery.1.temperature            | C     | Température batterie        | DS18B20                                                   |



A faire :



- [ ] [/vessels//electrical/batteries//capacity/dischargeLimit](https://signalk.org/specification/1.7.0/doc/vesselsBranch.html#vesselsregexpelectricalbatteriesregexpcapacitydischargelimit)
- [ ] [/vessels//electrical/batteries//capacity/dischargeSinceFull](https://signalk.org/specification/1.7.0/doc/vesselsBranch.html#vesselsregexpelectricalbatteriesregexpcapacitydischargesincefull)
- [ ] [/vessels//electrical/batteries//capacity/timeRemaining](https://signalk.org/specification/1.7.0/doc/vesselsBranch.html#vesselsregexpelectricalbatteriesregexpcapacitytimeremaining)
