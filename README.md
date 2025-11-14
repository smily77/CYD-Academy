# CYD Academy - Lerne ESP32 Display-Programmierung

**Eine umfassende Lernplattform für das CYD (Cheap Yellow Display)** mit praxisnahen Beispielen, Schritt-für-Schritt Tutorials und einer vollständigen Arcade-Spielesammlung als Master Class.

## 🎓 Was ist die CYD Academy?

Die CYD Academy ist dein **Einstieg in die ESP32 Display-Programmierung**. Von grundlegenden Konzepten bis zu fortgeschrittenen Techniken - hier lernst du alles, was du brauchst, um beeindruckende Projekte mit dem CYD zu erstellen.

**Lernpfad:**
1. 📚 **Basics** - Verstehe Display, Touch und Grafik-Grundlagen
2. 🎨 **Intermediate** - Lerne Animationen, WiFi und Sensoren
3. 🚀 **Advanced** - ESP-NOW, Multi-Threading, komplexe UI
4. 🎮 **Master Class** - Analysiere vollständige Arcade-Spiele

## 📦 Inhalt

### 📚 [Examples](examples/) - Lerne Schritt für Schritt
Von einfach bis fortgeschritten - 12 durchdachte Beispiele zum Lernen:
- **Basics:** Hello World, Touch Demo, Grafik, Text-Effekte
- **Intermediate:** WiFi-Uhren, Taschenrechner, Live-Daten
- **Advanced:** ESP-NOW Kommunikation zwischen CYDs
- Jedes Beispiel ist **ausführlich dokumentiert** und erklärt **warum**, nicht nur **wie**

### 🎮 [Arcade Games](arcade-games/) - Master Class
Vollständige Arcade-Spiele als **Lern-Ressource für Fortgeschrittene**:
- **14 Spiele:** Pong, Snake, Breakout, Space Invaders, Asteroids, Frogger, Tetris (je 2 Versionen)
- **CYD_Games Library:** Alle Spiele als wiederverwendbare Klassen - **Studiere professionellen Code**
- **Lerne:** Game-Loops, Kollisionserkennung, State-Management, Performance-Optimierung
- **2 Touch-Menüs:** Komplette UI-Implementierung als Referenz
- Mehr Details: [arcade-games/README.md](arcade-games/README.md)

## 🖥️ Hardware

- **CYD 2.8" ESP32 Display** (USB-C) - 320x240px
- **CYD 3.5" ESP32 Display** (USB-C) - 480x320px
- ESP32 basiertes Board mit TFT Display und Touch-Screen
- **Optionale Hardware:**
  - RGB LED (für visuelle Effekte)
  - Photoresistor (für Helligkeitssteuerung)
  - 2x Potentiometer (für Spiele-Steuerung)
  - 4x Digitale Buttons (für Spiele-Steuerung)
  - Rotary Encoder mit Button (alternative Steuerung)
  - ESP32 Board (als drahtloser Controller via ESP-NOW)

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

**Option 3: GPIO-Encoder (Rotary Encoder direkt am ESP32)**
```cpp
#define gpioEnc
#define encA 17         // Encoder Pin A
#define encB 5          // Encoder Pin B
#define encSW 16        // Encoder Button
```

**Option 4: I2C-Encoder (Rotary Encoder über PCF8574)**
```cpp
#define i2cEnc
#define encA 0          // PCF8574 Pin 0
#define encB 1          // PCF8574 Pin 1
#define encSW 2         // PCF8574 Pin 2
#define pcfAddress 0x20 // I2C-Adresse des PCF8574
```

**Option 5-7: ESP-NOW (Drahtlose Eingabe über zweites ESP32)**
```cpp
// Auf dem CYD (Empfänger):
#define espNowSwitch    // Für drahtlose Buttons
// ODER
#define espNowEnc       // Für drahtlosen Encoder
// ODER
#define espNowPoti      // Für drahtlose Potentiometer
```

**Potentiometer (optional für Pong & Breakout)**
```cpp
#define gpioPoti
#define potiLeft 34
#define potiRight 35
```

**Wichtig:**
- Genau EINE von `gpioSwitch`, `i2cSwitch`, `gpioEnc`, `i2cEnc`, `espNowSwitch`, `espNowEnc`, `espNowPoti` muss definiert sein
- Mit Encoder sind nicht alle Spiele kompatibel (Snake und Asteroids werden automatisch ausgeblendet)
- Pong läuft mit Encoder nur im AI-Modus (rechter Paddle mit Encoder steuerbar)
- Bei Encoder-Steuerung: Rotation = Links/Rechts, Button = Aktion
- Spiele die Potentiometer benötigen (Pong, Breakout) erscheinen nur im Menu wenn `gpioPoti` oder `espNowPoti` definiert ist
- Bei I2C (i2cSwitch oder i2cEnc) muss die PCF8574 Library installiert sein
- Bei ESP-NOW: Siehe [examples/ESP-NOW_Sender/](examples/ESP-NOW_Sender/) für Sender-Sketches

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

## 🎓 Empfohlener Lernpfad

### Level 1: Basics (1-4 Stunden)
1. **Display-Grundlagen:** `examples/01_HelloWorld` → `examples/02_BasicShapes` → `examples/04_Colors`
2. **Touch-Input:** `examples/03_TouchDemo` - Verstehe wie Touch funktioniert
3. **Erste Schritte:** Experimentiere mit den Beispielen, ändere Farben, Positionen, Texte

**Lernziele:** Display initialisieren, Grafik zeichnen, Touch lesen

### Level 2: Intermediate (4-8 Stunden)
4. **Text & Animation:** `examples/05_ScrollText` → `examples/06_BouncingBall`
5. **WiFi & NTP:** `examples/07_WiFiClock` → `examples/08_AnalogClock`
6. **Touch-UI:** `examples/09_Calculator` → `examples/10_TouchButtons`
7. **Live-Daten:** `examples/11_ScrollingChart`

**Lernziele:** Animationen, WiFi-Verbindung, komplexe Touch-UIs, Sensoren

### Level 3: Advanced (8-12 Stunden)
8. **ESP-NOW Remote:** `examples/ESP-NOW_Sender/` - Baue einen drahtlosen Controller
9. **CYD-zu-CYD:** `examples/12_CYD_to_CYD_Communication/` - Zwei CYDs kommunizieren
10. **Eigene Projekte:** Kombiniere Gelerntes für eigene Ideen

**Lernziele:** Drahtlose Kommunikation, Pairing-Mechanismen, Multi-Device-Systeme

### Level 4: Master Class (12+ Stunden)
11. **Game-Analyse:** Studiere `arcade-games/` - Von einfach (Snake) bis komplex (Space Invaders)
12. **Library-Code:** Verstehe `libraries/CYD_Games/` - Professionelle Code-Struktur
13. **Eigene Games:** Baue dein eigenes Spiel mit der CYD_Games Library

**Lernziele:** Game-Loops, State-Management, Kollisionserkennung, Performance-Optimierung

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
