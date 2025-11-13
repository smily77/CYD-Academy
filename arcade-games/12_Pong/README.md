# 12_Pong - Klassisches Pong-Spiel

Klassisches 2-Spieler Pong-Spiel für CYD mit **Analog-Regler Steuerung**.

## Hardware-Anforderungen

### Standard CYD Display
- ESP32-2432S028R (320x240)
- ILI9341 Display

### Spezielle Hardware: 2x Analog-Regler + 3x Taster
- **potiLeft**: Linker Spieler (Potentiometer)
- **potiRight**: Rechter Spieler (Potentiometer)
- **tasteC**: Auto-Modus EIN (Taster, Active LOW)
- **tasteB**: Auto-Modus AUS (Taster, Active LOW)
- **tasteA**: Spielstand zurücksetzen (Taster, Active LOW)

> **Hinweis**: Dieses Beispiel benötigt ein **spezielles CYD-Board** mit zwei analogen Reglern (Potentiometern) und drei Tastern. Alle Pins sind in `CYD_Display_Config.h` definiert.

## Spielprinzip

### Klassisches Pong
- 2 Spieler (links vs. rechts)
- Jeder steuert einen Schläger (Paddle) hoch/runter
- Ball prallt an Schlägern und Wänden ab
- Trifft Ball links/rechts außerhalb: Punkt für Gegner
- Score-Anzeige oben

### Steuerung
```
Linker Spieler    : Analog-Regler potiLeft (oder AI im Auto-Modus)
Rechter Spieler   : Analog-Regler potiRight

Taster tasteC     : Auto-Modus EIN (AI spielt links)
Taster tasteB     : Auto-Modus AUS (manuell links)
Taster tasteA     : Spielstand auf 0 setzen

Regler drehen → Schläger bewegt sich vertikal
```

## Features

### Gameplay
- ⚡ **Schnelles Gameplay**: ~60 FPS
- 🎯 **Präzise Steuerung**: Analog-Input für smooth movement
- 🤖 **AI-Gegner**: Auto-Modus für linken Schläger (folgt dem Ball)
- 🔄 **Score Reset**: Spielstand jederzeit zurücksetzen
- 📊 **Score-System**: Zählt Punkte für beide Spieler
- 🎱 **Ball-Physik**:
  - Konstante Geschwindigkeit
  - Prallt an Wänden ab
  - Winkel abhängig von Treffpunkt am Schläger
  - Zufällige Start-Richtung (besserer Zufallsgenerator)

### Visuals
- 🎨 **Retro-Design**: Klassisches Schwarz/Weiß Pong-Look
- 💙 **Cyan Schläger**: Gut sichtbare Paddle (manueller Modus)
- 🔴 **Roter Schläger**: Linker Schläger im Auto-Modus (AI)
- 💛 **Gelber Ball**: Leicht zu verfolgen
- ⚪ **Mittellinie**: Gestrichelte Linie (klassisch)
- 📈 **Score-Anzeige**: Groß und lesbar
- 🔄 **Sauberes Rendering**: Mittellinie und Score bleiben immer sichtbar

### Technisch
- **Effizientes Rendering**: Nur veränderte Bereiche neu zeichnen
- **Persistentes UI**: Mittellinie und Score werden jeden Frame neu gezeichnet
- **Float-Positionen**: Smooth ball movement
- **Collision Detection**: Paddle- und Wand-Kollision
- **Tastenentprellung**: Saubere Button-Erkennung mit Zustandsüberwachung
- **Dynamische Farben**: Schlägerfarbe wechselt mit Auto-Modus
- **Serial Debug**: Tore, Treffer und Modus-Wechsel werden ausgegeben

## Code-Struktur

### Game Objects
```cpp
struct Paddle {
  int x, y;      // Position
  int w, h;      // Größe
  uint16_t color;
};

struct Ball {
  float x, y;    // Position (float für smooth movement)
  float vx, vy;  // Geschwindigkeit
  int size;
  uint16_t color;
};
```

