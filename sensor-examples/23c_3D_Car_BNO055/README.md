# Beispiel 23c: 3D-Auto mit BNO055 Orientierungssensor

Ein interaktives 3D-Auto (blocky/retro-style), das virtuell im Raum "schwebt". Das CYD-Board funktioniert wie eine Kamera, die du um das Auto herum bewegen kannst. Drehe das Board nach rechts → siehst du das Auto von rechts. Kippe es nach vorne → siehst du das Auto von oben.

**Das Konzept:** Das Board ist die Kamera, nicht das Auto! Das Auto "steht still" im virtuellen Raum, während du die Kamera (das CYD-Board) um es herum bewegst.

---

## 📸 Features

- **Echtzeit 3D-Rendering**: Vollständiges 3D-Auto-Modell mit Perspektiv-Projektion
- **Komplexe Geometrie**: Karosserie + Kabine + Fenster (12 Faces, 16 Vertices)
- **BNO055-Integration**: Absolute Orientierung mit 9-DoF Sensor-Fusion
- **Direkte Sensor-Werte**: Keine Glättung für maximale Responsiveness
- **Back-face Culling**: Nur sichtbare Flächen werden gerendert
- **Painter's Algorithm**: Korrekte Tiefensortierung für realistische Darstellung
- **Wireframe-Modus**: Optional nur Kanten anzeigen
- **Touch-Interaktion**: Wireframe-Modus per Touch umschalten

## 🚗 Auto-Geometrie

Das Auto besteht aus **zwei Hauptblöcken**:

### Karosserie (unterer Block)
- 8 Vertices für den Auto-Körper
- Breite: 30 units, Höhe: 15 units, Länge: 60 units
- Farbe: Rot (CAR_BODY)

### Kabine/Dach (oberer Block)
- 8 Vertices für Kabine und Dach
- Schmaler und nach hinten versetzt
- Farben: Dunkelrot (Dach), Blau (Fenster)

**Gesamt: 16 Vertices, 12 Faces**

```
Koordinaten-System:
- X = Breite (links/rechts)
- Y = Höhe (oben/unten)
- Z = Länge (vorne/hinten)
```

## 🔌 Hardware-Anforderungen

- **ESP32-2432S028R (CYD)** - Cheap Yellow Display
- **BNO055** - 9-DoF Absolute Orientation Sensor
- **4x Dupont-Kabel** - Für I2C-Verbindung

### BNO055 Verkabelung (I2C)

```
BNO055            CYD (ESP32-2432S028R)
------            ---------------------
VCC     ------→   3.3V (oder 5V)
GND     ------→   GND
SDA     ------→   GPIO 22 (extSDA)
SCL     ------→   GPIO 27 (extSCL)
```

**I2C-Adresse**: 0x29 (Standard)

## 💻 Code-Highlights

### Auto-Geometrie (im Code)

```cpp
// Auto-Eckpunkte (16 Vertices = 8 Karosserie + 8 Kabine)
Vec3 carVertices[16] = {
  // Karosserie (unterer Block) - Vertices 0-7
  Vec3(-15, -10, -30), Vec3( 15, -10, -30), Vec3( 15, -10,  30), Vec3(-15, -10,  30),
  Vec3(-15,   5, -30), Vec3( 15,   5, -30), Vec3( 15,   5,  30), Vec3(-15,   5,  30),

  // Kabine/Dach (oberer Block) - Vertices 8-15
  Vec3(-12,   5,  -5), Vec3( 12,   5,  -5), Vec3( 12,   5,  25), Vec3(-12,   5,  25),
  Vec3(-12,  20,  -5), Vec3( 12,  20,  -5), Vec3( 12,  20,  25), Vec3(-12,  20,  25)
};

// Auto-Flächen (12 Faces = 6 Karosserie + 6 Kabine)
Face carFaces[12] = {
  // Karosserie
  Face(2, 1, 0, 3, CAR_BODY, 0),        // Boden
  Face(4, 5, 6, 7, CAR_BODY, 0),        // Karosserie-Top
  Face(6, 2, 3, 7, CAR_BODY, 0),        // Front
  Face(5, 4, 0, 1, CAR_BODY, 0),        // Heck
  Face(7, 3, 0, 4, CAR_BODY, 0),        // Links
  Face(6, 5, 1, 2, CAR_BODY, 0),        // Rechts

  // Kabine/Dach
  Face(10, 9, 8, 11, CAR_CABIN_TOP, 0), // Kabine-Boden
  Face(12, 13, 14, 15, CAR_CABIN_TOP, 0), // Dach
  Face(14, 10, 11, 15, CAR_WINDOW_FRONT, 0), // Windschutzscheibe
  Face(13, 12, 8, 9, CAR_WINDOW, 0),    // Heckscheibe
  Face(15, 11, 8, 12, CAR_WINDOW, 0),   // Linkes Fenster
  Face(14, 13, 9, 10, CAR_WINDOW, 0)    // Rechtes Fenster
};
```

### Farben

