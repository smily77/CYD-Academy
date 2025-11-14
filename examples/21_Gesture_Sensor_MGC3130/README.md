# Beispiel 21: 3D-Gestensensor MGC3130

Dieses Beispiel demonstriert berührungslose 3D-Gestensteuerung mit dem **MGC3130 GestIC Sensor** von Microchip. Der Sensor erkennt Hand-Position in 3D, Wischgesten, Touch-Events und kreisförmige Bewegungen (AirWheel).

## 📸 Features

- **🎯 3D-Positionserkennung**:
  - X, Y, Z Koordinaten (0-65535)
  - Erkennungsbereich: 0-15 cm über dem Sensor
  - Echtzeit-Tracking mit 200 Hz
- **👋 Gestenerkennung**:
  - Swipe Links/Rechts (West-East, East-West)
  - Swipe Hoch/Runter (South-North, North-South)
  - Kreisbewegungen (Uhrzeigersinn/Gegen Uhrzeigersinn)
- **👆 Touch-Events**:
  - 5 kapazitive Elektroden (N, S, E, W, Center)
  - Tap-Erkennung (einfach und doppelt)
  - Approach-Detection (Annäherung)
- **🎡 AirWheel**:
  - Kreisförmige Drehgesten
  - Winkel-Tracking (0-360°)
  - Kontinuierliche Rotation
- **🎨 Vier Anzeigemodi**:
  - 3D Position: Würfel-Visualisierung mit Punkt
  - Gesten: Animierte Pfeile und Icons
  - Touch: Interaktive Elektroden-Anzeige
  - AirWheel: Drehendes Rad mit Winkel

## 🔌 Hardware-Anforderungen

### Hauptkomponenten
- **ESP32-2432S028R** (CYD - Cheap Yellow Display)
- **MGC3130 Gesture Sensor** (verschiedene Breakout-Boards)

### MGC3130 Module (Auswahl)

**Pimoroni Skywriter HAT**:
- Kompaktes Board mit MGC3130
- 5 Touch-Elektroden integriert
- I2C Interface
- 3,3V Betrieb
- Transfer/Reset Pins optional

**Seeed Grove Gesture Sensor**:
- Grove-Connector (I2C)
- Kompakte Bauform
- Integrierte Touch-Pads
- 3,3V - 5V tolerant

**Generic MGC3130 Breakout**:
- Basis-Board nur mit Chip
- Externe Elektroden erforderlich
- Flexible Anpassung

### MGC3130 Spezifikationen

**Technologie**: GestIC (Gesture Interface Controller)

**Erkennungsprinzip**:
- E-Field Sensing (Elektrisches Feld)
- Kapazitive 3D-Erkennung
- Keine Kamera/Optik erforderlich

**Messbereiche**:
- **X-Achse**: 0-65535 (Links-Rechts)
- **Y-Achse**: 0-65535 (Vorne-Hinten)
- **Z-Achse**: 0-65535 (Höhe: 0-15 cm)
- **Auflösung**: 16 Bit (sehr präzise)
- **Update-Rate**: bis zu 200 Hz

**Interface**:
- **I2C**: Standard (100 kHz) oder Fast (400 kHz)
- **I2C-Adresse**: 0x42 (fest, nicht änderbar)
- **Transfer Pin**: GPIO für Daten-Ready Signal (optional)
- **Reset Pin**: Hardware-Reset (optional)

**Stromverbrauch**:
- Idle: ca. 2 mA
- Aktiv (Tracking): ca. 5-10 mA
- Sleep Mode: < 10 µA

**Gesten-Erkennung**:
- West-East (Links → Rechts)
- East-West (Rechts → Links)
- South-North (Unten → Oben)
- North-South (Oben → Unten)
- Clockwise (Kreis im Uhrzeigersinn)
- Counter-Clockwise (Kreis gegen Uhrzeigersinn)

**Touch-Elektroden**: 5 kapazitive Bereiche
- North, South, East, West, Center
- Tap und Double-Tap Erkennung
- Berührung oder Annäherung

### Verbindung (I2C)

```
MGC3130        CYD (ESP32-2432S028R)
-------        ---------------------
VCC     -----> 3.3V
GND     -----> GND
SDA     -----> GPIO 22 (extSDA)
SCL     -----> GPIO 27 (extSCL)
TS      -----> (optional, für Transfer Signal)
RST     -----> (optional, für Hardware-Reset)
```

**Wichtig**:
- MGC3130 arbeitet mit 3,3V (5V kann Sensor beschädigen!)
- I2C Pull-ups sind auf CYD bereits vorhanden
- Transfer (TS) Pin ist optional - Polling funktioniert auch
- Reset Pin ist optional - Software-Reset via I2C möglich

## 🔬 Technischer Hintergrund

### GestIC Technologie

**GestIC = Gesture Interface Controller**

Der MGC3130 nutzt **elektrisches Feld-Sensing** (E-Field) zur 3D-Positions- und Gestenerkennung:

#### Funktionsprinzip

```
┌─────────────────────────────────────────┐
│  MGC3130 E-Field Sensing:               │
│                                          │
│  1. Sensor erzeugt elektrisches Feld    │
│     ↓                                    │
│  2. Hand stört das Feld                 │
│     ↓                                    │
│  3. 4 Rx-Elektroden messen Änderungen   │
│     ↓                                    │
│  4. Algorithmus berechnet 3D-Position   │
│                                          │
│  ┌──────────────┐                       │
│  │  Tx (Sender) │  → Elektrisches Feld  │
│  └──────────────┘                       │
│         ↑                                │
│    Hand stört                            │
│         ↓                                │
│  ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐       │
│  │ Rx1 │ │ Rx2 │ │ Rx3 │ │ Rx4 │       │
│  └─────┘ └─────┘ └─────┘ └─────┘       │
│  (Empfänger messen Feldstärke)          │
│                                          │
└─────────────────────────────────────────┘
```

