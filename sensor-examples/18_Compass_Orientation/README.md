# 🧭 Beispiel 18: Compass & Orientation - 3D Kompass und Lage-Anzeige mit BNO055

## 📝 Was macht dieses Beispiel?

Eine **professionelle 3D-Orientierungsanzeige** mit Kompass und künstlichem Horizont!

**Features:**
- ✅ **3D Kompass** - Vollständige Kompass-Rose mit Himmelsrichtungen
- ✅ **Künstlicher Horizont** - Attitude Indicator wie in Flugzeugen
- ✅ **BNO055 9-DoF Sensor** - Accelerometer + Gyroscope + Magnetometer
- ✅ **Sensor-Fusion** - Automatische Quaternionen-basierte Orientierung
- ✅ **Euler-Winkel** - Heading (Yaw), Roll, Pitch
- ✅ **Kalibrierungsstatus** - Visuell für alle Sensoren
- ✅ **4 Ansichtsmodi** - Combo, Kompass, Horizont, Daten
- ✅ **Smooth Animationen** - Exponential Smoothing
- ✅ **Himmelsrichtungen** - N, NE, E, SE, S, SW, W, NW

**Steuerung:**
- **Touch Links:** Vorheriger Ansichtsmodus
- **Touch Rechts:** Nächster Ansichtsmodus
- **Touch Mitte:** Kalibrierungshinweis

**Ansichtsmodi:**
1. **Combo** - Kompass + Horizont (Standard)
2. **Compass** - Nur Kompass (groß)
3. **Horizon** - Nur Horizont (groß)
4. **Data** - Vollständige Daten-Ansicht

---

## 🎯 Lernziele

1. **I2C-Kommunikation** - Komplexer Sensor über I2C
2. **BNO055 Sensor** - 9-DoF Absolute Orientation Sensor
3. **Euler-Winkel** - Heading, Roll, Pitch verstehen
4. **Quaternionen** - Gimbal-Lock-freie 3D-Orientierung
5. **Sensor-Fusion** - Kombination von Accel, Gyro, Mag
6. **Kalibrierung** - Magnetometer, Gyroskop, Accelerometer
7. **Kompass-Mathematik** - Trigonometrie für Visualisierung
8. **Künstlicher Horizont** - Roll/Pitch Visualisierung

---

## ⚙️ Installation & Setup

### 1. Library installieren

**Arduino IDE:**
1. Library Manager öffnen (Strg+Shift+I)
2. "Adafruit BNO055" suchen
3. Installieren (installiert automatisch Dependencies)

**PlatformIO:**
```ini
[env:esp32]
lib_deps =
  lovyan03/LovyanGFX @ ^1.1.0
  adafruit/Adafruit BNO055 @ ^1.6.0
  adafruit/Adafruit Unified Sensor @ ^1.1.0
```

### 2. Hardware anschließen

**BNO055 (9-DoF IMU):**

```
BNO055           CYD
├─ VIN    →    3.3V oder 5V (Sensor akzeptiert beides!)
├─ GND    →    GND
├─ SDA    →    GPIO 22 (extSDA)
└─ SCL    →    GPIO 27 (extSCL)
```

**HINWEIS:** BNO055 hat einen Level-Shifter onboard und funktioniert mit 3.3V und 5V!

**I2C Pins (CYD Standard):**
- `extSDA = GPIO 22`
- `extSCL = GPIO 27`

### 3. I2C-Adresse

**Standard:** `0x28` (ADR Pin auf GND)

**Alternative:** `0x29` (ADR Pin auf VCC)

Zum Scannen:

```cpp
void scanI2C() {
  Wire.begin(extSDA, extSCL);
  Serial.println("Scanning I2C...");

  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("Found device at 0x%02X\n", addr);
    }
  }
}
```

### 4. Kalibrierung (WICHTIG!)

Der BNO055 benötigt Kalibrierung für optimale Genauigkeit:

**Schritt 1: Magnetometer kalibrieren**
- Bewege Sensor in **Achter-Form** (figure-8) durch die Luft
- Drehe in alle Richtungen
- Dauert ~30 Sekunden

**Schritt 2: Accelerometer & Gyro kalibrieren**
- Neige Sensor in **alle Richtungen**
- Halte kurz in verschiedenen Positionen
- Dauert ~15 Sekunden