### Haupt-Funktionen
```cpp
void initGame()        // Spiel initialisieren
void resetBall()       // Ball nach Tor zurücksetzen
void handleButtons()   // Taster verarbeiten (Auto-Modus, Reset)
void updatePaddles()   // Analog-Inputs lesen oder AI steuern
void updateBall()      // Ball-Position aktualisieren
void checkCollisions() // Kollisionen prüfen
void drawGame()        // Alles zeichnen
```

### Game Loop
```cpp
void loop() {
  handleButtons();     // Taster verarbeiten (Auto-Modus, Reset)
  updatePaddles();     // Schläger von Analog-Inputs oder AI
  updateBall();        // Ball bewegen
  checkCollisions();   // Treffer & Tore
  drawGame();          // Rendern
  delay(16);           // ~60 FPS
}
```

## Spielfeld-Layout

```
    Score Links        Score Rechts
        3                  5

┌──────────────────────────────────┐
│          :                       │
│  █       :                    █  │  ← Schläger
│  █       :                    █  │    (Paddels)
│  █       :                    █  │
│  █       :        ■           █  │  ← Ball
│  █       :                    █  │
│  █       :                    █  │
│          :                       │
└──────────────────────────────────┘
  ↑                               ↑
  potiLeft                   potiRight
  (Linker Regler)      (Rechter Regler)
```

### Dimensionen
- **Display**: 320 x 240 Pixel
- **Schläger**: 5 x 60 Pixel
- **Ball**: 6 x 6 Pixel
- **Ball-Speed**: 3.5 px/frame horizontal, ±2 px/frame vertikal

## Spielregeln

### Punktevergabe
1. Ball trifft **linke Seite** raus → **+1 für rechten Spieler**
2. Ball trifft **rechte Seite** raus → **+1 für linken Spieler**

### Ball-Verhalten
- **Wand-Kollision**: Ball prallt ab (oben/unten)
- **Schläger-Kollision**: Ball prallt ab, Winkel abhängig von Treffpunkt
- **Nach Tor**: Ball resettet in Bildschirm-Mitte
- **Start-Richtung**: Zufällig links oder rechts

### Schläger-Bewegung
- **Analog-Input**: 0-4095 (12-bit ADC)
- **Mapping**: Analog-Wert → Y-Position (0 bis 180)
- **Smooth**: Direkte Analog-Steuerung ohne Verzögerung

## Hardware-Setup

### Potentiometer anschließen

#### Linker Regler (potiLeft)
```
Potentiometer:
  [VCC] ──── 3.3V
  [SIG] ──── potiLeft (siehe CYD_Display_Config.h)
  [GND] ──── GND
```

#### Rechter Regler (potiRight)
```
Potentiometer:
  [VCC] ──── 3.3V
  [SIG] ──── potiRight (siehe CYD_Display_Config.h)
  [GND] ──── GND
```

> **Wichtig**: Verwende 3.3V, **NICHT** 5V!

### Taster anschließen

#### Auto-Modus EIN (tasteC)
```
Taster:
  [PIN 1] ──── tasteC (siehe CYD_Display_Config.h)
  [PIN 2] ──── GND
```

#### Auto-Modus AUS (tasteB)
```
Taster:
  [PIN 1] ──── tasteB (siehe CYD_Display_Config.h)
  [PIN 2] ──── GND
```

#### Score Reset (tasteA)
```
Taster:
  [PIN 1] ──── tasteA (siehe CYD_Display_Config.h)
  [PIN 2] ──── GND
```

> **Hinweis**: Alle Taster nutzen INPUT_PULLUP, daher keine externen Pull-Up-Widerstände nötig! Die physischen GPIO-Pins sind in `CYD_Display_Config.h` gemappt.

### GPIO-Eigenschaften
- potiLeft & potiRight sind typischerweise **Input-Only** Pins
- Haben **keinen** internen Pull-Up/Pull-Down
- Perfekt für Analog-Inputs
- 12-bit ADC (0-4095)

## Installation

1. **CYD_Display_Config.h** im Root-Verzeichnis vorhanden?
2. **LovyanGFX** Library installiert?
3. **Potentiometer** an GPIO 34 & 35 angeschlossen?
4. **Taster** an GPIO 16, 5 & 17 angeschlossen?
5. **Sketch hochladen**
6. **Spielen!** 🎮

## Spieltipps