**Schritte im Detail**:

1. **Feldgenerierung**:
   - Tx-Elektrode sendet hochfrequentes elektrisches Feld (~100 kHz)
   - Feld breitet sich über Sensor aus (ca. 15 cm Reichweite)

2. **Störung durch Hand**:
   - Hand ist leitfähig (Wasser/Salze im Körper)
   - Verändert lokale Feldstärke
   - Kapazitive Kopplung

3. **Messung**:
   - 4 Rx-Elektroden (in verschiedenen Positionen)
   - Messen Feldstärke-Änderungen
   - Differenzmessung für 3D-Lokalisierung

4. **Berechnung**:
   - Proprietary Algorithmus (Microchip)
   - Triangulation basierend auf 4 Messungen
   - Ausgabe: X, Y, Z Koordinaten

### E-Field vs. andere Technologien

| Technologie | Prinzip | Reichweite | Genauigkeit | Preis |
|-------------|---------|------------|-------------|-------|
| **E-Field (MGC3130)** | Elektr. Feld | 0-15 cm | Sehr hoch | ~15€ |
| Optisch (APDS-9960) | Infrarot-Reflexion | 0-20 cm | Mittel | ~5€ |
| Kamera (Leap Motion) | Stereo-Kamera | 0-60 cm | Sehr hoch | ~80€ |
| Ultraschall | Schallwellen | 2-400 cm | Niedrig | ~3€ |
| ToF (VL53L1X) | Laser-Laufzeit | 0-400 cm | Hoch (1D) | ~10€ |

**Vorteile E-Field**:
- ✅ Keine Optik → funktioniert bei Dunkelheit
- ✅ Keine beweglichen Teile
- ✅ Sehr präzise (16-Bit Auflösung)
- ✅ Geringer Stromverbrauch
- ✅ Kompakt

**Nachteile E-Field**:
- ❌ Begrenzte Reichweite (15 cm)
- ❌ Nur für leitfähige Objekte (Hand)
- ❌ Störungen durch Metall/Elektronik
- ❌ Kalibrierung nötig bei Umgebungsänderungen

### 3D-Koordinaten-System

Der MGC3130 liefert 3D-Koordinaten:

```
      Z (Höhe: 0-15 cm)
      ↑
      │
      │     Y (Tiefe: Vorne-Hinten)
      │    ↗
      │   ↗
      │  ↗
      │ ↗
      │↗
      └────────────→ X (Breite: Links-Rechts)
     Sensor
```

**Koordinaten-Werte**:
- **X**: 0 (ganz links) bis 65535 (ganz rechts)
- **Y**: 0 (hinten) bis 65535 (vorne, nah am Sensor)
- **Z**: 0 (auf Sensor) bis 65535 (15 cm über Sensor)

**Normalisierung** (für einfachere Verwendung):
```cpp
float normX = posX / 65535.0;  // 0.0 - 1.0
float normY = posY / 65535.0;  // 0.0 - 1.0
float normZ = posZ / 65535.0;  // 0.0 - 1.0
```

### Gesten-Algorithmus

Der MGC3130 hat einen **Hardware-Gesten-Erkenner** integriert:

#### Wie werden Gesten erkannt?

1. **Position-Tracking**:
   - Kontinuierliche Messung der 3D-Position (200 Hz)
   - Speicherung der letzten ~100ms

2. **Bewegungsvektor-Analyse**:
   - Berechnung von Geschwindigkeit und Richtung
   - Erkennung von konsistenten Bewegungen

3. **Pattern-Matching**:
   - Vergleich mit vordefinierten Gesten-Mustern
   - Schwellenwerte für Geschwindigkeit/Distanz

4. **Ausgabe**:
   - Gesten-Event nur bei klarer Erkennung
   - Vermeidung von Fehlerkennungen

**Beispiel: Swipe Right**
```
1. Hand erscheint links (X = 10000)
2. Hand bewegt sich nach rechts (X steigt)
3. Geschwindigkeit > Schwelle (schnelle Bewegung)
4. Konsistente Richtung über 200ms
5. Hand verlässt Sensor rechts (X = 55000)
→ Event: GESTURE_WEST_EAST
```

**Mindest-Anforderungen** (ungefähr):
- Bewegungsdistanz: > 50% des Sensor-Bereichs
- Geschwindigkeit: > 20 cm/s
- Konsistenz: > 80% der Samples in gleiche Richtung

### Touch-Elektroden

Der MGC3130 hat **5 kapazitive Touch-Bereiche**:

```
Layout (von oben betrachtet):
        ┌─────┐
        │  N  │  Nord
        └─────┘
┌─────┐ ┌─────┐ ┌─────┐
│  W  │ │  C  │ │  E  │  West - Center - East
└─────┘ └─────┘ └─────┘
        ┌─────┐
        │  S  │  Süd
        └─────┘
```

**Touch-Modi**:

1. **Touch**: Direkte Berührung der Elektrode
2. **Tap**: Kurzes Antippen (< 300ms)
3. **Double-Tap**: Zwei Taps schnell hintereinander (< 500ms)
4. **Approach**: Annäherung ohne Berührung (ca. 1-3 cm)

