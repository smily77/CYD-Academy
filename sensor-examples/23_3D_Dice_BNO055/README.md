# Beispiel 23: 3D-Würfel mit BNO055 Orientierungssensor

Ein interaktiver 3D-Würfel, der sich entsprechend der Orientierung des CYD im Raum bewegt. Durch Drehen des Boards kann man den Würfel von allen Seiten betrachten - als würde man das Board um einen echten Würfel herum bewegen!

## 📸 Features

- **Echtzeit 3D-Rendering**: Vollständiger 3D-Würfel mit Perspektiv-Projektion
- **6 farbige Seiten**: Jede Seite hat eine eigene Farbe und Würfelaugen (1-6)
- **BNO055-Integration**: Absolute Orientierung mit 9-DoF Sensor-Fusion
- **Smooth Animation**: Exponential Smoothing für flüssige Bewegungen
- **Back-face Culling**: Nur sichtbare Flächen werden gerendert
- **Painter's Algorithm**: Korrekte Tiefensortierung für realistische Darstellung
- **Demo-Modus**: Automatische Rotation wenn kein Sensor vorhanden
- **Wireframe-Modus**: Optional nur Kanten anzeigen
- **Touch-Interaktion**: Modi wechseln per Touch

## 🔌 Hardware-Anforderungen

### Hauptkomponenten

- **ESP32-2432S028R (CYD)** - Cheap Yellow Display
- **BNO055** - 9-DoF Absolute Orientation Sensor
- **4x Dupont-Kabel** - Für I2C-Verbindung

### BNO055 Spezifikationen

Der **Bosch BNO055** ist ein hochintegrierter 9-DoF-Sensor (9 Freiheitsgrade) mit integrierter Sensor-Fusion:

**Sensoren:**
- **3-Achsen-Beschleunigungssensor** (Accelerometer)
  - Messbereich: ±2g / ±4g / ±8g / ±16g
  - Auflösung: 14-Bit
- **3-Achsen-Gyroskop** (Gyroscope)
  - Messbereich: ±125° / ±250° / ±500° / ±1000° / ±2000° /s
  - Auflösung: 16-Bit
- **3-Achsen-Magnetometer** (Compass)
  - Messbereich: ±1300 µT (x/y-Achse), ±2500 µT (z-Achse)
  - Auflösung: 0.3 µT

**Besonderheiten:**
- **Integrierte Sensor-Fusion**: 32-Bit Cortex M0+ Mikrocontroller berechnet absolute Orientierung
- **Quaternionen & Euler-Winkel**: Direkte Ausgabe von Orientierungsdaten
- **Auto-Kalibrierung**: Speichert Kalibrierungsdaten im Flash
- **Mehrere Betriebsmodi**: NDOF, IMU, Compass, etc.
- **I2C & UART**: Flexible Kommunikation
- **3.3V oder 5V**: Flexible Spannungsversorgung

**Ausgabe-Formate:**
- Absolute Orientierung (Euler: Heading, Roll, Pitch)
- Absolute Orientierung (Quaternionen: w, x, y, z)
- Lineare Beschleunigung
- Gravitations-Vektor
- Temperatur

### Verbindung (I2C)

```
BNO055            CYD (ESP32-2432S028R)
------            ---------------------
VCC     ------→   3.3V (oder 5V - Sensor akzeptiert beides)
GND     ------→   GND
SDA     ------→   GPIO 22 (extSDA)
SCL     ------→   GPIO 27 (extSCL)
```

**Wichtig:**
- **I2C-Adresse**: Standard 0x28 (alternativ 0x29 wenn ADR-Pin auf HIGH)
- **Pull-Up-Widerstände**: Meist auf BNO055-Breakout-Boards bereits vorhanden (4.7kΩ)
- **Spannungsversorgung**: 3.3V empfohlen (5V funktioniert auch, Sensor hat Spannungsregler)
- **I2C-Geschwindigkeit**: Standard 100 kHz oder Fast-Mode 400 kHz

## 🔬 Technischer Hintergrund

### 3D-Grafik-Grundlagen

Dieses Beispiel demonstriert die komplette 3D-Grafik-Pipeline von den Grundlagen:

#### 1. 3D-Koordinatensystem

Wir verwenden ein **rechtshändiges Koordinatensystem**:
- **X-Achse**: Nach rechts (positiv)
- **Y-Achse**: Nach oben (positiv)
- **Z-Achse**: Zur Kamera/aus dem Bildschirm (positiv)

```
    Y
    |
    |
    |
    +-----→ X
   /
  /
 ↙
Z
```

#### 2. 3D-Transformationen

##### Rotation um Achsen

**Rotation um X-Achse (Pitch):**
```
x' = x
y' = y * cos(θ) - z * sin(θ)
z' = y * sin(θ) + z * cos(θ)
```

**Rotation um Y-Achse (Yaw/Heading):**
```
x' = x * cos(θ) - z * sin(θ)
y' = y
z' = x * sin(θ) + z * cos(θ)
```

**Rotation um Z-Achse (Roll):**
```
x' = x * cos(θ) - y * sin(θ)
y' = x * sin(θ) + y * cos(θ)
z' = z
```

##### Rotations-Reihenfolge

Die Reihenfolge der Rotationen ist **wichtig**! Wir verwenden:
1. **Yaw** (Y-Achse) - Kompass-Richtung
2. **Pitch** (X-Achse) - Neigung vor/zurück
3. **Roll** (Z-Achse) - Neigung links/rechts

Diese Reihenfolge entspricht dem **Aerospace-Standard** (Tait-Bryan-Winkel, ZYX-Konvention).

#### 3. Perspektiv-Projektion

Um 3D-Punkte auf einen 2D-Bildschirm zu projizieren, verwenden wir **Perspektiv-Projektion**:

```
scale = distance / (z + distance)
screenX = centerX + x * scale
screenY = centerY - y * scale
```

Dabei gilt:
- **distance**: Distanz der Kamera zum Objekt
- **z**: Tiefe des Punktes im 3D-Raum
- Je weiter weg (größeres z), desto kleiner erscheint das Objekt

**Vereinfachte Perspektiv-Formel:**
```
        d · x
x_2D = -------
        z + d

        d · y
y_2D = -------
        z + d
```

Wo:
- `d` = Kamera-Distanz (z.B. 250 Pixel)
- `(x, y, z)` = 3D-Koordinate
- `(x_2D, y_2D)` = 2D-Bildschirm-Koordinate

#### 4. Back-face Culling

**Back-face Culling** verhindert das Zeichnen von Flächen, die von der Kamera weg zeigen (nicht sichtbar).

**Algorithmus:**
1. Berechne **Normalen-Vektor** der Fläche (mit Kreuzprodukt):
   ```
   N = (V1 - V0) × (V2 - V0)
   ```

2. Berechne Vektor **zur Kamera**:
   ```
   toCamera = CameraPosition - FaceCenter
   ```

