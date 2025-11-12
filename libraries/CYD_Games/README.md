# CYD_Games Library

Arduino Library mit gekapselten Arcade-Spielen für CYD (Cheap Yellow Display) ESP32 Board.

## Enthaltene Spiele

- **SnakeGame** - Klassisches Snake-Spiel
- **PongGame** - 2-Spieler Pong mit AI-Modus
- **BreakoutGame** - Breakout/Arkanoid mit Levels

## Installation

### Option 1: Aus Repository (Empfohlen)
Diese Library ist bereits im CYD-Academy Repository enthalten unter `libraries/CYD_Games/`.

### Option 2: Manuelle Installation
1. Kopiere den `CYD_Games` Ordner nach `Arduino/libraries/`
2. Starte Arduino IDE neu
3. Die Library erscheint unter **Sketch → Include Library → CYD_Games**

## Verwendung

### Einzelnes Spiel (Standalone)

```cpp
#include <CYD_Display_Config.h>
#include <SnakeGame.h>

LGFX lcd;
SnakeGame game;

void setup() {
  lcd.init();
  lcd.setRotation(1);
  lcd.setBrightness(255);

  pinMode(tasteA, INPUT_PULLUP);
  pinMode(tasteB, INPUT_PULLUP);
  pinMode(tasteC, INPUT_PULLUP);
  pinMode(tasteD, INPUT_PULLUP);

  game.init(&lcd);
}

void loop() {
  game.update();
}
```

### Alle Spiele in einem Menu

```cpp
#include <CYD_Display_Config.h>
#include <SnakeGame.h>
#include <PongGame.h>
#include <BreakoutGame.h>

LGFX lcd;
SnakeGame snakeGame;
PongGame pongGame;
BreakoutGame breakoutGame;

// ... Menu-Logik ...
// Bei Auswahl: game.init(&lcd)
// In loop(): game.update()
```

Siehe `examples/00_RetroGames/` für vollständiges Menu-Beispiel.

## API

Alle Spiele haben die gleiche API:

### Initialisierung
```cpp
void init(LGFX* display)
```
Initialisiert das Spiel mit dem Display-Objekt.

### Update
```cpp
void update()
```
Muss in `loop()` aufgerufen werden. Aktualisiert Game-Logik und Rendering.

### Status-Funktionen

**SnakeGame:**
- `bool isGameOver()` - Ist das Spiel vorbei?
- `int getScore()` - Aktueller Score

**PongGame:**
- `int getScoreLeft()` - Score linker Spieler
- `int getScoreRight()` - Score rechter Spieler
- `bool isAutoMode()` - Ist AI-Modus aktiv?

**BreakoutGame:**
- `int getScore()` - Aktueller Score
- `int getLives()` - Verbleibende Leben
- `int getLevel()` - Aktuelles Level
- `bool isGameOver()` - Ist das Spiel vorbei?

## Steuerung

### SnakeGame
- **tasteA** (GPIO 17) - Oben
- **tasteB** (GPIO 5) - Links
- **tasteC** (GPIO 16) - Rechts
- **tasteD** (GPIO 18) - Unten

### PongGame
- **potiLeft** (GPIO 34) - Linker Schläger
- **potiRight** (GPIO 35) - Rechter Schläger
- **tasteA** (GPIO 17) - Score Reset
- **tasteB** (GPIO 5) - AI-Modus AUS
- **tasteC** (GPIO 16) - AI-Modus EIN

### BreakoutGame
- **potiLeft** (GPIO 34) - Schläger Links/Rechts
- **tasteA** (GPIO 17) - Ball starten
- **tasteD** (GPIO 18) - Pause

## Beispiele

Siehe `examples/` Verzeichnis:
- `12_Pong/` - Pong standalone
- `13_Snake/` - Snake standalone
- `14_Breakout/` - Breakout standalone
- `00_RetroGames/` - Alle Spiele in einem Menu

## Abhängigkeiten

- **LovyanGFX** - Display-Treiber
- **CYD_Display_Config.h** - Hardware-Konfiguration

## Lizenz

Teil des CYD-Academy Projekts.