**Hardware-Implementation**:
- Eigenständiger kapazitiver Controller im MGC3130
- Unabhängig von 3D-Position-Tracking
- Kann gleichzeitig mit Gesten genutzt werden

### AirWheel

**AirWheel** ist eine spezielle Gesten-Art für **Rotations-Steuerung**:

#### Konzept
```
Kreisförmige Bewegung über dem Sensor:

      ↑
   ←     →  (Hand kreist)
      ↓

→ Ausgabe: Delta-Winkel pro Frame
```

**Funktionsweise**:
1. Hand macht kreisförmige Bewegung
2. MGC3130 erkennt Bewegungsrichtung
3. Berechnet Delta-Winkel (z.B. +5° oder -5°)
4. Akkumuliert für Gesamt-Rotation

**Verwendung**:
```cpp
if (gesture.airWheelAvailable()) {
  int8_t delta = gesture.getAirWheelDelta();
  totalAngle += delta;  // z.B. für Lautstärke, Helligkeit
}
```

**Anwendungen**:
- Lautstärke-Regler (wie physisches Drehrad)
- Helligkeit einstellen
- Menü-Navigation (Kreismenü)
- Werte inkrementieren/dekrementieren

## 📡 I2C-Kommunikation

### SparkFun MGC3130 Library

Der MGC3130 hat ein **komplexes Protokoll**. Die SparkFun-Library abstrahiert dies:

```cpp
#include <MGC3130.h>

MGC3130 gesture;

// Initialisierung
if (!gesture.begin(&Wire, 0x42)) {
  Serial.println("MGC3130 nicht gefunden!");
}

// Features aktivieren
gesture.enableGestures(true);
gesture.enableAirWheel(true);
gesture.enableTouch(true);

// Daten auslesen
if (gesture.dataAvailable()) {
  // 3D Position
  if (gesture.positionAvailable()) {
    uint16_t x = gesture.getX();
    uint16_t y = gesture.getY();
    uint16_t z = gesture.getZ();
  }

  // Gesten
  if (gesture.gestureAvailable()) {
    uint8_t gest = gesture.getGesture();
    // GESTURE_WEST_EAST, GESTURE_EAST_WEST, etc.
  }

  // Touch
  if (gesture.touchAvailable()) {
    uint16_t touch = gesture.getTouchInfo();
    // TOUCH_NORTH, TAP_CENTER, DOUBLE_TAP_SOUTH, etc.
  }

  // AirWheel
  if (gesture.airWheelAvailable()) {
    int8_t delta = gesture.getAirWheelDelta();
  }
}
```

### I2C-Protokoll (Low-Level)

**Für Fortgeschrittene** - Das MGC3130-Protokoll im Detail:

#### Register-Map (vereinfacht)

```
Register    Adresse    Funktion              R/W
--------------------------------------------------------
DATA_OUT    0x0000     Sensor-Daten (132 Bytes) R
STATUS      0x0001     Status-Register         R
CONFIG      0x0006     Konfiguration           R/W
CALIB       0x0020     Kalibrierung            W
FW_VERSION  0x0083     Firmware-Version        R
```

#### Daten-Paket-Struktur

MGC3130 sendet **132-Byte Daten-Pakete**:

```
Byte 0-3:   Header (Magic Number, Size)
Byte 4:     Flags (welche Daten gültig sind)
Byte 5:     Sequence Number
Byte 6-7:   Data Output Enable Mask
Byte 8-9:   Timestamp
Byte 10-11: System Info
Byte 12-25: DSP Status
Byte 26-27: Gesture Info
Byte 28-29: Touch Info
Byte 30-31: AirWheel Info
Byte 32-37: XYZ Position (jeweils 16 Bit)
Byte 38-131: Reserved/Future
```

**Position auslesen** (Low-Level):
```cpp
Wire.requestFrom(0x42, 132);
byte data[132];
for (int i = 0; i < 132; i++) {
  data[i] = Wire.read();
}

uint16_t x = (data[33] << 8) | data[32];  // X Position
uint16_t y = (data[35] << 8) | data[34];  // Y Position
uint16_t z = (data[37] << 8) | data[36];  // Z Position
```

**Empfehlung**: Nutze die SparkFun Library - das Protokoll ist sehr komplex!

## 💻 Code-Struktur

### Hauptkomponenten

#### 1. Sensor-Initialisierung

```cpp
#include <MGC3130.h>

MGC3130 gesture;

void setup() {
  Wire.begin(extSDA, extSCL);

  if (!gesture.begin(&Wire, 0x42)) {
    Serial.println("MGC3130 nicht gefunden!");
    return;
  }

  // Features aktivieren
  gesture.enableGestures(true);      // Swipe-Gesten
  gesture.enableAirWheel(true);      // Kreisförmige Bewegungen
  gesture.enableTouch(true);         // Touch-Elektroden
  gesture.enableApproach(true);      // Annäherungs-Erkennung
}
```

#### 2. Daten auslesen

