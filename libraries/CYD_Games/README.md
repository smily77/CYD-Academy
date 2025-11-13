# CYD_Games Library

Arduino Library mit gekapselten Arcade-Spielen für CYD (Cheap Yellow Display) ESP32 Board.

## Enthaltene Spiele

### Retro-Versionen (Klassisches Design)
- **PongGame** - 2-Spieler Pong mit AI-Modus
- **SnakeGame** - Klassisches Snake-Spiel
- **BreakoutGame** - Breakout/Arkanoid mit Levels
- **SpaceInvadersGame** - Verteidige gegen Alien-Wellen
- **AsteroidsGame** - Weltraum-Shooter
- **FroggerGame** - Überquere Straße und Fluss
- **TetrisGame** - Puzzle-Klassiker (Portrait-Modus)

### Modern-Versionen (Mit Effekten)
- **PongGameModern** - Mit Neon, Glow & Partikeln
- **SnakeGameModern** - Mit Gradient & Trail
- **BreakoutGameModern** - Mit 3D-Blöcken & Explosionen
- **SpaceInvadersGameModern** - Mit Glow-Effekten & animierten Schilden
- **AsteroidsGameModern** - Mit Partikel-Explosionen & Bullet-Trails
- **FroggerGameModern** - Mit animiertem Wasser & Smooth-Animationen
- **TetrisGameModern** - Mit Ghost-Piece & Rainbow-Effekten

### Support-Library
- **CYD_Input** - Hardware-Abstraktion für Buttons & Potentiometer

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
#include <CYD_Input.h>
#include <SnakeGame.h>

LGFX lcd;
SnakeGame game;

void setup() {
  lcd.init();
  lcd.setRotation(1);
  lcd.setBrightness(255);

  // Buttons initialisieren (GPIO oder I2C, je nach Config)
  CYD_Input::init();

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

## Hardware-Konfiguration

Die Library nutzt `CYD_Input.h` für flexible Hardware-Unterstützung:

**Option 1: GPIO-Buttons (direkt am ESP32)**
```cpp
// In CYD_Display_Config.h:
#define gpioSwitch
#define tasteA 17
#define tasteB 5
#define tasteC 16
#define tasteD 18
```

**Option 2: I2C-Buttons (über PCF8574 Portextender)**
```cpp
// In CYD_Display_Config.h:
#define i2cSwitch
#define tasteA 0        // PCF8574 Pin 0
#define tasteB 1        // PCF8574 Pin 1
#define tasteC 2        // PCF8574 Pin 2
#define tasteD 3        // PCF8574 Pin 3
#define pcfAddress 0x20
#define switchInterrupt 22  // Optional
```

**Potentiometer (optional für Pong & Breakout)**
```cpp
#define gpioPoti
#define potiLeft 34
#define potiRight 35
```

**Wichtig:**
- Entweder `gpioSwitch` ODER `i2cSwitch` muss definiert sein
- Pong und Breakout benötigen `gpioPoti`
- Bei I2C: PCF8574 Library installieren

## Steuerung

### SnakeGame
- **tasteA** (CYD_BTN_A) - Oben
- **tasteB** (CYD_BTN_B) - Links
- **tasteC** (CYD_BTN_C) - Rechts
- **tasteD** (CYD_BTN_D) - Unten

### PongGame (benötigt Potentiometer)
- **potiLeft** - Linker Schläger
- **potiRight** - Rechter Schläger
- **tasteA** (CYD_BTN_A) - Score Reset
- **tasteB** (CYD_BTN_B) - AI-Modus AUS
- **tasteC** (CYD_BTN_C) - AI-Modus EIN

### BreakoutGame (benötigt Potentiometer)
- **potiLeft** - Schläger Links/Rechts
- **tasteA** (CYD_BTN_A) - Ball starten
- **tasteD** (CYD_BTN_D) - Pause

### SpaceInvadersGame
- **tasteB** (CYD_BTN_B) - Links
- **tasteC** (CYD_BTN_C) - Rechts
- **tasteA** (CYD_BTN_A) - Schießen
- **tasteD** (CYD_BTN_D) - Pause

### AsteroidsGame
- **tasteB** (CYD_BTN_B) - Links drehen
- **tasteC** (CYD_BTN_C) - Rechts drehen
- **tasteA** (CYD_BTN_A) - Schießen
- **tasteD** (CYD_BTN_D) - Schub

### FroggerGame
- **tasteA** (CYD_BTN_A) - Oben
- **tasteB** (CYD_BTN_B) - Links
- **tasteC** (CYD_BTN_C) - Rechts
- **tasteD** (CYD_BTN_D) - Unten

### TetrisGame (Portrait-Modus!)
- **tasteD** (CYD_BTN_D) - Links
- **tasteA** (CYD_BTN_A) - Rechts
- **tasteC** (CYD_BTN_C) - Schneller fallen
- **tasteB** (CYD_BTN_B) - Drehen

## Beispiele

Siehe `arcade-games/` Verzeichnis:

**Retro-Versionen:**
- `12_Pong/` - Pong standalone
- `13_Snake/` - Snake standalone
- `14_Breakout/` - Breakout standalone
- `15_SpaceInvaders/` - Space Invaders standalone
- `16_Asteroids/` - Asteroids standalone
- `17_Frogger/` - Frogger standalone
- `18_Tetris/` - Tetris standalone

**Modern-Versionen:**
- `12b_Pong_Modern/` - Mit Effekten
- `13b_Snake_Modern/` - Mit Effekten
- `14b_Breakout_Modern/` - Mit Effekten
- `15b_SpaceInvaders_Modern/` - Mit Effekten
- `16b_Asteroids_Modern/` - Mit Effekten
- `17b_Frogger_Modern/` - Mit Effekten
- `18b_Tetris_Modern/` - Mit Effekten

**Menüs:**
- `00_RetroGames/` - Alle Retro-Spiele in einem Menu
- `01_ModernGames/` - Alle Modern-Spiele in einem Menu

## Abhängigkeiten

### Erforderlich:
- **LovyanGFX** - Display-Treiber (über Library Manager)
- **CYD_Display_Config.h** - Hardware-Konfiguration

### Optional (je nach Konfiguration):
- **PCF8574** - I2C Port-Extender Library (nur bei `i2cSwitch`)

## Lizenz

Teil des CYD-Academy Projekts.
