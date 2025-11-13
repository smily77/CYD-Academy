# Space Invaders - Retro

Klassischer Alien-Shooter - Verteidige die Erde gegen die Invasion!

## Features

- **5 Reihen Aliens:** Unterschiedliche Typen und Punktwerte
- **3 Schilde:** Schützen vor Alien-Schüssen (werden zerstört)
- **UFO Bonus:** Erscheint gelegentlich für Extra-Punkte
- **3 Leben:** Game Over bei 0 Leben
- **Progressive Schwierigkeit:** Aliens werden schneller
- **Retro-Design:** Klassisches Schwarz/Weiß Look

## Hardware

- **Display:** CYD 2.8" oder 3.5" (320x240 Landscape)
- **Buttons:** 4x Digital (tasteA, tasteB, tasteC, tasteD)

**Hinweis:** Alle Pins sind in `CYD_Display_Config.h` definiert.

## Steuerung

- **tasteB:** Links bewegen
- **tasteC:** Rechts bewegen
- **tasteA:** Schießen
- **tasteD:** Pause

## Spielregeln

### Punktevergabe
- **Obere Reihe (Klein):** 30 Punkte
- **Mittlere Reihen (Mittel):** 20 Punkte
- **Untere Reihen (Groß):** 10 Punkte
- **UFO:** 50-100 Punkte

### Gameplay
- Aliens bewegen sich horizontal und rücken näher
- Aliens schießen zufällig zurück
- Schilde schützen und werden nach Hits zerstört
- Level komplett wenn alle Aliens zerstört
- Game Over wenn Aliens unten ankommen

## Quick Start

```cpp
#include <CYD_Display_Config.h>
#include <SpaceInvadersGame.h>

LGFX lcd;
SpaceInvadersGame game;

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
- `update()` - Game-Loop (Eingabe, Physik, Render)
- `movePlayer()` - Spieler-Bewegung
- `playerShoot()` - Schuss abfeuern
- `updateAliens()` - Alien-Bewegung und Schüsse
- `checkCollisions()` - Treffer-Erkennung

### Game Objects
- **Player:** Position, Leben
- **Aliens:** 5x11 Grid, verschiedene Typen
- **Bullets:** Spieler- und Alien-Schüsse
- **Shields:** 3 Schutzschilde
- **UFO:** Bonus-Ziel

## Spieltipps

### Für Anfänger
- Bleib in Bewegung
- Nutze Schilde als Deckung
- Schieße auf untere Reihen zuerst
- UFO ignorieren am Anfang

### Für Profis
- Schieße von Seiten (mehr Treffer)
- Zerstöre Schilde für freies Schussfeld
- Timing für UFO (50-100 Punkte)
- Letzte Aliens sind am schnellsten!

## Installation

1. **CYD_Display_Config.h** vorhanden?
2. **CYD_Games Library** installiert?
3. **LovyanGFX** installiert?
4. **Sketch hochladen**
5. **Die Erde verteidigen!**

## Troubleshooting

### Aliens bewegen sich zu schnell/langsam
- Ändere `alienMoveDelay` im Code
- Normal: Aliens werden schneller wenn weniger übrig

### Spieler reagiert nicht
- ✅ Buttons richtig angeschlossen?
- ✅ Pin-Definitionen in CYD_Display_Config.h korrekt?

## Vergleich: Retro vs Modern

Möchtest du **Glow-Effekte** und **Partikel**?
Siehe: `15b_SpaceInvaders_Modern/` - Mit Neon-Design!

## Credits

Basiert auf **Space Invaders** (1978, Taito)

## Viel Spaß beim Verteidigen!
