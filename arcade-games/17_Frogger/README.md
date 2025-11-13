# Frogger - Retro

Klassischer Straßen-Überquerungs-Klassiker - Bring den Frosch nach Hause!

## Features

- **5 Verkehrsreihen:** Autos in verschiedenen Geschwindigkeiten
- **5 Fluss-Reihen:** Logs und Schildkröten zum Springen
- **5 Ziel-Slots:** Bringe 5 Frösche nach Hause
- **3 Leben:** Game Over bei 0 Leben
- **Zeit-Limit:** 60 Sekunden pro Frosch
- **Progressive Schwierigkeit:** Schnellerer Verkehr

## Hardware

- **Display:** CYD 2.8" oder 3.5" (320x240 Landscape)
- **Buttons:** 4x Digital (tasteA, tasteB, tasteC, tasteD)

**Hinweis:** Alle Pins sind in `CYD_Display_Config.h` definiert.

## Steuerung

- **tasteA:** Oben (vorwärts)
- **tasteB:** Links
- **tasteC:** Rechts
- **tasteD:** Unten (rückwärts)

Grid-basierte Bewegung (Frosch springt von Feld zu Feld)

## Spielregeln

### Punktevergabe
- **Vorwärts bewegen:** +10 Punkte
- **Ziel erreichen:** +50 Punkte
- **Zeit-Bonus:** Restzeit × 10
- **Flieg fangen:** +200 Punkte (gelegentlich in Zielen)

### Gefahren
- **Von Auto getroffen:** -1 Leben
- **Ins Wasser fallen:** -1 Leben
- **Zeit abgelaufen:** -1 Leben
- **Bildschirm-Kanten:** Tod

### Level-Ziel
- Bringe 5 Frösche zu den Zielen oben
- Alle 5 Ziele gefüllt = Level komplett
- Nächstes Level: Schnellerer Verkehr

## Quick Start

```cpp
#include <CYD_Display_Config.h>
#include <FroggerGame.h>

LGFX lcd;
FroggerGame game;

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
- `moveFrog()` - Frosch-Bewegung
- `updateTraffic()` - Autos bewegen
- `updateRiver()` - Logs/Turtles bewegen
- `checkCollisions()` - Treffer-Erkennung

### Game Objects
- **Frog:** Position, Grid-basiert
- **Cars:** 5 Reihen, verschiedene Geschwindigkeiten
- **Logs:** Sichere Plattformen im Fluss
- **Turtles:** Tauchende Plattformen
- **Goals:** 5 Ziel-Slots oben

## Spieltipps

### Für Anfänger
- Beobachte Verkehrsmuster
- Warte auf Lücken
- Spring auf Log-Mitte
- Achte auf Timer
- Nutze Rückwärtsbewegung

### Für Profis
- Schnelle Vorwärts-Sprünge (+10 Punkte!)
- Time-Bonus maximieren
- Fliegen in Zielen fangen (+200)
- Nutze Turtle-Timing
- Diagonal-Strategie (vorwärts + seitlich)

## Spielfeld-Layout

```
┌────────────────────────────────┐
│ [GOAL][GOAL][GOAL][GOAL][GOAL] │  ← Ziele
│ ≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈ │  ← Fluss
│ ████LOG████  ████LOG████       │  ← Logs
│  🐢🐢🐢🐢    🐢🐢🐢🐢           │  ← Turtles
│ ████LOG████  ████LOG████       │
│  🐢🐢🐢🐢    🐢🐢🐢🐢           │
│ ████LOG████  ████LOG████       │
├────────────────────────────────┤  ← Sicherer Streifen
│ 🚗     🚗     🚗               │  ← Autos
│     🚙     🚙     🚙           │  ← Trucks
│ 🚗     🚗     🚗               │
│     🚙     🚙     🚙           │
│ 🚗     🚗     🚗               │
└────────────────────────────────┘
  🐸 ← Start
```

## Installation

1. **CYD_Display_Config.h** vorhanden?
2. **CYD_Games Library** installiert?
3. **LovyanGFX** installiert?
4. **Sketch hochladen**
5. **Frosch nach Hause bringen!**

## Troubleshooting

### Frosch reagiert nicht
- ✅ Buttons richtig angeschlossen?
- ✅ Grid-basierte Bewegung (1 Schritt pro Taste)
- ✅ Pin-Definitionen korrekt?

### Zeit zu kurz/lang
- Ändere `TIME_LIMIT` im Code
- Standard: 60 Sekunden
- Reduziere für mehr Challenge

### Verkehr zu schnell
- Ändere Speed-Werte für Cars/Logs
- Normal: Wird schneller pro Level

## Vergleich: Retro vs Modern

Möchtest du **animiertes Wasser** und **smooth Movement**?
Siehe: `17b_Frogger_Modern/` - Mit schönen Animationen!

## Credits

Basiert auf **Frogger** (1981, Konami)

## Viel Erfolg beim Überqueren!
