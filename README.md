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
Von einfach bis fortgeschritten - 16 durchdachte Beispiele zum Lernen:
- **Basics:** Board-Test, Hello World, Touch Demo, Grafik, Text-Effekte
- **Intermediate:** WiFi-Uhren, Taschenrechner, Live-Daten, Racing Track
- **Advanced:** ESP-NOW Kommunikation zwischen CYDs, LVGL UI-Framework
- Jedes Beispiel ist **ausführlich dokumentiert** und erklärt **warum**, nicht nur **wie**

### 🔬 [Sensor Examples](sensor-examples/) - Hardware-Integration
Fortgeschrittene Beispiele mit externen I2C/SPI-Sensoren:
- **Bewegungssensoren:** Spirit Level, Compass (BNO055, MPU6050)
- **Wettersensoren:** Lightning Sensor (AS3935), Weather Station (BMP180)
- **Distanzsensoren:** Laser Distance (VL53L0X, R50-C)
- **Luftqualität:** Air Quality Monitor (CCS811)
- **Gestensensoren:** 3D Gesture Control (MGC3130)
- Vollständige I2C-Verkabelung und Library-Guides

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
| 00 | BoardConfigTest | Board-Test (Display + Touch + Rotation) |
| 01 | HelloWorld | Einfache Textausgabe |
| 02 | BasicShapes | Grundformen zeichnen |
| 03 | TouchDemo | Touch-Input verarbeiten |
| 04 | Colors | Farben und Farbverläufe |
| 05 | ScrollText | Text-Scroll-Effekte |
| 06 | BouncingBall | Physik-Simulation mit Touch |
| 07 | WiFiClock | Digitale Uhr mit NTP |
| 08 | AnalogClock | Analoge Uhr mit NTP |
| 09 | Calculator | Touch-Taschenrechner (inkl. LVGL-Version) |
| 10 | TouchButtons | Button-Verhaltensweisen (inkl. LVGL & Premium) |
| 11 | ScrollingChart | Live-Daten-Visualisierung |
| 12 | CYD_to_CYD | ESP-NOW Kommunikation zwischen CYDs |
| 13 | Racing_Track | Rennstrecke mit Touch-Steuerung |
| 14 | Selection_Menu | Menü-Navigation (Native & LVGL) |
| 15 | Image_Viewer | Bild-Anzeige (SPIFFS & SD-Karte) |

Alle Beispiele sind **vollständig skalierbar** und funktionieren auf 2.8" und 3.5" Displays.

### Sensor-Beispiele (sensor-examples/)

Fortgeschrittene Hardware-Integration mit I2C/SPI-Sensoren - siehe [sensor-examples/README.md](sensor-examples/README.md):

