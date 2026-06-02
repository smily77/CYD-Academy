# Spiel 01: 15-Puzzle (Schiebepuzzle)

Das legendäre **15-Puzzle** - ein zeitloser Denksport-Klassiker seit 1874! Erlebe eines der berühmtesten Logikspiele aller Zeiten auf deinem CYD Display mit beeindruckender 3D-Grafik und flüssigen Animationen.

![15-Puzzle Banner](../../assets/15-puzzle-banner.png)

## 📸 Features

- **Klassisches Spielprinzip**: 15 nummerierte Kacheln in einem 4x4-Feld ordnen
- **Realistische 3D-Grafik**: Kacheln mit Schatten, Highlights und 3D-Kanten-Effekten
- **Glatte Animationen**: Flüssige Schiebe-Animationen mit Ease-in-out Interpolation
- **Holzrahmen**: Authentischer Holzrahmen mit Maserung um das Spielfeld
- **Farbverläufe**: 15 verschiedene Farben von Blau über Grün bis Orange
- **Intelligenter Shuffle**: Garantiert lösbares Puzzle durch Inversions-Check
- **Züge-Zähler & Timer**: Verfolge deine Performance
- **Bester Score**: Speicherung des besten Ergebnisses (wenigste Züge)
- **Undo-Funktion**: Bis zu 100 Züge rückgängig machen
- **Gewinn-Animation**: Beeindruckendes Feuerwerk beim Lösen
- **Auto-Scaling**: Passt sich automatisch an 2.8" und 3.5" Displays an
- **Touch-Steuerung**: Intuitive Touch-Bedienung

## 🔌 Hardware-Anforderungen

### Hauptkomponenten
- **ESP32-2432S028R (CYD - Cheap Yellow Display)**
  - 2.8" Version: 320x240 Pixel
  - 3.5" Version: 480x320 Pixel
- **Touch-Screen** (bereits im CYD integriert)

### Optional
- **4x Buttons** für erweiterte Funktionen:
  - Button A: Neues Spiel (Shuffle)
  - Button B: Lösung anzeigen
  - Button C: Undo (letzter Zug)
  - Button D: Pause/Fortsetzen

### Keine zusätzliche Hardware erforderlich!

Das 15-Puzzle läuft perfekt nur mit dem Touch-Screen. Die Button-Steuerung ist optional und kann über die CYD_Display_Config.h konfiguriert werden.

## 🎯 Spielanleitung

### Ziel des Spiels

Ordne die 15 nummerierten Kacheln (1-15) in aufsteigender Reihenfolge:
```
+----+----+----+----+
|  1 |  2 |  3 |  4 |
+----+----+----+----+
|  5 |  6 |  7 |  8 |
+----+----+----+----+
|  9 | 10 | 11 | 12 |
+----+----+----+----+
| 13 | 14 | 15 |    |  <- Leeres Feld rechts unten
+----+----+----+----+
```

### Steuerung

**Touch-Steuerung (empfohlen):**
- **Kachel antippen**: Tippe auf eine Kachel, die neben dem leeren Feld liegt
- Die Kachel gleitet mit einer flüssigen Animation in das leere Feld
- Nur Kacheln direkt neben dem leeren Feld (horizontal oder vertikal) können bewegt werden

**Button-Steuerung (optional):**
- **Button A**: Neues Spiel starten (Puzzle neu mischen)
- **Button B**: Lösung anzeigen (kommende Version)
- **Button C**: Letzten Zug rückgängig machen
- **Button D**: Spiel pausieren/fortsetzen

### Spielablauf

1. **Start**: Das Puzzle wird automatisch gemischt (200 zufällige Züge)
2. **Spielen**: Verschiebe Kacheln durch Antippen
3. **Strategie**: Plane deine Züge im Voraus
4. **Gewinn**: Ordne alle Kacheln in der richtigen Reihenfolge
5. **Feuerwerk**: Genieße die Gewinn-Animation
6. **Neustart**: Tippe zum Starten eines neuen Spiels

### Tipps für Anfänger

1. **Beginne mit den oberen Reihen**: Bringe zuerst 1-4 in Position
2. **Arbeite Zeile für Zeile**: Fixiere obere Zeilen und arbeite nach unten
3. **Nutze die Undo-Funktion**: Mache fehlerhafte Züge rückgängig
4. **Übe Muster**: Lerne Standard-Bewegungssequenzen
5. **Keine Eile**: Das 15-Puzzle belohnt Planung, nicht Geschwindigkeit

## 🔬 Technischer Hintergrund

### Geschichte des 15-Puzzles

Das 15-Puzzle wurde 1874 von **Noyes Palmer Chapman** erfunden und durch den Postmeister **Sam Loyd** ab 1880 popularisiert. Es löste damals eine wahre **Puzzle-Manie** in den USA und Europa aus!

**Historische Fakten:**
- Ursprünglicher Name: "Gem Puzzle" oder "Boss Puzzle"
- Sam Loyd's berühmte Herausforderung: $1.000 für die Lösung der "14-15 Puzzle" Konfiguration
- Mathematischer Beweis 1879: Die Hälfte aller Konfigurationen sind **unlösbar**!
- Verkaufszahlen: Millionen von Exemplaren weltweit

### Mathematische Grundlagen

#### Lösbarkeit (Solvability)

Nicht jede zufällige Anordnung der Kacheln ist lösbar! Die Lösbarkeit hängt von **Inversionen** ab.

**Inversion**: Eine Inversion liegt vor, wenn eine höhere Zahl vor einer niedrigeren Zahl steht (in Lese-Reihenfolge, leeres Feld ignoriert).

**Beispiel:**
```
Anordnung: 2, 1, 3, 4, 5, ...
           ^  ^
           Inversion! (2 steht vor 1)
```