```cpp
void readSensor() {
  if (!gesture.dataAvailable()) return;

  // 3D Position
  if (gesture.positionAvailable()) {
    posX = gesture.getX();  // 0-65535
    posY = gesture.getY();
    posZ = gesture.getZ();

    // Normalisieren
    normX = posX / 65535.0;  // 0.0-1.0
    normY = posY / 65535.0;
    normZ = posZ / 65535.0;
  }

  // Gesten
  if (gesture.gestureAvailable()) {
    uint8_t gest = gesture.getGesture();

    switch (gest) {
      case GESTURE_WEST_EAST:
        Serial.println("Swipe Rechts");
        break;
      case GESTURE_EAST_WEST:
        Serial.println("Swipe Links");
        break;
      // ...
    }
  }

  // Touch
  if (gesture.touchAvailable()) {
    uint16_t touch = gesture.getTouchInfo();

    if (touch & TAP_CENTER) {
      Serial.println("Center getappt");
    }
    if (touch & DOUBLE_TAP_NORTH) {
      Serial.println("Nord doppelt getappt");
    }
  }

  // AirWheel
  if (gesture.airWheelAvailable()) {
    int8_t delta = gesture.getAirWheelDelta();
    airwheelAngle += delta;

    // Wrap-around
    if (airwheelAngle > 360) airwheelAngle -= 360;
    if (airwheelAngle < 0) airwheelAngle += 360;
  }
}
```

#### 3. 3D-Visualisierung

```cpp
void draw3DPosition() {
  // Würfel als Referenz
  int centerX = 160, centerY = 200;
  int cubeSize = 80;

  lcd.drawRect(centerX - cubeSize, centerY - cubeSize/2,
               cubeSize * 2, cubeSize, TFT_DARKGREY);

  // Position als Punkt
  // X: Links-Rechts, Y: Vorne-Hinten, Z: Höhe
  int pointX = centerX + (normX - 0.5) * cubeSize * 2;
  int pointY = centerY + (normY - 0.5) * cubeSize * 1.5
                       - (normZ - 0.5) * cubeSize;

  // Größe abhängig von Z (näher = größer)
  int pointSize = map(posZ, 0, 65535, 3, 15);
  lcd.fillCircle(pointX, pointY, pointSize, TFT_RED);
}
```

#### 4. Gesten-Icons

```cpp
void drawGestureIcon(GestureType gest) {
  int centerX = 160, centerY = 180;
  int arrowSize = 60;

  switch (gest) {
    case GESTURE_SWIPE_RIGHT:
      // Pfeil nach rechts
      lcd.fillTriangle(centerX + arrowSize, centerY,
                       centerX - arrowSize/2, centerY - arrowSize,
                       centerX - arrowSize/2, centerY + arrowSize,
                       TFT_YELLOW);
      break;

    case GESTURE_CIRCLE_CW:
      // Kreis mit Pfeil (Uhrzeigersinn)
      lcd.drawCircle(centerX, centerY, arrowSize, TFT_YELLOW);
      lcd.fillTriangle(centerX + arrowSize, centerY,
                       centerX + arrowSize - 15, centerY - 15,
                       centerX + arrowSize - 15, centerY + 15,
                       TFT_YELLOW);
      break;

    // ... andere Gesten
  }
}
```

#### 5. Touch-Visualisierung

```cpp
void drawTouchPads() {
  int centerX = 160, centerY = 200;
  int padSpacing = 80;

  // Center
  uint16_t color = touchCenter ? TFT_GREEN : TFT_DARKGREY;
  lcd.fillCircle(centerX, centerY, 30, color);

  // Nord (oben)
  color = touchNorth ? TFT_GREEN : TFT_DARKGREY;
  lcd.fillCircle(centerX, centerY - padSpacing, 30, color);

  // Süd, West, East analog
}
```

#### 6. AirWheel Animation

```cpp
void drawAirWheel() {
  int centerX = 160, centerY = 200;
  int radius = 80;

  // Rad-Kreis
  lcd.drawCircle(centerX, centerY, radius, TFT_MAGENTA);

  // Speichen (rotierend)
  for (int i = 0; i < 8; i++) {
    float angle = (i * 45 + airwheelAngle) * PI / 180.0;
    int x = centerX + cos(angle) * radius;
    int y = centerY + sin(angle) * radius;
    lcd.drawLine(centerX, centerY, x, y, TFT_MAGENTA);
  }

  // Aktueller Winkel
  lcd.printf("%d°", airwheelAngle);
}
```

## 🚀 Installation & Verwendung

### 1. Library installieren

```
Arduino IDE -> Tools -> Manage Libraries
Suche: "SparkFun MGC3130"
Installieren: "SparkFun MGC3130 GestIC Arduino Library"
```

**Alternative** (falls nicht gefunden):
```
# Manuelle Installation
git clone https://github.com/sparkfun/SparkFun_MGC3130_Arduino_Library
# Kopiere Ordner nach Arduino/libraries/
```

### 2. Hardware aufbauen

```
MGC3130 mit CYD verbinden:
- VCC → 3.3V (WICHTIG: Nicht 5V!)
- GND → GND
- SDA → GPIO 22
- SCL → GPIO 27
```

### 3. Code hochladen

```bash
# In Arduino IDE:
# 1. Board: "ESP32 Dev Module" auswählen
# 2. Datei → Beispiele → CYD-Academy → 21_Gesture_Sensor_MGC3130 öffnen
# 3. Hochladen
```

### 4. Bedienung

**Modi wechseln**: Tippe auf die Buttons unten
- **3D**: Zeigt 3D-Position als Würfel
- **GEST**: Zeigt erkannte Gesten
- **TOUCH**: Zeigt Touch-Elektroden
- **WHEEL**: Zeigt AirWheel-Rotation