**Schritt 3: Warte auf grünen Indikator**
- Kalibrierung: 0 = Rot (schlecht) → 3 = Grün (perfekt)
- System-Kalibrierung muss mindestens 2 sein für gute Ergebnisse

### 5. Kompilieren & Upload

```
Arduino IDE: Upload (Strg+U)
PlatformIO: pio run -t upload
```

### 6. Testen

1. Display zeigt "Calibration" Hinweis
2. Bewege Sensor für Kalibrierung
3. Warte auf grünen Indikator (oben rechts)
4. Drehe CYD → Kompass-Nadel zeigt immer nach Norden!
5. Neige CYD → Horizont zeigt Lage

---

## 🔍 Technische Konzepte

### 1. BNO055 - Absolute Orientation Sensor

**Was ist der BNO055?**

Der **BNO055** ist ein hochintegrierter 9-DoF Sensor von Bosch mit **eingebauter Sensor-Fusion**.

#### Hardware

```
BNO055 = 3 Sensoren + ARM Cortex-M0 Prozessor

Sensoren:
  - Accelerometer (3-Achsen) - Beschleunigung
  - Gyroskop (3-Achsen)      - Drehrate
  - Magnetometer (3-Achsen)  - Magnetfeld (Kompass)

Prozessor:
  - Sensor-Fusion Algorithmus
  - Quaternionen-Berechnung
  - Automatische Kalibrierung
```

**Warum ist das besonders?**

Die meisten IMUs (wie MPU6050) geben nur **Rohdaten**. Du musst selbst Sensor-Fusion implementieren!

Der BNO055 macht das **intern** und gibt direkt:
- ✅ Absolute Orientierung (Quaternionen)
- ✅ Euler-Winkel (Heading, Roll, Pitch)
- ✅ Schwerkraft-Vektor
- ✅ Linear-Beschleunigung (ohne Gravitation)

#### Operating Modes

Der BNO055 hat verschiedene Modi:

```cpp
// Fusion Modes (Sensor-Fusion aktiv)
OPERATION_MODE_NDOF_FMC_OFF  // 9-DoF ohne Fast Mag Calibration
OPERATION_MODE_NDOF          // 9-DoF mit Fast Mag Calibration (Standard)
OPERATION_MODE_IMU           // 6-DoF (ohne Magnetometer)

// Non-Fusion Modes (nur Rohdaten)
OPERATION_MODE_ACCONLY       // Nur Accelerometer
OPERATION_MODE_MAGONLY       // Nur Magnetometer
OPERATION_MODE_GYROONLY      // Nur Gyroskop
```

**Wir nutzen:** `OPERATION_MODE_NDOF` (9-DoF mit allen Sensoren)

---

### 2. Sensor-Fusion

**Was ist Sensor-Fusion?**

Kombiniere **mehrere Sensoren** um bessere Ergebnisse zu erzielen als mit einem allein!

#### Problem ohne Fusion

```
Nur Accelerometer:
  ✅ Misst Gravitation (Neigung)
  ❌ Anfällig für Beschleunigung (z.B. im Auto)
  ❌ Kann nicht zwischen "nach oben neigen" und "beschleunigen" unterscheiden

Nur Gyroskop:
  ✅ Misst Drehrate (sehr präzise kurzfristig)
  ❌ Driftet über Zeit (Integration von Rauschen)
  ❌ Kein absoluter Bezugspunkt

Nur Magnetometer:
  ✅ Zeigt magnetisch Nord
  ❌ Anfällig für Störungen (Metall, Elektronik)
  ❌ Nur Heading, kein Roll/Pitch
```

#### Lösung: Sensor-Fusion

```
Complementary Filter (vereinfacht):

orientation = 0.98 * (gyro_integration) + 0.02 * (accel_angle)

→ Gyro liefert kurzfristige Genauigkeit
→ Accel korrigiert langfristigen Drift

+ Magnetometer korrigiert Heading
```

**BNO055 nutzt Kalman-Filter:**

