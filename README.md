# Zuglift – Arduino / RocRail Steuerung

## Projektidee
Einen Modell-Eisenbahn-Bahnhof mit 32 Gleisen, in Etagen übereinander angeordnet, in einem Liftsystem.

Jede Etage hat zwei Gleise nebeneinander, ergibt 16 Etagen (Ebenen). Die Gleise sind auf einer waagrechten Platte montiert, welche an einer senkrechten Wand angebracht ist. Ein Motor bewegt die Wand auf und ab (Liftsystem). [RocRail](https://wiki.rocrail.net) steuert via [Arduino](https://www.arduino.cc) die Anlage.

## Lösungsansatz
**RocRail** schickt **DCC** Signale an ein **Arduino**. Das DCC Signal enthält die gewünschte Ebene. Das Arduino steuert dann den Motor an, dreht diesen also so lange auf- / abwärts bis die gewünschte Ebene erreicht ist. Verwendet wird ein **Schrittmotor**. Auf dem Arduino ist gespeichert, welche Ebene wie viele Schritte vom Nullpunkt (Ebene 0) entfernt ist.

### Variante 
Bei einer früheren Variante versuchten wir die Ebenen anhand eines elektrischen Widerstands zu erkennen. Sobald eine Ebene kontakt hatte mit dem Tisch (Sensor / Lichtschranke), konnte via Arduino ein bestimmter elektrischer Widerstand gemessen werden. Die Schalter waren via Widerstandsleiter mit einem Arduino-Analog Pin verbunden. \
Die Lösung hatte folgende Nachteile:

* Der gemessene Widerstand schwankte bei uns zu stark. Teilweise waren die Werte überlappend mit den Soll-Werten benachbarter Ebenen.
* Die Elektronik war fehleranfällig. Viele Kabel und Verbindungen.
* Installation und Wartung Ebenen-Schalter (zuerst Rollentaster, später Lichtschranken) war aufwändig.

Durch die Verwendung eines Schrittmotors sind alle obigen Probleme obsolet. Die minimale Drehbewegung eines Schrittmotors entspricht 0.001 mm vertikale Lift-Bewegung.

## Implementierung
TBD Optokoppler, Sensoren, DCC Signal, Programmierung via Shield, Motorsteuerung

### Aufbau Liftsystem

![aufbau.svg](doc/aufbau.svg)

### Schema – Verdrahtung
[schema.pdf](spec/schema.pdf)

### Source Code für Arduino Programm
[arduino-zuglift.ino](arduino-zuglift%2Farduino-zuglift.ino)

### Zustandsdiagram von Arduino Programm
![zustandsdiagram.svg](spec%2Fzustandsdiagram.svg)


### Komponenten

| Bezeichnung                 | Beschreibung                                                                |
|-----------------------------|-----------------------------------------------------------------------------|
| TB6600 Stepper Motor Driver | https://www.dfrobot.com/product-1547.html                                   |
| Arduino UNO Rev3            | https://store.arduino.cc/products/arduino-uno-rev3/                         |
| LCD1602 with Keypad Shield  | https://protosupplies.com/product/lcd1602-16x2-blue-lcd-shield-with-keypad/ |
| Motor?                      | Nema17?                                                                     |