**Gesten ausführen**:
1. Hand über Sensor halten (3-10 cm Abstand)
2. Für Swipe: Schnelle Wischbewegung
3. Für Kreis: Kreisförmige Bewegung
4. Für Touch: Elektroden berühren

### Serielle Ausgabe

```
=== CYD 3D Gesture Sensor - MGC3130 ===
I2C initialisiert auf SDA: 22, SCL: 27
MGC3130 gefunden!
MGC3130 konfiguriert:
- Gesten: Aktiviert
- AirWheel: Aktiviert
- Touch: Aktiviert
- 3D Position: Aktiviert

Position - X: 32000, Y: 45000, Z: 15000
Geste: Swipe Rechts
Touch: Tap Center
Position - X: 28000, Y: 48000, Z: 18000
AirWheel: Delta=5, Angle=45
```

## 🔧 Troubleshooting

### Problem: "MGC3130 nicht gefunden"

**Symptome**:
- Display zeigt "MGC3130 FEHLER!"
- Serielle Ausgabe: "MGC3130 nicht gefunden auf 0x42"

**Lösungen**:

1. **I2C-Scanner**:
   ```cpp
   void scanI2C() {
     for (byte addr = 1; addr < 127; addr++) {
       Wire.beginTransmission(addr);
       if (Wire.endTransmission() == 0) {
         Serial.printf("Gefunden: 0x%02X\n", addr);
       }
     }
   }
   ```
   - Erwartetes Ergebnis: `0x42`

2. **Spannung prüfen**:
   - ⚠️ MGC3130 benötigt **3,3V** (nicht 5V!)
   - Mit Multimeter Spannung messen

3. **Verkabelung**:
   - SDA und SCL vertauscht?
   - Lose Verbindungen?

4. **Library-Version**:
   - Neueste SparkFun MGC3130 Library installiert?

### Problem: Keine 3D-Position erkannt

**Symptome**:
- `positionAvailable()` liefert immer `false`
- X, Y, Z bleiben bei 0

**Lösungen**:

1. **Hand-Position**:
   - Hand zu nah (< 1 cm)
   - Hand zu weit (> 15 cm)
   - → Optimaler Bereich: 3-10 cm

2. **Sensor-Orientierung**:
   - Elektroden müssen nach oben zeigen
   - Sensor nicht verdeckt

3. **Kalibrierung**:
   ```cpp
   // Sensor zurücksetzen
   gesture.calibrate();
   delay(1000);
   ```

### Problem: Gesten werden nicht erkannt

**Symptome**:
- `gestureAvailable()` liefert immer `false`
- Swipes werden ignoriert

**Ursachen & Lösungen**:

1. **Zu langsame Bewegung**:
   - Swipe muss **schnell** sein (> 20 cm/s)
   - Übe schnellere Bewegungen

2. **Zu kurze Distanz**:
   - Swipe über mindestens **50%** des Sensor-Bereichs
   - Von einer Seite zur anderen wischen

3. **Gesten nicht aktiviert**:
   ```cpp
   gesture.enableGestures(true);  // Prüfen ob aufgerufen
   ```

4. **Interferenzen**:
   - Andere elektronische Geräte in der Nähe
   - Metallflächen unter dem Sensor
   - → Sensor auf Holz/Plastik legen

### Problem: Touch-Events funktionieren nicht

**Symptome**:
- `touchAvailable()` liefert immer `false`
- Keine Tap-Erkennung

**Lösungen**:

1. **Touch aktivieren**:
   ```cpp
   gesture.enableTouch(true);
   ```

2. **Elektroden-Position**:
   - Bei Skywriter HAT: Elektroden sind auf der Platine
   - Bei Generic Breakout: Externe Pads erforderlich
   - Prüfe Modul-Dokumentation

3. **Berührung vs. Annäherung**:
   - Manche Module erkennen nur Berührung
   - Andere auch Annäherung (1-3 cm)
   - → Direkt berühren statt schweben

### Problem: AirWheel reagiert nicht

**Symptome**:
- `airWheelAvailable()` liefert immer `false`
- Kreisbewegungen ignoriert

**Lösungen**:

1. **AirWheel aktivieren**:
   ```cpp
   gesture.enableAirWheel(true);
   ```

2. **Richtige Bewegung**:
   - Kreisförmige Bewegung (nicht linear!)
   - Konsistente Kreisrichtung
   - Nicht zu schnell, nicht zu langsam

3. **Höhe**:
   - AirWheel funktioniert am besten bei 5-8 cm Höhe
   - Zu nah: Wird als Touch erkannt
   - Zu weit: Kein Signal

### Problem: Erratische/Springende Werte

**Symptome**:
- Position springt wild
- Gesten werden mehrfach erkannt

**Ursachen**:

1. **Elektromagnetische Interferenz (EMI)**:
   - WLAN, Bluetooth, USB-Kabel
   - → Sensor weiter weg von ESP32
   - → Abschirmung mit Kupferfolie (geerdet)

2. **Schlechte Stromversorgung**:
   - 3,3V-Rail instabil
   - → Kondensator (10µF) parallel zu VCC/GND am Sensor

3. **Grounding-Probleme**:
   - Sensor "schwimmt" elektrisch
   - → Großflächiges GND um Sensor
   - → Touchpad auf geerdeter Fläche

4. **Software-Filtering**:
   ```cpp
   // Exponential Smoothing
   #define SMOOTH_FACTOR 0.3
   smoothX = smoothX * (1 - SMOOTH_FACTOR) + normX * SMOOTH_FACTOR;
   ```

