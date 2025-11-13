# RetroGames Menu - Klassische Arcade-Spiele

Zentrale Menü-Anwendung mit allen 7 klassischen Arcade-Spielen in einem Programm.

## Enthaltene Spiele

1. **Pong** - 2-Spieler Paddle-Klassiker
2. **Snake** - Sammle Food, werde länger!
3. **Breakout** - Zerstöre alle Steine!
4. **Space Invaders** - Verteidige gegen Aliens!
5. **Asteroids** - Zerstöre Asteroids!
6. **Frogger** - Überquere die Straße!
7. **Tetris** - Puzzle-Klassiker (Portrait-Modus)

## Hardware

- **Display:** CYD 2.8" oder 3.5" (320x240 Landscape, Tetris: 240x320 Portrait)
- **Touch Screen:** Menu-Navigation
- **Buttons:** 4x Digital (tasteA, tasteB, tasteC, tasteD)
- **Potentiometer:** 2x Analog (potiLeft, potiRight) - für Pong/Breakout

**Hinweis:** Alle Pins sind in `CYD_Display_Config.h` definiert.

## Steuerung

### Im Menu
- **Touch Screen:** Spiel auswählen und starten

### Spiel-spezifisch
Je nach Spiel:
- **Pong:** potiLeft & potiRight (Analog-Regler)
- **Snake:** tasteA/B/C/D (Richtungstasten)
- **Breakout:** potiLeft (Schläger-Steuerung)
- **Space Invaders:** tasteB/C (Links/Rechts), tasteA (Schießen)
- **Asteroids:** tasteB/C (Drehen), tasteA (Schießen), tasteD (Schub)
- **Frogger:** tasteA/B/C/D (Richtungstasten)
- **Tetris:** tasteD/A (Links/Rechts), tasteC (Runter), tasteB (Drehen)

## Features

- **7 Spiele in einem:** Komplette Arcade-Sammlung
- **Touch-Menu:** Einfache Navigation
- **Automatischer Mode-Wechsel:** Tetris wechselt zu Portrait-Modus
- **Zurück zum Menu:** Bei Game Over
- **Memory-Effizient:** Spiele werden dynamisch geladen

## Quick Start

```cpp
#include <CYD_Display_Config.h>
// + alle Game-Header

LGFX lcd;
// Game-Instanzen...

void setup() {
  lcd.init();
  lcd.setRotation(1);  // Landscape
  // Pin-Konfiguration...
  drawMenu();
}

void loop() {
  // Touch-Handling
  // Game-Loop für aktives Spiel
}
```

## Code-Struktur

### Hauptfunktionen
```cpp
void drawMenu()              // Menu anzeigen
void checkMenuTouch()        // Touch-Eingabe prüfen
void switchToGame(GameState) // Spiel starten
void returnToMenu()          // Zurück zum Menu
```

### Game States
- `MENU` - Hauptmenü
- `GAME_PONG` - Pong aktiv
- `GAME_SNAKE` - Snake aktiv
- `GAME_BREAKOUT` - Breakout aktiv
- `GAME_SPACEINVADERS` - Space Invaders aktiv
- `GAME_ASTEROIDS` - Asteroids aktiv
- `GAME_FROGGER` - Frogger aktiv
- `GAME_TETRIS` - Tetris aktiv

## Technische Details

### Memory Usage
- **7 Game-Instanzen:** ~15 KB
- **Display-Buffer:** Abhängig von LovyanGFX
- **Gesamt:** ~20-25 KB RAM

### Display-Modi
- **Landscape (320x240):** 6 Spiele
- **Portrait (240x320):** Tetris (automatischer Wechsel)

## Installation

1. **CYD_Display_Config.h** vorhanden?
2. **CYD_Games Library** installiert?
3. **LovyanGFX** Library installiert?
4. **Sketch hochladen**
5. **Spielen!**

## Tipps

### Menu-Navigation
- Tippe auf ein Spiel zum Starten
- Bei Game Over: Automatisch zurück zum Menu
- Menu wird nach jedem Spiel neu gezeichnet

### Hardware-Setup
- **Pong & Breakout:** Benötigen Potentiometer
- **Andere Spiele:** Nur Buttons benötigt
- Alle Spiele funktionieren auch ohne Touch (Button-Steuerung)

## Vergleich: Retro vs Modern

Möchtest du **moderne Grafik** mit Neon-Effekten, Partikeln und Animationen?
Siehe: `01_ModernGames/` - Alle Spiele mit modernem Design!

## Troubleshooting

### Menu reagiert nicht auf Touch
- ✅ Touch-Screen korrekt kalibriert?
- ✅ Seriellen Monitor prüfen (Touch-Koordinaten)
- ✅ Display-Rotation korrekt (1 = Landscape)?

### Spiel startet nicht
- ✅ Genug RAM verfügbar?
- ✅ CYD_Games Library installiert?
- ✅ Alle Header-Dateien vorhanden?

### Tetris zeigt falsch
- Normal! Tetris nutzt Portrait-Modus (240x320)
- Automatischer Wechsel bei Start
- Bei Game Over: Zurück zu Landscape

## Credits

- **Original Games:** Atari, Nintendo, Taito, u.a.
- **Implementation:** CYD_Games Library
- **Platform:** ESP32-2432S028R (CYD)

## Viel Spaß beim Spielen!

```
    ____  ___________________  ____     ________    __  ___________
   / __ \/ ____/_  __/ __ \  / __ \   / ____/ /   / / / / ____/ _ \
  / /_/ / __/   / / / /_/ / / /_/ /  / / __/ /   / / / / __/  / /_/ /
 / _, _/ /___  / / / _, _/ / ____/  / /_/ / /___/ /_/ / /___ / _, _/
/_/ |_/_____/ /_/ /_/ |_| /_/       \____/_____/\____/_____//_/ |_|
```
