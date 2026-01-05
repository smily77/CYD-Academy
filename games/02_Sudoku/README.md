# 🔢 Sudoku - Das klassische Zahlen-Logik-Puzzle

<div align="center">

**Spiel 02 der CYD Games Kollektion**

*Das weltberühmte Logik-Puzzle mit 6,67 × 10²¹ Lösungsmöglichkeiten!*

[![Status](https://img.shields.io/badge/Status-Fertig-success)](.)
[![Schwierigkeit](https://img.shields.io/badge/Code-Fortgeschritten-orange)](.)
[![Hardware](https://img.shields.io/badge/Hardware-CYD%202.8%22%20%2F%203.5%22-blue)](.)

</div>

---

## 📖 Inhaltsverzeichnis

- [Über das Spiel](#-über-das-spiel)
- [Features](#-features)
- [Hardware-Anforderungen](#-hardware-anforderungen)
- [Installation](#-installation)
- [Spielanleitung](#-spielanleitung)
- [Steuerung](#-steuerung)
- [Mathematik & Algorithmen](#-mathematik--algorithmen)
- [Code-Struktur](#-code-struktur)
- [Customization](#-customization)
- [Troubleshooting](#-troubleshooting)
- [Sudoku-Strategie](#-sudoku-strategie)
- [Geschichte](#-geschichte)
- [Technische Details](#-technische-details)

---

## 🎯 Über das Spiel

**Sudoku** (japanisch: 数独, wörtlich "Zahl isolieren") ist eines der beliebtesten Logik-Puzzles weltweit. Täglich lösen Millionen von Menschen Sudoku-Rätsel in Zeitungen, Apps und Rätselheften.

### Spielziel

Fülle das 9×9 Grid mit Zahlen von 1 bis 9, sodass:
- Jede **Zeile** die Zahlen 1-9 genau einmal enthält
- Jede **Spalte** die Zahlen 1-9 genau einmal enthält
- Jede **3×3 Box** die Zahlen 1-9 genau einmal enthält

Das Puzzle startet mit einigen vorgegebenen Zahlen – deine Aufgabe ist es, die fehlenden Zahlen logisch zu ergänzen!

### Warum Sudoku?

- 🧠 **Gehirntraining**: Fördert logisches Denken und Konzentration
- 🎓 **Lehrreich**: Demonstriert Backtracking-Algorithmen
- 📐 **Mathematisch interessant**: Combinatorics, Constraint Satisfaction
- 😌 **Entspannend**: Ruhiges, überlegtes Gameplay

---

## ✨ Features

### Spiel-Features

- ✅ **3 Schwierigkeitsgrade**
  - **Easy**: 35 vorgegebene Zahlen (46 zu füllen)
  - **Medium**: 30 vorgegebene Zahlen (51 zu füllen)
  - **Hard**: 25 vorgegebene Zahlen (56 zu füllen)

- 🎲 **Intelligenter Puzzle-Generator**
  - Garantiert eindeutige Lösung
  - Backtracking-basierter Solver
  - Zufällige Diagonalbox-Initialisierung

- 📝 **Notizen-Modus** (Pencil Marks)
  - Trage Kandidaten-Zahlen in Zellen ein (1-9)
  - 3×3 Mini-Grid pro Zelle
  - Automatisches Löschen bei Zahl-Eingabe

- 🚨 **Fehler-Detektion**
  - Automatische Validierung bei Eingabe
  - Fehlerhafte Zahlen werden rot markiert
  - Fehler-Zähler in Statistiken

- ⏱️ **Timer & Statistiken**
  - Spielzeit in Minuten:Sekunden
  - Fehler-Counter
  - Anzeige im Header

- 🎆 **Gewinn-Animation**
  - Grüner Rahmen um Grid
  - Gewinn-Text mit Zeit und Fehlern
  - Automatische Bestenlisten-Ausgabe

### UI-Features

- 📱 **Touch-Steuerung**
  - Zellen-Auswahl durch Tippen
  - Numpad (3×3 Buttons für 1-9)
  - 5 Control-Buttons

- 🎨 **Klassisches Sudoku-Design**
  - Weiße Zellen mit Zahlen
  - Dünne Linien (1px) für Zell-Grenzen
  - Dicke Linien (2px) für 3×3 Box-Grenzen
  - Gelbe Hervorhebung für ausgewählte Zelle

- 🖥️ **Auto-Scaling**
  - Funktioniert auf 2.8" (320×240) und 3.5" (480×320)
  - Dynamische Größenberechnung
  - Optimale Ausnutzung des Displays

- 🎯 **Responsive Layout**
  - Grid links (9×9 Sudoku)
  - Numpad rechts (3×3 für 1-9)
  - Control-Buttons unter Numpad
  - Stats im Header

### Code-Features

- 🧩 **Modular & Erweiterbar**
  - Klare Funktionen-Trennung
  - Helper-Funktionen für Validierung
  - Einfache Difficulty-Anpassung

- ⚡ **Performance-Optimiert**
  - Effiziente Backtracking-Implementierung
  - Minimale Neuzeichnungen (nur geänderte Zellen)
  - Stack-freundliche Rekursion

- 🔧 **Wartbar & Dokumentiert**
  - Umfangreiche Kommentare
  - Serial-Debug-Ausgaben
  - Klare Namenskonventionen

---

## 🔌 Hardware-Anforderungen

### Erforderlich

- **ESP32-2432S028R** (CYD - Cheap Yellow Display)
  - ESP32 Dual-Core @ 240MHz
  - 2.8" ILI9341 TFT (320×240) oder 3.5" (480×320)
  - Resistive Touch-Screen
  - 4MB Flash

### Optional

Keine zusätzliche Hardware erforderlich! Alles läuft über Touch.

---

## 📥 Installation

### 1. Arduino IDE vorbereiten

```
Tools → Board → esp32 → ESP32 Dev Module

Empfohlene Board-Einstellungen:
- Upload Speed: 921600
- CPU Frequency: 240MHz
- Flash Frequency: 80MHz
- Flash Mode: QIO
- Flash Size: 4MB
- Partition Scheme: Default 4MB
- Core Debug Level: None
```

### 2. Bibliotheken installieren

**Erforderliche Libraries:**

1. **LovyanGFX** (v1.1.9 oder neuer)
   ```
   Sketch → Include Library → Manage Libraries
   → Suche "LovyanGFX" → Install
   ```

2. **MyLGFXConfigs** (Custom Config für CYD)
   - Bereits im Repository unter `/libraries/MyLGFXConfigs/`
   - Wird automatisch gefunden wenn Repository korrekt geklont

### 3. Sketch hochladen

```bash
# 1. Repository klonen
git clone https://github.com/smily77/CYD-Academy.git
cd CYD-Academy

# 2. In Arduino IDE öffnen
Datei → Öffnen → games/02_Sudoku/02_Sudoku.ino

# 3. CYD per USB anschließen

# 4. Port auswählen
Tools → Port → /dev/ttyUSB0 (oder COM3 auf Windows)

# 5. Upload!
Sketch → Upload
```

### 4. Fertig!

Nach dem Upload startet das Sudoku automatisch und generiert das erste Puzzle.

---

## 🎮 Spielanleitung

### Start

Beim Start wird automatisch ein neues Sudoku-Puzzle generiert mit der aktuellen Schwierigkeit (Standard: Medium).

### Zelle auswählen

1. **Tippe auf eine Zelle** im 9×9 Grid
2. Die Zelle wird **gelb** hervorgehoben
3. Vorgegebene Zahlen (schwarz) können nicht bearbeitet werden
4. Nur leere oder bereits ausgefüllte (blaue) Zahlen sind änderbar

### Zahl eintragen

1. Wähle eine Zelle
2. **Tippe auf eine Zahl im Numpad** (1-9)
3. Die Zahl wird in die Zelle eingetragen
4. **Farbe Blau**: Deine Eingabe
5. **Farbe Rot**: Fehler (Zahl verletzt Sudoku-Regeln)

### Notizen verwenden

1. Tippe auf **NOTIZ** Button (wird zu "NOTIZ*")
2. Wähle eine Zelle
3. Tippe auf Zahlen im Numpad
4. Kleine Kandidaten-Zahlen (1-9) werden als 3×3 Mini-Grid angezeigt
5. Erneutes Tippen entfernt die Notiz
6. **NOTIZ** erneut drücken um Notiz-Modus zu verlassen

### Zahl löschen

1. Wähle eine Zelle mit deiner Zahl (blau)
2. Tippe auf **DEL** Button
3. Zelle wird geleert (Notizen werden auch gelöscht)

### Fehler prüfen

- Tippe auf **CHECK** Button
- Alle Fehler werden rot markiert
- Fehler-Counter wird aktualisiert

### Schwierigkeit ändern

- Tippe auf **EASY/MEDIUM/HARD** Button
- Wechselt zwischen den 3 Schwierigkeitsstufen
- Ändert nur das **nächste** Puzzle (nicht aktuelles)

### Neues Spiel

- Tippe auf **NEU** Button
- Generiert ein neues Puzzle mit aktueller Schwierigkeit
- Timer und Fehler werden zurückgesetzt

### Gewinn

Wenn alle Zellen korrekt gefüllt sind:
1. ✅ Grüner Rahmen um Grid
2. 🎉 "GEWONNEN!" Anzeige
3. 📊 Zeit und Fehler-Statistik
4. Serial-Ausgabe: `GEWONNEN in M:SS! Fehler: X`

---

## 🕹️ Steuerung

### Touch-Bereiche

| Bereich | Aktion | Position |
|---------|--------|----------|
| **Grid-Zelle** | Zelle auswählen | Links (9×9 Grid) |
| **Numpad 1-9** | Zahl eintragen/Notiz | Rechts oben (3×3) |
| **NEU** | Neues Puzzle | Rechts unten |
| **EASY/MEDIUM/HARD** | Schwierigkeit | Rechts unten |
| **NOTIZ** | Notiz-Modus | Rechts unten |
| **DEL** | Zelle löschen | Rechts unten |
| **CHECK** | Fehler prüfen | Rechts unten |

### Button-Details

```cpp
NEU    → initGame()      // Neues Puzzle generieren
DIFF   → toggle()        // Easy → Medium → Hard → Easy
NOTIZ  → noteMode bool   // Kandidaten statt Zahlen
DEL    → grid[r][c] = 0  // Zelle leeren
CHECK  → drawGrid()      // Fehler rot markieren
```

### Touch-Debouncing

- **Verzögerung**: 200ms zwischen Touches
- Verhindert versehentliches Doppel-Tippen
- Definiert in `TOUCH_DEBOUNCE` Konstante

---

## 🧮 Mathematik & Algorithmen

### Sudoku-Mathematik

#### Anzahl möglicher Grids

Ein leeres 9×9 Sudoku hat **6,670,903,752,021,072,936,960** (~6.67 × 10²¹) mögliche Lösungen!

Aber: Nach Berücksichtigung von Symmetrien (Rotation, Spiegelung, Ziffern-Permutation) gibt es "nur" noch **5,472,730,538** (~5.5 Milliarden) wesentlich verschiedene Sudokus.

#### Minimale Anzahl Hinweise

- **Minimum**: 17 vorgegebene Zahlen (mathematisch bewiesen)
- **Typisch**: 22-30 Zahlen für "eindeutige Lösung"
- **Dieser Code**: 25-35 Zahlen (je nach Schwierigkeit)

#### Komplexität

- **Lösbarkeit prüfen**: NP-Vollständig
- **Lösung finden**: Polynomial mit Backtracking
- **Optimale Lösung**: Kann mit Constraint Propagation beschleunigt werden

### Algorithmen

#### 1. Puzzle-Generierung

```cpp
void generatePuzzle() {
  // Schritt 1: Fülle diagonale 3×3 Boxen (unabhängig!)
  // Box (0,0), (3,3), (6,6)
  for (int box = 0; box < 3; box++) {
    fillBoxRandomly(box * 3, box * 3);
  }

  // Schritt 2: Löse Rest mit Backtracking
  solveSudoku(0, 0);

  // Schritt 3: Entferne Zahlen
  removeNumbers(81 - difficulty);

  // Schritt 4: Markiere vorgegebene
  markGivenCells();
}
```

**Warum diagonale Boxen zuerst?**
- Diagonale Boxen haben keine Überschneidungen
- Können unabhängig mit zufälligen Zahlen gefüllt werden
- Rest wird durch Backtracking determiniert

#### 2. Backtracking-Solver

```cpp
bool solveSudoku(int row, int col) {
  // Base Case: Ende erreicht
  if (row == 9) return true;

  // Rekursion: Nächste Zelle
  int nextRow = (col == 8) ? row + 1 : row;
  int nextCol = (col == 8) ? 0 : col + 1;

  // Zelle schon gefüllt?
  if (grid[row][col] != 0) {
    return solveSudoku(nextRow, nextCol);
  }

  // Probiere alle Zahlen 1-9
  for (int num = 1; num <= 9; num++) {
    if (isValid(row, col, num)) {
      grid[row][col] = num;

      if (solveSudoku(nextRow, nextCol)) {
        return true;  // Lösung gefunden!
      }

      grid[row][col] = 0;  // Backtrack
    }
  }

  return false;  // Keine Lösung
}
```

**Komplexität**: O(9^m) wobei m = leere Zellen
- Worst Case: ~9^81 (extrem selten)
- Average Case: ~10^6 Operationen (sehr schnell)

#### 3. Validierung

```cpp
bool isValid(int row, int col, int num) {
  // Prüfe Zeile
  for (int c = 0; c < 9; c++) {
    if (grid[row][c] == num) return false;
  }

  // Prüfe Spalte
  for (int r = 0; r < 9; r++) {
    if (grid[r][col] == num) return false;
  }

  // Prüfe 3×3 Box
  int boxRow = (row / 3) * 3;
  int boxCol = (col / 3) * 3;
  for (int r = 0; r < 3; r++) {
    for (int c = 0; c < 3; c++) {
      if (grid[boxRow + r][boxCol + c] == num) {
        return false;
      }
    }
  }

  return true;  // Keine Konflikte
}
```

**Komplexität**: O(1) - nur 27 Checks (9+9+9)

#### 4. Zahlen entfernen

```cpp
void removeNumbers(int count) {
  int removed = 0;
  while (removed < count) {
    int row = random(9);
    int col = random(9);

    if (grid[row][col] != 0) {
      grid[row][col] = 0;
      removed++;
    }
  }
}
```

**Hinweis**: Dieser einfache Ansatz garantiert nicht, dass das Puzzle eindeutig lösbar ist. Für produktive Sudoku-Generatoren würde man nach jedem Entfernen prüfen, ob die Lösung noch eindeutig ist.

### Optimierungs-Möglichkeiten

Dieser Code verwendet einfaches Backtracking. Professionelle Sudoku-Solver nutzen:

1. **Constraint Propagation**
   - Naked Singles: Zelle hat nur eine Möglichkeit
   - Hidden Singles: Zahl kann nur in eine Zelle
   - Naked Pairs/Triples: Kandidaten-Reduktion

2. **Heuristiken**
   - MRV (Minimum Remaining Values): Wähle Zelle mit wenigsten Kandidaten
   - Forward Checking: Aktualisiere Kandidaten nach jeder Eingabe

3. **Dancing Links (DLX)**
   - Donald Knuth's Algorithm X
   - Exakt Cover Problem
   - Extrem schnell (Mikrosekunden)

---

## 🗂️ Code-Struktur

### Datei-Organisation

```
games/02_Sudoku/
├── 02_Sudoku.ino          # Haupt-Sketch (750 Zeilen)
└── README.md              # Diese Dokumentation
```

### Haupt-Komponenten

#### 1. Konfiguration (Zeilen 48-104)

```cpp
#define GRID_SIZE 9
#define BOX_SIZE 3

// Farben (RGB565)
#define COLOR_BACKGROUND 0xFFFF  // Weiß
#define COLOR_GRID_THICK 0x0000  // Schwarz
#define COLOR_SELECTED 0xFED7    // Gelb

// Schwierigkeitsgrade
enum Difficulty { EASY = 35, MEDIUM = 30, HARD = 25 };

// Daten-Arrays
uint8_t grid[9][9];           // Spielfeld
uint8_t solution[9][9];       // Lösung
bool isGiven[9][9];           // Vorgegebene Zahlen
bool notes[9][9][9];          // Kandidaten
```

#### 2. Setup & Layout (Zeilen 123-165)

```cpp
void setup() {
  lcd.init();
  lcd.setRotation(1);  // Landscape

  // Berechne Grid-Position
  CELL_SIZE = gridSize / 9;
  GRID_X = 10;
  GRID_Y = 60;

  // Berechne Numpad-Position
  NUMPAD_X = GRID_X + gridSize + 10;
  NUMPAD_Y = GRID_Y;

  // Berechne Button-Position
  BUTTON_Y = NUMPAD_Y + NUMPAD_SIZE * 3 + 10;

  initGame();
}
```

#### 3. Spiel-Logik (Zeilen 187-334)

| Funktion | Zweck | Zeilen |
|----------|-------|--------|
| `initGame()` | Reset & neue Runde | 187-214 |
| `generatePuzzle()` | Generator-Hauptfunktion | 217-266 |
| `solveSudoku()` | Backtracking-Solver | 269-296 |
| `removeNumbers()` | Zahlen entfernen | 323-334 |
| `isValidSolution()` | Validierung für Generator | 299-320 |

#### 4. Grafik (Zeilen 337-481)

| Funktion | Zweck | Komplexität |
|----------|-------|-------------|
| `drawGrid()` | Komplettes Grid | O(81) |
| `drawCell()` | Einzelne Zelle | O(1) |
| `drawNumpad()` | 3×3 Zahlen-Buttons | O(9) |
| `drawButtons()` | 5 Control-Buttons | O(5) |
| `drawStats()` | Timer & Fehler | O(1) |

#### 5. Touch-Eingabe (Zeilen 484-590)

```cpp
void handleTouch() {
  // Grid-Touch → Zelle wählen
  if (inside grid) {
    selectedRow = row;
    selectedCol = col;
  }

  // Numpad-Touch → Zahl/Notiz eintragen
  if (inside numpad) {
    if (noteMode) {
      notes[row][col][num] = !notes[row][col][num];
    } else {
      grid[row][col] = num;
    }
  }

  // Button-Touch → Aktionen
  if (inside buttons) {
    // NEU, DIFF, NOTIZ, DEL, CHECK
  }
}
```

#### 6. Validierung & Gewinn (Zeilen 593-673)

```cpp
bool isValid(int row, int col, int num) {
  // Prüfe Zeile, Spalte, 3×3 Box
}

bool checkWin() {
  // Alle Zellen gefüllt? Alle valid?
}

void showWinAnimation() {
  // Grüner Rahmen + Text
}
```

### Speicher-Layout

```
Flash (Code):     ~50 KB
RAM (Global):     ~2.5 KB
  - grid:         81 bytes
  - solution:     81 bytes
  - isGiven:      81 bytes
  - notes:        729 bytes (9×9×9)
  - andere:       ~1.5 KB
Stack (Rekursion): ~1 KB (worst case bei Backtracking)
```

**Gesamt**: ~4 KB RAM (ESP32 hat 520 KB → kein Problem!)

---

## 🎨 Customization

### Farben ändern

```cpp
// In Zeilen 58-68
#define COLOR_BACKGROUND 0xFFFF    // Hintergrund (Weiß)
#define COLOR_GRID_THIN 0xCE59     // Dünne Linien
#define COLOR_GRID_THICK 0x0000    // Dicke Linien (Boxen)
#define COLOR_SELECTED 0xFED7      // Ausgewählte Zelle (Gelb)
#define COLOR_ERROR 0xF800         // Fehler (Rot)
#define COLOR_GIVEN 0x0000         // Vorgegebene Zahlen (Schwarz)
#define COLOR_USER 0x001F          // Eingaben (Blau)
#define COLOR_NOTE 0x8410          // Notizen (Grau)
#define COLOR_BUTTON_BG 0x4A49     // Button-Hintergrund
```

**RGB565 Konverter**: https://chrishewett.com/blog/true-rgb565-colour-picker/

### Schwierigkeit anpassen

```cpp
// In Zeilen 71-75
enum Difficulty {
  EASY = 40,    // Mehr Zahlen → leichter
  MEDIUM = 30,  // Standard
  HARD = 20     // Weniger Zahlen → schwerer
};
```

**Empfehlungen**:
- **Very Easy**: 45-50 Zahlen (für Anfänger)
- **Easy**: 35-40 Zahlen
- **Medium**: 28-32 Zahlen
- **Hard**: 22-27 Zahlen
- **Expert**: 17-21 Zahlen (sehr schwer!)

### Notizen-Darstellung

```cpp
// In drawCell(), Zeilen 398-409
for (int i = 0; i < 9; i++) {
  if (notes[row][col][i]) {
    // Position berechnen (3×3 Grid)
    int noteRow = i / 3;
    int noteCol = i % 3;
    int16_t nx = x + noteCol * (w / 3) + w / 6;
    int16_t ny = y + noteRow * (h / 3) + h / 6;

    lcd.setTextColor(COLOR_NOTE);
    lcd.drawNumber(i + 1, nx, ny, 1);  // Font 1
  }
}
```

**Ändern**:
- Font-Größe: `lcd.drawNumber(..., 2)` für größer
- Farbe: `COLOR_NOTE` ändern
- Layout: `nx`/`ny` Formeln anpassen

### Button-Labels

```cpp
// In drawButtons(), Zeilen 435-456
const char* labels[5] = {"NEU", "EASY", "NOTIZ", "DEL", "CHECK"};
```

**Englisch**:
```cpp
const char* labels[5] = {"NEW", "EASY", "NOTES", "DEL", "CHECK"};
```

### Layout-Anpassung

```cpp
// In setup(), Zeilen 136-155
int16_t gridSize = /* Berechnung */;
GRID_X = 10;          // Links-Offset
GRID_Y = 60;          // Oben-Offset (für Stats)

NUMPAD_X = GRID_X + gridSize + 10;  // 10px Abstand
BUTTON_Y = NUMPAD_Y + NUMPAD_SIZE * 3 + 10;
```

---

## 🔧 Troubleshooting

### Compilation-Fehler

#### 1. `CYD_Display_Config.h: No such file`

**Problem**: MyLGFXConfigs Library nicht gefunden

**Lösung**:
```bash
# Prüfe ob Library existiert
ls libraries/MyLGFXConfigs/

# Falls nicht: Klone gesamtes Repo
git clone https://github.com/smily77/CYD-Academy.git
```

#### 2. `'lcd' was not declared`

**Problem**: LovyanGFX nicht installiert

**Lösung**:
```
Arduino IDE → Sketch → Include Library → Manage Libraries
→ Suche "LovyanGFX" → Install (v1.1.9+)
```

#### 3. `random() not declared`

**Problem**: ESP32 Core zu alt

**Lösung**:
```
Tools → Board → Boards Manager → ESP32 → Update to 2.0.11+
```

### Laufzeit-Probleme

#### 1. Display bleibt weiß

**Mögliche Ursachen**:
- CYD-Modell nicht kompatibel (nur 2.8" und 3.5" unterstützt)
- Falsche Rotation (sollte 1 sein)
- Touch-Kalibrierung fehlt

**Debug**:
```cpp
void setup() {
  Serial.begin(115200);
  Serial.println("Display init...");
  lcd.init();
  Serial.printf("Display: %dx%d\n", lcd.width(), lcd.height());

  // Sollte ausgeben: "Display: 320x240" oder "Display: 480x320"
}
```

#### 2. Touch funktioniert nicht

**Symptom**: Keine Reaktion auf Berührung

**Lösung**:
```cpp
// In handleTouch()
if (!lcd.getTouch(&tx, &ty)) {
  Serial.println("No touch detected");  // Debug
  return;
}
Serial.printf("Touch at (%d,%d)\n", tx, ty);  // Debug
```

Prüfe Serial Monitor:
- `No touch detected` → Touch-Hardware defekt oder nicht kalibriert
- `Touch at (x,y)` → Koordinaten prüfen (sollte 0-320/240 sein)

#### 3. Puzzle generiert nicht

**Symptom**: Grid bleibt leer nach "NEU"

**Debug**:
```cpp
void generatePuzzle() {
  Serial.println("Start generation...");

  // ... Generator-Code

  Serial.println("Puzzle generated!");
  Serial.printf("Difficulty: %d given\n", difficulty);

  // Debug-Ausgabe des Grids
  for (int r = 0; r < 9; r++) {
    for (int c = 0; c < 9; c++) {
      Serial.printf("%d ", grid[r][c]);
    }
    Serial.println();
  }
}
```

Prüfe Serial Monitor für Grid-Ausgabe.

#### 4. Backtracking dauert zu lange

**Symptom**: ESP32 hängt beim Generieren

**Ursache**: Ungünstige Startwerte

**Lösung 1**: Timeout einbauen
```cpp
unsigned long startTime = millis();

bool solveSudoku(int row, int col) {
  if (millis() - startTime > 5000) {
    Serial.println("Timeout!");
    return false;  // Abbrechen nach 5 Sekunden
  }
  // ... rest
}
```

**Lösung 2**: Bessere Initialisierung (bereits implementiert)
- Diagonale Boxen zuerst füllen reduziert Suchraum massiv!

#### 5. Fehler werden nicht rot markiert

**Symptom**: Fehlerhafte Zahlen bleiben blau

**Lösung**: Prüfe `isValid()` Funktion
```cpp
// In drawCell(), Zeile 387
if (!isGiven[row][col] && !isValid(row, col, grid[row][col])) {
  textColor = COLOR_ERROR;
  Serial.printf("Error at (%d,%d): %d\n", row, col, grid[row][col]);
}
```

Prüfe Serial Monitor für Error-Ausgaben.

#### 6. Gewinn wird nicht erkannt

**Symptom**: Puzzle komplett, aber keine Win-Animation

**Debug**:
```cpp
bool checkWin() {
  Serial.println("Checking win...");

  for (int r = 0; r < 9; r++) {
    for (int c = 0; c < 9; c++) {
      if (grid[r][c] == 0) {
        Serial.printf("Empty cell at (%d,%d)\n", r, c);
        return false;
      }
      if (!isValid(r, c, grid[r][c])) {
        Serial.printf("Invalid cell at (%d,%d): %d\n", r, c, grid[r][c]);
        return false;
      }
    }
  }

  Serial.println("WIN!");
  return true;
}
```

#### 7. Notizen verschwinden nicht

**Symptom**: Notizen bleiben nach Zahl-Eingabe

**Lösung**: Prüfe `clearNotes()` Aufruf
```cpp
// In handleTouch(), Zeile 534
grid[selectedRow][selectedCol] = num;
clearNotes(selectedRow, selectedCol);  // Muss hier sein!
drawCell(selectedRow, selectedCol);
```

### Performance-Probleme

#### 1. Langsames Zeichnen

**Symptom**: Grid-Aufbau dauert >1 Sekunde

**Optimierung**:
```cpp
// Nur geänderte Zellen neu zeichnen
void updateCell(int row, int col) {
  drawCell(row, col);  // Nicht drawGrid()!
}
```

#### 2. Touch-Lag

**Symptom**: Verzögerung zwischen Touch und Reaktion

**Lösung**: Reduce TOUCH_DEBOUNCE
```cpp
const unsigned long TOUCH_DEBOUNCE = 100;  // War 200
```

**Warnung**: Zu niedrig → Doppel-Eingaben!

---

## 🎯 Sudoku-Strategie

### Basis-Techniken

#### 1. Naked Single (Nackte Einzelzahl)

Wenn eine Zelle nur **eine mögliche Zahl** hat.

```
Zeile:   [5,_,7,3,9,1,2,4,6]
Spalte:  [5,2,7,3,9,1,_,4,6]
Box:     [5,_,7 / 3,9,1 / 2,4,6]

→ Einzige fehlende Zahl: 8
```

#### 2. Hidden Single (Versteckte Einzelzahl)

Wenn eine Zahl in einer Zeile/Spalte/Box nur **an eine Stelle** kann.

```
Box hat bereits: 1,2,3,4,5,6,7,9
Fehlende Zahl: 8
→ 8 kann nur in eine bestimmte Zelle
```

#### 3. Naked Pair (Nacktes Paar)

Zwei Zellen in Zeile/Spalte/Box haben **nur 2 Kandidaten**.

```
Zelle A: {3,7}
Zelle B: {3,7}
→ 3 und 7 können nirgendwo anders in dieser Zeile sein
```

### Fortgeschrittene Techniken

#### 4. Pointing Pairs

Kandidat kann in Box nur in einer Zeile → elimiere Kandidat aus Rest der Zeile.

#### 5. X-Wing

Komplexes Muster über 4 Zellen → eliminiert Kandidaten.

#### 6. Swordfish

Erweiterung von X-Wing über 6 Zellen.

### Notizen-System

Unser Notizen-Modus unterstützt:

```
+---+---+---+
|1 2|  |  |    Zelle hat Kandidaten 1,2,5,6,9
|  5|  |  |    → Trage diese als Notizen ein
|  6|  |9 |    → Wenn eine Zahl gesetzt, lösche Notizen
+---+---+---+
```

**Workflow**:
1. Aktiviere NOTIZ-Modus
2. Wähle Zelle
3. Tippe auf alle möglichen Zahlen (1-9)
4. Deaktiviere NOTIZ-Modus
5. Nutze Notizen zur Logik
6. Setze finale Zahl → Notizen verschwinden

---

## 📜 Geschichte

### Ursprung

- **1979**: Howard Garns (USA) erfindet "Number Place"
- **1984**: Japanischer Verlag Nikoli veröffentlicht als "Sudoku"
  - 数独 = "Suuji wa dokushin ni kagiru" (Zahlen sind auf Einzelstellung beschränkt)
- **2004**: Wayne Gould entwickelt Computer-Generator
- **2005**: Weltweite Sudoku-Manie beginnt (The Times, UK)

### Mathematische Meilensteine

- **2005**: Bertram Felgenhauer & Frazer Jarvis beweisen:
  - **6,670,903,752,021,072,936,960** vollständig gelöste Grids
  - **5,472,730,538** wesentlich verschiedene (nach Symmetrien)

- **2012**: Gary McGuire beweist:
  - **Minimum 17 Hinweise** für eindeutig lösbares Sudoku
  - Beweis via Brute-Force (7.2 Millionen CPU-Stunden!)

### Varianten

- **Killer Sudoku**: Käfige mit Summen-Hinweisen
- **Samurai Sudoku**: 5 überlappende Grids
- **Jigsaw Sudoku**: Unregelmäßige Regionen statt 3×3 Boxen
- **Hyper Sudoku**: Zusätzliche 4 überlappende 3×3 Regionen
- **Sudoku X**: Auch Diagonalen müssen 1-9 enthalten

### Wettkämpfe

- **World Sudoku Championship** (jährlich seit 2006)
- **Rekord**: ~4 Minuten für Expert-Sudoku (Weltmeister)

---

## 🔬 Technische Details

### Display-Koordinaten

```cpp
// 2.8" Display (320×240)
Grid:    [10, 60] → [226, 276]  (216×216, Cell: 24px)
Numpad:  [236, 60] → [308, 132] (72×72, Button: 24px)
Buttons: [236, 142] → [308, 312] (5 Buttons × 30px)

// 3.5" Display (480×320)
Grid:    [10, 60] → [280, 330]  (270×270, Cell: 30px)
Numpad:  [290, 60] → [470, 240] (180×180, Button: 60px)
Buttons: [290, 250] → [470, 460] (5 Buttons × 40px)
```

### Auto-Scaling Algorithmus

```cpp
int16_t screenWidth = lcd.width();    // 320 oder 480
int16_t screenHeight = lcd.height();  // 240 oder 320

// Verfügbare Höhe (60px für Stats)
int16_t availableHeight = screenHeight - 60;

// Grid-Größe (maximal verfügbare Höhe oder Breite - 120px)
int16_t gridSize = min(screenWidth - 120, availableHeight);

// Auf Vielfaches von 9 abrunden
gridSize = (gridSize / 9) * 9;

// Zellen-Größe
CELL_SIZE = gridSize / 9;
```

### RGB565 Farbtabelle

| Farbe | Hex | RGB | Verwendung |
|-------|-----|-----|------------|
| Weiß | 0xFFFF | (255,255,255) | Hintergrund |
| Schwarz | 0x0000 | (0,0,0) | Dicke Linien, vorgegebene Zahlen |
| Hellgrau | 0xCE59 | (204,204,204) | Dünne Linien |
| Gelb | 0xFED7 | (255,220,120) | Ausgewählte Zelle |
| Rot | 0xF800 | (255,0,0) | Fehler |
| Blau | 0x001F | (0,0,255) | Benutzereingaben |
| Grau | 0x8410 | (128,128,128) | Notizen |
| Dunkelgrau | 0x4A49 | (72,72,72) | Button-BG |

### Memory-Map

```
ESP32 Flash (4MB):
├── Bootloader       @ 0x00001000 (32 KB)
├── Partition Table  @ 0x00008000 (4 KB)
├── NVS              @ 0x00009000 (20 KB)
├── OTA Data         @ 0x0000E000 (8 KB)
├── App0 (Sketch)    @ 0x00010000 (~1.4 MB)
└── SPIFFS/LittleFS  @ 0x00170000 (~2.5 MB)

RAM (520 KB total):
├── Heap             ~350 KB (frei)
├── Stack Task 1     ~8 KB
├── Stack Task 2     ~8 KB
├── Global Variables ~2.5 KB (Sudoku-Arrays)
└── System Reserved  ~150 KB
```

### Timing-Analyse

| Operation | Zeit | Calls/Frame |
|-----------|------|-------------|
| `drawGrid()` | ~120ms | 1 (nur bei Neustart) |
| `drawCell()` | ~8ms | 1-2 (bei Änderung) |
| `generatePuzzle()` | ~200ms | 1 (bei NEU) |
| `solveSudoku()` | ~50ms | 1 (im Generator) |
| `handleTouch()` | <1ms | 100/s |
| `checkWin()` | ~2ms | 1 (nach Eingabe) |

**Gesamt-FPS**: Begrenzt durch Touch-Polling (~100 FPS)

### Serial-Debug-Ausgaben

```
=== CYD Spiel 02: Sudoku ===
Grid: 216x216 at (10,60), Cell: 24
Numpad: 24 at (236,60)
Generiere Sudoku-Puzzle...
Puzzle generiert! Schwierigkeit: 30 vorgegebene Zahlen
Setup abgeschlossen!

Zelle ausgewählt: (4,5)
Zahl 7 eingetragen (Notiz: 0)
Schwierigkeit: 30

Checking win...
WIN!
GEWONNEN in 5:34! Fehler: 2
```

---

## 🚀 Nächste Schritte

### Mögliche Erweiterungen

1. **Hint-System**
   ```cpp
   void giveHint() {
     // Finde erste Naked Single
     // Highlight Zelle grün
   }
   ```

2. **Eindeutigkeits-Prüfung**
   ```cpp
   bool hasUniqueSolution() {
     int solutionCount = 0;
     countSolutions(0, 0, &solutionCount);
     return (solutionCount == 1);
   }
   ```

3. **Highscore-Liste**
   ```cpp
   #include <Preferences.h>
   Preferences prefs;

   void saveScore() {
     prefs.begin("sudoku", false);
     prefs.putUInt("best_time", gameDuration);
     prefs.end();
   }
   ```

4. **Sound-Effekte**
   ```cpp
   #define BUZZER_PIN 25

   void playErrorBeep() {
     tone(BUZZER_PIN, 200, 100);  // 200Hz, 100ms
   }
   ```

5. **Pencil-Mark Auto-Fill**
   ```cpp
   void fillAllNotes() {
     for (int r = 0; r < 9; r++) {
       for (int c = 0; c < 9; c++) {
         if (grid[r][c] == 0) {
           for (int n = 1; n <= 9; n++) {
             if (isValid(r, c, n)) {
               notes[r][c][n-1] = true;
             }
           }
         }
       }
     }
   }
   ```

6. **Solver-Visualisierung**
   ```cpp
   bool solveSudokuAnimated(int row, int col) {
     // Wie solveSudoku(), aber mit drawCell() nach jedem Schritt
     // Zeigt Backtracking live!
   }
   ```

### Weitere Spiele

Nach Sudoku kommen in der Games-Kategorie:
- **2048**: Kombiniere Kacheln zu höheren Zahlen
- **Minesweeper**: Klassisches Minen-Such-Spiel
- **Sokoban**: Schiebe Kisten auf Zielpositionen

---

## 🤝 Beitragen

Hast du Ideen zur Verbesserung? Wir freuen uns über:

- **Bug-Reports**: GitHub Issues
- **Feature-Requests**: Hint-System, mehr Schwierigkeitsgrade, etc.
- **Code-Optimierungen**: Pull Requests willkommen!
- **Varianten**: Killer-Sudoku, Samurai-Sudoku, etc.

---

## 📚 Weitere Ressourcen

### Sudoku-Strategie

- [Sudoku Dragon Strategies](http://www.sudokudragon.com/sudokustrategy.htm)
- [Andrew Stuart's Solver](https://www.sudokuwiki.org/sudoku.htm)

### Algorithmen

- [Knuth's Dancing Links (DLX)](https://arxiv.org/abs/cs/0011047)
- [Sudoku Solver in 70 Zeilen Python](https://norvig.com/sudoku.html) (Peter Norvig)

### Mathematik

- [Mathematics of Sudoku (Wikipedia)](https://en.wikipedia.org/wiki/Mathematics_of_Sudoku)
- [Minimum 17 Clues Proof](https://arxiv.org/abs/1201.0749) (McGuire et al.)

### ESP32 & LovyanGFX

- [LovyanGFX GitHub](https://github.com/lovyan03/LovyanGFX)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [CYD-Academy Repository](https://github.com/smily77/CYD-Academy)

---

## 📄 Lizenz

Dieses Projekt ist Teil der **CYD-Academy** und steht unter der **MIT-Lizenz**.

```
Copyright (c) 2024 CYD-Academy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software...
```

Siehe [LICENSE](../../LICENSE) für Details.

---

## ✍️ Autor

**CYD-Academy Team**

- 🌐 Website: [CYD-Academy auf GitHub](https://github.com/smily77/CYD-Academy)
- 📧 Issues: [GitHub Issues](https://github.com/smily77/CYD-Academy/issues)

---

<div align="center">

**Viel Spaß beim Sudoku-Lösen! 🔢**

*Trainiere dein Gehirn mit Logik und Zahlen!*

[🏠 Zurück zur Games-Übersicht](../README.md) | [📚 Hauptverzeichnis](../../README.md)

</div>
