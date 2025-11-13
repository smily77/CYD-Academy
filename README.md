# CYD Academy - Arduino Beispiele & Arcade Games

Dieses Repository enthält Arduino IDE Beispiele und eine vollständige Arcade-Spielesammlung für das **CYD (Cheap Yellow Display)** mit der **LovyanGFX** Library.

## 📦 Inhalt

### 🎮 [Arcade Games](arcade-games/)
Vollständige Sammlung klassischer Arcade-Spiele in Retro- und Modern-Versionen:
- **14 Spiele:** Pong, Snake, Breakout, Space Invaders, Asteroids, Frogger, Tetris (je 2 Versionen)
- **2 Touch-Menüs:** Retro Games & Modern Games für einfache Spielauswahl
- **CYD_Games Library:** Alle Spiele als wiederverwendbare Klassen gekapselt
- Mehr Details: [arcade-games/README.md](arcade-games/README.md)

### 📚 [Examples](examples/)
11 grundlegende Beispiele zum Lernen der Display-Programmierung:
- Hello World, Touch Demo, Grafik, Text-Effekte
- WiFi-Uhren (Digital & Analog)
- Taschenrechner, Touch-Buttons
- Live-Daten-Visualisierung

## 🖥️ Hardware

- **CYD 2.8" ESP32 Display** (USB-C) - 320x240px
- **CYD 3.5" ESP32 Display** (USB-C) - 480x320px
- ESP32 basiertes Board mit TFT Display und Touch-Screen
- **Optionale Hardware:**
  - RGB LED (für visuelle Effekte)
  - Photoresistor (für Helligkeitssteuerung)
  - 2x Potentiometer (für Spiele-Steuerung)
  - 4x Digitale Buttons (für Spiele-Steuerung)

## 🚀 Quick Start

### Installation

1. **Arduino IDE** installieren
2. **ESP32 Board-Unterstützung** über Board Manager installieren
3. **LovyanGFX Library** über Library Manager installieren
4. Repository klonen oder als ZIP herunterladen
5. **CYD_Games Library** in Arduino/libraries/ Ordner kopieren
6. Beispiel oder Spiel öffnen und hochladen

### Display-Konfiguration

Die Beispiele verwenden `CYD_Display_Config.h` aus der MyLGFXConfigs Library für die Display-Konfiguration:
- LGFX Display-Setup für CYD Boards
- Pin-Definitionen für RGB LED, Photoresistor, Buttons, Potentiometer
- Automatische Display-Erkennung (2.8" / 3.5")
- Flexible Hardware-Konfiguration (GPIO oder I2C Buttons)

**Installation MyLGFXConfigs:**
```bash
# In Arduino/libraries/ Verzeichnis:
git clone https://github.com/dein-repo/MyLGFXConfigs.git
```

### Hardware-Konfiguration

Die CYD_Games Library unterstützt verschiedene Hardware-Konfigurationen über `CYD_Display_Config.h`:

**Option 1: GPIO-Buttons (direkt am ESP32)**
```cpp
#define gpioSwitch
#define tasteA 17
#define tasteB 5
#define tasteC 16
#define tasteD 18
```

**Option 2: I2C-Buttons (über PCF8574 Portextender)**
```cpp
#define i2cSwitch
#define tasteA 0        // PCF8574 Pin 0
#define tasteB 1        // PCF8574 Pin 1
#define tasteC 2        // PCF8574 Pin 2
#define tasteD 3        // PCF8574 Pin 3
#define pcfAddress 0x20 // I2C-Adresse des PCF8574
#define switchInterrupt 22  // Optional: Interrupt Pin
```

**Potentiometer (optional für Pong & Breakout)**
```cpp
#define gpioPoti
#define potiLeft 34
#define potiRight 35
```

**Wichtig:**
- Entweder `gpioSwitch` ODER `i2cSwitch` muss definiert sein (nicht beide!)
- Spiele die Potentiometer benötigen (Pong, Breakout) erscheinen nur im Menu wenn `gpioPoti` definiert ist
- Bei I2C-Buttons muss die PCF8574 Library installiert sein

### Board-Einstellungen

**Arduino IDE:**
- Board: "ESP32 Dev Module"
- Upload Speed: 921600
- Flash Frequency: 80MHz
- Partition Scheme: "Default 4MB with spiffs"

## 📖 Dokumentation

### Beispiele (examples/)

Jedes Beispiel demonstriert spezifische Features:

| # | Name | Beschreibung |
|---|------|--------------|
| 01 | HelloWorld | Einfache Textausgabe |
| 02 | BasicShapes | Grundformen zeichnen |
| 03 | TouchDemo | Touch-Input verarbeiten |
| 04 | Colors | Farben und Farbverläufe |
| 05 | ScrollText | Text-Scroll-Effekte |
| 06 | BouncingBall | Physik-Simulation mit Touch |
| 07 | WiFiClock | Digitale Uhr mit NTP |
| 08 | AnalogClock | Analoge Uhr mit NTP |
| 09 | Calculator | Touch-Taschenrechner |
| 10 | TouchButtons | Button-Verhaltensweisen |
| 11 | ScrollingChart | Live-Daten-Visualisierung |

Alle Beispiele sind **vollständig skalierbar** und funktionieren auf 2.8" und 3.5" Displays.

### Arcade Games (arcade-games/)

**Retro-Versionen (12-18):**
- Klassisches Retro-Design mit einfachen Farben
- Optimiert für Performance
- Original-Arcade-Feeling

**Modern-Versionen (12b-18b):**
- Gradient-Hintergründe
- Glow-Effekte & Partikel-Systeme
- 3D-Rendering & Animationen
- Ghost-Piece (Tetris), Bullet-Trails (Asteroids), etc.

