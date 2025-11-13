# 14_Breakout - Klassisches Breakout/Arkanoid Spiel

Klassisches Breakout-Spiel für CYD mit **Analog-Regler Steuerung**.

## Hardware-Anforderungen

### Standard CYD Display
- ESP32-2432S028R (320x240)
- ILI9341 Display

### Spezielle Hardware: Analog-Regler
- **potiLeft**: Schläger-Steuerung (Potentiometer)

> **Hinweis**: Dieses Beispiel benötigt ein **spezielles CYD-Board** mit Analog-Regler (Potentiometer). Der Pin ist in `CYD_Display_Config.h` definiert.

## Spielprinzip

### Klassisches Breakout
- Steuere einen Schläger horizontal
- Halte den Ball im Spiel
- Zerstöre alle Steine
- Verschiedene Stein-Farben = verschiedene Punkte
- 3 Leben

### Steuerung
```
Analog-Regler potiLeft: Schläger Links/Rechts bewegen
Touch Screen:           Ball starten (wenn am Schläger)
```

## Features

### Gameplay
- ⚡ **Schnelles Gameplay**: ~60 FPS
- 🎯 **Präzise Steuerung**: Analog-Input für smooth Schläger-Bewegung
- 🧱 **6 Reihen Steine**: Verschiedene Farben und Punktwerte
- 💯 **Score-System**: 2-7 Punkte pro Stein (abhängig von Farbe)
- ❤️ **3 Leben**: Ball darf 3x unten rausfallen
- 🎮 **Level-System**: Automatisch nächstes Level wenn alle Steine zerstört
- 🎱 **Ball-Physik**:
  - Prallt an Wänden, Schläger und Steinen ab
  - Winkel abhängig von Schläger-Treffpunkt
  - Geschwindigkeits-Begrenzung

### Visuals
- 🎨 **Retro-Design**: Klassisches Breakout-Look
- 🌈 **Bunte Steine**: 6 Farben (Rot, Orange, Gelb, Grün, Cyan, Blau)
- ⚪ **Weißer Schläger & Ball**: Gut sichtbar
- 📈 **UI-Anzeige**: Score, Level, Lives oben

### Technisch
- **Effizientes Rendering**: Nur veränderte Bereiche neu zeichnen
- **Float-Positionen**: Smooth ball movement
- **Collision Detection**: Schläger, Steine, Wände
- **Touch-Integration**: Ball-Start per Touch
- **Serial Debug**: Score, Treffer und Game Over Events

## Stein-Farben und Punkte

| Reihe | Farbe | Punkte |
|-------|-------|--------|
| 1 | 🔴 Rot | 7 |
| 2 | 🟠 Orange | 6 |
| 3 | 🟡 Gelb | 5 |
| 4 | 🟢 Grün | 4 |
| 5 | 🔵 Cyan | 3 |
| 6 | 🔵 Blau | 2 |

**Gesamt:** 6 Reihen × 10 Spalten = 60 Steine
**Max. Score pro Level:** (7+6+5+4+3+2) × 10 = **270 Punkte**

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
  bool stuck;    // Am Schläger festgeklebt
};

struct Brick {
  int x, y;      // Position
  int w, h;      // Größe
  uint16_t color;
  int points;    // Punktwert
  bool active;   // Noch vorhanden?
};
```

### Haupt-Funktionen
```cpp
void initGame()           // Spiel initialisieren
void initLevel()          // Level initialisieren (Steine neu)
void resetBall()          // Ball zurücksetzen
void launchBall()         // Ball vom Schläger starten
void updatePaddle()       // Schläger von Analog-Input
void updateBall()         // Ball-Position aktualisieren
void checkCollisions()    // Kollisionen prüfen
void drawGame()           // Alles zeichnen
```

### Game Loop
```cpp
void loop() {
  if (gameOver) { /* Neustart warten */ return; }
  if (levelComplete) { /* Nächstes Level */ }

  updatePaddle();         // Schläger bewegen

  if (ball.stuck) {
    // Warte auf Touch zum Ball-Start
  } else {
    updateBall();         // Ball bewegen
    checkCollisions();    // Treffer prüfen
  }

  drawGame();             // Rendern
  delay(16);              // ~60 FPS
}
```

## Spielfeld-Layout

```
Score:120   Level:2   Lives:2
────────────────────────────────────
🔴🔴🔴🔴🔴🔴🔴🔴🔴🔴  ← Rot (7 Pkt)
🟠🟠🟠🟠🟠🟠🟠🟠🟠🟠  ← Orange (6 Pkt)
🟡🟡🟡🟡🟡🟡🟡🟡🟡🟡  ← Gelb (5 Pkt)
🟢🟢🟢🟢🟢🟢🟢🟢🟢🟢  ← Grün (4 Pkt)
🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵  ← Cyan (3 Pkt)
🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵  ← Blau (2 Pkt)


            ●          ← Ball




      ════════         ← Schläger
