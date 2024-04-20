# Zuglift – Arduino / RocRail Steuerung

## Projektidee
Einen Modell-Eisenbahn-Bahnhof mit 32 Gleisen, senkrecht übereinander angeordnet, in einem Liftsystem.

Der Lift kennt 16 Stockwerke (Ebenen), auf jedem Stockwerk (Ebene) sind zwei Gleise nebeneinander. Via Motor wird der Lift auf und ab bewegt. Die Steuerung des Liftsystems, also welches Stockwerk angefahren werden soll, geschieht über [RocRail](https://wiki.rocrail.net).

## Lösungsansatz
**RocRail** schickt **DCC** Signale an ein **Arduino**. Das DCC Signal enthält die gewünschte Ebene. Das Arduino steuert dann den Motor an, dreht diesen also so lange auf- / abwärts bis die gewünschte Ebene erreicht ist. Verwendet wird ein **Schrittmotor**. Auf dem Arduino ist gespeichert, welche Ebene, wie viele Schritte vom Nullpunkt entfernt ist.

## Implementierung
TBD Optokoppler, Sensoren, DCC Signal, Programmierung via Shield, ...

### Aufbau Liftsystem
TBD Skizze / Schnitt des Liftsystems

### Schema – Verdrahtung
[schema.pdf](spec%2Fschema.pdf)

### Source Code für Arduino
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