**Spiele-Menüs:**
- `00_RetroGames` - Touch-Menü für alle Retro-Spiele
- `01_ModernGames` - Touch-Menü für alle Modern-Spiele
- Automatischer Portrait/Landscape-Wechsel für Tetris

Siehe [arcade-games/README.md](arcade-games/README.md) für Details zu jedem Spiel.

## 🎯 Features

### Automatische Display-Anpassung
- ✓ **Alle Beispiele und Spiele** passen sich automatisch an
- Funktionieren auf 2.8" (320x240) und 3.5" (480x320) Displays
- Dynamische Koordinaten- und Größenberechnung

### CYD_Games Library
Alle Arcade-Spiele sind als wiederverwendbare C++-Klassen gekapselt:
- **Header-Only:** Vollständige Inline-Implementierung
- **Einfache API:** `init()`, `update()`, `isGameOver()`, `getScore()`
- **Keine Abhängigkeiten:** Nur Arduino.h und LovyanGFX.hpp
- **Namespace-Clean:** Alle Defines und Structs mit Prefix

**Verwendung:**
```cpp
#include <CYD_Display_Config.h>
#include <SnakeGame.h>

LGFX lcd;
SnakeGame game;

void setup() {
  lcd.init();
  lcd.setRotation(1);
  game.init(&lcd);
}

void loop() {
  game.update();
}
```

### Optional: Hardware-Features
- **RGB LED Feedback:** Farbige Rückmeldung bei Aktionen
- **Photoresistor:** Automatische Helligkeitsanpassung & Live-Daten
- **Buttons & Potentiometer:** Spiele-Steuerung

Deaktiviere Features durch Auskommentieren der entsprechenden `#define` Zeilen in `CYD_Display_Config.h`.

## 📂 Struktur

```
CYD-Academy/
├── README.md                      # Diese Datei
├── arcade-games/                  # Alle Arcade-Spiele
│   ├── README.md                  # Spiele-Dokumentation
│   ├── 00_RetroGames/            # Retro-Spiele-Menü
│   ├── 01_ModernGames/           # Modern-Spiele-Menü
│   ├── 12_Pong/                  # Retro Pong
│   ├── 12b_Pong_Modern/          # Modern Pong
│   ├── 13_Snake/                 # Retro Snake
│   ├── 13b_Snake_Modern/         # Modern Snake
│   ├── 14_Breakout/              # Retro Breakout
│   ├── 14b_Breakout_Modern/      # Modern Breakout
│   ├── 15_SpaceInvaders/         # Retro Space Invaders
│   ├── 15b_SpaceInvaders_Modern/ # Modern Space Invaders
│   ├── 16_Asteroids/             # Retro Asteroids
│   ├── 16b_Asteroids_Modern/     # Modern Asteroids
│   ├── 17_Frogger/               # Retro Frogger
│   ├── 17b_Frogger_Modern/       # Modern Frogger
│   ├── 18_Tetris/                # Retro Tetris
│   └── 18b_Tetris_Modern/        # Modern Tetris
├── examples/                      # Grundlegende Beispiele
│   ├── 01_HelloWorld/
│   ├── 02_BasicShapes/
│   ├── ...
│   └── 11_ScrollingChart/
└── libraries/
    └── CYD_Games/                 # Spiele-Library
        ├── src/
        │   ├── PongGame.h         # Retro-Versionen
        │   ├── SnakeGame.h
        │   ├── BreakoutGame.h
        │   ├── SpaceInvadersGame.h
        │   ├── AsteroidsGame.h
        │   ├── FroggerGame.h
        │   ├── TetrisGame.h
        │   ├── PongGameModern.h   # Modern-Versionen
        │   ├── SnakeGameModern.h
        │   ├── BreakoutGameModern.h
        │   ├── SpaceInvadersGameModern.h
        │   ├── AsteroidsGameModern.h
        │   ├── FroggerGameModern.h
        │   └── TetrisGameModern.h
        └── library.properties
```

## 🎓 Lernpfad

1. **Basics:** Starte mit `examples/01_HelloWorld` bis `examples/04_Colors`
2. **Interaktion:** Lerne Touch-Input mit `examples/03_TouchDemo` und `examples/10_TouchButtons`
3. **Grafik:** Fortgeschrittene Techniken in `examples/06_BouncingBall` und `examples/11_ScrollingChart`
4. **Projekte:** Baue eigene Apps mit `examples/09_Calculator` als Vorlage
5. **Games:** Erkunde die Arcade-Spiele in `arcade-games/`
6. **Library:** Lerne die CYD_Games Library kennen für eigene Spiele

## 🛠️ WiFi-Konfiguration

Für WiFi-basierte Beispiele (07_WiFiClock, 08_AnalogClock):

**Option 1:** Erstelle `Credentials.h` im Beispiel-Ordner:
```cpp
const char* ssid = "DeinWiFiName";
const char* password = "DeinWiFiPasswort";
```

**Option 2:** Kopiere `Credentials.h.example` zu `Credentials.h` und trage deine Daten ein.

## 📊 Statistiken

- **📚 11 Basis-Beispiele** zum Lernen
- **🎮 14 Arcade-Spiele** (7 Retro + 7 Modern)
- **🎯 2 Touch-Menüs** für einfachen Zugriff
- **📦 14 Klassen** in der CYD_Games Library
- **~15.000 Zeilen Code** vollständig dokumentiert
- **✓ 100% Skalierbar** für alle CYD Displays

## 🤝 Beitragen

Feedback, Bug-Reports und Pull Requests sind willkommen!

## 📄 Lizenz

MIT License - siehe LICENSE Datei für Details.

---

**Happy Coding! 🚀** Viel Spaß beim Experimentieren mit dem CYD Display und den Arcade-Spielen!