```
Kalman-Filter ist optimal für Sensor-Fusion:

1. Prediction Step
   - Nutze Gyro für schnelle Änderungen
   - Prädiziere nächsten Zustand

2. Update Step
   - Nutze Accel + Mag als Referenz
   - Korrigiere Prediction
   - Minimiere Fehler

Resultat:
  ✅ Schnelle Response (Gyro)
  ✅ Kein Drift (Accel/Mag Korrektur)
  ✅ Robust gegen Störungen
```

---

### 3. Euler-Winkel

**Die drei Rotations-Achsen:**

```
Euler-Winkel beschreiben Orientierung mit 3 Winkeln:

  Z (Yaw/Heading)
  ↑
  │     Y (Pitch)
  │   ↗
  │ ↗
  └────────→ X (Roll)


Heading (Yaw):   0-360°   Drehung um Z-Achse (Kompass)
Roll:            -180-180° Drehung um X-Achse (seitlich kippen)
Pitch:           -90-90°   Drehung um Y-Achse (vor/zurück kippen)
```

**Anwendung:**

```cpp
sensors_event_t event;
bno.getEvent(&event);

float heading = event.orientation.x;  // 0° = Nord, 90° = Ost, 180° = Süd, 270° = West
float pitch = event.orientation.y;    // + = nach oben, - = nach unten
float roll = event.orientation.z;     // + = rechts, - = links
```

#### Gimbal Lock Problem

**Problem:** Euler-Winkel haben eine Singularität!

```
Gimbal Lock tritt auf bei Pitch = ±90°

Beispiel:
  1. Pitch = 0°, Roll = 0°, Heading = 0°  (horizontal, Nord)
  2. Pitch = 90° (senkrecht nach oben)
  3. Jetzt sind Roll und Heading nicht unterscheidbar!

     Heading = 90°, Roll = 0°  ist gleich wie
     Heading = 0°,  Roll = 90°

→ Verlust einer Freiheitsgrade
→ Unstetige Sprünge
```

**Lösung: Quaternionen!**

---

### 4. Quaternionen

**Was sind Quaternionen?**

```
Quaternion = 4 Zahlen (w, x, y, z)

q = w + x*i + y*j + z*k

Mit:
  i² = j² = k² = -1
  i*j = k,  j*k = i,  k*i = j
```

**Warum Quaternionen für 3D-Rotation?**

```
Vorteile:
  ✅ Kein Gimbal Lock
  ✅ Smooth Interpolation (SLERP)
  ✅ Effiziente Berechnung
  ✅ Direkte Rotations-Darstellung

Nachteile:
  ❌ Weniger intuitiv als Euler-Winkel
  ❌ Schwerer zu visualisieren
```

**BNO055 nutzt intern Quaternionen:**

```cpp
imu::Quaternion quat = bno.getQuat();

float w = quat.w();
float x = quat.x();
float y = quat.y();
float z = quat.z();

// Konvertiere zu Euler (wenn nötig)
float heading = atan2(2*(w*z + x*y), 1 - 2*(y*y + z*z)) * 180/PI;
// ... (komplex, BNO055 macht das für uns!)
```

**Wir nutzen:** Euler-Winkel (einfacher), BNO055 konvertiert intern!

---

### 5. Kompass-Mathematik

#### Nord-Richtung finden

```
Magnetometer misst Erdmagnetfeld (3D-Vektor):

  MagX = Komponente nach Norden
  MagY = Komponente nach Osten
  MagZ = Komponente nach unten

Heading = atan2(MagY, MagX) * 180 / PI

Problem: Neigung verfälscht Messung!
```

#### Tilt-Kompensation

```
Wenn Sensor geneigt ist:
  - Magnetometer misst nicht nur horizontale Komponente
  - Vertikale Komponente verfälscht Heading

Lösung: Rotiere Mag-Vektor zurück zur Horizontalen

1. Messe Roll & Pitch (Accelerometer)
2. Berechne Rotations-Matrix
3. Rotiere Mag-Vektor
4. Berechne Heading aus kompensiertem Vektor

BNO055 macht das automatisch in Sensor-Fusion!
```

#### Deklinationskorrektur

```
Magnetisch Nord ≠ Geographisch Nord

Magnetische Deklination variiert je nach Standort:

  Berlin:     ~4° Ost
  New York:   ~13° West
  Sydney:     ~12° Ost

Korrigiere:
  headingTrue = headingMagnetic + declination
```

**In unserem Beispiel:** Zeigen wir magnetisch Nord (für meiste Anwendungen ausreichend)