### Für Anfänger
- Paddle in der **Mitte** halten
- Ball mit **Paddle-Mitte** treffen
- Nicht zu schnell bewegen
- **Auto-Modus** nutzen zum Üben (tasteC)

### Für Profis
- Winkel mit **Paddle-Rand** ändern
- Gegner in **Ecke** drängen
- **Spin-Shots** mit Paddle-Kante
- Gegen **AI spielen** (Auto-Modus links, manuell rechts)

## Anpassungen

### Ball schneller machen
```cpp
// In resetBall():
ball.vx = (random(0, 2) == 0) ? -4.5 : 4.5;  // statt -3.5/3.5
```

### Schläger größer machen
```cpp
// In initGame():
paddleLeft.h = 80;   // statt 60
paddleRight.h = 80;  // statt 60
```

### Farben ändern
```cpp
#define COLOR_PADDLE  0xF800  // Rot statt Cyan
#define COLOR_BALL    0x07E0  // Grün statt Gelb
```

### Gewinn-Limit
```cpp
// In checkCollisions() nach scoreLeft++ bzw. scoreRight++:
if (scoreLeft >= 10) {
  // Linker Spieler gewinnt!
  lcd.fillScreen(COLOR_BG);
  lcd.setCursor(80, 120);
  lcd.print("LINKS GEWINNT!");
  while(1);  // Spiel stoppt
}
```

## Troubleshooting

### Schläger bewegt sich nicht
- ✅ Potentiometer richtig angeschlossen?
- ✅ Korrekte Pins (potiLeft & potiRight) verwendet?
- ✅ 3.3V (nicht 5V)?
- ✅ Seriellen Monitor öffnen - Analog-Werte ausgeben lassen

### Schläger zittert
- Potentiometer eventuell defekt
- Versuche Software-Filter:
  ```cpp
  int filtered = (lastValue * 3 + newValue) / 4;
  ```

### Ball zu langsam/schnell
- Ändere `ball.vx` und `ball.vy` in `resetBall()`
- Oder ändere `delay(16)` in `loop()`

### Display flackert
- Normal bei direktem Pixel-Drawing
- Für weniger Flackern: Double-Buffering implementieren

## Implementierte Features

- [x] **Auto-Modus (AI)**: Linker Schläger kann von AI gesteuert werden
- [x] **Score Reset**: Spielstand zurücksetzen per Tastendruck
- [x] **Visuelle Feedback**: Schlägerfarbe zeigt Auto-Modus an (rot)
- [x] **Tastenentprellung**: Saubere Button-Erkennung
- [x] **Persistentes UI**: Mittellinie und Score bleiben immer sichtbar

## Erweiterungsideen

### Easy
- [ ] **Gewinn-Limit**: Spiel endet bei 10 Punkten
- [ ] **Pause-Funktion**: Touch auf Mittellinie pausiert
- [ ] **Ball-Farbe** wechselt bei jedem Treffer
- [ ] **AI-Schwierigkeitsgrad**: Einstellbare Geschwindigkeit der AI

### Medium
- [ ] **Schwierigkeits-Level**: Ball wird schneller nach jedem Treffer
- [ ] **Sound-Effekte**: Beep bei Treffer (mit Buzzer)
- [ ] **Power-Ups**: Ball wird größer/kleiner
- [ ] **2 AI-Spieler**: Beide Schläger im Auto-Modus

### Advanced
- [ ] **Turnier-Modus**: Best of 5 mit Gewinn-Screen
- [ ] **Multiplayer-Levels**: Hindernisse im Spielfeld
- [ ] **Replay-System**: Letzte Punkte wiedergeben
- [ ] **Highscore-Speicherung**: EEPROM/Preferences nutzen

## Credits

Basiert auf dem klassischen **Pong** (1972, Atari).

Implementiert für CYD mit analoger Steuerung.

## Viel Spaß beim Spielen! 🎮

```
     _____  ____  _   _  _____
    |  __ \/ __ \| \ | |/ ____|
    | |__) | |  | |  \| | |  __
    |  ___/| |  | | . ` | | |_ |
    | |    | |__| | |\  | |__| |
    |_|     \____/|_| \_|\_____|
```