## 🎯 Erweiterungsideen

### 1. Lautstärke-Regler mit AirWheel

**Konzept**: Kreisförmige Bewegung steuert Lautstärke

```cpp
#include <ESP8266Audio.h>  // Für Audio-Wiedergabe

int volume = 50;  // 0-100

void controlVolume() {
  if (gesture.airWheelAvailable()) {
    int8_t delta = gesture.getAirWheelDelta();
    volume += delta / 5;  // Dämpfung

    // Begrenzen
    volume = constrain(volume, 0, 100);

    // Audio-Library Lautstärke setzen
    audio.setVolume(volume);

    // Visualisierung
    lcd.fillRect(50, 200, 220, 30, TFT_DARKGREY);
    int barWidth = map(volume, 0, 100, 0, 220);
    lcd.fillRect(50, 200, barWidth, 30, TFT_GREEN);

    lcd.setCursor(140, 240);
    lcd.printf("%d%%", volume);
  }
}
```

**Anwendungen**:
- Media Player
- Smart Home Lautstärke
- LED-Helligkeit

### 2. 3D-Mauszeiger (Desktop-Control)

**Konzept**: Hand-Position steuert Mauszeiger

```cpp
#include <BleKeyboard.h>  // Bluetooth HID

BleKeyboard bleKeyboard("CYD Gesture Mouse");

void setup() {
  bleKeyboard.begin();
}

void controlMouse() {
  if (gesture.positionAvailable()) {
    // Position auf Bildschirm mappen
    int16_t mouseX = map(posX, 0, 65535, -127, 127);
    int16_t mouseY = map(posY, 0, 65535, -127, 127);

    // Maus bewegen
    bleKeyboard.move(mouseX / 10, mouseY / 10);

    // Klick bei Center-Touch
    if (touchCenter) {
      bleKeyboard.click(MOUSE_LEFT);
    }
  }
}
```

**Features**:
- Berührungslose Maussteuerung
- Gesten für Scrollen (Swipe Hoch/Runter)
- Touch für Klicks

### 3. Smart Home Gestensteuerung (MQTT)

**Konzept**: Gesten steuern Smarthome-Geräte

```cpp
#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient mqtt(espClient);

void setup() {
  WiFi.begin("SSID", "PASSWORD");
  mqtt.setServer("192.168.1.100", 1883);
  mqtt.connect("CYD-Gesture");
}

void handleSmartHomeGestures() {
  if (gesture.gestureAvailable()) {
    uint8_t gest = gesture.getGesture();

    switch (gest) {
      case GESTURE_WEST_EAST:
        // Swipe Rechts: Licht an
        mqtt.publish("home/light/living", "ON");
        Serial.println("Licht AN");
        break;

      case GESTURE_EAST_WEST:
        // Swipe Links: Licht aus
        mqtt.publish("home/light/living", "OFF");
        Serial.println("Licht AUS");
        break;

      case GESTURE_SOUTH_NORTH:
        // Swipe Hoch: Jalousie hoch
        mqtt.publish("home/blinds/living", "OPEN");
        break;

      case GESTURE_NORTH_SOUTH:
        // Swipe Runter: Jalousie runter
        mqtt.publish("home/blinds/living", "CLOSE");
        break;

      case GESTURE_CLOCKWISE:
        // Kreis CW: Helligkeit +20%
        mqtt.publish("home/light/living/brightness", "+20");
        break;

      case GESTURE_COUNTERCLOCKWISE:
        // Kreis CCW: Helligkeit -20%
        mqtt.publish("home/light/living/brightness", "-20");
        break;
    }
  }
}
```

**Home Assistant Integration**:
```yaml
# automation.yaml
- alias: "Gesten Licht"
  trigger:
    platform: mqtt
    topic: "home/light/living"
  action:
    service: light.turn_{{ trigger.payload | lower }}
    entity_id: light.wohnzimmer
```

### 4. Spiel-Controller

**Konzept**: Gesten steuern ein Spiel auf dem Display

```cpp
// Snake-Game mit Gestensteuerung
int snakeDir = 0;  // 0=Rechts, 1=Unten, 2=Links, 3=Oben

void gameControl() {
  if (gesture.gestureAvailable()) {
    uint8_t gest = gesture.getGesture();

    switch (gest) {
      case GESTURE_WEST_EAST:  snakeDir = 0; break;  // Rechts
      case GESTURE_SOUTH_NORTH: snakeDir = 1; break;  // Unten
      case GESTURE_EAST_WEST:  snakeDir = 2; break;  // Links
      case GESTURE_NORTH_SOUTH: snakeDir = 3; break;  // Oben
    }
  }

  // Snake-Logik
  updateSnake();
  drawSnake();
}

// Oder: Flappy Bird mit Swipe Hoch zum Springen
void flappyControl() {
  if (gesture.gestureAvailable()) {
    if (gesture.getGesture() == GESTURE_SOUTH_NORTH) {
      birdVelocity = -5;  // Springen
    }
  }
}
```

### 5. Foto-Galerie Navigator

**Konzept**: Durch Fotos blättern mit Gesten