---

### 6. Künstlicher Horizont

**Attitude Indicator:**

Visualisiert Roll und Pitch wie in Flugzeugen!

```
Konzept:
  - Himmel = oben (blau)
  - Boden = unten (braun)
  - Horizont-Linie = Trennlinie

Roll:  Horizont rotiert
Pitch: Horizont bewegt sich hoch/runter
```

#### Mathematik

**1. Pitch → Vertikale Position**

```cpp
// Pitch = +90° → schaue nach oben → Horizont ganz unten
// Pitch = -90° → schaue nach unten → Horizont ganz oben

int horizonOffset = -(pitch / 90.0) * halfHeight;
```

**2. Roll → Rotation**

```cpp
float rollRad = roll * PI / 180.0;

// Rotiere jeden Pixel um Mittelpunkt
float rotX = px * cos(rollRad) - py * sin(rollRad);
float rotY = px * sin(rollRad) + py * cos(rollRad);
```

**3. Kombiniere für jeden Pixel**

```cpp
for (int py = -radius; py <= radius; py++) {
  for (int px = -radius; px <= radius; px++) {
    // Rotiere um Roll
    float rotX = px * cos(rollRad) - py * sin(rollRad);
    float rotY = px * sin(rollRad) + py * cos(rollRad);

    // Addiere Pitch-Offset
    int finalY = rotY + horizonOffset;

    // Farbe
    if (finalY < 0) {
      color = SKY;  // Über Horizont
    } else {
      color = GROUND;  // Unter Horizont
    }

    lcd.drawPixel(centerX + px, centerY + py, color);
  }
}
```

---

### 7. Kalibrierung

Der BNO055 hat ein ausgeklügeltes Kalibrierungssystem!

#### Kalibrierungs-Status

```
Jeder Sensor hat Status 0-3:

0 = Unkalibriert (rot)
1 = Wenig Daten (orange)
2 = Gut kalibriert (gelb)
3 = Voll kalibriert (grün)

System-Status:
  - Kombiniert alle Sensoren
  - Mindestens 2 für gute Ergebnisse
```

#### Kalibrierungs-Prozess

**Magnetometer (wichtigster für Kompass):**

```
Ziel: Erfasse Magnetfeld in allen Richtungen

Bewegung:
  1. Achter-Form (figure-8) in der Luft
  2. Drehe Sensor in alle Richtungen
  3. Mehrmals wiederholen

Dauer: ~30 Sekunden

Warum?
  - Magnetfeld variiert je nach Orientierung
  - Sensor muss min/max Werte in allen Achsen kennen
  - Kompensiert Hard-Iron & Soft-Iron Störungen
```

**Accelerometer:**

```
Ziel: Kalibriere Gravitations-Vektor

Bewegung:
  1. Sensor horizontal halten (5 Sekunden)
  2. Auf Seite legen (5 Sekunden)
  3. Senkrecht stellen (5 Sekunden)
  4. Andere Seiten wiederholen

Dauer: ~15 Sekunden

Warum?
  - Erfasse 1g in allen Richtungen (+X, -X, +Y, -Y, +Z, -Z)
  - Kompensiert Offset und Skalierungsfehler
```

**Gyroskop:**

```
Ziel: Kompensiere Drift

Bewegung:
  1. Sensor stillhalten (5 Sekunden)
  2. Langsam drehen in verschiedene Richtungen

Dauer: ~10 Sekunden

Warum?
  - Misst Zero-Rate-Offset (Drift wenn stillgehalten)
  - Kalibriert Empfindlichkeit
```

#### Kalibrierungs-Daten speichern

```cpp
// Kalibrierung auslesen
adafruit_bno055_offsets_t calibData;
bool isCalibrated = bno.isFullyCalibrated();

if (isCalibrated) {
  bno.getSensorOffsets(calibData);

  // Speichere in EEPROM oder SPIFFS
  // Beim nächsten Start laden:
  bno.setSensorOffsets(calibData);
}
```

**Vorteil:** Kalibrierung nur einmal nötig!

---

## 🚀 Experimente & Erweiterungen

### 1. 📊 GPS-Integration (Heading mit Bewegungsrichtung)

**Idee:** Vergleiche Kompass-Heading mit GPS-Richtung!

