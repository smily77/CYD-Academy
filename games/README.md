# 🧩 CYD Games - Denksport-Klassiker

Willkommen in der **Games-Kategorie** der CYD Academy! Hier findest du zeitlose **Puzzle- und Denkspiele**, die durch beeindruckende Grafik, flüssige Animationen und intuitive Touch-Steuerung überzeugen.

## 🎯 Unterschied zu Arcade Games

Die Games-Kategorie konzentriert sich auf **Denksport und Logik**, während die [Arcade Games](../arcade-games/) actionreiche Reflexspiele bieten:

### 🧩 Games (diese Kategorie)
- **Fokus:** Logik, Strategie, Denken
- **Tempo:** Ruhig, überlegtes Spielen
- **Beispiele:** 15-Puzzle, Sudoku, 2048, Sokoban, Minesweeper
- **Ziel:** Problemlösung und Mustererkennung

### 🎮 Arcade Games
- **Fokus:** Reaktion, Koordination, Action
- **Tempo:** Schnell, hektisch
- **Beispiele:** Pong, Snake, Space Invaders, Tetris
- **Ziel:** Highscores und Geschicklichkeit

## 📦 Enthaltene Spiele

| # | Spiel | Status | Beschreibung |
|---|-------|--------|--------------|
| 01 | **[15-Puzzle](01_Puzzle_15/)** | ✅ Fertig | Das legendäre Schiebepuzzle seit 1874 |
| 02 | **Sudoku** | 🔜 Geplant | Logik-Rätsel mit Zahlen 1-9 |
| 03 | **2048** | 🔜 Geplant | Kombiniere Kacheln zu höheren Zahlen |
| 04 | **Minesweeper** | 🔜 Geplant | Klassisches Minen-Such-Spiel |
| 05 | **Sokoban** | 🔜 Geplant | Schiebe Kisten auf Zielpositionen |
| 06 | **Lights Out** | 🔜 Geplant | Schalte alle Lichter aus |
| 07 | **Nonogram** | 🔜 Geplant | Logik-Puzzle mit Zahlenhinweisen |
| 08 | **Tower of Hanoi** | 🔜 Geplant | Klassisches Turm-Puzzle |

## 🎮 Spiel 01: 15-Puzzle

### Über das Spiel

Das **15-Puzzle** (auch als "Fünfzehnerspiel", "Schiebepuzzle" oder "Ohne-Fleiß-kein-Preis-Spiel" bekannt) ist ein zeitloser Klassiker, der 1874 erfunden wurde und eine weltweite Puzzle-Manie auslöste!

**Ziel:** Ordne 15 nummerierte Kacheln (1-15) in aufsteigender Reihenfolge, indem du sie in das leere Feld schiebst.

### Features

- ✨ **Realistische 3D-Grafik**: Kacheln mit Schatten, Highlights und 3D-Kanten
- 🎬 **Glatte Animationen**: Flüssige Schiebe-Animationen mit Ease-in-out
- 🎨 **Farbverläufe**: 15 verschiedene Farben von Blau bis Orange
- 🪵 **Holzrahmen**: Authentischer Holzrahmen mit Maserung
- 🧠 **Intelligenter Shuffle**: Garantiert lösbares Puzzle durch Inversions-Check
- 📊 **Statistiken**: Züge-Zähler, Timer, Best Score
- ↩️ **Undo-Funktion**: Bis zu 100 Züge rückgängig machen
- 🎆 **Gewinn-Animation**: Beeindruckendes Feuerwerk beim Lösen
- 📱 **Touch-Steuerung**: Intuitive Touch-Bedienung
- 🖥️ **Auto-Scaling**: Funktioniert auf 2.8" und 3.5" Displays

### Steuerung

- **Touch**: Tippe auf eine Kachel neben dem leeren Feld
- **Button A** (optional): Neues Spiel
- **Button C** (optional): Undo
- **Button D** (optional): Pause

### Mathematik & Algorithmen

Das 15-Puzzle ist ein faszinierendes mathematisches Problem:
- **Zustandsraum**: 10,5 Billionen lösbare Konfigurationen
- **God's Number**: Maximal 80 Züge zur optimalen Lösung
- **Lösbarkeit**: Basiert auf Inversions-Parität
- **Heuristiken**: Manhattan-Distanz, Linear Conflict, Walking Distance

### Dokumentation

Für detaillierte Informationen siehe: [01_Puzzle_15/README.md](01_Puzzle_15/README.md)

## 🚀 Quick Start