```cpp
#include <SD.h>
#include <JPEGDEC.h>

String photoFiles[100];
int photoCount = 0;
int currentPhoto = 0;

void loadPhotoList() {
  File root = SD.open("/photos");
  while (File entry = root.openNextFile()) {
    if (!entry.isDirectory()) {
      photoFiles[photoCount++] = entry.name();
    }
  }
}

void navigatePhotos() {
  if (gesture.gestureAvailable()) {
    uint8_t gest = gesture.getGesture();

    if (gest == GESTURE_WEST_EAST) {
      // Nächstes Foto
      currentPhoto = (currentPhoto + 1) % photoCount;
      displayPhoto(photoFiles[currentPhoto]);
    } else if (gest == GESTURE_EAST_WEST) {
      // Vorheriges Foto
      currentPhoto = (currentPhoto - 1 + photoCount) % photoCount;
      displayPhoto(photoFiles[currentPhoto]);
    }
  }

  // Zoom mit AirWheel
  if (gesture.airWheelAvailable()) {
    int8_t delta = gesture.getAirWheelDelta();
    photoZoom += delta / 10.0;
    photoZoom = constrain(photoZoom, 1.0, 3.0);
    displayPhoto(photoFiles[currentPhoto]);
  }
}
```

### 6. 3D-Objekt-Rotation

**Konzept**: 3D-Position steuert Objekt-Rotation

```cpp
// Rotierenden Würfel steuern
float cubeRotX = 0, cubeRotY = 0, cubeRotZ = 0;

void controlCubeRotation() {
  if (gesture.positionAvailable()) {
    // Position auf Rotation mappen
    cubeRotX = (normY - 0.5) * 360;  // Y → Rotation um X-Achse
    cubeRotY = (normX - 0.5) * 360;  // X → Rotation um Y-Achse
    cubeRotZ = (normZ - 0.5) * 180;  // Z → Rotation um Z-Achse

    // 3D-Würfel zeichnen
    draw3DCube(cubeRotX, cubeRotY, cubeRotZ);
  }
}

void draw3DCube(float rotX, float rotY, float rotZ) {
  // Vereinfachte 3D-Projektion
  // (Komplexe Matrixberechnung für echte 3D-Grafik)

  // Würfel-Eckpunkte
  float vertices[8][3] = {
    {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
    {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}
  };

  // Rotation anwenden (vereinfacht)
  // ... Matrix-Multiplikation ...

  // Projektion auf 2D und zeichnen
  // ...
}
```

### 7. Präsentations-Fernsteuerung

**Konzept**: PowerPoint/Keynote mit Gesten steuern

```cpp
#include <BleKeyboard.h>

BleKeyboard bleKeyboard("CYD Presenter");

void presentationControl() {
  if (gesture.gestureAvailable()) {
    uint8_t gest = gesture.getGesture();

    switch (gest) {
      case GESTURE_WEST_EAST:
        // Nächste Folie
        bleKeyboard.write(KEY_RIGHT_ARROW);
        Serial.println("Nächste Folie");
        break;

      case GESTURE_EAST_WEST:
        // Vorherige Folie
        bleKeyboard.write(KEY_LEFT_ARROW);
        Serial.println("Vorherige Folie");
        break;

      case GESTURE_SOUTH_NORTH:
        // Präsentation starten
        bleKeyboard.write(KEY_F5);
        Serial.println("Präsentation Start");
        break;

      case GESTURE_NORTH_SOUTH:
        // Präsentation beenden
        bleKeyboard.write(KEY_ESC);
        Serial.println("Präsentation Ende");
        break;
    }
  }

  // Laserpointer mit Position
  if (gesture.positionAvailable() && touchCenter) {
    // Mauszeiger zur Position bewegen
    int16_t x = map(posX, 0, 65535, -50, 50);
    int16_t y = map(posY, 0, 65535, -50, 50);
    bleKeyboard.move(x, y);
  }
}
```

## 📚 Weiterführende Themen

### E-Field Sensing im Detail

**Physikalische Grundlagen**:

Das elektrische Feld folgt den **Maxwell-Gleichungen**:
```
∇ · E = ρ / ε₀  (Gaußsches Gesetz)

Wobei:
E = Elektrisches Feld
ρ = Ladungsdichte
ε₀ = Permittivität des Vakuums
```

Für MGC3130:
- Tx-Elektrode: Erzeugt AC-Feld (~100 kHz)
- Hand: Verändert lokale Permittivität (ε)
- Rx-Elektroden: Messen Feldänderung

**Kapazitive Kopplung**:
```
C = ε₀ × εᵣ × A / d

C = Kapazität zwischen Hand und Elektrode
ε₀ = Permittivität Vakuum (8,85 × 10⁻¹² F/m)
εᵣ = Relative Permittivität (Hand ≈ 50-60)
A = Fläche der Elektrode
d = Abstand Hand-Elektrode
```

→ Kleinerer Abstand = Höhere Kapazität = Stärkeres Signal

### Vergleich: MGC3130 vs. APDS-9960

Beide sind Gestensensoren, aber mit unterschiedlichen Technologien:

| Feature | MGC3130 (E-Field) | APDS-9960 (Optisch) |
|---------|-------------------|---------------------|
| **Prinzip** | Elektrisches Feld | IR-Photodioden (4×) |
| **3D-Position** | Sehr präzise (16-Bit) | Grob (8-Bit) |
| **Reichweite** | 0-15 cm | 0-20 cm |
| **Licht-Empfindlichkeit** | Nein | Ja (IR-Interferenz) |
| **Stromverbrauch** | 5-10 mA | 100 mA (LED) |
| **Genauigkeit** | Hoch | Mittel |
| **Preis** | ~15€ | ~5€ |
| **Touch-Elektroden** | Ja (5×) | Nein |
| **AirWheel** | Ja | Nein |