```cpp
#include <TinyGPS++.h>

TinyGPSPlus gps;
HardwareSerial gpsSerial(1);

void setup() {
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);
}

void loop() {
  // GPS-Kurs (Bewegungsrichtung)
  if (gps.course.isValid()) {
    float gpsCourse = gps.course.deg();  // 0-360°

    // Vergleiche mit Kompass
    float difference = abs(heading - gpsCourse);
    if (difference > 180) difference = 360 - difference;

    Serial.printf("Compass: %.1f°  GPS: %.1f°  Diff: %.1f°\n",
                  heading, gpsCourse, difference);

    // Zeige beide auf Display
    drawCompass(heading);      // Magnetisch
    drawGPSArrow(gpsCourse);   // Bewegungsrichtung
  }
}
```

**Anwendung:** Navigation! Zeige ob du in richtige Richtung gehst.

---

### 2. 🎮 VR Head-Tracking

**Idee:** Nutze als Head-Tracker für Virtual Reality!

```cpp
// Sende Orientierung über Serial zu PC/VR-Software
void sendVRData() {
  imu::Quaternion quat = bno.getQuat();

  // OpenVR/SteamVR Format
  Serial.printf("VR,%f,%f,%f,%f\n",
                quat.w(), quat.x(), quat.y(), quat.z());
}

// PC-Software (Python + OpenVR):
import serial
import openvr

ser = serial.Serial('COM3', 115200)

while True:
    line = ser.readline().decode()
    if line.startswith('VR,'):
        parts = line.split(',')
        w, x, y, z = map(float, parts[1:5])

        # Update VR Headset Pose
        pose = [[w, x, y, z], [0, 0, 0]]  # Quaternion + Position
        vr.setPose(pose)
```

---

### 3. 🚁 Drone Telemetrie

**Idee:** Baue Telemetrie-Display für Drohne!

```cpp
// Erweitere UI mit zusätzlichen Infos
void drawDroneTelemetry() {
  // Horizont (wie im Beispiel)
  drawHorizon(120, 140, 200, 160);

  // Höhe (von Barometer)
  lcd.setCursor(10, 10);
  lcd.printf("ALT: %dm", altitude);

  // Geschwindigkeit (von GPS)
  lcd.setCursor(10, 30);
  lcd.printf("SPD: %.1fm/s", speed);

  // Batterie
  lcd.setCursor(10, 50);
  lcd.printf("BAT: %d%%", battery);

  // GPS Satellit

en
  lcd.setCursor(10, 70);
  lcd.printf("SAT: %d", satellites);

  // Home-Richtung (Pfeil zurück zum Start)
  float homeDirection = calculateBearing(currentLat, currentLng, homeLat, homeLng);
  drawHomeArrow(homeDirection - heading);
}
```

---

### 4. 🏔️ Augmented Reality Kompass

**Idee:** Overlays auf Kamera-Bild!

```cpp
#include <esp32cam.h>

void drawARCompass() {
  // Zeige Kamera-Bild
  displayCameraFrame();

  // Overlay: Kompass-Ring (halbtransparent)
  drawTransparentCompass(120, 280, 50);

  // Overlay: POI-Marker (Points of Interest)
  struct POI {
    const char* name;
    float bearing;  // Richtung vom Standort
    int distance;   // Meter
  };

  POI pois[] = {
    {"Brandenburger Tor", 45.5, 1200},
    {"Fernsehturm", 315.0, 800},
    {"Reichstag", 30.0, 1500}
  };

  for (POI& poi : pois) {
    // Berechne Position auf Screen basierend auf Heading
    float relBearing = poi.bearing - heading;
    if (relBearing < -180) relBearing += 360;
    if (relBearing > 180) relBearing -= 360;

    // Nur zeigen wenn im Sichtfeld (-90° bis +90°)
    if (abs(relBearing) < 90) {
      int x = 120 + relBearing * 2;  // Skalierung
      int y = 50;

      // Zeichne Marker
      lcd.fillCircle(x, y, 5, COLOR_WARNING);
      lcd.setTextSize(1);
      lcd.setCursor(x - 20, y + 10);
      lcd.print(poi.name);
      lcd.setCursor(x - 15, y + 20);
      lcd.printf("%dm", poi.distance);
    }
  }
}
```