```cpp
#define CAR_BODY 0xF800          // Rot (Karosserie)
#define CAR_CABIN_TOP 0xC000     // Dunkelrot (Dach)
#define CAR_WINDOW 0x1084        // Dunkelblau (Fenster)
#define CAR_WINDOW_FRONT 0x2104  // Helleres Blau (Windschutzscheibe)
#define CAR_EDGE 0x0000          // Schwarz (Kanten)
```

## 🚀 Installation & Verwendung

### 1. Library installieren

**Arduino IDE:**
```
Tools → Manage Libraries → "Adafruit BNO055" suchen → Installieren
```

### 2. Hardware aufbauen

1. **Stromversorgung trennen** (USB-Kabel abziehen!)
2. **Verkabelung** wie oben beschrieben
3. **Verkabelung prüfen**
4. **Stromversorgung anschließen**

### 3. Code hochladen

**Arduino IDE Einstellungen:**
```
Board: "ESP32 Dev Module"
Upload Speed: 921600
CPU Frequency: 240MHz
```

### 4. Bedienung

**Nach dem Start:**
1. **Splash Screen** (2 Sekunden) - zeigt Auto-Symbol
2. **Sensor-Initialisierung**
3. **Kalibrierungs-Hinweis** (3 Sekunden)
4. **3D-Auto erscheint**

**Interaktion:**
- **Bewege das CYD um das Auto herum**: Board = Kamera
  - Nach rechts neigen → Auto von rechts betrachten
  - Nach vorne kippen → Auto von oben betrachten
  - Drehen → Auto von allen Seiten sehen
- **Touch**: Wireframe-Modus umschalten (gefüllt ↔ nur Kanten)

**Kalibrierung:**
- **Figure-8-Bewegung** in der Luft (Magnetometer)
- **Alle Richtungen neigen** (Accelerometer & Gyroscope)
- **Grüner Indikator** oben rechts = voll kalibriert (Wert 3)

## 🔧 Unterschiede zu Beispiel 23/23b (Würfel)

| Feature | Würfel (23/23b) | Auto (23c) |
|---------|-----------------|------------|
| **Geometrie** | 8 Vertices, 6 Faces | 16 Vertices, 12 Faces |
| **Komplexität** | Einfacher Quader | Zwei verbundene Quader |
| **Farben** | 6 verschiedene (Würfelaugen) | 4 verschiedene (Karosserie, Dach, Fenster) |
| **Details** | Würfelaugen (1-6) | Keine zusätzlichen Details |
| **Zweck** | Demo für 3D-Rendering-Grundlagen | Demo für komplexere 3D-Modelle |

## 🎯 Erweiterungsideen

### 1. Räder hinzufügen
Füge 4 Räder als kleine Zylinder oder Quader hinzu (8 Vertices pro Rad = 32 zusätzliche Vertices)

### 2. Mehr Details
- Scheinwerfer (kleine Kreise vorne)
- Rücklichter (kleine Rechtecke hinten)
- Türgriffe
- Spoiler

### 3. Mehrfarbige Karosserie
- Verschiedene Farben für Front/Heck/Seiten
- Rennen-Streifen
- Custom-Lackierungen

### 4. Auto-Physik
- Auto "fährt" wenn CYD geschüttelt wird
- Schwerkraft-Simulation
- Lenkung basierend auf Neigung

### 5. Multiple Autos
- Zeige mehrere Autos an (wie Verkehr)
- Kollisionserkennung
- Rennen-Spiel

## 📚 Technische Details

### Rendering-Pipeline

1. **Rotation**: Alle 16 Vertices mit Quaternionen rotieren
2. **Projektion**: 3D → 2D Perspektiv-Projektion
3. **Tiefenberechnung**: Durchschnittliche Z-Tiefe pro Face berechnen
4. **Sortierung**: Painter's Algorithm (von hinten nach vorne)
5. **Back-face Culling**: Nur sichtbare Flächen zeichnen
6. **Rendering**: Gefüllte Polygone + Kanten zeichnen

### Performance

- **Vertices**: 16 (statt 8 beim Würfel)
- **Faces**: 12 (statt 6 beim Würfel)
- **Performance**: ~30-40 FPS auf ESP32 @ 240 MHz
- **RAM-Nutzung**: ~2-3 KB für Geometrie-Daten

## 🔜 Nächste Schritte

**Weitere Beispiele:**
- **Beispiel 23**: 3D-Würfel (Original-Größe)
- **Beispiel 23b**: 3D-Würfel (Klein)
- **Beispiel 18**: Kompass Orientation (mehr BNO055-Features)

**Lernpfad:**
1. ✅ Beispiel 23c (3D-Auto) abgeschlossen
2. → Eigene 3D-Modelle erstellen (Roboter, Haus, etc.)
3. → Komplexere Geometrie mit mehr Vertices
4. → Texturen und Beleuchtung hinzufügen

---

**Viel Spaß mit dem 3D-Auto!** 🚗

*Beispiel 23c - CYD-Academy - November 2025*