**Empfehlung**:
- **MGC3130**: Präzise 3D-Steuerung, professionelle Anwendungen
- **APDS-9960**: Einfache Gesten, Budgetprojekte

### Kalibration & Umgebungsanpassung

Der MGC3130 passt sich automatisch an:

**Auto-Calibration**:
1. Beim Start: Misst Umgebung ohne Hand
2. Setzt Baseline für "Keine Erkennung"
3. Kontinuierlich: Passt Baseline an (langsame Änderungen)

**Manuelle Kalibrierung**:
```cpp
gesture.calibrate();  // Erzwingt Neu-Kalibrierung
```

**Wann kalibrieren?**
- Nach Standortwechsel
- Wenn Metallflächen hinzugefügt/entfernt wurden
- Bei dauerhaft schlechter Erkennung

## 🛠️ Praktische Tipps

### Optimale Sensor-Platzierung

```
OPTIMAL:
✅ Horizontal montiert (Elektroden nach oben)
✅ Freier Raum über Sensor (mindestens 20 cm)
✅ Holz- oder Plastik-Unterlage
✅ Weit weg von anderen Elektronik (> 10 cm)

SCHLECHT:
❌ Vertikal montiert (Gesten funktionieren nicht)
❌ Metall direkt unter Sensor (Feldstörung)
❌ Neben ESP32/WLAN-Modul (EMI)
❌ Hinter Glas/Acryl (reduzierte Sensitivität)
```

### Gesten-Erkennung optimieren

**Tipps für beste Erkennung**:
1. **Geschwindigkeit**: Swipes schnell ausführen (ca. 50 cm/s)
2. **Distanz**: Über gesamten Sensor-Bereich (nicht nur Mitte)
3. **Höhe**: 5-8 cm optimal
4. **Konsistenz**: Gerade Linie für Swipes, sauberer Kreis für Circle

**Debugging**:
```cpp
// Position während Geste ausgeben
void debugGesture() {
  if (gesture.positionAvailable()) {
    Serial.printf("X: %5d, Y: %5d, Z: %5d\n", posX, posY, posZ);
  }
}
```

### Stromverbrauch optimieren

```cpp
// Power-Saving bei Inaktivität
unsigned long lastActivity = 0;
#define SLEEP_TIMEOUT 30000  // 30 Sekunden

void loop() {
  if (gesture.dataAvailable()) {
    lastActivity = millis();
    // ... normale Verarbeitung
  }

  // Sleep nach Timeout
  if (millis() - lastActivity > SLEEP_TIMEOUT) {
    gesture.sleep();  // MGC3130 in Sleep-Modus
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_XX, 0);  // Wake auf Transfer-Pin
    esp_light_sleep_start();

    // Nach Aufwachen
    gesture.wake();
  }
}
```

## 📖 Ressourcen & Links

### Datenblätter & Dokumentation
- **MGC3130 Datasheet**: [Microchip MGC3130](https://www.microchip.com/en-us/product/MGC3130)
- **GestIC Technology**: [Microchip Application Notes](https://www.microchip.com/en-us/solutions/human-machine-interface/gestic-technology)
- **SparkFun Library**: [GitHub](https://github.com/sparkfun/SparkFun_MGC3130_Arduino_Library)

### Module & Breakouts
- **Skywriter HAT**: [Pimoroni](https://shop.pimoroni.com/products/skywriter-hat)
- **Grove Gesture**: [Seeed Studio](https://www.seeedstudio.com/Grove-Gesture-PAJ7620U2.html)

### CYD-Spezifisch
- **CYD Display Config**: `lib/CYD_Display_Config/CYD_Display_Config.h`
- **I2C Pins**: extSDA (GPIO 22), extSCL (GPIO 27)

### Verwandte Beispiele
- **Beispiel 18**: Compass & Orientation (Sensor Fusion)
- **Beispiel 19**: Laser Distance (ToF Sensor)
- **Beispiel 20**: Air Quality (MOX Sensor)

### Arduino Libraries
```cpp
// Erforderliche Libraries:
#include <Wire.h>              // I2C (ESP32 Core)
#include <MGC3130.h>           // SparkFun MGC3130
#include <CYD_Display_Config.h> // CYD Display
```

**Installation**:
```
Arduino IDE -> Tools -> Manage Libraries
- "SparkFun MGC3130"
```

## 🎓 Lernziele

Nach Abschluss dieses Beispiels solltest du:

- ✅ **E-Field Sensing** Prinzip verstehen
- ✅ **3D-Koordinaten** auslesen und visualisieren
- ✅ **Gesten-Algorithmen** kennen
- ✅ **Touch-Elektroden** nutzen (kapazitiv)
- ✅ **AirWheel** für Rotations-Steuerung implementieren
- ✅ **MGC3130 vs. APDS-9960** unterscheiden können
- ✅ **Kalibration** durchführen
- ✅ **HMI (Human-Machine Interface)** gestalten
- ✅ **SparkFun Library** verwenden

## 🔜 Nächste Schritte

Weitere Sensor-Beispiele:
- **Beispiel 22**: Farbsensor TCS34725 (RGB-Erkennung)
- **Beispiel 23**: Umwelt-Multi-Sensor BME680 (Temp, Feuchte, Druck, Gas)
- **Beispiel 24**: MEMS-Mikrofon INMP441 (Audio-FFT-Spektrum)

---

**Viel Erfolg mit deiner Gestensteuerung! 👋🎯**

Bei Fragen: CYD-Academy GitHub Issues