---

### 5. 🧲 Metall-Detektor

**Idee:** Nutze Magnetometer-Stärke zur Metallerkennung!

```cpp
void detectMetal() {
  imu::Vector<3> mag = bno.getVector(Adafruit_BNO055::VECTOR_MAGNETOMETER);

  // Berechne Feldstärke
  float fieldStrength = sqrt(mag.x()*mag.x() + mag.y()*mag.y() + mag.z()*mag.z());

  // Erdmagnetfeld: ~25-65 µT (je nach Standort)
  float baseline = 50.0;  // µT

  float deviation = abs(fieldStrength - baseline);

  // Visualisierung
  if (deviation > 20) {
    lcd.fillScreen(COLOR_DANGER);
    lcd.setTextSize(3);
    lcd.setCursor(50, 140);
    lcd.print("METAL!");

    // Ton
    ledcWriteTone(0, 1000);
  } else {
    lcd.fillScreen(COLOR_SAFE);
    ledcWriteTone(0, 0);
  }

  // Balken-Anzeige
  int barHeight = map(deviation, 0, 50, 0, 200);
  lcd.fillRect(100, 300 - barHeight, 40, barHeight, COLOR_WARNING);
}
```

---

### 6. 📐 Neigungsmesser für Bilder aufhängen

**Idee:** Präziser Neigungsmesser mit Grad-Anzeige!

```cpp
void drawLevelTool() {
  lcd.fillScreen(COLOR_BG);

  // Große Roll-Anzeige
  lcd.setTextSize(5);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(40, 100);

  if (abs(roll) < 0.5) {
    lcd.setTextColor(COLOR_CAL_FULL);
    lcd.print("LEVEL");
  } else {
    if (roll > 0) {
      lcd.print("+");
    }
    lcd.printf("%.1f", roll);
    lcd.print((char)247);
  }

  // Balken-Anzeige (wie Wasserwaage)
  int barX = 120;
  int barY = 200;
  int offset = roll * 3;  // Skalierung

  lcd.fillCircle(barX + offset, barY, 15, COLOR_DANGER);
  lcd.drawLine(20, barY, 220, barY, COLOR_TEXT);
  lcd.drawLine(barX, barY - 20, barX, barY + 20, COLOR_CAL_FULL);

  // Pitch auch anzeigen
  lcd.setTextSize(2);
  lcd.setCursor(80, 250);
  lcd.printf("Pitch: %.1f", pitch);
  lcd.print((char)247);
}
```

---

### 7. 🏃 Step Counter & Activity Tracking

**Idee:** Schrittzähler basierend auf Beschleunigung!

```cpp
// Schrittzähler-Logik
int stepCount = 0;
float lastAccelMagnitude = 0;
unsigned long lastStepTime = 0;

void detectSteps() {
  imu::Vector<3> linearAccel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);

  // Berechne Magnitude (ohne Gravitation)
  float accelMag = sqrt(linearAccel.x()*linearAccel.x() +
                        linearAccel.y()*linearAccel.y() +
                        linearAccel.z()*linearAccel.z());

  // Schritt-Erkennung: Peak-Detection
  if (accelMag > 1.5 && lastAccelMagnitude < 1.5) {
    // Aufwärts-Schwelle überschritten

    unsigned long now = millis();
    if (now - lastStepTime > 300) {  // Debounce (min 300ms zwischen Schritten)
      stepCount++;
      lastStepTime = now;

      Serial.printf("Step detected! Total: %d\n", stepCount);
    }
  }

  lastAccelMagnitude = accelMag;
}

// Aktivitäts-Erkennung
void detectActivity() {
  imu::Vector<3> linearAccel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
  float accelMag = sqrt(...);

  if (accelMag < 0.2) {
    activity = "Still";
  } else if (accelMag < 1.0) {
    activity = "Walking";
  } else if (accelMag < 3.0) {
    activity = "Running";
  } else {
    activity = "Jumping";
  }
}
```

---

## 🐛 Troubleshooting

### Problem: "BNO055 not found!"

**Symptom:** Sensor wird nicht erkannt.

**Ursachen:**
1. Falsche Verkabelung
2. Falsche I2C-Adresse
3. Defektes Modul
4. SDA/SCL vertauscht