**Lösbarkeits-Regel für 4x4 Puzzle:**

Ein 4x4 Puzzle ist lösbar, wenn:
```
(Anzahl_Inversionen + Zeile_des_leeren_Felds) ist UNGERADE
```

**Beweis-Skizze:**
- Jeder gültige Zug (horizontal oder vertikal) ändert die Parität der Inversionen
- Die gelöste Position hat 0 Inversionen und leeres Feld in Zeile 4 (von oben) → 0 + 4 = 4 (gerade)
- Durch Züge kann nur zwischen geraden und ungeraden Paritäten gewechselt werden
- Daher: Startposition muss ungerade Parität haben, um durch Züge zu gerader Parität zu gelangen

**In unserem Code:**
```cpp
bool isSolvable() {
  int inversions = 0;
  for (int i = 0; i < TILE_COUNT - 1; i++) {
    if (tiles[i] == 0) continue;
    for (int j = i + 1; j < TILE_COUNT; j++) {
      if (tiles[j] == 0) continue;
      if (tiles[i] > tiles[j]) inversions++;
    }
  }

  int emptyRow = emptyPos / GRID_SIZE;
  return ((inversions + emptyRow) % 2 == 1);
}
```

#### Komplexität

**Zustandsraum:**
- Anzahl möglicher Konfigurationen: **16! = 20.922.789.888.000** (~21 Billionen)
- Davon lösbar: **16! / 2 = 10.461.394.944.000** (~10 Billionen)
- Maximale Anzahl Züge zur Lösung: **80 Züge** (God's Number)

**Algorithmen zur Lösung:**
- **Brute Force**: Unmöglich (zu viele Zustände)
- **A\* Search**: Effizient mit Manhattan-Distanz Heuristik
- **IDA\* (Iterative Deepening A\*)**: Speichereffizient, findet optimale Lösung
- **Walking Distance Heuristik**: Fortgeschrittene Heuristik für schnellere Lösung

**Manhattan-Distanz Heuristik:**
```
Für jede Kachel: Berechne |aktuelle_x - ziel_x| + |aktuelle_y - ziel_y|
Summe aller Manhattan-Distanzen = Mindestanzahl Züge (lower bound)
```

### Shuffle-Algorithmus

Unser Shuffle-Algorithmus garantiert Lösbarkeit durch **nur gültige Züge**:

```cpp
void shufflePuzzle() {
  // Starte mit gelöster Position (definitiv lösbar)
  // Mache 200 zufällige GÜLTIGE Züge
  // -> Resultat ist garantiert lösbar!

  for (int i = 0; i < 200; i++) {
    // Finde alle Nachbar-Kacheln des leeren Felds
    // Wähle zufällig eine aus (außer die zuletzt bewegte)
    // Bewege diese Kachel ins leere Feld
  }
}
```

**Warum funktioniert das?**
- Gelöste Position ist lösbar
- Jeder gültige Zug führt zu einer lösbaren Position
- 200 Züge sind ausreichend für gute Durchmischung
- Keine Duplikation durch Vermeidung von Hin-und-Her-Bewegungen

**Alternative (nicht verwendet):**
- Zufällige Permutation + Lösbarkeits-Check
- Problem: 50% der Permutationen sind unlösbar → viel Rechenaufwand

## 💻 Code-Struktur

### Hauptkomponenten

#### 1. Spielfeld-Repräsentation

```cpp
// 1D-Array für 4x4 Grid (16 Felder)
uint8_t tiles[16];  // 0 = leer, 1-15 = Kacheln

// Position des leeren Feldes
uint8_t emptyPos;   // 0-15

// Konvertierung Position <-> Koordinaten
void getXY(uint8_t pos, int8_t &x, int8_t &y) {
  x = pos % GRID_SIZE;
  y = pos / GRID_SIZE;
}

uint8_t getPos(int8_t x, int8_t y) {
  return y * GRID_SIZE + x;
}
```

**Warum 1D-Array statt 2D?**
- Einfachere Iteration über alle Kacheln
- Weniger Speicher-Overhead
- Schnellerer Zugriff (keine doppelte Indizierung)
- Position als einzelner Index

#### 2. Grafik-Rendering

**3D-Kachel mit Schatten:**
```cpp
void drawTile(uint8_t pos) {
  // 1. Schatten zeichnen (rechts + unten versetzt)
  lcd.fillRect(px + shadowOffset, py + shadowOffset,
               TILE_SIZE, TILE_SIZE, COLOR_SHADOW);

  // 2. Hauptkachel zeichnen
  lcd.fillRect(px, py, TILE_SIZE - shadowOffset,
               TILE_SIZE - shadowOffset, tileColor);

  // 3. Helle 3D-Kanten (oben, links)
  for (int i = 0; i < edgeSize; i++) {
    lcd.drawFastHLine(..., lightEdge);  // Oben
    lcd.drawFastVLine(..., lightEdge);  // Links
  }

  // 4. Dunkle 3D-Kanten (unten, rechts)
  for (int i = 0; i < edgeSize; i++) {
    lcd.drawFastHLine(..., darkEdge);  // Unten
    lcd.drawFastVLine(..., darkEdge);  // Rechts
  }

  // 5. Zahl mit Text-Schatten
  lcd.drawNumber(tile, px + 2, py + 2, COLOR_SHADOW);  // Schatten
  lcd.drawNumber(tile, px, py, TFT_WHITE);             // Haupttext
}
```

**Farben-System:**
```cpp
// 15 Farbverläufe: Blau -> Cyan -> Grün -> Orange
const uint16_t TILE_COLORS[15] = {
  // Blau-Töne (1-4)
  0x3C5F, 0x3C9F, 0x3CDF, 0x3D1F,

  // Cyan-Grün-Töne (5-8)
  0x3D5F, 0x45BF, 0x4DDF, 0x55FF,

  // Grün-Töne (9-12)
  0x5DFF, 0x6DDF, 0x7D9F, 0x8D5F,

  // Orange-Töne (13-15)
  0x9D1F, 0xAD1F, 0xBCDF
};
```

#### 3. Animation-System

**Ease-in-out Interpolation:**
```cpp
void updateAnimation() {
  animProgress += ANIM_SPEED;  // 0.0 -> 1.0

  // Ease-in-out Kurve (smooth start & end)
  float t = animProgress;
  if (t < 0.5) {
    t = 2 * t * t;              // Ease-in (beschleunigen)
  } else {
    t = 1 - pow(-2 * t + 2, 2) / 2;  // Ease-out (verlangsamen)
  }

  // Interpoliere Position
  int16_t px = startPx + (endPx - startPx) * t;
  int16_t py = startPy + (endPy - startPy) * t;

  // Kachel an interpolierter Position zeichnen
  drawTile(pos, px, py);
}
```

**Animation-States:**
```
1. moveTile() aufgerufen
2. animTile = tile_number, animProgress = 0.0
3. Jedes Frame: animProgress += 0.15
4. updateAnimation() berechnet interpolierte Position
5. Kachel wird an neuer Position gezeichnet
6. animProgress >= 1.0 -> Animation beendet
7. Win-Check durchführen
```

#### 4. Touch-Verarbeitung

```cpp
void handleTouch() {
  uint16_t touchX, touchY;
  if (lcd.getTouch(&touchX, &touchY)) {
    // Debounce (200ms)
    if (millis() - lastTouchTime < 200) return;

    // Touch in Spielfeld?
    if (touchX < FIELD_X || touchX > FIELD_X + FIELD_SIZE) return;

    // Welche Kachel?
    int tileX = (touchX - FIELD_X - FRAME_WIDTH) / TILE_SIZE;
    int tileY = (touchY - FIELD_Y - FRAME_WIDTH) / TILE_SIZE;
    uint8_t pos = getPos(tileX, tileY);

    // Kann bewegt werden?
    if (canMove(pos)) {
      moveTile(pos);
    }
  }
}
```

**Debouncing:**
- Problem: Touch-Screen kann mehrere Events pro Berührung auslösen
- Lösung: Ignoriere Touch-Events innerhalb von 200ms
- Verhindert versehentliche Doppel-Züge

#### 5. Undo-System

```cpp
struct Move {
  uint8_t tile;   // Welche Kachel wurde bewegt?
  uint8_t from;   // Von welcher Position?
  uint8_t to;     // Zu welcher Position?
};

std::vector<Move> moveHistory;  // Stack der letzten Züge
const int MAX_HISTORY = 100;

void undoMove() {
  if (moveHistory.empty()) return;

  Move lastMove = moveHistory.back();
  moveHistory.pop_back();

  // Finde Kachel und bewege zurück
  // (ohne neuen History-Eintrag!)
  moveTile(findTilePos(lastMove.tile), false);
  moveCount--;  // Zug nicht zählen
}
```

#### 6. Auto-Scaling System

**Problem:** Verschiedene Display-Größen (2.8" = 320x240, 3.5" = 480x320)

**Lösung:** Dynamische Berechnung aller Dimensionen:

```cpp
void setup() {
  int screenWidth = lcd.width();
  int screenHeight = lcd.height();

  // Verfügbarer Platz (60px für Header)
  int availableHeight = screenHeight - 60;
  int maxSize = min(screenWidth - 40, availableHeight);

  // Spielfeld-Größe (auf Grid abrunden)
  FIELD_SIZE = (maxSize / GRID_SIZE) * GRID_SIZE;
  TILE_SIZE = FIELD_SIZE / GRID_SIZE;
  FRAME_WIDTH = TILE_SIZE / 8;

  // Zentrierung
  FIELD_X = (screenWidth - FIELD_SIZE - FRAME_WIDTH * 2) / 2;
  FIELD_Y = 60 + (availableHeight - FIELD_SIZE - FRAME_WIDTH * 2) / 2;
}
```

**Ergebnis:**
- 2.8" Display (320x240): TILE_SIZE ≈ 50px, FIELD_SIZE = 200px
- 3.5" Display (480x320): TILE_SIZE ≈ 75px, FIELD_SIZE = 300px
- Perfekt zentriert auf beiden Displays

## 🚀 Installation & Verwendung

### 1. Arduino IDE vorbereiten

**Board-Manager:**
```
1. Arduino IDE öffnen
2. File -> Preferences
3. Additional Board Manager URLs:
   https://dl.espressif.com/dl/package_esp32_index.json
4. Tools -> Board -> Boards Manager
5. Suche "ESP32" -> Install "ESP32 by Espressif Systems"
```

**Library installieren:**
```
1. Tools -> Manage Libraries
2. Suche "LovyanGFX" -> Install
3. Schließe Library Manager
```

### 2. CYD_Display_Config.h einrichten

Die MyLGFXConfigs Library enthält die CYD_Display_Config.h:

```bash
# Terminal / CMD:
cd ~/Arduino/libraries/
git clone https://github.com/[username]/MyLGFXConfigs.git
```

**Oder manuell:**
1. Lade MyLGFXConfigs.zip herunter
2. Entpacke in Arduino/libraries/
3. Arduino IDE neu starten

### 3. Code hochladen

**Board-Einstellungen:**
```
Board: "ESP32 Dev Module"
Upload Speed: 921600
CPU Frequency: 240MHz
Flash Frequency: 80MHz
Flash Mode: QIO
Flash Size: 4MB (32Mb)
Partition Scheme: "Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)"
Core Debug Level: "None"
Port: [Dein CYD USB-Port]
```

**Hochladen:**
```
1. Öffne 01_Puzzle_15.ino
2. Klicke Upload (Pfeil-Symbol)
3. Warte bis "Done uploading"
4. Genieße das Spiel!
```

### 4. Erste Schritte

**Nach dem Upload:**
1. Display zeigt kurz gelöstes Puzzle
2. Puzzle wird automatisch gemischt (200 Züge)
3. Tippe auf Kacheln neben dem leeren Feld
4. Beobachte die flüssigen Animationen
5. Löse das Puzzle!

### 5. Optional: Button-Steuerung

**In CYD_Display_Config.h:**
```cpp
// Option 1: GPIO Buttons
#define gpioSwitch
#define tasteA 17
#define tasteB 5
#define tasteC 16
#define tasteD 18

// Option 2: I2C Buttons (PCF8574)
#define i2cSwitch
#define tasteA 0
#define tasteB 1
#define tasteC 2
#define tasteD 3
#define pcfAddress 0x20
```

**Hardware:**
- 4x Taster zwischen GPIO und GND
- Optional: Pull-up Widerstände (10kΩ)
- Oder: PCF8574 I2C Port Expander

### Serielle Ausgabe

**Öffne Serial Monitor (115200 Baud):**
```
=== CYD Spiel 01: 15-Puzzle ===
Display: 320x240
Field: 200x200 at (60,60)
Tile: 50x50, Frame: 6
Setup abgeschlossen!
Viel Spaß beim Puzzeln!
Puzzle gemischt!

[... Spielen ...]

GEWONNEN in 127 Zügen! (4:23)
```

## 🔧 Troubleshooting

### Problem: Display bleibt schwarz

**Symptome:**
- Kein Bild nach Upload
- Serielle Ausgabe funktioniert

**Lösungen:**
1. **Rotation prüfen**: Ändere `lcd.setRotation(1)` zu `0`, `2` oder `3`
2. **Display-Typ prüfen**: CYD_Display_Config.h für dein Board anpassen
3. **Power-Cycle**: USB-Kabel trennen, 10 Sekunden warten, neu verbinden
4. **Flash löschen**:
   ```
   Tools -> Erase Flash: "All Flash Contents"
   Neu hochladen
   ```

### Problem: Touch funktioniert nicht

**Symptome:**
- Kacheln lassen sich nicht antippen
- Display zeigt Grafik korrekt

**Lösungen:**
1. **Touch-Kalibrierung prüfen**:
   ```cpp
   // In setup() nach lcd.init():
   uint16_t x, y;
   if (lcd.getTouch(&x, &y)) {
     Serial.printf("Touch: %d, %d\n", x, y);
   }
   ```
2. **Touch-Controller-Typ**: In CYD_Display_Config.h korrigieren (XPT2046 vs. FT6336)
3. **Pin-Konfiguration**: Touch-CS Pin prüfen (meist GPIO 33)
4. **Debounce-Zeit erhöhen**: `TOUCH_DEBOUNCE` von 200 auf 500 ändern

### Problem: Langsame/ruckelige Animationen

**Symptome:**
- Kacheln bewegen sich nicht flüssig
- Animation stockt

**Lösungen:**
1. **Animations-Geschwindigkeit reduzieren**:
   ```cpp
   const float ANIM_SPEED = 0.10;  // Statt 0.15
   ```
2. **Delay erhöhen**:
   ```cpp
   delay(20);  // Statt 16 in updateAnimation()
   ```
3. **CPU-Frequenz prüfen**: Stelle sicher, dass 240MHz eingestellt ist
4. **Grafik vereinfachen**: Kommentiere 3D-Kanten aus (in drawTile())

### Problem: Puzzle ist unlösbar

**Symptome:**
- Nach langem Spielen keine Lösung möglich
- Fehler in serieller Ausgabe

**Lösungen:**
1. **Sollte nicht passieren!** Unser Shuffle garantiert Lösbarkeit
2. **Debug-Ausgabe prüfen**:
   ```
   WARNUNG: Puzzle nicht lösbar! Nochmal mischen...
   ```
   Wenn diese Meldung erscheint: Bug in shufflePuzzle()
3. **Neustart**: Button A (oder Touch auf "Tippe zum Neustart")
4. **Code-Update**: Aktuellste Version von GitHub laden

### Problem: Speicher-Fehler / Absturz

**Symptome:**
- ESP32 startet neu
- "Guru Meditation Error" in Serial Monitor

**Lösungen:**
1. **Partition Scheme ändern**:
   ```
   Tools -> Partition Scheme: "Huge APP (3MB No OTA/1MB SPIFFS)"
   ```
2. **Move-History reduzieren**:
   ```cpp
   const int MAX_HISTORY = 50;  // Statt 100
   ```
3. **Vector durch Array ersetzen**:
   ```cpp
   Move moveHistory[50];
   int historyCount = 0;
   ```

### Problem: Farben sehen falsch aus

**Symptome:**
- Kacheln haben seltsame Farben
- Blau sieht rot aus (oder umgekehrt)

**Lösungen:**
1. **RGB/BGR Modus**: In CYD_Display_Config.h ändern:
   ```cpp
   cfg.rgb_order = false;  // oder true
   ```
2. **Farbformat prüfen**: Stelle sicher, dass RGB565 verwendet wird
3. **Display-Typ**: ILI9341 vs. ILI9488 in Config prüfen

## 🎯 Erweiterungsideen

### 1. Verschiedene Puzzle-Größen

**Konzept**: 3x3, 4x4, 5x5 wählbar

**Implementierung:**
```cpp
// In Setup-Screen wählen
enum PuzzleSize {
  SIZE_3x3,   // 8 Kacheln (einfacher)
  SIZE_4x4,   // 15 Kacheln (klassisch)
  SIZE_5x5    // 24 Kacheln (schwierig!)
};

PuzzleSize currentSize = SIZE_4x4;

void initGame(PuzzleSize size) {
  GRID_SIZE = (size == SIZE_3x3) ? 3 :
              (size == SIZE_4x4) ? 4 : 5;

  TILE_COUNT = GRID_SIZE * GRID_SIZE;

  // Rest wie gehabt...
}
```

**Anwendungen:**
- 3x3 für Kinder oder Anfänger
- 4x4 als Standard (klassisch)
- 5x5 für Profis und Speedsolver

### 2. Bild-Puzzle-Modus

**Konzept**: Statt Zahlen ein Bild in Kacheln zerlegen

**Implementierung:**
```cpp
// Bild in PROGMEM speichern oder von SD-Karte laden
const uint16_t* puzzleImage;  // 240x240 Pixel

void drawTile(uint8_t pos) {
  if (tiles[pos] == 0) return;  // Leeres Feld

  // Berechne Quell-Position im Original-Bild
  int8_t targetX = (tiles[pos] - 1) % GRID_SIZE;
  int8_t targetY = (tiles[pos] - 1) / GRID_SIZE;

  // Kopiere Bild-Ausschnitt zur Kachel
  lcd.pushImage(px, py, TILE_SIZE, TILE_SIZE,
                &puzzleImage[targetY * IMAGE_WIDTH * TILE_SIZE +
                            targetX * TILE_SIZE]);

  // Optional: Zahl als Overlay
  lcd.drawNumber(tiles[pos], px + 10, py + 10, 2);
}
```

**Anwendungen:**
- Persönliche Fotos als Puzzle
- Logos oder Kunstwerke
- Lernspiel mit Bildern (für Kinder)

### 3. Automatischer Löser (AI)

**Konzept**: A* Algorithmus zur optimalen Lösung

**Implementierung:**
```cpp
#include <queue>
#include <unordered_set>

struct State {
  uint8_t tiles[16];
  int cost;           // Züge bisher (g)
  int heuristic;      // Geschätzte restliche Züge (h)

  int f() const { return cost + heuristic; }
};

int manhattanDistance(uint8_t tiles[]) {
  int sum = 0;
  for (int i = 0; i < 16; i++) {
    if (tiles[i] == 0) continue;

    int targetX = (tiles[i] - 1) % 4;
    int targetY = (tiles[i] - 1) / 4;
    int currentX = i % 4;
    int currentY = i / 4;

    sum += abs(targetX - currentX) + abs(targetY - currentY);
  }
  return sum;
}

void solvePuzzle() {
  std::priority_queue<State> openList;
  std::unordered_set<State> closedList;

  State start;
  memcpy(start.tiles, tiles, 16);
  start.cost = 0;
  start.heuristic = manhattanDistance(tiles);

  openList.push(start);

  while (!openList.empty()) {
    State current = openList.top();
    openList.pop();

    if (current.heuristic == 0) {
      // Lösung gefunden!
      reconstructPath(current);
      return;
    }

    // Alle Nachbarn generieren...
  }
}
```

**Anwendungen:**
- Lösungs-Assistent (zeigt nächsten Zug)
- Auto-Solve mit Animation
- Optimale Lösung für Speedrun-Vergleich

### 4. Multiplayer-Modus

**Konzept**: Zwei Spieler lösen dasselbe Puzzle gleichzeitig

**Implementierung:**
```cpp
// Zwei CYDs via ESP-NOW verbunden
#include <esp_now.h>

struct GameState {
  uint8_t tiles[16];
  uint16_t moveCount;
  uint32_t timeMs;
};

void onDataReceived(const uint8_t* mac, const uint8_t* data, int len) {
  GameState* otherPlayer = (GameState*)data;

  // Zeige Fortschritt des Gegners
  drawOpponentProgress(otherPlayer);
}

void drawOpponentProgress(GameState* state) {
  // Kleines Vorschau-Fenster zeigen
  lcd.drawString("Gegner:", 10, lcd.height() - 40, 2);
  lcd.drawString(String(state->moveCount) + " Züge",
                 10, lcd.height() - 20, 2);
}

void sendGameState() {
  GameState myState;
  memcpy(myState.tiles, tiles, 16);
  myState.moveCount = moveCount;
  myState.timeMs = gameDuration;

  esp_now_send(partnerMAC, (uint8_t*)&myState, sizeof(myState));
}
```

**Anwendungen:**
- Wettrennen: Wer löst zuerst?
- Kooperativ: Gemeinsam an Strategie arbeiten
- Online-Rangliste via WiFi

### 5. Sound-Effekte

**Konzept**: Akustisches Feedback bei Zügen und Gewinn

**Implementierung:**
```cpp
// Piezo-Buzzer an GPIO Pin
#define BUZZER_PIN 25

void playMoveSound() {
  tone(BUZZER_PIN, 800, 50);  // 800Hz für 50ms
}

void playWinSound() {
  int melody[] = {523, 587, 659, 698, 784, 880, 988, 1047};
  for (int i = 0; i < 8; i++) {
    tone(BUZZER_PIN, melody[i], 200);
    delay(250);
  }
}

void playErrorSound() {
  tone(BUZZER_PIN, 200, 100);  // Tiefer Ton
  delay(150);
  tone(BUZZER_PIN, 150, 100);
}

// In moveTile():
void moveTile(uint8_t pos) {
  if (!canMove(pos)) {
    playErrorSound();
    return;
  }

  playMoveSound();
  // ... Rest des Codes
}
```

**Anwendungen:**
- Akustisches Feedback verbessert UX
- Melodien für Meilensteine (25, 50, 75 Züge)
- Verschiedene Töne für Undo, Shuffle, etc.

### 6. Schwierigkeitsgrade

**Konzept**: Verschiedene Shuffle-Intensitäten

**Implementierung:**
```cpp
enum Difficulty {
  EASY,     // 50 Züge Shuffle
  MEDIUM,   // 150 Züge Shuffle
  HARD,     // 300 Züge Shuffle
  INSANE    // 500 Züge Shuffle
};

void shufflePuzzle(Difficulty diff) {
  int shuffleCount = (diff == EASY) ? 50 :
                    (diff == MEDIUM) ? 150 :
                    (diff == HARD) ? 300 : 500;

  for (int i = 0; i < shuffleCount; i++) {
    // ... Shuffle-Logik
  }
}

// Zusätzlich: Schwierigkeits-basierte Features
void initGame(Difficulty diff) {
  shufflePuzzle(diff);

  if (diff == EASY) {
    showHints = true;          // Nächsten Zug vorschlagen
    allowUnlimitedUndo = true; // Unbegrenzt Undo
  } else if (diff == INSANE) {
    showHints = false;
    allowUndo = false;         // Kein Undo!
    timeLimit = 600;           // 10 Minuten Zeitlimit
  }
}
```

**Anwendungen:**
- Anfänger können mit Easy starten
- Fortgeschrittene wählen Hard oder Insane
- Zeitlimit-Modus für Speedrun-Challenges

### 7. Statistiken & Achievements

**Konzept**: Tracking von Leistungen und Belohnungen

**Implementierung:**
```cpp
#include <Preferences.h>

Preferences prefs;

struct Statistics {
  uint32_t gamesPlayed;
  uint32_t gamesWon;
  uint16_t bestScore;        // Wenigste Züge
  uint32_t bestTime;         // Schnellste Zeit (Sekunden)
  uint32_t totalMoves;
  uint32_t totalTime;
};

void saveStats(Statistics& stats) {
  prefs.begin("puzzle15", false);
  prefs.putUInt("gamesPlayed", stats.gamesPlayed);
  prefs.putUInt("gamesWon", stats.gamesWon);
  prefs.putUShort("bestScore", stats.bestScore);
  prefs.putUInt("bestTime", stats.bestTime);
  prefs.end();
}

void loadStats(Statistics& stats) {
  prefs.begin("puzzle15", true);
  stats.gamesPlayed = prefs.getUInt("gamesPlayed", 0);
  stats.gamesWon = prefs.getUInt("gamesWon", 0);
  stats.bestScore = prefs.getUShort("bestScore", 9999);
  stats.bestTime = prefs.getUInt("bestTime", 999999);
  prefs.end();
}

// Achievements
enum Achievement {
  FIRST_WIN,          // Erstes Puzzle gelöst
  SPEED_DEMON,        // Unter 3 Minuten
  EFFICIENT,          // Unter 100 Züge
  MASTER,             // 100 Puzzles gelöst
  PERFECTIONIST       // Optimale Lösung (80 Züge)
};

void checkAchievements() {
  if (stats.gamesWon == 1) {
    unlockAchievement(FIRST_WIN);
  }
  if (gameDuration < 180 && gameWon) {
    unlockAchievement(SPEED_DEMON);
  }
  // ... weitere Checks
}
```

**Anwendungen:**
- Langzeit-Motivation durch Achievements
- Vergleich mit eigenen Rekorden
- Rangliste (lokal oder online)
- Trophäen-Anzeige auf Menü-Screen

## 📚 Weiterführende Themen

### Algorithmische Komplexität

**Zustandsraum-Analyse:**

Das 15-Puzzle hat einen **diskreten Zustandsraum** mit definierten Zuständen und Übergängen:

```
Zustände: S = {alle möglichen Kachel-Anordnungen}
|S| = 16! = 20.922.789.888.000

Lösbare Zustände: S_solvable = |S| / 2 = 10.461.394.944.000

Übergänge: E = {(s1, s2) | s1 kann durch einen Zug zu s2 werden}
```

**Branching Factor:**

Durchschnittliche Anzahl möglicher Züge pro Zustand:
- Leeres Feld in Ecke: 2 Züge
- Leeres Feld am Rand: 3 Züge
- Leeres Feld in Mitte: 4 Züge

Durchschnitt: `(4×2 + 8×3 + 4×4) / 16 = 3.125`

**Tiefe des Zustandsraums:**

God's Number für 15-Puzzle: **80 Züge**
- Maximale Anzahl Züge zur optimalen Lösung
- Beweis durch exhaustive Suche (Computer)

**Vergleich mit anderen Puzzles:**

| Puzzle | Zustände | God's Number | Branching Factor |
|--------|----------|--------------|------------------|
| 8-Puzzle (3x3) | 181.440 | 31 | ~2.67 |
| 15-Puzzle (4x4) | 10.5 Billionen | 80 | ~3.13 |
| 24-Puzzle (5x5) | 7.76×10²⁴ | ~208 | ~3.52 |
| Rubik's Cube | 4.3×10¹⁹ | 20 | ~13.5 |

### Heuristiken für Löser-Algorithmen

#### Manhattan-Distanz

Summe der horizontalen und vertikalen Distanzen aller Kacheln zu ihren Zielpositionen:

```
h(n) = Σ |x_current - x_goal| + |y_current - y_goal|
```

**Eigenschaften:**
- **Admissible**: Unterschätzt niemals die tatsächliche Distanz
- **Consistent**: h(n) ≤ cost(n, n') + h(n')
- Garantiert optimale Lösung mit A*

**Beispiel:**
```
Aktuell:        Ziel:
+--+--+--+--+   +--+--+--+--+
| 5| 1| 3| 4|   | 1| 2| 3| 4|
+--+--+--+--+   +--+--+--+--+
| 2|  | 7| 8|   | 5| 6| 7| 8|
+--+--+--+--+   +--+--+--+--+
| 9|10|11|12|   | 9|10|11|12|
+--+--+--+--+   +--+--+--+--+
|13|14| 6|15|   |13|14|15|   |
+--+--+--+--+   +--+--+--+--+

Kachel 5: |0-0| + |0-1| = 1
Kachel 1: |1-0| + |0-0| = 1
Kachel 2: |0-0| + |1-1| = 0
Kachel 6: |2-1| + |3-1| = 3

Gesamt: h = 1 + 1 + 0 + 3 + ... = 10
```

#### Linear Conflict

Erweiterte Heuristik: Wenn zwei Kacheln in der richtigen Reihe/Spalte sind, aber in falscher Reihenfolge, mindestens 2 zusätzliche Züge nötig:

```
h_lc(n) = h_manhattan(n) + 2 × linear_conflicts(n)
```

**Beispiel:**
```
Zeile 1: [2, 1, 3, 4]
         ^  ^
         Konflikt! (2 und 1 beide in Zeile 1, aber falsche Reihenfolge)

Mindestens 2 zusätzliche Züge:
  1. Schiebe 2 raus
  2. Schiebe 2 wieder rein (nach 1)
```

#### Walking Distance

Fortgeschrittene Heuristik basierend auf Muster-Datenbanken:

```
WD = max(WD_rows, WD_cols)

WD_rows = Minimale Züge um alle Reihen zu sortieren
WD_cols = Minimale Züge um alle Spalten zu sortieren
```

**Eigenschaften:**
- Deutlich stärker als Manhattan (weniger Knoten expandiert)
- Benötigt Vorberechnung (Pattern Database)
- Kombiniert mit Manhattan: `h = WD + Manhattan`

### Permutations-Theorie

**Warum ist die Hälfte unlösbar?**

Das 15-Puzzle ist eine **Permutations-Gruppe** mit spezieller Struktur:

```
Gruppe G = Alle Permutationen von {1,2,...,15,leer}
Untergruppe H = Lösbare Permutationen

|G| = 16!
|H| = 16! / 2

H ist eine Untergruppe vom Index 2 in G
```

**Parität-Argument:**

Jeder Zug (horizontal oder vertikal) erhält die **Parität** der Permutation:
- Horizontaler Zug: Keine Paritäts-Änderung (gerade Anzahl Transpositionen)
- Vertikaler Zug: Ändert Parität des Zustands durch Zeilen-Offset

**Signatur einer Permutation:**

```
sign(π) = (-1)^(Anzahl Inversionen)

sign(π) = +1  →  gerade Permutation  →  lösbar
sign(π) = -1  →  ungerade Permutation  →  unlösbar
```

**14-15 Puzzle (Sam Loyd's Herausforderung):**

```
+--+--+--+--+
| 1| 2| 3| 4|
+--+--+--+--+
| 5| 6| 7| 8|
+--+--+--+--+
| 9|10|11|12|
+--+--+--+--+
|13|15|14|   |  <- 14 und 15 vertauscht!
+--+--+--+--+
```

Diese Konfiguration hat **ungerade Parität** → **UNLÖSBAR!**

Sam Loyd's $1.000 Preis war **sicher** - niemand konnte es jemals lösen!

## 🛠️ Praktische Tipps

### Lösungs-Strategien

**1. Layer-by-Layer Methode:**
```
Schritt 1: Obere Reihe (1-4)
Schritt 2: Linke Spalte (5, 9, 13)
Schritt 3: Zweite Reihe (6, 7, 8)
Schritt 4: 3x3 Rest (Standard 8-Puzzle)
```

**2. Corner-First Strategie:**
```
Schritt 1: Ecken platzieren (1, 4, 13, 15)
Schritt 2: Kanten platzieren (2, 3, 5, 8, 12, 14)
Schritt 3: Zentrum sortieren
```

**3. Makros für häufige Muster:**
```
3-Cycle (Rotiere 3 Kacheln):
  R U R' U'  (Notation wie Rubik's Cube)

4-Cycle:
  R U R' U' L' U' L U
```

### Code-Optimierungen

**1. Speicher sparen:**
```cpp
// Statt std::vector<Move> (dynamischer Speicher)
Move moveHistory[50];  // Fixer Array
uint8_t historyCount = 0;

void addMove(Move m) {
  if (historyCount < 50) {
    moveHistory[historyCount++] = m;
  }
}
```

**2. Schnelleres Rendering:**
```cpp
// Nutze Sprites für Kacheln (nur einmal zeichnen, dann kopieren)
LGFX_Sprite tileSprite[15];

void setup() {
  for (int i = 0; i < 15; i++) {
    tileSprite[i].createSprite(TILE_SIZE, TILE_SIZE);
    drawTileToSprite(tileSprite[i], i + 1);
  }
}

void drawTile(uint8_t pos) {
  if (tiles[pos] == 0) return;
  tileSprite[tiles[pos] - 1].pushSprite(px, py);
}
```

**3. Dirty Rectangles:**
```cpp
// Zeichne nur geänderte Bereiche
void drawTile(uint8_t pos, bool forceRedraw = false) {
  if (!forceRedraw && tiles[pos] == lastTiles[pos]) {
    return;  // Nicht geändert
  }

  // ... zeichnen
  lastTiles[pos] = tiles[pos];
}
```

### Debug-Tipps

**1. Visualisiere Inversionen:**
```cpp
void debugInversions() {
  int inv = 0;
  for (int i = 0; i < 16; i++) {
    if (tiles[i] == 0) continue;
    for (int j = i + 1; j < 16; j++) {
      if (tiles[j] == 0) continue;
      if (tiles[i] > tiles[j]) {
        Serial.printf("Inversion: tiles[%d]=%d > tiles[%d]=%d\n",
                     i, tiles[i], j, tiles[j]);
        inv++;
      }
    }
  }
  Serial.printf("Total inversions: %d\n", inv);
}
```

**2. Trace Züge:**
```cpp
void moveTile(uint8_t pos) {
  Serial.printf("[Move %d] Tile %d: %d -> %d\n",
               moveCount + 1, tiles[pos], pos, emptyPos);

  // ... Move-Logik
}
```

**3. Performance-Messung:**
```cpp
unsigned long frameTime = 0;

void updateAnimation() {
  unsigned long start = micros();

  // ... Animation

  frameTime = micros() - start;
  if (frameTime > 20000) {  // > 20ms = < 50 FPS
    Serial.printf("WARNING: Slow frame: %lu us\n", frameTime);
  }
}
```

## 📖 Ressourcen & Links

### Wissenschaftliche Arbeiten

- **[The 15 Puzzle - Wikipedia](https://en.wikipedia.org/wiki/15_puzzle)**
  - Umfassende Historie und Mathematik

- **[Solvability of the Tiles Game (PDF)](http://www.cs.bham.ac.uk/~mdr/teaching/modules04/java2/TilesSolvability.html)**
  - Mathematischer Beweis der Lösbarkeit

- **[A* Algorithm for 15-Puzzle](https://algorithmsinsight.wordpress.com/graph-theory-2/a-star-in-general/implementing-a-star-to-solve-n-puzzle/)**
  - Implementierung von Löser-Algorithmen

### CYD-Spezifisch

- **CYD Display Config**: `Arduino/libraries/MyLGFXConfigs/CYD_Display_Config.h`
- **Pin-Definitionen**:
  - I2C: extSDA (GPIO 22), extSCL (GPIO 27)
  - RGB LED: rgbLedR (GPIO 4), rgbLedG (GPIO 16), rgbLedB (GPIO 17)
  - Touch CS: GPIO 33
- **LovyanGFX Dokumentation**: [GitHub](https://github.com/lovyan03/LovyanGFX)

### Verwandte CYD-Academy Beispiele

- **[03_TouchDemo](../../examples/03_TouchDemo/)**: Touch-Grundlagen
- **[06_BouncingBall](../../examples/06_BouncingBall/)**: Animations-Techniken
- **[09_Calculator](../../examples/09_Calculator/)**: Touch-UI Design
- **[13_Racing_Track](../../examples/13_Racing_Track/)**: Komplexe Grafik und Animationen

### Tools & Solver

- **[15 Puzzle Optimal Solver (Online)](http://kociemba.org/themen/fifteen/fifteensolver.html)**
  - Optimale Lösungen berechnen

- **[IDA* Solver Implementation](https://github.com/speix/15-puzzle-solver)**
  - Open-Source Löser in Python

## 🎓 Lernziele

Nach Abschluss dieses Projekts solltest du:

- ✅ **Spiellogik implementieren können**: Zustands-Management, Regelvalidierung, Win-Conditions
- ✅ **3D-Grafik-Effekte verstehen**: Schatten, Highlights, Kanten-Rendering, Farbverläufe
- ✅ **Flüssige Animationen erstellen**: Interpolation, Easing-Funktionen, Frame-Timing
- ✅ **Touch-Eingabe verarbeiten**: Koordinaten-Mapping, Debouncing, Event-Handling
- ✅ **Mathematische Algorithmen anwenden**: Inversions-Check, Lösbarkeits-Beweis, Manhattan-Distanz
- ✅ **Auto-Scaling implementieren**: Responsive Design für verschiedene Display-Größen
- ✅ **Speicher-effizient programmieren**: Arrays vs. Vectors, Speicher-Optimierung
- ✅ **Code-Struktur organisieren**: Modularisierung, Helper-Funktionen, Forward Declarations
- ✅ **Debugging-Techniken anwenden**: Serielle Ausgabe, Visualisierung, Performance-Messung
- ✅ **Benutzer-Erfahrung optimieren**: Feedback, Statistiken, Undo-Funktion

## 🔜 Nächste Schritte

### Weitere Spiele in der Games-Kategorie

Nachdem du das 15-Puzzle gemeistert hast, probiere weitere Denksport-Klassiker:

- **Spiel 02: Sudoku** - Logik-Puzzle mit Zahlen 1-9
- **Spiel 03: 2048** - Verschmelze Kacheln zu höheren Zahlen
- **Spiel 04: Minesweeper** - Klassisches Minen-Such-Spiel
- **Spiel 05: Sokoban** - Schiebe Kisten auf Zielpositionen
- **Spiel 06: Lights Out** - Schalte alle Lichter aus

### Arcade Games

Für actionreiche Spiele siehe die **[Arcade Games Sammlung](../../arcade-games/)**:
- Pong, Snake, Breakout, Space Invaders, Asteroids, Frogger, Tetris
- Jeweils in Retro und Modern Versionen!

### Advanced Projekte

- **[Sensor-Beispiele](../../sensor-examples/)**: Hardware-Integration mit I2C/SPI-Sensoren
- **[ESP-NOW Communication](../../examples/12_CYD_to_CYD_Communication/)**: Drahtlose Kommunikation zwischen CYDs

---

**Viel Erfolg beim Puzzeln! 🧩**

Bei Fragen: CYD-Academy GitHub Issues

*Entwickelt mit ❤️ für die CYD-Academy*