3. Berechne **Dot-Product**:
   ```
   visible = N · toCamera > 0
   ```

**Kreuzprodukt-Formel:**
```
a × b = (ay·bz - az·by, az·bx - ax·bz, ax·by - ay·bx)
```

**Skalarprodukt (Dot-Product):**
```
a · b = ax·bx + ay·by + az·bz
```

Wenn das Skalarprodukt **positiv** ist, zeigt die Fläche zur Kamera und ist sichtbar.

#### 5. Painter's Algorithm

Der **Painter's Algorithm** sortiert Flächen nach Tiefe und zeichnet sie von hinten nach vorne:

1. Berechne **durchschnittliche Z-Koordinate** jeder Fläche
2. **Sortiere** Flächen nach Z (kleinste zuerst = am weitesten weg)
3. **Zeichne** in dieser Reihenfolge (weiter entfernte Objekte werden von näheren übermalt)

```
avgZ = (z0 + z1 + z2 + z3) / 4
```

**Vorteil:** Einfach zu implementieren
**Nachteil:** Kann bei überlappenden Objekten versagen (für einfache Würfel ausreichend)

### Euler-Winkel vs. Quaternionen

#### Euler-Winkel

**Definition:** Drei Winkel (Yaw, Pitch, Roll) die drei aufeinanderfolgende Rotationen beschreiben.

**Vorteile:**
- ✅ Intuitiv verständlich (Grad-Angaben)
- ✅ Direkt visualisierbar
- ✅ Einfach zu lesen

**Nachteile:**
- ❌ **Gimbal Lock**: Bei bestimmten Winkeln (z.B. Pitch = 90°) gehen Freiheitsgrade verloren
- ❌ Nicht-kommutativ (Reihenfolge wichtig)
- ❌ Interpolation schwierig (nicht linear)

**Gimbal Lock Beispiel:**
Wenn Pitch = 90° (senkrecht nach oben schauen), fallen Yaw und Roll zusammen - eine Rotation ist "verloren".

#### Quaternionen

**Definition:** 4D-Zahlensystem zur Darstellung von Rotationen: `q = (w, x, y, z)`

**Mathematische Form:**
```
q = w + xi + yj + zk

Wobei: i² = j² = k² = ijk = -1
```

**Vorteile:**
- ✅ **Kein Gimbal Lock**
- ✅ Effiziente Rotation-Komposition
- ✅ Smooth Interpolation (SLERP)
- ✅ Numerisch stabiler

**Nachteile:**
- ❌ Nicht intuitiv
- ❌ Schwer zu visualisieren
- ❌ 4 Zahlen statt 3

**Konvertierung Quaternion → Euler:**
```cpp
// Roll (x-Achse)
roll = atan2(2*(w*x + y*z), 1 - 2*(x*x + y*y))

// Pitch (y-Achse)
pitch = asin(2*(w*y - z*x))

// Yaw (z-Achse)
yaw = atan2(2*(w*z + x*y), 1 - 2*(y*y + z*z))
```

**In diesem Beispiel:**
Wir nutzen **Euler-Winkel**, da sie vom BNO055 direkt geliefert werden und für die Visualisierung intuitiv sind. Gimbal Lock ist kein Problem, da wir nur die Kamera-Orientierung übernehmen (keine komplexen Rotations-Animationen).

### Sensor-Fusion im BNO055

Der BNO055 kombiniert drei Sensoren zu einer **präzisen Orientierungsschätzung**:

#### Sensoren und ihre Limitierungen

**1. Accelerometer (Beschleunigungssensor)**
- Misst: Lineare Beschleunigung + Gravitation
- Problem: Kann nicht zwischen Gravitation und Bewegung unterscheiden
- Drift: Nein (absoluter Sensor)

**2. Gyroscope (Gyroskop)**
- Misst: Winkelgeschwindigkeit (Rotation)
- Problem: Integrierter Drift (kleine Messfehler summieren sich über Zeit)
- Drift: Ja (relativer Sensor)

**3. Magnetometer (Kompass)**
- Misst: Magnetfeld-Richtung (Nord)
- Problem: Störungen durch Metalle, Elektronik, Motoren
- Drift: Nein (absoluter Sensor)

#### Sensor-Fusion-Algorithmus

Der BNO055 nutzt einen **Kalman-Filter** oder ähnlichen Algorithmus:

1. **Gyroscope**: Liefert schnelle, aber driftende Orientierung
2. **Accelerometer**: Korrigiert Pitch/Roll (Gravitations-Richtung)
3. **Magnetometer**: Korrigiert Yaw/Heading (Nord-Richtung)

**Ablauf:**
```
1. Prediction Step (Gyro):
   orientation_predicted = orientation_old + gyro_rate * dt

2. Correction Step (Accel + Mag):
   orientation_corrected = kalman_filter(
     orientation_predicted,
     accel_data,
     mag_data
   )
```

**Ergebnis:**
- Schnelle Reaktion (Gyro)
- Kein langfristiger Drift (Accel/Mag korrigieren)
- Robuste Orientierung

#### Kalibrierung

Der BNO055 benötigt Kalibrierung für optimale Ergebnisse:

**System-Kalibrierung (0-3):**
- 0: Nicht kalibriert
- 1: Teilweise kalibriert
- 2: Gut kalibriert
- 3: Voll kalibriert (beste Genauigkeit)

**Kalibrierungs-Prozess:**
1. **Magnetometer**: Figure-8-Bewegung in der Luft (alle Achsen rotieren)
2. **Accelerometer**: Board in 6 verschiedene Orientierungen halten
3. **Gyroscope**: Board in alle Richtungen langsam drehen

**Auto-Kalibrierung:**
Der BNO055 speichert Kalibrierungsdaten automatisch im Flash-Speicher!

### Vergleich: BNO055 vs. MPU6050

| Feature | BNO055 | MPU6050 |
|---------|--------|---------|
| **Achsen** | 9-DoF (Accel + Gyro + Mag) | 6-DoF (Accel + Gyro) |
| **Sensor-Fusion** | ✅ Integriert (Cortex M0+) | ❌ Extern nötig |
| **Absolute Orientierung** | ✅ Ja (mit Kompass) | ❌ Nein (Drift) |
| **Quaternionen** | ✅ Ja | ❌ Nein |
| **Euler-Winkel** | ✅ Ja (direkt) | ⚠️ Berechnung nötig |
| **Kalibrierung** | ✅ Auto + Speicherung | ⚠️ Manuell |
| **Präzision** | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ |
| **Preis** | ~15-25€ | ~5-10€ |
| **I2C-Adresse** | 0x28/0x29 | 0x68/0x69 |
| **Anwendung** | Drohnen, Roboter, AR/VR | Bewegungserkennung, Spiele |

**Empfehlung:**
- **BNO055**: Wenn absolute Orientierung wichtig ist (z.B. Kompass, Kamera-Stabilisierung)
- **MPU6050**: Wenn nur relative Bewegung gebraucht wird (z.B. Wasserwaage, Schrittzähler)