**Debug:**

```cpp
void debugBNO055() {
  Wire.begin(extSDA, extSCL);

  // I2C-Scan
  Serial.println("Scanning I2C...");
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("Device at 0x%02X\n", addr);
    }
  }

  // Erwartete Adresse: 0x28 oder 0x29
}
```

**Lösung:**
- Überprüfe VCC (3.3V oder 5V), GND, SDA (GPIO 22), SCL (GPIO 27)
- Prüfe I2C-Adresse (ADR Pin auf GND = 0x28, auf VCC = 0x29)
- Teste mit anderem Modul

---

### Problem: Kompass zeigt falsche Richtung

**Symptom:** Nord-Pfeil zeigt nicht nach Norden.

**Ursachen:**
1. Magnetometer nicht kalibriert
2. Magnetische Störungen
3. Metall in der Nähe

**Lösung:**

```cpp
// 1. Prüfe Kalibrierungsstatus
uint8_t sys, gyro, accel, mag;
bno.getCalibration(&sys, &gyro, &accel, &mag);

Serial.printf("Mag Calibration: %d/3\n", mag);

if (mag < 2) {
  Serial.println("Magnetometer not calibrated!");
  Serial.println("Move in figure-8 motion");
}

// 2. Prüfe auf Störungen
imu::Vector<3> mag = bno.getVector(Adafruit_BNO055::VECTOR_MAGNETOMETER);
float fieldStrength = sqrt(mag.x()*mag.x() + mag.y()*mag.y() + mag.z()*mag.z());

// Erdmagnetfeld: 25-65 µT
if (fieldStrength < 20 || fieldStrength > 80) {
  Serial.println("Magnetic interference detected!");
}
```

**Tipps:**
- Kalibriere fernab von Metall und Elektronik
- Vermeide Stahlgebäude, Autos, Handys
- Outdoor-Kalibrierung ist besser als Indoor

---

### Problem: Horizont "wackelt" oder springt

**Symptom:** Unstete Anzeige trotz ruhigem Sensor.

**Ursachen:**
1. Zu niedriger SMOOTH_FACTOR
2. Sensor nicht voll kalibriert
3. Vibrationen

**Lösung:**

```cpp
// Erhöhe Smoothing
const float SMOOTH_FACTOR = 0.05;  // War: 0.1 → Jetzt: 0.05 (mehr Smoothing)

// Oder: Exponential Moving Average
float emaRoll = 0;
const float EMA_ALPHA = 0.1;

void smoothRoll() {
  emaRoll = EMA_ALPHA * roll + (1 - EMA_ALPHA) * emaRoll;
}

// Oder: Median-Filter (3 Samples)
float rollSamples[3] = {0, 0, 0};
int sampleIndex = 0;

float getMedianRoll() {
  rollSamples[sampleIndex] = roll;
  sampleIndex = (sampleIndex + 1) % 3;

  float sorted[3];
  memcpy(sorted, rollSamples, sizeof(rollSamples));
  sort(sorted, sorted + 3);

  return sorted[1];  // Median
}
```

---

### Problem: Gimbal Lock bei Pitch = 90°

**Symptom:** Kompass spinnt wenn Sensor senkrecht steht.

**Ursache:** Euler-Winkel Singularität.

**Lösung:**

```cpp
// Option 1: Nutze Quaternionen direkt
imu::Quaternion quat = bno.getQuat();

// Konvertiere zu Euler nur für Display
// Logik nutzt Quaternionen

// Option 2: Begrenze Pitch
if (abs(pitch) > 85) {
  // Zeige Warnung
  lcd.print("GIMBAL LOCK!");
  // Oder: Nutze letzten gültigen Wert
}

// Option 3: Nutze Axis-Angle Representation
imu::Vector<3> axis = ...;  // Rotations-Achse
float angle = ...;           // Rotations-Winkel

// Keine Singularität!
```

---

### Problem: Kalibrierung geht verloren

**Symptom:** Nach Reset/Neustart muss neu kalibriert werden.

**Lösung: Speichere Kalibrierungsdaten**