| # | Sensor | Beschreibung |
|---|--------|--------------|
| 16 | Spirit Level | BNO055/MPU6050 - Wasserwaage & Labyrinth |
| 17 | Lightning Sensor | AS3935 - Blitzdetektor (0-40 km) |
| 18 | Compass | BNO055 - Digitaler Kompass |
| 19 | Laser Distance | VL53L0X - ToF Distanzmessung |
| 20 | Air Quality | CCS811 - eCO2 & TVOC Monitor |
| 21 | Gesture Sensor | MGC3130 - 3D Gestensteuerung |
| 22 | Weather Station | BMP180 - Luftdruck & Temperatur |

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
├── TOUCH_ROTATION_FIX.md          # Touch-Rotation-Dokumentation
├── Repo_Guideline.txt             # Entwickler-Richtlinien
│
├── examples/                      # Grundlegende Beispiele (00-15)
│   ├── 00_BoardConfigTest/        # Board-Test Tool
│   ├── 01_HelloWorld/
│   ├── 02_BasicShapes/
│   ├── 03_TouchDemo/
│   ├── 04_Colors/
│   ├── 05_ScrollText/
│   ├── 06_BouncingBall/
│   ├── 07_WiFiClock/
│   ├── 08_AnalogClock/
│   ├── 09_Calculator/             # + 09b_LVGL
│   ├── 10_TouchButtons/           # + 10b_LVGL, 10c_Premium
│   ├── 11_ScrollingChart/
│   ├── 12_CYD_to_CYD_Communication/
│   ├── 13_Racing_Track/           # + 13b_Obstacles
│   ├── 14_Selection_Menu_Native/  # + 14b_LVGL
│   ├── 15_Image_Viewer/           # + 15b_SD
│   └── ESP-NOW_Sender/            # Sender für drahtlose Steuerung
│
├── sensor-examples/               # Sensor-Integration (16-22)
│   ├── README.md                  # Sensor-Dokumentation
│   ├── 16_Spirit_Level/           # BNO055/MPU6050 + 16b_Maze
│   ├── 17_Lightning_Sensor/       # AS3935 + 17b_Landscape
│   ├── 18_Compass_Orientation/    # BNO055 Magnetometer
│   ├── 19_Laser_Distance/         # VL53L0X/R50-C ToF
│   ├── 20_Air_Quality_CCS811/     # eCO2 & TVOC Monitor
│   ├── 21_Gesture_Sensor_MGC3130/ # 3D Gestensteuerung
│   └── 22_Weather_Station_BMP180/ # Luftdruck + 22b_History
│
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
│
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

### Level 0: Setup & Test (30 Minuten)
0. **Board-Test:** `examples/00_BoardConfigTest` - Teste Display, Touch und Rotation
   - Validiere deine Board-Konfiguration
   - Prüfe RGB-Farben
   - Teste Touch-Genauigkeit

**Lernziele:** Funktionierende Hardware-Setup bestätigen

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
8. **Advanced UI:** `examples/13_Racing_Track` → `examples/14_Selection_Menu`

**Lernziele:** Animationen, WiFi-Verbindung, komplexe Touch-UIs

### Level 3: Advanced (8-12 Stunden)
9. **ESP-NOW Remote:** `examples/ESP-NOW_Sender/` - Baue einen drahtlosen Controller
10. **CYD-zu-CYD:** `examples/12_CYD_to_CYD_Communication/` - Zwei CYDs kommunizieren
11. **Hardware-Sensoren:** `sensor-examples/` - Integriere I2C/SPI-Sensoren
    - Start: `22_Weather_Station` (einfach)
    - Fortgeschritten: `17_Lightning_Sensor`, `20_Air_Quality`
12. **Eigene Projekte:** Kombiniere Gelerntes für eigene Ideen

**Lernziele:** Drahtlose Kommunikation, I2C-Protokoll, Sensor-Integration

### Level 4: Master Class (12+ Stunden)
13. **Game-Analyse:** Studiere `arcade-games/` - Von einfach (Snake) bis komplex (Space Invaders)
14. **Library-Code:** Verstehe `libraries/CYD_Games/` - Professionelle Code-Struktur
15. **Eigene Games:** Baue dein eigenes Spiel mit der CYD_Games Library

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

- **📚 16 Basis-Beispiele** zum Lernen (examples/)
- **🔬 10 Sensor-Beispiele** für Hardware-Integration (sensor-examples/)
- **🎮 14 Arcade-Spiele** (7 Retro + 7 Modern)
- **🎯 2 Touch-Menüs** für einfachen Zugriff
- **📦 14 Klassen** in der CYD_Games Library
- **~20.000 Zeilen Code** vollständig dokumentiert
- **✓ 100% Skalierbar** für alle CYD Displays

## 🤝 Beitragen

Feedback, Bug-Reports und Pull Requests sind willkommen!

## 📄 Lizenz

MIT License - siehe LICENSE Datei für Details.

---

**Happy Coding! 🚀** Viel Spaß beim Experimentieren mit dem CYD Display und den Arcade-Spielen!