## 💻 Code-Struktur

### Hauptkomponenten

#### 1. Datenstrukturen

**Vec3 - 3D-Vektor:**
```cpp
struct Vec3 {
  float x, y, z;

  Vec3 operator+(const Vec3& v);  // Addition
  Vec3 operator-(const Vec3& v);  // Subtraktion
  Vec3 operator*(float s);        // Skalierung
  float length();                 // Länge
  Vec3 normalize();               // Normalisieren
  Vec3 cross(const Vec3& v);     // Kreuzprodukt
  float dot(const Vec3& v);      // Skalarprodukt
};
```

**Face - Würfel-Fläche:**
```cpp
struct Face {
  int vertices[4];  // 4 Eckpunkte-Indices
  uint16_t color;   // Farbe
  int diceValue;    // Würfelaugen (1-6)
  float avgZ;       // Tiefe (für Sortierung)
};
```

#### 2. Würfel-Geometrie

**8 Vertices (Eckpunkte):**
```cpp
Vec3 cubeVertices[8] = {
  Vec3(-1, -1, -1),  // 0: Hinten-Links-Unten
  Vec3( 1, -1, -1),  // 1: Hinten-Rechts-Unten
  Vec3( 1,  1, -1),  // 2: Hinten-Rechts-Oben
  Vec3(-1,  1, -1),  // 3: Hinten-Links-Oben
  Vec3(-1, -1,  1),  // 4: Vorne-Links-Unten
  Vec3( 1, -1,  1),  // 5: Vorne-Rechts-Unten
  Vec3( 1,  1,  1),  // 6: Vorne-Rechts-Oben
  Vec3(-1,  1,  1)   // 7: Vorne-Links-Oben
};
```

**6 Faces (Flächen):**
```cpp
Face cubeFaces[6] = {
  Face(4,5,6,7, RED,     1),  // Vorne (Z+)
  Face(1,0,3,2, CYAN,    6),  // Hinten (Z-)
  Face(0,4,7,3, GREEN,   2),  // Links (X-)
  Face(5,1,2,6, MAGENTA, 5),  // Rechts (X+)
  Face(3,7,6,2, BLUE,    3),  // Oben (Y+)
  Face(0,1,5,4, YELLOW,  4)   // Unten (Y-)
};
```

**Würfel-Topologie:**
```
    3 -------- 2
   /|         /|
  / |        / |
 7 -------- 6  |
 |  0 ------|--1
 | /        | /
 |/         |/
 4 -------- 5
```

#### 3. Rendering-Pipeline

**Haupt-Loop:**
```cpp
void renderCube() {
  rotateCube();          // 1. Rotation anwenden
  projectVertices();     // 2. 3D → 2D Projektion
  calculateFaceDepths(); // 3. Tiefe berechnen
  sortFaces();           // 4. Z-Sortierung
  drawFaces();           // 5. Zeichnen
}
```

**1. Rotation:**
```cpp
void rotateCube() {
  // Euler-Winkel → Radiant
  float yaw = -smoothHeading * PI / 180.0;
  float pitch_rad = smoothPitch * PI / 180.0;
  float roll_rad = smoothRoll * PI / 180.0;

  // Trigonometrische Werte
  float cosY = cos(yaw), sinY = sin(yaw);
  float cosP = cos(pitch_rad), sinP = sin(pitch_rad);
  float cosR = cos(roll_rad), sinR = sin(roll_rad);

  // Für jeden Vertex: Rotations-Matrizen anwenden
  for (int i = 0; i < 8; i++) {
    Vec3 v = cubeVertices[i] * CUBE_SIZE;

    // Yaw (Y-Achse)
    float x1 = v.x * cosY - v.z * sinY;
    float z1 = v.x * sinY + v.z * cosY;

    // Pitch (X-Achse)
    float y2 = v.y * cosP - z1 * sinP;
    float z2 = v.y * sinP + z1 * cosP;

    // Roll (Z-Achse)
    float x3 = x1 * cosR - y2 * sinR;
    float y3 = x1 * sinR + y2 * cosR;

    transformedVertices[i] = Vec3(x3, y3, z2);
  }
}
```

**2. Projektion:**
```cpp
void projectVertices() {
  for (int i = 0; i < 8; i++) {
    Vec3 v = transformedVertices[i];

    // Perspektiv-Division
    float z = v.z + PERSPECTIVE_DISTANCE;
    float scale = PERSPECTIVE_DISTANCE / z;

    // 2D-Koordinaten
    int x = SCREEN_CENTER_X + (int)(v.x * scale);
    int y = SCREEN_CENTER_Y - (int)(v.y * scale);

    projectedPoints[i] = Point2D(x, y);
  }
}
```

**3. Back-face Culling:**
```cpp
bool isFacingCamera(const Face& face) {
  // Berechne Normale (Kreuzprodukt)
  Vec3 v0 = transformedVertices[face.vertices[0]];
  Vec3 v1 = transformedVertices[face.vertices[1]];
  Vec3 v2 = transformedVertices[face.vertices[2]];

  Vec3 edge1 = v1 - v0;
  Vec3 edge2 = v2 - v0;
  Vec3 normal = edge1.cross(edge2);

  // Vektor zur Kamera
  Vec3 faceCenter = (v0 + v1 + v2 + v3) * 0.25;
  Vec3 toCamera = Vec3(0, 0, -DISTANCE) - faceCenter;

  // Dot-Product: positiv = sichtbar
  return normal.dot(toCamera) > 0;
}
```