### 1. Arduino IDE vorbereiten

```
1. ESP32 Board-Support installieren
2. LovyanGFX Library installieren
3. MyLGFXConfigs Library einrichten
```

### 2. Spiel hochladen

```
1. Öffne games/01_Puzzle_15/01_Puzzle_15.ino
2. Board: "ESP32 Dev Module"
3. Upload Speed: 921600
4. Upload!
```

### 3. Spielen!

Tippe auf die Kacheln neben dem leeren Feld und löse das Puzzle!

## 🎨 Design-Philosophie

### Grafik-Standards

Alle Spiele in der Games-Kategorie folgen hohen Grafik-Standards:

1. **Realistische 3D-Effekte**
   - Schatten unter Objekten
   - Highlights auf Kanten (hell oben-links)
   - Dunklere Kanten unten-rechts für Tiefe
   - Text-Schatten für bessere Lesbarkeit

2. **Farbverläufe**
   - Harmonische Farbpaletten
   - Gradienten für visuellen Appeal
   - Klare Kontraste für Lesbarkeit

3. **Flüssige Animationen**
   - Ease-in-out Interpolation
   - Mindestens 30 FPS
   - Smooth Bewegungen ohne Ruckeln

4. **Responsive Design**
   - Auto-Scaling für 2.8" (320x240) und 3.5" (480x320)
   - Dynamische Größenberechnung
   - Optimale Nutzung des verfügbaren Platzes

### Code-Standards

1. **Modular & Wartbar**
   - Klare Funktionen-Trennung
   - Helper-Funktionen für wiederverwendbaren Code
   - Kommentare für komplexe Algorithmen

2. **Performance-Optimiert**
   - Effiziente Grafik-Routinen
   - Minimale Neuzeichnungen (Dirty Rectangles wo möglich)
   - Speicher-Optimierung (Arrays statt Vectors wo sinnvoll)

3. **Benutzerfreundlich**
   - Intuitive Touch-Steuerung
   - Optionale Button-Unterstützung
   - Hilfreiches Feedback (visuell und optional akustisch)

## 🎓 Lernziele

Durch die Spiele in dieser Kategorie lernst du:

### Für Anfänger
- ✅ **Spiellogik implementieren**: Regeln, Validierung, Win-Conditions
- ✅ **Touch-Eingabe verarbeiten**: Koordinaten, Debouncing, Events
- ✅ **Grafik zeichnen**: Formen, Farben, Text, Bilder
- ✅ **Animationen erstellen**: Interpolation, Timing, Easing

### Für Fortgeschrittene
- ✅ **Algorithmen umsetzen**: Shuffle, Solver, Optimierung
- ✅ **State-Management**: Zustände speichern, Undo/Redo
- ✅ **Performance-Optimierung**: FPS, Speicher, Rendering
- ✅ **Mathematik anwenden**: Permutationen, Heuristiken, Komplexität

### Für Profis
- ✅ **AI implementieren**: A* Search, IDA*, Pattern Databases
- ✅ **Multiplayer**: ESP-NOW, WiFi, Synchronisation
- ✅ **Data Persistence**: EEPROM, Preferences, SD-Karte
- ✅ **Professioneller Code**: Struktur, Dokumentation, Testing

## 🔄 Geplante Erweiterungen

### Sudoku (Spiel 02)

**Konzept**: 9x9 Grid mit Zahlen 1-9, löse Logik-Rätsel

**Features**:
- Verschiedene Schwierigkeitsgrade (Easy, Medium, Hard, Expert)
- Hint-System für Anfänger
- Fehler-Detektion mit visuellem Feedback
- Timer und Move-Counter
- Puzzle-Generator mit garantiert eindeutiger Lösung
- Notiz-Modus (Kandidaten eintragen)

**Technisch**:
- Backtracking-Algorithmus für Generator
- Constraint-Propagation für Hint-System
- Touch-Grid mit Numpad

### 2048 (Spiel 03)

**Konzept**: Kombiniere Kacheln zu höheren Zahlen (2 → 4 → 8 → ... → 2048)

**Features**:
- 4x4 Grid mit Wisch-Steuerung
- Smooth Merge-Animationen
- Score-System mit Best Score
- "Undo" Funktion
- Verschiedene Themes (Klassisch, Neon, Pastell)
- Sound-Effekte beim Merge

**Technisch**:
- Merge-Algorithmus mit Animation
- Swipe-Detection (Touch-Drag)
- Spawn-Strategie (10% = 4, 90% = 2)

