# Tetris - Retro

Der ultimative Puzzle-Klassiker - Fülle Zeilen und erreiche High-Scores!

**Besonderheit:** Nutzt **Portrait-Modus** (240x320) für authentisches Tetris-Gefühl!

## Features

- **7 Tetromino-Typen:** I, O, T, S, Z, L, J
- **Next-Piece Preview:** Sehe nächstes Teil
- **Line-Clear System:** 1-4 Zeilen gleichzeitig
- **Score-System:** Mehr Zeilen = mehr Punkte
- **Level-System:** Höheres Level = schnellere Fallgeschwindigkeit
- **Retro-Design:** Klassische Farben und Sounds

## Hardware

- **Display:** CYD 2.8" oder 3.5" (240x320 Portrait!)
- **Buttons:** 4x Digital (tasteA, tasteB, tasteC, tasteD)

**Hinweis:** Alle Pins sind in `CYD_Display_Config.h` definiert. Display wechselt automatisch zu Portrait-Modus!

## Steuerung

- **tasteD:** Links bewegen
- **tasteA:** Rechts bewegen
- **tasteC:** Schnell-Fall (Hard Drop)
- **tasteB:** Drehen (Rotation im Uhrzeigersinn)

## Spielregeln

### Punktevergabe
- **1 Zeile:** 100 × Level
- **2 Zeilen:** 300 × Level
- **3 Zeilen:** 500 × Level
- **4 Zeilen (Tetris!):** 800 × Level

### Level-System
- Start: Level 1
- Jede 10 Zeilen: +1 Level
- Höheres Level: Schnellerer Fall
- Max Level: 15

### Game Over
- Neues Teil passt nicht mehr
- Stack erreicht Oberseite

## Quick Start

```cpp
#include <CYD_Display_Config.h>
#include <TetrisGame.h>

LGFX lcd;
TetrisGame game;

void setup() {
  lcd.init();
  lcd.setRotation(0);  // Portrait!
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
- `movePiece()` - Teil bewegen
- `rotatePiece()` - Teil drehen
- `dropPiece()` - Schnell-Fall
- `checkLines()` - Volle Zeilen prüfen
- `clearLine()` - Zeile löschen

### Game Objects
- **Current Piece:** Aktives Tetromino
- **Next Piece:** Preview
- **Board:** 10×20 Grid
- **Score:** Punkte, Zeilen, Level

## Tetromino-Typen

```
I-Piece (Cyan):     O-Piece (Gelb):
████████              ████
                      ████

T-Piece (Lila):     S-Piece (Grün):
  ██                  ████
██████              ████

Z-Piece (Rot):      L-Piece (Orange):
████                ██
  ████              ██████

J-Piece (Blau):
    ██
██████
```

## Spieltipps

### Für Anfänger
- Baue flach (keine Löcher)
- Nutze I-Piece für Tetris
- Schaue Next-Piece Preview
- Langsame Drehungen

### Für Profis
- T-Spin Tricks
- 4-Zeilen-Combos (Tetris!)
- Baue Brunnen für I-Piece
- Schnelle Drops (tasteC)
- Wall-Kicks nutzen

## Scoring-Strategie

### Maximiere Punkte
- **Tetris (4 Zeilen):** 800 × Level (beste Punktzahl!)
- **Level erhöhen:** Mehr Multiplikator
- **Kombos:** Mehrere Zeilen hintereinander
- **Keine Game Over:** Spielzeit maximieren

### Level-Management
- Frühe Levels: Üben und aufbauen
- Mittlere Levels: Tetris-Combos
- Hohe Levels: Survival-Modus

## Board-Layout

```
┌──────────────┐
│ Next:        │  ← Preview
│   ████       │
│   ████       │
│              │
│ Score: 1200  │
│ Lines: 12    │
│ Level: 2     │
├──────────────┤
│              │  ← Spielfeld
│              │     (10×20)
│      ██      │
│    ████      │
│  ██████      │
│  ████████    │
│████████████  │
└──────────────┘
```

## Installation

1. **CYD_Display_Config.h** vorhanden?
2. **CYD_Games Library** installiert?
3. **LovyanGFX** installiert?
4. **Display auf Portrait:** Rotation = 0
5. **Sketch hochladen**
6. **Tetris spielen!**

## Troubleshooting

### Display zeigt falsch
- ✅ Rotation = 0 (Portrait)?
- ✅ Display-Größe 240×320?
- ✅ LovyanGFX korrekt konfiguriert?

### Teil dreht sich nicht
- ✅ tasteB richtig angeschlossen?
- ✅ Rotation erlaubt? (Nicht an Wand)
- Wall-Kick sollte funktionieren

### Zu schnell/langsam
- Ändere `fallDelay` per Level
- Level 1: 800ms, Level 15: 50ms
- Anpassbar im Code

## Vergleich: Retro vs Modern

Möchtest du **Ghost-Piece**, **Rainbow-Effekte** und **Partikel**?
Siehe: `18b_Tetris_Modern/` - Mit spektakulären Effekten!

## Credits

Basiert auf **Tetris** (1984, Alexey Pajitnov)

## Viel Spaß beim Stapeln!