```cpp
#include <EEPROM.h>

#define EEPROM_SIZE 512
#define CAL_DATA_ADDR 0

// Speichere Kalibrierung
void saveCalibration() {
  adafruit_bno055_offsets_t calibData;
  bool isCalibrated = bno.isFullyCalibrated();

  if (isCalibrated) {
    bno.getSensorOffsets(calibData);

    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(CAL_DATA_ADDR, calibData);
    EEPROM.commit();

    Serial.println("Calibration saved to EEPROM");
  }
}

// Lade Kalibrierung
void loadCalibration() {
  adafruit_bno055_offsets_t calibData;

  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(CAL_DATA_ADDR, calibData);

  // Prüfe ob gültig (einfache Heuristik)
  if (calibData.accel_offset_x != 0xFFFF) {  // Nicht leer
    bno.setSensorOffsets(calibData);
    Serial.println("Calibration loaded from EEPROM");
  } else {
    Serial.println("No calibration data found");
  }
}

// Im Setup:
void setup() {
  // ...
  bno.begin();
  loadCalibration();  // Lade gespeicherte Kalibrierung

  // Nach Kalibrierung:
  if (bno.isFullyCalibrated()) {
    saveCalibration();
  }
}
```

---

## 📚 Weiterführende Konzepte

### 1. AHRS (Attitude and Heading Reference System)

**Professionelle Orientierungs-Systeme:**

```cpp
// Erweiterte Sensor-Fusion mit zusätzlichen Sensoren

Barometer → Höhe
GPS → Position & Geschwindigkeit
Magnetometer → Heading
Accel + Gyro → Attitude

→ Vollständiges AHRS

Anwendungen:
  - Flugzeuge
  - Drohnen
  - Schiffe
  - Roboter
```

### 2. Extended Kalman Filter (EKF)

**Noch präzisere Sensor-Fusion:**

```
Kalman-Filter für lineare Systeme
Extended Kalman-Filter für nicht-lineare Systeme (wie Rotation!)

BNO055 nutzt vereinfachten EKF:
  - Prädiziere mit Gyro (Prozess-Modell)
  - Update mit Accel + Mag (Messungs-Modell)
  - Minimiere Kovarianz (Unsicherheit)
```

### 3. Sensor Coordinate Frames

**Wichtig für Multi-Sensor-Systeme:**

```
Body Frame:    Sensor-festes Koordinatensystem
World Frame:   Globales Koordinatensystem (Nord/Ost/Down)
Display Frame: Display-Koordinaten

Transformationen zwischen Frames:
  - Rotations-Matrizen
  - Quaternionen
  - Euler-Winkel

BNO055 gibt World Frame aus (praktisch!)
```

---

## 🎓 Was hast du gelernt?

Nach diesem Beispiel verstehst du:

- ✅ **BNO055 Sensor** - 9-DoF mit integrierter Sensor-Fusion
- ✅ **Euler-Winkel** - Heading, Roll, Pitch
- ✅ **Quaternionen** - Gimbal-Lock-freie 3D-Orientierung
- ✅ **Sensor-Fusion** - Kombination von Accel, Gyro, Mag
- ✅ **Kalibrierung** - Magnetometer, Accelerometer, Gyroskop
- ✅ **Kompass-Visualisierung** - Trigonometrie & Rotation
- ✅ **Künstlicher Horizont** - Roll/Pitch Darstellung
- ✅ **Smooth Animationen** - Exponential Smoothing

**Nächste Schritte:**
1. Experimentiere mit verschiedenen Ansichtsmodi
2. Implementiere GPS-Integration
3. Baue Drone-Telemetrie
4. Nutze für VR Head-Tracking
5. Erweitere mit Barometer (Höhe)

---

## 🔗 Siehe auch

- [BNO055 Datasheet](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bno055-ds000.pdf)
- [Adafruit BNO055 Guide](https://learn.adafruit.com/adafruit-bno055-absolute-orientation-sensor)
- [Sensor Fusion Explained](https://www.nxp.com/docs/en/application-note/AN3461.pdf)

**Externe Ressourcen:**
- [Quaternions Tutorial](https://www.3dgep.com/understanding-quaternions/)
- [Euler Angles vs Quaternions](https://www.youtube.com/watch?v=zjMuIxRvygQ)
- [Kalman Filter Explained](https://www.kalmanfilter.net/)

---

**Professionelle 3D-Orientierung mit modernster Sensor-Fusion! 🧭✨**