### Minesweeper (Spiel 04)

**Konzept**: Markiere alle Minen ohne sie aufzudecken

**Features**:
- Verschiedene Größen (9x9, 16x16, 16x30)
- Schwierigkeitsgrade (Beginner, Intermediate, Expert)
- Chord-Funktion (beide Tasten gleichzeitig)
- Timer und Minen-Counter
- Highscore-Liste
- Erste-Klick-Garantie (keine Mine)

**Technisch**:
- Minen-Verteilungs-Algorithmus
- Flood-Fill für leere Felder
- Touch-Gesten (Tap = aufdecken, Long-Press = flaggen)

### Sokoban (Spiel 05)

**Konzept**: Schiebe alle Kisten auf Zielpositionen

**Features**:
- 100+ klassische Level
- Level-Editor
- Undo/Redo unbegrenzt
- Move-Counter und Best Solution
- Verschiedene Grafik-Themes
- Animation beim Schieben

**Technisch**:
- Level-Format (XSB/SOK)
- Deadlock-Detektion
- Pfadfindung für Solver
- Touch-Steuerung: Tippe Zielfeld

### Lights Out (Spiel 06)

**Konzept**: Schalte alle Lichter aus durch Tippen (Toggle auch Nachbarn)

**Features**:
- 5x5 Grid (klassisch)
- Verschiedene Muster
- Minimal-Lösungs-Anzeige
- Verschiedene Themes
- Glow-Effekte

**Technisch**:
- Linear Algebra für Lösbarkeit (GF(2))
- Gauss-Elimination für optimale Lösung
- Toggle-Animation

### Nonogram (Spiel 07)

**Konzept**: Male Bilder basierend auf Zahlen-Hinweisen

**Features**:
- 10x10 bis 20x20 Grids
- Farbige Nonograms (später)
- Fehler-Detektion
- Hint-System
- Timer
- 50+ vordefinierte Bilder

**Technisch**:
- Line-Solver-Algorithmus
- Constraint-Propagation
- Touch-Drag zum Füllen

### Tower of Hanoi (Spiel 08)

**Konzept**: Bewege alle Scheiben von Stab A zu Stab C

**Features**:
- 3 bis 8 Scheiben wählbar
- Visualisierung der Lösung
- Move-Counter (optimal: 2^n - 1)
- Animierte Bewegungen
- Auto-Solve mit Animation

**Technisch**:
- Rekursiver Algorithmus
- Drag & Drop Steuerung
- Smooth Bezier-Animationen

## 📊 Vergleich: Games vs. Arcade Games

| Aspekt | Games 🧩 | Arcade Games 🎮 |
|--------|----------|-----------------|
| **Tempo** | Ruhig, überlegtes Spielen | Schnell, reaktionsbasiert |
| **Fokus** | Logik & Strategie | Reflex & Koordination |
| **Lernkurve** | Flach, leicht zu verstehen | Steil, Übung erforderlich |
| **Sessions** | Kurz bis lang (je nach Puzzle) | Kurz (wenige Minuten) |
| **Stresslevell** | Niedrig bis mittel | Mittel bis hoch |
| **Mathematik** | Oft komplex (Algorithmen) | Einfacher (Physik, Kollision) |
| **Alterszielgruppe** | Alle Altersgruppen | Eher jüngere Spieler |
| **Solo/Multiplayer** | Meist Solo | Oft Multiplayer |
| **Highscore-Fokus** | Nicht zentral | Zentral |

**Beide Kategorien ergänzen sich perfekt und bieten Abwechslung!**

## 🤝 Beitragen

Hast du Ideen für weitere Denksport-Spiele? Wir freuen uns über:

- **Spiel-Vorschläge**: Welches klassische Puzzle-Spiel fehlt noch?
- **Verbesserungen**: Optimierungen an bestehenden Spielen
- **Varianten**: Alternative Grafik-Themes oder Modi
- **Bug-Reports**: Fehler melden via GitHub Issues

## 📚 Weiterführende Informationen

- **[Haupt-README](../README.md)**: Übersicht über die gesamte CYD Academy
- **[Examples](../examples/)**: Basis-Beispiele zum Lernen
- **[Sensor-Examples](../sensor-examples/)**: Hardware-Integration
- **[Arcade Games](../arcade-games/)**: Action-Spiele
- **[Repo Guidelines](../Repo_Guideline.txt)**: Entwickler-Richtlinien

---

**Viel Spaß beim Puzzeln! 🧩**

Bei Fragen: CYD-Academy GitHub Issues