**4. Z-Sortierung (Painter's):**
```cpp
void sortFaces() {
  // Bubble Sort nach avgZ (hinten → vorne)
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5-i; j++) {
      if (cubeFaces[j].avgZ > cubeFaces[j+1].avgZ) {
        swap(cubeFaces[j], cubeFaces[j+1]);
      }
    }
  }
}
```

**5. Würfelaugen zeichnen:**
```cpp
void drawDiceDots(Point2D p[4], int value) {
  // Berechne Flächen-Zentrum
  int cx = (p[0].x + p[1].x + p[2].x + p[3].x) / 4;
  int cy = (p[0].y + p[1].y + p[2].y + p[3].y) / 4;

  // Punkt-Radius basierend auf Flächengröße
  float avgDist = calculateAverageDistance(p, cx, cy);
  int dotRadius = max(2, (int)(avgDist * 0.15));
  int spacing = (int)(avgDist * 0.5);

  // Zeichne Muster basierend auf Wert (1-6)
  switch (value) {
    case 1: drawCenterDot(cx, cy, dotRadius); break;
    case 2: drawDiagonalDots(cx, cy, spacing, dotRadius); break;
    case 3: drawDiagonalWithCenter(cx, cy, spacing, dotRadius); break;
    case 4: drawCornerDots(cx, cy, spacing, dotRadius); break;
    case 5: drawCornerDotsWithCenter(cx, cy, spacing, dotRadius); break;
    case 6: drawTwoColumns(cx, cy, spacing, dotRadius); break;
  }
}
```

#### 4. Sensor-Integration

**BNO055 auslesen:**
```cpp
void readSensor() {
  // Euler-Winkel holen
  sensors_event_t event;
  bno.getEvent(&event);

  heading = event.orientation.x;  // 0-360°
  pitch = event.orientation.y;    // -180 bis +180°
  roll = event.orientation.z;     // -90 bis +90°

  // Kalibrierungsstatus
  bno.getCalibration(&calSystem, &calGyro, &calAccel, &calMag);
}
```

**Exponential Smoothing:**
```cpp
void smoothValues() {
  // Glättung für flüssige Animation
  const float SMOOTH_FACTOR = 0.15;

  // Heading mit Wrap-around
  float diff = heading - smoothHeading;
  if (diff > 180) diff -= 360;
  if (diff < -180) diff += 360;
  smoothHeading += diff * SMOOTH_FACTOR;

  // Roll & Pitch
  smoothRoll += (roll - smoothRoll) * SMOOTH_FACTOR;
  smoothPitch += (pitch - smoothPitch) * SMOOTH_FACTOR;
}
```

## 🚀 Installation & Verwendung

### 1. Library installieren

**Arduino IDE:**
```
Tools → Manage Libraries → "Adafruit BNO055" suchen → Installieren
```

Dies installiert automatisch:
- Adafruit BNO055 (Haupt-Library)
- Adafruit Unified Sensor (Basis-Bibliothek)
- Adafruit BusIO (I2C/SPI Kommunikation)

**PlatformIO:**
```ini
[env:esp32]
lib_deps =
    adafruit/Adafruit BNO055 @ ^1.6.0
```

### 2. Hardware aufbauen

**Schritt-für-Schritt:**

1. **Stromversorgung trennen** (USB-Kabel abziehen!)

2. **Verkabelung** (Dupont-Kabel):
   ```
   BNO055 VCC  →  CYD 3.3V (Pin oben rechts)
   BNO055 GND  →  CYD GND
   BNO055 SDA  →  CYD GPIO 22 (extSDA)
   BNO055 SCL  →  CYD GPIO 27 (extSCL)
   ```

3. **Verkabelung prüfen** (mit Multimeter Durchgang testen)

4. **I2C-Adresse prüfen** (falls mehrere I2C-Geräte):
   ```cpp
   // Standard-Adressen:
   // 0x28 (ADR-Pin auf GND oder offen)
   // 0x29 (ADR-Pin auf VCC)
   ```

5. **Stromversorgung anschließen**

**Warnung:**
- ⚠️ **Niemals** während des Betriebs verkabeln!
- ⚠️ **Polarität prüfen** (VCC/GND nicht vertauschen!)
- ⚠️ **I2C Pull-Ups**: Meist auf Breakout-Boards vorhanden (sonst 4.7kΩ hinzufügen)

### 3. Code hochladen

**Arduino IDE Einstellungen:**
```
Board: "ESP32 Dev Module"
Upload Speed: 921600
CPU Frequency: 240MHz
Flash Size: 4MB
Partition Scheme: Default 4MB
```

**Upload:**
1. USB-Kabel anschließen
2. COM-Port auswählen (Tools → Port)
3. Upload-Button drücken (→)
4. Warten auf "Done uploading"

### 4. Bedienung

**Nach dem Start:**
1. **Splash Screen** (2 Sekunden)
2. **Sensor-Initialisierung** (1-2 Sekunden)
3. **Kalibrierungs-Hinweis** (3 Sekunden) - wenn Sensor gefunden
4. **3D-Würfel erscheint**

**Interaktion:**
- **Drehe das CYD**: Würfel rotiert entsprechend (quasi "um den Würfel herum bewegen")
- **Touch Links**: Demo-Modus aktivieren/deaktivieren (automatische Rotation)
- **Touch Rechts**: Wireframe-Modus aktivieren/deaktivieren (nur Kanten)
- **Touch Mitte**: Kalibrierungs-Hilfe anzeigen

**Kalibrierung:**
Der BNO055 benötigt Kalibrierung für beste Ergebnisse:
1. **Figure-8-Bewegung** in der Luft (Magnetometer)
2. **Alle Richtungen neigen** (Accelerometer & Gyroscope)
3. **Grüner Indikator** oben rechts = voll kalibriert (Wert 3)

**Demo-Modus:**
Falls kein BNO055 gefunden wird, läuft das Beispiel im **Demo-Modus** mit automatischer Rotation. Perfekt zum Testen ohne Sensor!

### Serielle Ausgabe

```
=== 3D Dice BNO055 Demo ===
Initializing I2C (SDA=22, SCL=27)...
BNO055 found!
BNO055 initialized!
Setup complete! Rotate the board to see the dice from all sides.

Heading: 45.2°  Roll: 12.3°  Pitch: -5.7°  Cal: 3
Heading: 48.1°  Roll: 11.8°  Pitch: -6.2°  Cal: 3
Demo Mode: ON
Wireframe: ON
...
```

## 🔧 Troubleshooting

### Problem: "BNO055 not found!"

**Symptome:**
- Fehlermeldung beim Start
- Display zeigt "BNO055 Error!"
- Läuft automatisch im Demo-Modus

**Lösungen:**

1. **I2C-Verkabelung prüfen:**
   ```
   - SDA auf GPIO 22? (nicht GPIO 21!)
   - SCL auf GPIO 27? (nicht GPIO 22!)
   - VCC auf 3.3V?
   - GND auf GND?
   ```

2. **I2C-Adresse prüfen:**
   ```cpp
   // Im Code ändern (Zeile ~104):
   Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);  // oder 0x29
   ```

   Adresse herausfinden mit I2C-Scanner:
   ```cpp
   #include <Wire.h>

   void setup() {
     Serial.begin(115200);
     Wire.begin(22, 27);  // SDA, SCL

     Serial.println("I2C Scanner");
     for (byte addr = 1; addr < 127; addr++) {
       Wire.beginTransmission(addr);
       if (Wire.endTransmission() == 0) {
         Serial.printf("Found device at 0x%02X\n", addr);
       }
     }
   }
   ```

3. **Pull-Up-Widerstände:**
   - BNO055-Breakout hat meist 4.7kΩ Pull-Ups integriert
   - Falls nicht: Externe 4.7kΩ zwischen SDA/SCL und 3.3V

4. **Spannungsversorgung:**
   - BNO055 benötigt stabile 3.3V (oder 5V)
   - USB-Kabel prüfen (manche liefern nur 4.5V statt 5V)
   - Externes Netzteil testen

5. **Sensor defekt?**
   - Mit anderem Mikrocontroller testen (z.B. Arduino Uno)
   - Library-Version prüfen (mindestens 1.6.0)

### Problem: Würfel zittert/springt

**Symptome:**
- Würfel bewegt sich ruckartig
- Rotation nicht flüssig
- Sprünge bei kleinen Bewegungen

**Lösungen:**

1. **Smoothing erhöhen:**
   ```cpp
   // Zeile ~83 ändern:
   const float SMOOTH_FACTOR = 0.15;  // Verringern auf 0.05-0.10
   ```

2. **Update-Rate verringern:**
   ```cpp
   // In loop() (Zeile ~208):
   delay(20);  // Erhöhen auf 50 (langsamer, aber smoother)
   ```

3. **Kalibrierung verbessern:**
   - Sensor besser kalibrieren (Figure-8-Bewegung)
   - Alle drei Sensor-Typen müssen kalibriert sein (Cal: 3)

4. **Magnetische Störungen:**
   - Von Metallen/Motoren fernhalten
   - Laptop/Handy wegnehmen
   - Sensor-Position ändern

### Problem: Falsche Orientierung

**Symptome:**
- Würfel dreht sich in falsche Richtung
- Roll und Pitch vertauscht
- Spiegelverkehrt

**Lösungen:**

1. **Sensor-Orientierung:**
   - BNO055 Chip sollte nach oben zeigen
   - Pin-Header nach unten
   - Achsen-Beschriftung beachten

2. **Rotations-Vorzeichen ändern:**
   ```cpp
   // In rotateCube() (Zeile ~383):
   float yaw = -smoothHeading * PI / 180.0;   // Vorzeichen umkehren
   float pitch_rad = -smoothPitch * PI / 180.0;  // Falls nötig
   float roll_rad = smoothRoll * PI / 180.0;
   ```

3. **Achsen tauschen:**
   ```cpp
   // In readSensor() (Zeile ~358):
   heading = event.orientation.x;  // Evtl. .y oder .z verwenden
   pitch = event.orientation.y;    // Achsen tauschen
   roll = event.orientation.z;
   ```

### Problem: Kalibrierung bleibt bei 0-1

**Symptome:**
- Kalibrierungs-Indikator zeigt nur 0 oder 1
- Wird nicht grün (3)
- Orientierung ungenau

**Lösungen:**

1. **Magnetometer kalibrieren:**
   - **Figure-8-Bewegung** in der Luft
   - Alle drei Achsen rotieren
   - 10-20 Sekunden lang
   - Langsame, gleichmäßige Bewegung

2. **Accelerometer kalibrieren:**
   - Board in **6 Orientierungen** halten:
     * Flach auf Tisch
     * Auf der Seite (alle 4 Seiten)
     * Kopfüber
   - Jeweils 3-5 Sekunden ruhig halten

3. **Gyroscope kalibrieren:**
   - Board **langsam** in alle Richtungen drehen
   - Pitch, Roll, Yaw einzeln testen
   - Vollständige 360°-Rotationen

4. **Auto-Kalibrierung deaktiviert?**
   ```cpp
   // Nach bno.begin() hinzufügen:
   bno.setMode(Adafruit_BNO055::OPERATION_MODE_NDOF);
   ```

5. **Kalibrierungsdaten speichern/laden:**
   ```cpp
   // Nach Kalibrierung speichern:
   adafruit_bno055_offsets_t offsets;
   bno.getSensorOffsets(offsets);
   // In EEPROM/Flash speichern

   // Beim Start laden:
   bno.setSensorOffsets(offsets);
   ```

### Problem: Display zeigt nichts

**Symptome:**
- Schwarzer Bildschirm
- Keine Reaktion auf Touch
- LED leuchtet aber

**Lösungen:**

1. **Display-Konfiguration prüfen:**
   ```cpp
   #include <CYD_Display_Config.h>  // Vorhanden?
   LGFX lcd;  // Richtig deklariert?
   ```

2. **Helligkeit erhöhen:**
   ```cpp
   lcd.setBrightness(255);  // Maximale Helligkeit
   ```

3. **Code kompiliert?**
   - Serial Monitor öffnen (115200 Baud)
   - Ausgaben prüfen
   - Fehlermeldungen?

4. **CYD defekt?**
   - Einfaches Test-Programm hochladen
   - Beispiel 00_BoardConfigTest ausführen

## 🎯 Erweiterungsideen

### 1. Würfel-Physik-Simulation

**Konzept:**
Lasse den Würfel "rollen" wenn das CYD geschüttelt wird, mit realistischer Physik (Gravitation, Reibung, Rotation).

**Implementation:**
```cpp
// Globale Variablen
Vec3 angularVelocity(0, 0, 0);  // Rotations-Geschwindigkeit
Vec3 gravity(0, -9.81, 0);      // Gravitation
float friction = 0.95;           // Reibung

void updatePhysics() {
  // Lese lineare Beschleunigung (ohne Gravitation)
  imu::Vector<3> accel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);

  // Konvertiere zu Dreh-Moment (Torque)
  Vec3 torque(accel.y(), -accel.x(), 0);  // Beschleunigung → Rotation

  // Füge zu Winkel-Geschwindigkeit hinzu
  angularVelocity = angularVelocity + torque * 0.01;

  // Reibung anwenden
  angularVelocity = angularVelocity * friction;

  // Aktualisiere Orientierung
  smoothHeading += angularVelocity.z;
  smoothRoll += angularVelocity.x;
  smoothPitch += angularVelocity.y;
}

void loop() {
  readSensor();
  updatePhysics();  // Statt smoothValues()
  renderCube();
  // ...
}
```

**Anwendungen:**
- Würfel-Spiel (virtueller Spielwürfel)
- Physik-Demo für Schulen
- Interaktives Spielzeug

### 2. Multiple Würfel mit Kollision

**Konzept:**
Zeige mehrere Würfel an, die miteinander kollidieren können.

**Implementation:**
```cpp
struct Cube {
  Vec3 position;
  Vec3 rotation;
  Vec3 velocity;
  float size;
};

Cube cubes[3] = {
  { Vec3(-80, 0, 0), Vec3(0,0,0), Vec3(0,0,0), 40 },
  { Vec3(  0, 0, 0), Vec3(0,0,0), Vec3(0,0,0), 40 },
  { Vec3( 80, 0, 0), Vec3(0,0,0), Vec3(0,0,0), 40 }
};

void checkCollisions() {
  for (int i = 0; i < 3; i++) {
    for (int j = i+1; j < 3; j++) {
      Vec3 diff = cubes[i].position - cubes[j].position;
      float dist = diff.length();
      float minDist = cubes[i].size + cubes[j].size;

      if (dist < minDist) {
        // Kollision! Bouncing-Effekt
        Vec3 normal = diff.normalize();
        cubes[i].velocity = cubes[i].velocity + normal * 0.5;
        cubes[j].velocity = cubes[j].velocity - normal * 0.5;
      }
    }
  }
}

void renderAllCubes() {
  for (int i = 0; i < 3; i++) {
    // Transformiere Vertices mit cubes[i].position
    // Rotiere mit cubes[i].rotation
    // Zeichne Würfel
  }
}
```

**Anwendungen:**
- Mehr-Spieler-Würfel-Spiel
- Physik-Simulation
- Partikel-Effekte

### 3. Transparente Würfel mit X-Ray-Modus

**Konzept:**
Zeige den Würfel halbtransparent, sodass man durch ihn hindurchsehen und alle Seiten gleichzeitig sehen kann.

**Implementation:**
```cpp
bool xrayMode = false;

void drawFaces() {
  for (int i = 0; i < 6; i++) {
    Face& face = cubeFaces[i];

    if (!xrayMode && !isFacingCamera(face)) continue;  // Normal: nur sichtbare

    // Berechne Alpha basierend auf Winkel zur Kamera
    float alpha = calculateFaceAlpha(face);

    // Zeichne mit Transparenz
    uint16_t color = face.color;
    if (xrayMode) {
      color = blendColor(color, COLOR_BG, alpha);
    }

    drawFilledQuad(p[0], p[1], p[2], p[3], color);
    drawDiceDots(p, face.diceValue);
  }
}

uint16_t blendColor(uint16_t fg, uint16_t bg, float alpha) {
  // RGB565 entpacken
  uint8_t r1 = (fg >> 11) & 0x1F;
  uint8_t g1 = (fg >> 5) & 0x3F;
  uint8_t b1 = fg & 0x1F;

  uint8_t r2 = (bg >> 11) & 0x1F;
  uint8_t g2 = (bg >> 5) & 0x3F;
  uint8_t b2 = bg & 0x1F;

  // Mischen
  uint8_t r = r1 * alpha + r2 * (1-alpha);
  uint8_t g = g1 * alpha + g2 * (1-alpha);
  uint8_t b = b1 * alpha + b2 * (1-alpha);

  // RGB565 packen
  return (r << 11) | (g << 5) | b;
}
```

**Anwendungen:**
- Würfel-Scan-Effekt
- Lehrmittel (zeige innere Struktur)
- Künstlerische Visualisierung

### 4. Augmented Reality Marker

**Konzept:**
Nutze den Würfel als AR-Marker. Erkenne die Orientierung und zeige zusätzliche 3D-Objekte an (z.B. Pfeil zeigt immer nach Norden).

**Implementation:**
```cpp
void drawAROverlay() {
  // Zeichne Würfel
  renderCube();

  // Berechne Nord-Richtung im Würfel-Koordinatensystem
  float northAngle = -smoothHeading * PI / 180.0;

  // 3D-Pfeil der nach Norden zeigt
  Vec3 northDir(sin(northAngle), 0, cos(northAngle));
  northDir = northDir * (CUBE_SIZE * 1.5);  // Außerhalb Würfel

  // Transformiere und projiziere
  Vec3 arrowStart(0, CUBE_SIZE, 0);  // Oben auf Würfel
  Vec3 arrowEnd = arrowStart + northDir;

  // ... Rotation & Projektion anwenden
  // ... Pfeil zeichnen

  // Label "N" (Nord)
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_NORTH);
  lcd.drawString("N", arrowEndX, arrowEndY);
}
```

**Anwendungen:**
- Kompass mit 3D-Visualisierung
- Navigation-App
- AR-Spiele

### 5. Würfel-Animationen (Unfold/Fold)

**Konzept:**
Animiere den Würfel, wie er sich "entfaltet" zu einem 2D-Netz und wieder zusammenfaltet.

**Implementation:**
```cpp
float unfoldProgress = 0.0;  // 0.0 = gefaltet, 1.0 = entfaltet

void animateUnfold() {
  // Für jede Fläche: Berechne Ziel-Orientierung im entfalteten Zustand

  // Beispiel: Vordere Fläche bleibt fix
  // Linke Fläche klappt 90° nach links
  // Rechte Fläche klappt 90° nach rechts
  // etc.

  for (int i = 0; i < 6; i++) {
    float targetAngle = getFaceUnfoldAngle(i);
    float currentAngle = lerp(0, targetAngle, unfoldProgress);

    // Rotiere Fläche um Kante
    rotateFaceAroundEdge(i, currentAngle);
  }
}

float lerp(float a, float b, float t) {
  return a + (b - a) * t;
}

void loop() {
  // ...

  if (unfoldButton pressed) {
    unfoldProgress += 0.02;
    if (unfoldProgress > 1.0) unfoldProgress = 1.0;
  }

  if (foldButton pressed) {
    unfoldProgress -= 0.02;
    if (unfoldProgress < 0.0) unfoldProgress = 0.0;
  }

  animateUnfold();
  renderCube();
  // ...
}
```

**Anwendungen:**
- Geometrie-Lehrmittel
- Würfel-Netz visualisieren
- Origami-Simulation

### 6. Texturen & Bilder auf Würfel

**Konzept:**
Statt einfarbigen Flächen, zeige Bilder oder Texturen auf den Würfel-Seiten (z.B. Fotos, Logos).

**Implementation:**
```cpp
// Sprites für jede Seite (gespeichert in PROGMEM oder SD-Karte)
LGFX_Sprite faceSprites[6];

void setup() {
  // ...

  // Lade Bilder (aus SD oder embedded)
  for (int i = 0; i < 6; i++) {
    faceSprites[i].createSprite(64, 64);
    faceSprites[i].drawPngFile(SD, String("/face") + i + ".png");
  }
}

void drawTexturedFace(Face& face, Point2D p[4]) {
  // Berechne Textur-Koordinaten für jede Ecke
  // Perspektiv-korrektes Texture-Mapping

  // Vereinfacht: Rechteckiges Mapping
  int minX = min(min(p[0].x, p[1].x), min(p[2].x, p[3].x));
  int minY = min(min(p[0].y, p[1].y), min(p[2].y, p[3].y));
  int maxX = max(max(p[0].x, p[1].x), max(p[2].x, p[3].x));
  int maxY = max(max(p[0].y, p[1].y), max(p[2].y, p[3].y));

  // Skaliere Sprite auf Flächen-Größe
  int width = maxX - minX;
  int height = maxY - minY;

  // Zeichne Sprite (skaliert)
  faceSprites[face.diceValue - 1].pushRotateZoom(
    lcd, minX + width/2, minY + height/2,
    0, 1.0, 1.0  // Rotation, ScaleX, ScaleY
  );
}
```

**Anwendungen:**
- Foto-Würfel (6 Familienfotos)
- Branding (Logo auf allen Seiten)
- Spiele mit Custom-Grafiken

**Hinweis für PSRAM:**
Falls Sprites zu groß für SRAM sind:
```cpp
// In CYD_Display_Config.h oder platformio.ini:
#define BOARD_HAS_PSRAM
lcd.setSwapBytes(true);

// Sprites können jetzt größer sein
LGFX_Sprite sprite;
sprite.setPsram(true);  // Nutze PSRAM
sprite.createSprite(128, 128);
```

### 7. Würfel-Spiel: Yahtzee / Kniffel

**Konzept:**
Implementiere ein vollständiges Würfel-Spiel. Schüttle das CYD um zu würfeln, Würfel zeigen zufällige Zahlen.

**Implementation:**
```cpp
int diceValues[5] = {1, 2, 3, 4, 5};  // 5 Würfel
bool diceHeld[5] = {false, false, false, false, false};  // Gehalten?

void rollDice() {
  for (int i = 0; i < 5; i++) {
    if (!diceHeld[i]) {
      diceValues[i] = random(1, 7);  // 1-6
    }
  }
}

void detectShake() {
  // Lese Beschleunigung
  imu::Vector<3> accel = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
  float magnitude = sqrt(accel.x()*accel.x() + accel.y()*accel.y() + accel.z()*accel.z());

  // Schüttel-Schwellwert (z.B. 2g)
  if (magnitude > 20.0) {
    rollDice();
    showRollingAnimation();
  }
}

void drawGameScreen() {
  // Zeige 5 kleine Würfel
  for (int i = 0; i < 5; i++) {
    int x = 20 + i * 50;
    int y = 100;

    // Zeichne Mini-Würfel
    drawMiniDice(x, y, diceValues[i], diceHeld[i]);
  }

  // Zeige Score-Tabelle
  drawYahtzeeScoreTable();
}

void handleTouch(int x, int y) {
  // Klick auf Würfel = Halten/Loslassen
  int diceIndex = (x - 20) / 50;
  if (diceIndex >= 0 && diceIndex < 5) {
    diceHeld[diceIndex] = !diceHeld[diceIndex];
  }
}
```

**Anwendungen:**
- Kniffel/Yahtzee Spiel
- Ludo/Mensch ärgere dich nicht
- Backgammon
- Craps (Casino-Würfelspiel)

## 📚 Weiterführende Themen

### Quaternionen für Gimbal-Lock-freie Rotation

Das aktuelle Beispiel nutzt **Euler-Winkel** (Yaw, Pitch, Roll), die bei extremen Winkeln (z.B. Pitch = 90°) zu **Gimbal Lock** führen können.

**Gimbal Lock Problem:**
```
Normal (Pitch = 0°):
  Yaw  ≠ Roll  (zwei unabhängige Achsen)

Gimbal Lock (Pitch = 90°):
  Yaw  = Roll  (Rotation um gleiche Achse, Freiheitsgrad verloren!)
```

**Lösung: Quaternionen**

Der BNO055 bietet direkt **Quaternionen-Output**:

```cpp
void readSensorQuaternion() {
  // Hole Quaternion (w, x, y, z)
  imu::Quaternion quat = bno.getQuat();

  // Nutze direkt für Rotation
  rotateWithQuaternion(quat);
}

void rotateWithQuaternion(imu::Quaternion q) {
  for (int i = 0; i < 8; i++) {
    Vec3 v = cubeVertices[i] * CUBE_SIZE;

    // Quaternion-Rotation: v' = q * v * q^(-1)
    Vec3 rotated = quaternionRotate(q, v);

    transformedVertices[i] = rotated;
  }
}

Vec3 quaternionRotate(imu::Quaternion q, Vec3 v) {
  // Konvertiere Vec3 zu Quaternion (0, v.x, v.y, v.z)
  imu::Quaternion vQuat(0, v.x, v.y, v.z);

  // q^(-1) (Konjugiert)
  imu::Quaternion qConj = q.conjugate();

  // Rotation: q * v * q^(-1)
  imu::Quaternion result = q * vQuat * qConj;

  return Vec3(result.x(), result.y(), result.z());
}
```

**Vorteile:**
- ✅ Kein Gimbal Lock
- ✅ Smooth Interpolation (SLERP)
- ✅ Numerisch stabiler
- ✅ Effizienter (keine Trigonometrie)

### Z-Buffer statt Painter's Algorithm

Der **Painter's Algorithm** (von hinten nach vorne zeichnen) versagt bei:
- Überlappenden Objekten
- Zyklischen Überlappungen (A vor B, B vor C, C vor A)

**Lösung: Z-Buffer**

Ein **Z-Buffer** ist ein 2D-Array, das für jeden Pixel die Tiefe (Z-Koordinate) speichert:

```cpp
float zBuffer[240][320];  // Für jeden Pixel

void clearZBuffer() {
  for (int y = 0; y < 320; y++) {
    for (int x = 0; x < 240; x++) {
      zBuffer[x][y] = INFINITY;  // Weit weg
    }
  }
}

void drawPixelWithDepth(int x, int y, float z, uint16_t color) {
  // Nur zeichnen wenn näher als bisheriger Pixel
  if (z < zBuffer[x][y]) {
    lcd.drawPixel(x, y, color);
    zBuffer[x][y] = z;
  }
}
```

**Problem auf ESP32:**
- 240×320×4 Bytes (float) = **307 KB** RAM
- ESP32 hat nur ~320 KB SRAM
- **Zu groß!**

**Alternative: Scanline Z-Buffer**
Nur eine Zeile speichern (240×4 = 960 Bytes):
```cpp
float scanlineZBuffer[240];

void drawTriangleWithDepth(...) {
  for (int y = minY; y <= maxY; y++) {
    clearScanlineZBuffer();

    // Für jedes Pixel in Zeile
    for (int x = minX; x <= maxX; x++) {
      float z = interpolateZ(x, y);  // Interpoliere Tiefe

      if (z < scanlineZBuffer[x]) {
        lcd.drawPixel(x, y, color);
        scanlineZBuffer[x] = z;
      }
    }
  }
}
```

### Lighting & Shading (Beleuchtung)

Für realistischere Grafik: **Beleuchtung** hinzufügen.

**Flat Shading (pro Fläche):**
```cpp
void calculateFaceBrightness(Face& face) {
  // Berechne Normale der Fläche
  Vec3 normal = calculateNormal(face);

  // Licht-Richtung (z.B. von oben-links)
  Vec3 lightDir(-1, 1, -1);
  lightDir = lightDir.normalize();

  // Dot-Product: Winkel zwischen Normale und Licht
  float brightness = max(0.0, normal.dot(lightDir));

  // Ambient Light (Grundhelligkeit)
  brightness = 0.3 + brightness * 0.7;  // 30% ambient + 70% diffuse

  return brightness;
}

void drawFaces() {
  for (int i = 0; i < 6; i++) {
    Face& face = cubeFaces[i];

    // Berechne Helligkeit
    float brightness = calculateFaceBrightness(face);

    // Farbe anpassen
    uint16_t color = adjustBrightness(face.color, brightness);

    drawFilledQuad(..., color);
  }
}

uint16_t adjustBrightness(uint16_t color, float brightness) {
  // RGB565 entpacken
  uint8_t r = (color >> 11) & 0x1F;
  uint8_t g = (color >> 5) & 0x3F;
  uint8_t b = color & 0x1F;

  // Helligkeit anwenden
  r = r * brightness;
  g = g * brightness;
  b = b * brightness;

  // RGB565 packen
  return (r << 11) | (g << 5) | b;
}
```

**Gouraud Shading (pro Vertex):**
Interpoliere Helligkeit über Fläche für smoothen Übergang.

**Phong Shading (pro Pixel):**
Berechne Beleuchtung für jeden Pixel (sehr aufwändig für ESP32).

### Vergleich: 3D-Grafik-Techniken

| Technik | Qualität | Performance | RAM-Bedarf | Komplexität |
|---------|----------|-------------|------------|-------------|
| **Wireframe** | ⭐ | ⭐⭐⭐⭐⭐ | Minimal | Einfach |
| **Flat Shading** | ⭐⭐ | ⭐⭐⭐⭐ | Minimal | Mittel |
| **Painter's Algorithm** | ⭐⭐⭐ | ⭐⭐⭐ | Minimal | Mittel |
| **Z-Buffer (voll)** | ⭐⭐⭐⭐ | ⭐⭐ | 307 KB | Komplex |
| **Scanline Z-Buffer** | ⭐⭐⭐⭐ | ⭐⭐⭐ | 1 KB | Komplex |
| **Gouraud Shading** | ⭐⭐⭐⭐ | ⭐⭐ | Minimal | Komplex |
| **Texture Mapping** | ⭐⭐⭐⭐⭐ | ⭐ | Groß | Sehr komplex |

**Empfehlung für ESP32:**
- **Painter's Algorithm** + **Back-face Culling** (wie in diesem Beispiel)
- Für bessere Qualität: **Flat Shading** hinzufügen
- Für viele Objekte: **Scanline Z-Buffer**

## 🛠️ Praktische Tipps

**Performance-Optimierung:**
- ✅ Nutze `float` statt `double` (ESP32 hat keine Hardware-FPU für double)
- ✅ Vorberechne Trigonometrie (cos/sin) nur einmal pro Frame
- ✅ Vermeide Division (nutze Multiplikation mit 1/x)
- ✅ Nutze Integer-Arithmetik wo möglich (z.B. für Pixel-Koordinaten)
- ✅ Optimiere Rendering (nur sichtbare Flächen, früher Abbruch)

**Speicher-Optimierung:**
- ✅ Nutze `PROGMEM` für konstante Daten (Lookup-Tabellen)
- ✅ Sprites in PSRAM auslagern (wenn verfügbar)
- ✅ Vermeide große Arrays auf dem Stack

**Kalibrierung-Tipps:**
- ✅ Kalibriere in der Umgebung wo du den Würfel nutzen willst
- ✅ Meide Metalle/Motoren beim Kalibrieren
- ✅ Speichere Kalibrierungsdaten im EEPROM für schnellen Start
- ✅ Re-kalibriere bei Standortwechsel (Magnetfeld ändert sich)

**Debugging-Tipps:**
- ✅ Nutze Serial Monitor für Orientierungs-Werte
- ✅ Zeichne Koordinatensystem-Achsen zum Debuggen
- ✅ Teste einzelne Rotationsachsen separat
- ✅ Prüfe Vertex-Reihenfolge (CCW = Counter-Clockwise)

## 📖 Ressourcen & Links

### Datenblätter & Dokumentation

- **BNO055 Datasheet**: [Bosch BNO055 PDF](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bno055-ds000.pdf)
- **Adafruit BNO055 Guide**: [learn.adafruit.com/adafruit-bno055-absolute-orientation-sensor](https://learn.adafruit.com/adafruit-bno055-absolute-orientation-sensor)
- **Sensor-Fusion Whitepaper**: [Bosch Sensor Fusion](https://www.bosch-sensortec.com/media/boschsensortec/downloads/application_notes_1/bst-bno055-an012.pdf)

### 3D-Grafik Theorie

- **3D Math Primer**: [gamemath.com](http://gamemath.com/)
- **Real-Time Rendering**: Buch von Tomas Akenine-Möller
- **Scratchapixel**: [scratchapixel.com](https://www.scratchapixel.com/) - Exzellente 3D-Tutorials

### Quaternionen

- **Quaternions and Rotation Sequences**: Buch von Jack B. Kuipers
- **Visualizing Quaternions**: [eater.net/quaternions](https://eater.net/quaternions)
- **3Blue1Brown Video**: [youtube.com/watch?v=zjMuIxRvygQ](https://www.youtube.com/watch?v=zjMuIxRvygQ)

### CYD-Spezifisch

- **CYD Display Config**: `CYD_Display_Config.h`
- **I2C Pins**: extSDA (GPIO 22), extSCL (GPIO 27)
- **CYD Hardware Guide**: [github.com/witnessmenow/ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)

### Verwandte Beispiele

- **Beispiel 16**: Spirit Level (Wasserwaage mit MPU6050)
- **Beispiel 16b**: Spirit Level Maze (Labyrinth-Spiel)
- **Beispiel 18**: Compass Orientation (Kompass mit BNO055)

## 🎓 Lernziele

Nach Abschluss dieses Beispiels solltest du:

- ✅ **3D-Koordinatensysteme** verstehen (rechtshändig, X/Y/Z-Achsen)
- ✅ **Rotations-Matrizen** anwenden können (Yaw, Pitch, Roll)
- ✅ **Perspektiv-Projektion** implementieren (3D → 2D)
- ✅ **Back-face Culling** verstehen (Sichtbarkeit berechnen)
- ✅ **Painter's Algorithm** für Tiefensortierung nutzen
- ✅ **Vektor-Mathematik** anwenden (Kreuzprodukt, Skalarprodukt, Normalenvektoren)
- ✅ **BNO055 Sensor-Fusion** verstehen (9-DoF, Kalibrierung)
- ✅ **Euler-Winkel vs. Quaternionen** kennen (Vor-/Nachteile)
- ✅ **Exponential Smoothing** für flüssige Animationen einsetzen
- ✅ **3D-Grafik-Pipeline** komplett implementiert haben

## 🔜 Nächste Schritte

Weitere fortgeschrittene Beispiele:

- **Beispiel 24**: Lautstärke-Spektrum mit FFT (INMP441 Mikrofon)
- **Beispiel 25**: Kamera-Integration (OV2640 mit Gesichtserkennung)
- **Beispiel 26**: Machine Learning (TinyML Gesten-Erkennung)

**Empfohlener Lernpfad:**
1. ✅ Beispiel 23 (3D-Würfel) abgeschlossen
2. → Beispiel 18 (Kompass) für mehr BNO055-Features
3. → Beispiel 16b (Maze) für Spiel-Integration
4. → Arcade Games (Master Class) für vollständige Spiele

---

**Viel Erfolg mit dem 3D-Würfel!** 🎲

Bei Fragen: CYD-Academy GitHub Issues oder Community-Forum

---

*Beispiel 23 - CYD-Academy - November 2025*