```

### Dimensionen
- **Display**: 320 x 240 Pixel
- **Schläger**: 60 x 8 Pixel
- **Ball**: 6 x 6 Pixel
- **Stein**: 28 x 10 Pixel (+ 4px Abstand)
- **Ball-Speed**: ~4 px/frame

## Spielregeln

### Punktevergabe
- **Rote Steine**: 7 Punkte
- **Orange Steine**: 6 Punkte
- **Gelbe Steine**: 5 Punkte
- **Grüne Steine**: 4 Punkte
- **Cyan Steine**: 3 Punkte
- **Blaue Steine**: 2 Punkte

### Level-System
1. **Level komplett**: Alle Steine zerstört
2. **Automatisch**: Nächstes Level startet
3. **Steine regenerieren**: Alle Steine kommen zurück
4. **Score bleibt**: Score wird nicht zurückgesetzt

### Leben-System
- **Start**: 3 Leben
- **Leben verlieren**: Ball fällt unten raus
- **Game Over**: Bei 0 Leben
- **Neustart**: Touch Screen bei Game Over

### Ball-Verhalten
- **Festgeklebt**: Ball startet am Schläger
- **Start**: Touch Screen → Ball fliegt los
- **Winkel**: Abhängig von Schläger-Treffpunkt
  - Mitte = gerade nach oben
  - Links/Rechts = schräg
- **Geschwindigkeit**: Konstant (~4 px/frame)

## Hardware-Setup

### Potentiometer anschließen

#### Schläger-Regler (potiLeft)
```
Potentiometer:
  [VCC] ──── 3.3V
  [SIG] ──── potiLeft (siehe CYD_Display_Config.h)
  [GND] ──── GND
```

> **Wichtig**: Verwende 3.3V, **NICHT** 5V!

### GPIO-Eigenschaften
- potiLeft ist typischerweise ein **Input-Only** Pin
- Hat **keinen** internen Pull-Up/Pull-Down
- Perfekt für Analog-Input
- 12-bit ADC (0-4095)
- Die physische GPIO-Nummer ist in `CYD_Display_Config.h` gemappt

## Installation

1. **CYD_Display_Config.h** im Root-Verzeichnis vorhanden?
2. **LovyanGFX** Library installiert?
3. **Potentiometer** an potiLeft angeschlossen?
4. **Sketch hochladen**
5. **Spielen!** 🎮

## Spieltipps

### Für Anfänger
- Schläger in der **Mitte** halten
- Ball mit **Schläger-Mitte** treffen
- Nicht zu hektisch bewegen
- **Rote Steine** zuerst (7 Punkte!)

### Für Profis
- Winkel mit **Schläger-Rand** steuern
- Ball in **schwer erreichbare Ecken** lenken
- **Kombos**: Mehrere Steine mit einem Schlag
- **Top-Down**: Von oben nach unten abarbeiten

## Anpassungen

### Schläger größer machen
```cpp
// In initGame():
paddle.w = 80;  // statt 60
```

### Ball schneller/langsamer
```cpp
// In launchBall():
ball.vx = cos(rad) * 5.0;  // statt 4.0
ball.vy = -abs(sin(rad)) * 5.0;
```

### Mehr Leben
```cpp
// In initGame():
lives = 5;  // statt 3
```

### Mehr Stein-Reihen
```cpp
#define BRICK_ROWS 8  // statt 6

// rowColors und rowPoints erweitern!
```

### Andere Farben
```cpp
#define COLOR_PADDLE  0x07FF  // Cyan statt Weiß
#define COLOR_BALL    0xFFE0  // Gelb statt Weiß
```

## Troubleshooting

### Schläger bewegt sich nicht
- ✅ Potentiometer richtig angeschlossen?
- ✅ Korrekter Pin (potiLeft) verwendet?
- ✅ 3.3V (nicht 5V)?
- ✅ Seriellen Monitor prüfen

### Schläger zittert
- Potentiometer eventuell defekt
- Software-Filter hinzufügen:
  ```cpp
  static int lastPos = 0;
  int newPos = map(potValue, 0, 1000, 0, SCREEN_WIDTH - paddle.w);
  paddle.x = (lastPos * 3 + newPos) / 4;  // Glättung
  lastPos = paddle.x;
  ```

### Ball startet nicht
- Touch Screen funktioniert?
- Ball muss `stuck = true` sein
- Tippe auf Bildschirm

### Steine regenerieren nicht
- Normal! Jedes Level hat neue Steine
- Alle Steine zerstören → automatisch nächstes Level

## Erweiterungsideen

### Easy
- [ ] **Power-Ups**: Größerer Schläger, Multi-Ball, Extra-Leben
- [ ] **Sound**: Beep bei Treffer
- [ ] **Highscore**: Bester Score speichern

### Medium
- [ ] **Verschiedene Level-Layouts**: Nicht nur Reihen
- [ ] **Unzerstörbare Steine**: Graue Steine
- [ ] **Geschwindigkeits-Steigerung**: Ball wird schneller

### Advanced
- [ ] **Boss-Level**: Bewegliche Steine
- [ ] **Spezial-Steine**: Explodierende Steine (zerstören Nachbarn)
- [ ] **Multiplayer**: 2 Schläger (oben + unten)

## Credits

Basiert auf **Breakout** (1976, Atari) und **Arkanoid** (1986, Taito).

Implementiert für CYD mit analoger Schläger-Steuerung.

## Viel Spaß beim Spielen! 🎮

```
    ____  ____  _________    __ ______  __  ________
   / __ )/ __ \/ ____/   |  / //_/ __ \/ / / /_  __/
  / __  / /_/ / __/ / /| | / ,< / / / / / / / / /
 / /_/ / _, _/ /___/ ___ |/ /| / /_/ / /_/ / / /
/_____/_/ |_/_____/_/  |_/_/ |_\____/\____/ /_/
```
