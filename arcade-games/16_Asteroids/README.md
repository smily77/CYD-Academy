# Asteroids - Retro

Klassischer Weltraum-Shooter - Zerstöre alle Asteroids!

## Features

- **Vektorbasiertes Raumschiff:** Rotierbar in 360°
- **Asteroids in 3 Größen:** Groß, Mittel, Klein
- **Schub-Mechanik:** Realistische Physik
- **UFO-Gegner:** Schießt auf dich
- **Wrap-Around:** Bildschirm-Kanten verbunden
- **3 Leben:** Game Over bei 0 Leben
- **Progressive Schwierigkeit:** Mehr Asteroids pro Level

## Hardware

- **Display:** CYD 2.8" oder 3.5" (320x240 Landscape)
- **Buttons:** 4x Digital (tasteA, tasteB, tasteC, tasteD)
- **Optional Potentiometer:** potiLeft für Rotation (statt Buttons)

**Hinweis:** Alle Pins sind in `CYD_Display_Config.h` definiert.

## Steuerung

### Mit Buttons (Standard)
- **tasteB:** Links drehen
- **tasteC:** Rechts drehen
- **tasteA:** Schießen
- **tasteD:** Schub (Thrust)

### Mit Potentiometer (Optional)
- **potiLeft:** Rotation 0-360°
- **tasteA:** Schießen
- **tasteD:** Schub

## Spielregeln

### Punktevergabe
- **Großer Asteroid:** 20 Punkte (teilt in 2 Mittlere)
- **Mittlerer Asteroid:** 50 Punkte (teilt in 2 Kleine)
- **Kleiner Asteroid:** 100 Punkte (wird zerstört)
- **UFO:** 200 Punkte

### Gameplay
- Raumschiff schwebt im Weltraum
- Schub gibt Geschwindigkeit in Blickrichtung
- Trägheit (Raumschiff gleitet weiter)
- Asteroids zerbrechen in kleinere
- UFO erscheint bei höherem Score
- Level komplett wenn alle Asteroids zerstört

## Quick Start

```cpp
#include <CYD_Display_Config.h>
#include <AsteroidsGame.h>

LGFX lcd;
AsteroidsGame game;

void setup() {
  lcd.init();
  lcd.setRotation(1);  // Landscape
  game.init(&lcd);
}

void loop() {
  game.update();
}
```

## Code-Struktur

### Hauptfunktionen
- `init()` - Spiel initialisieren
- `update()` - Game-Loop
- `updateShip()` - Raumschiff-Physik
- `updateAsteroids()` - Asteroid-Bewegung
- `checkCollisions()` - Treffer-Erkennung
- `spawnAsteroids()` - Neue Asteroids generieren

### Game Objects
- **Ship:** Position, Rotation, Geschwindigkeit
- **Asteroids:** Position, Größe, Geschwindigkeit
- **Bullets:** Schüsse des Spielers
- **UFO:** Gegner (erscheint ab Score > 1000)

## Spieltipps

### Für Anfänger
- Nutze Schub sparsam (Trägheit!)
- Schieße viel
- Halte Abstand zu großen Asteroids
- Bleib in der Bildschirm-Mitte

### Für Profis
- Nutze Wrap-Around strategisch
- Zerstöre große Asteroids schnell
- UFO ist gefährlich aber wertvoll
- Schieße in Bewegungsrichtung des Asteroids

## Physics-System

### Raumschiff
- **Rotation:** 5° pro Frame
- **Schub:** 0.3 px/frame² Beschleunigung
- **Max Speed:** 8 px/frame
- **Reibung:** Keine (Weltraum!)

### Asteroids
- **Wrap-Around:** Verlassen Bildschirm = kommen gegenüber wieder
- **Konstante Geschwindigkeit:** Keine Reibung
- **Split-Mechanik:** Große → 2 Mittlere, Mittlere → 2 Kleine

## Installation

1. **CYD_Display_Config.h** vorhanden?
2. **CYD_Games Library** installiert?
3. **LovyanGFX** installiert?
4. **Sketch hochladen**
5. **Asteroids zerstören!**

## Troubleshooting

### Raumschiff dreht nicht
- ✅ Buttons richtig angeschlossen?
- ✅ Pin-Definitionen korrekt?
- ✅ Potentiometer angeschlossen? (falls genutzt)

### Asteroids zu schnell/langsam
- Ändere Asteroid-Geschwindigkeit im Code
- Normal: Wird schneller mit mehr Asteroids

## Vergleich: Retro vs Modern

Möchtest du **Partikel-Explosionen** und **Bullet-Trails**?
Siehe: `16b_Asteroids_Modern/` - Mit spektakulären Effekten!

## Credits

Basiert auf **Asteroids** (1979, Atari)

## Viel Erfolg im Weltraum!
