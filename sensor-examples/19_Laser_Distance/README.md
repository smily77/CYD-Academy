# Beispiel 19: Laser Distanzmessung mit R50-C ToF Sensor

Dieses Beispiel demonstriert präzise Distanzmessungen mit dem **Aero Selfie R50-C Laser Range Finder** über die I2C-Schnittstelle. Der Sensor nutzt ToF (Time-of-Flight) Technologie für zuverlässige Messungen bis zu 50 Meter Reichweite.

## 📸 Features

- **🎯 Echtzeit-Distanzmessung**: 0 - 50 Meter Messbereich mit cm-Genauigkeit
- **📊 Drei Messmodi**:
  - Single Shot: Messung auf Knopfdruck
  - Continuous: Automatische Messung mit 2 Hz
  - Fast: Schnelle Messungen mit 10 Hz
- **📈 Visualisierungen**:
  - Großes numerisches Display mit Meter/Zentimeter
  - Horizontaler Balken mit Farbcodierung (Grün→Gelb→Orange→Rot)
  - Scrollendes Verlaufsdiagramm mit 110 Datenpunkten
- **📊 Statistik-Tracking**:
  - Minimum/Maximum/Durchschnitt
  - Genauigkeitsanzeige (erfolgreiche Messungen in %)
  - Messzähler
- **🎨 Professionelles UI**:
  - Touch-Buttons für Modusauswahl und Reset
  - Echtzeit-Sensorstatus-Anzeige
  - Smooth Scrolling Grafik

## 🔌 Hardware-Anforderungen

### Hauptkomponenten
- **ESP32-2432S028R** (CYD - Cheap Yellow Display)
- **Aero Selfie R50-C Laser Range Finder** (I2C Modus)

### R50-C Spezifikationen
- **Messbereich**: 0,03 m - 50 m
- **Genauigkeit**: ±1% (typisch), ±3% (max)
- **Auflösung**: 1 cm
- **Messfrequenz**: bis zu 50 Hz
- **Interface**: I2C (Standard-Modus: 100 kHz, Fast-Modus: 400 kHz)
- **I2C-Adresse**: 0x52 (fest)
- **Spannung**: 3,3V - 5V
- **Stromaufnahme**:
  - Idle: < 10 mA
  - Aktiv: ca. 100-150 mA während Messung
- **Laser-Klasse**: Class 1 (augensicher)
- **Wellenlänge**: 905 nm (infrarot)
- **Schutzklasse**: IP54 (staubgeschützt, spritzwassergeschützt)

### Verbindung (I2C)

```
R50-C          CYD (ESP32-2432S028R)
------         ---------------------
VCC     -----> 3.3V
GND     -----> GND
SDA     -----> GPIO 22 (extSDA)
SCL     -----> GPIO 27 (extSCL)
```

**Wichtig**:
- Der R50-C wird mit 3,3V vom CYD versorgt
- I2C Pull-up Widerstände sind bereits auf dem CYD vorhanden
- Keine externen Komponenten erforderlich

## 🔬 Technischer Hintergrund

### Time-of-Flight (ToF) Prinzip

Der R50-C nutzt **ToF-Technologie** (Time-of-Flight) zur Distanzmessung:

#### Funktionsweise
1. **Laser-Puls senden**: Der Sensor emittiert einen kurzen Infrarot-Laserpuls (905 nm)
2. **Reflexion**: Der Puls trifft auf ein Objekt und wird reflektiert
3. **Detektion**: Der reflektierte Puls wird vom Empfänger detektiert
4. **Zeitmessung**: Die Zeit zwischen Aussendung und Empfang wird gemessen
5. **Berechnung**: Distanz = (Laufzeit × Lichtgeschwindigkeit) / 2

```
Mathematisch:
d = (t × c) / 2

Wobei:
d = Distanz in Metern
t = Laufzeit in Sekunden
c = Lichtgeschwindigkeit (≈ 3 × 10^8 m/s)
```

#### Beispiel-Berechnung
Für eine Distanz von 10 Metern:
```
t = (2 × d) / c
t = (2 × 10 m) / (3 × 10^8 m/s)
t = 66,7 Nanosekunden
```

Der Sensor muss also Zeitdifferenzen im **Nanosekunden-Bereich** präzise messen können!

### ToF vs. andere Technologien

| Technologie | Reichweite | Genauigkeit | Geschwindigkeit | Kosten |
|-------------|------------|-------------|-----------------|--------|
| **ToF Laser** | 0,03-50m | ±1% | Sehr hoch (50 Hz) | Mittel |
| Ultraschall | 0,02-4m | ±1cm | Mittel (10 Hz) | Niedrig |
| Infrarot Sharp | 0,1-1,5m | ±5% | Hoch (30 Hz) | Niedrig |
| LiDAR | 0-100m+ | ±2cm | Sehr hoch (100 Hz+) | Hoch |

### Vorteile des R50-C
- ✅ **Präzise**: ±1% Genauigkeit über den gesamten Messbereich
- ✅ **Schnell**: Bis zu 50 Hz Messfrequenz
- ✅ **Große Reichweite**: 50 Meter Maximum
- ✅ **Unabhängig von Farbe**: Im Gegensatz zu Infrarot-Sensoren
- ✅ **Wetterfest**: IP54 Schutzklasse für Außeneinsatz
- ✅ **Augensicher**: Class 1 Laser

### Limitierungen
- ❌ **Reflexion**: Stark absorbierende Oberflächen (schwarz, matt) reduzieren Reichweite
- ❌ **Transparenz**: Glas/Wasser können nicht zuverlässig gemessen werden
- ❌ **Umgebungslicht**: Direkte Sonneneinstrahlung kann Messungen beeinflussen
- ❌ **Winkel**: Objekt muss relativ senkrecht zum Sensor stehen
- ❌ **Stromverbrauch**: Höher als passive Sensoren (100-150 mA aktiv)

## 📡 I2C-Kommunikation

### I2C-Protokoll Details

Der R50-C nutzt ein einfaches **I2C-Protokoll** ohne spezielle Library:

#### Register-Map
```
Register    Adresse    Funktion              R/W    Bytes
--------------------------------------------------------
DISTANCE_H  0x00       Distanz High Byte     R      1
DISTANCE_L  0x01       Distanz Low Byte      R      1
TRIGGER     0x02       Messung auslösen      W      1
MODE        0x03       Messmodus setzen      R/W    1
STATUS      0x04       Sensor-Status         R      1
VERSION     0x05       Firmware-Version      R      1
```

#### Distanz auslesen (Standard-Methode)

```cpp
uint16_t readDistance() {
  Wire.beginTransmission(0x52);        // R50-C I2C-Adresse
  Wire.write(0x00);                     // Register: DISTANCE_H
  Wire.endTransmission(false);          // Repeated Start

  Wire.requestFrom(0x52, 2);            // 2 Bytes anfordern
  if (Wire.available() >= 2) {
    uint8_t highByte = Wire.read();     // High Byte (MSB)
    uint8_t lowByte = Wire.read();      // Low Byte (LSB)

    uint16_t distance = (highByte << 8) | lowByte;  // Kombinieren
    return distance;  // in Zentimetern
  }
  return 0;  // Fehler
}
```

**Wichtig**: Die Distanz wird als 16-Bit Wert in **Zentimetern** zurückgegeben.

#### Messung auslösen (Single-Shot Modus)

```cpp
void triggerMeasurement() {
  Wire.beginTransmission(0x52);
  Wire.write(0x02);           // Register: TRIGGER
  Wire.write(0x01);           // Wert: Messung starten
  Wire.endTransmission();

  delay(100);                 // Warten auf Messung (ca. 50-100ms)
}
```

#### Messmo dus setzen

```cpp
void setMeasurementMode(uint8_t mode) {
  Wire.beginTransmission(0x52);
  Wire.write(0x03);           // Register: MODE
  Wire.write(mode);           // 0 = Single, 1 = Continuous, 2 = Fast
  Wire.endTransmission();
}

// Modi:
// 0 = Single Shot (auf Anfrage)
// 1 = Continuous (ca. 2 Hz)
// 2 = Fast (ca. 10 Hz)
```

### I2C Timing-Diagramm

```
Single Shot Messung:
--------------------
Master: START | ADDR+W | REG(0x02) | DATA(0x01) | STOP
            ↓
        [Sensor misst: ~50-100ms]
            ↓
Master: START | ADDR+W | REG(0x00) | REP-START | ADDR+R | DATA_H | DATA_L | STOP
                                                           ↓        ↓
                                                        High Byte Low Byte
```

### Fehlerbehandlung

```cpp
// Validierung der Messwerte
if (distance > 5000) {           // Max 50m = 5000cm
  errorCount++;
  return 0;                       // Ungültig
}

if (distance == 0 || distance == 0xFFFF) {
  errorCount++;                   // Keine Reflexion
  return 0;
}

// Genauigkeit berechnen
float accuracy = ((float)(totalMeasurements - errorCount) /
                  totalMeasurements) * 100.0;
```

## 💻 Code-Struktur

### Hauptkomponenten

#### 1. Sensor-Kommunikation
```cpp
uint16_t readDistance() {
  // Direkte I2C-Kommunikation (kein Library)
  // Liest 2 Bytes (High + Low) vom Sensor
  // Gibt Distanz in cm zurück
}
```

#### 2. Drei Messmodi

**Single Shot** (Manuell):
```cpp
if (measurementMode == MODE_SINGLE) {
  // Nur bei Touch messen
  if (touchTriggered) {
    triggerMeasurement();
    currentDistance = readDistance();
  }
}
```

**Continuous** (2 Hz):
```cpp
if (measurementMode == MODE_CONTINUOUS) {
  if (millis() - lastMeasurement >= 500) {  // 500ms = 2 Hz
    currentDistance = readDistance();
    lastMeasurement = millis();
  }
}
```

**Fast** (10 Hz):
```cpp
if (measurementMode == MODE_FAST) {
  if (millis() - lastMeasurement >= 100) {  // 100ms = 10 Hz
    currentDistance = readDistance();
    lastMeasurement = millis();
  }
}
```

#### 3. Statistik-Tracking

```cpp
void updateStatistics(uint16_t distance) {
  if (distance > 0 && distance <= 5000) {
    // Min/Max
    if (distance < minDistance) minDistance = distance;
    if (distance > maxDistance) maxDistance = distance;

    // Durchschnitt (Moving Average)
    distanceSum += distance;
    totalMeasurements++;
    avgDistance = distanceSum / totalMeasurements;

    // Genauigkeit
    float accuracy = ((float)(totalMeasurements - errorCount) /
                      totalMeasurements) * 100.0;
  }
}
```

#### 4. Ring-Buffer für Verlaufsgrafik

```cpp
#define MAX_HISTORY 110  // Anzahl Datenpunkte im Diagramm

uint16_t distanceHistory[MAX_HISTORY];
int historyIndex = 0;

void addToHistory(uint16_t distance) {
  distanceHistory[historyIndex] = distance;
  historyIndex = (historyIndex + 1) % MAX_HISTORY;  // Ring-Buffer
}
```

#### 5. Horizontal Bar Visualisierung

```cpp
void drawDistanceBar(uint16_t distance) {
  int barWidth = map(distance, 0, 5000, 0, 240);  // 0-5000cm → 0-240px

  // Farbcodierung basierend auf Distanz
  uint16_t color;
  if (distance < 500) color = TFT_RED;           // < 5m: Rot
  else if (distance < 1500) color = TFT_ORANGE;  // < 15m: Orange
  else if (distance < 3000) color = TFT_YELLOW;  // < 30m: Gelb
  else color = TFT_GREEN;                        // > 30m: Grün

  lcd.fillRect(40, 140, barWidth, 30, color);
}
```

#### 6. Scrollendes Diagramm

```cpp
void drawHistoryGraph() {
  // Graph-Bereich: 320x100 Pixel
  // 110 Datenpunkte → ca. 2-3 Pixel pro Punkt

  for (int i = 0; i < MAX_HISTORY - 1; i++) {
    int idx1 = (historyIndex + i) % MAX_HISTORY;
    int idx2 = (historyIndex + i + 1) % MAX_HISTORY;

    if (distanceHistory[idx1] > 0 && distanceHistory[idx2] > 0) {
      int x1 = map(i, 0, MAX_HISTORY, 0, 320);
      int x2 = map(i + 1, 0, MAX_HISTORY, 0, 320);

      int y1 = map(distanceHistory[idx1], 0, 5000, 100, 0);  // Invertiert
      int y2 = map(distanceHistory[idx2], 0, 5000, 100, 0);

      lcd.drawLine(x1, y1 + 250, x2, y2 + 250, TFT_CYAN);
    }
  }
}
```

### Touch-Handler

```cpp
void handleTouch() {
  uint16_t x, y;
  if (lcd.getTouch(&x, &y)) {
    // Mode-Buttons (y: 380-430)
    if (y >= 380 && y <= 430) {
      if (x >= 10 && x < 100) {
        measurementMode = MODE_SINGLE;
        resetStatistics();
      } else if (x >= 110 && x < 210) {
        measurementMode = MODE_CONTINUOUS;
        resetStatistics();
      } else if (x >= 220 && x < 310) {
        measurementMode = MODE_FAST;
        resetStatistics();
      }
    }

    // Reset-Button (y: 440-470)
    if (y >= 440 && y <= 470 && x >= 10 && x <= 310) {
      resetStatistics();
    }
  }
}
```

## 🚀 Installation & Verwendung

### 1. Hardware aufbauen
- R50-C mit CYD verbinden (siehe Schaltplan oben)
- USB-Kabel an CYD anschließen

### 2. Code hochladen
```bash
# In Arduino IDE:
# 1. Board: "ESP32 Dev Module" auswählen
# 2. Datei → Beispiele → CYD-Academy → 19_Laser_Distance öffnen
# 3. Hochladen
```

### 3. Bedienung
1. **Messmo dus wählen**:
   - Tippe auf "SINGLE" für manuelle Messungen
   - Tippe auf "CONT" für kontinuierliche Messungen (2 Hz)
   - Tippe auf "FAST" für schnelle Messungen (10 Hz)

2. **Messwerte ablesen**:
   - Große Anzeige: Aktuelle Distanz in Meter und Zentimeter
   - Balken: Visuelle Darstellung (Farbcodierung)
   - Graph: Verlauf der letzten 110 Messungen
   - Statistik: Min/Max/Durchschnitt

3. **Reset**: Tippe auf "RESET" um Statistik zu löschen

### Serielle Ausgabe
```
=== CYD Laser Distance Sensor - R50-C ===
I2C initialisiert auf SDA: 22, SCL: 27

[12:34:56] Distanz: 1234 cm (12.34 m) | Avg: 1250 cm | Genauigkeit: 98.5%
[12:34:57] Distanz: 1245 cm (12.45 m) | Avg: 1251 cm | Genauigkeit: 98.6%
[12:34:58] Fehler beim Lesen des Sensors
[12:34:59] Distanz: 1240 cm (12.40 m) | Avg: 1250 cm | Genauigkeit: 98.3%
```

## 🔧 Troubleshooting

### Problem: Keine Verbindung zum Sensor

**Symptome**:
- Display zeigt "WARTE AUF SENSOR..." dauerhaft
- Serielle Ausgabe: "I2C Gerät nicht gefunden auf Adresse 0x52"

**Lösungen**:
```cpp
// 1. I2C-Scanner ausführen
void scanI2C() {
  Serial.println("Scanne I2C-Bus...");
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("Gerät gefunden: 0x");
      Serial.println(addr, HEX);
    }
  }
}
```

- ✅ **Verkabelung prüfen**: SDA/SCL vertauscht?
- ✅ **Spannung messen**: R50-C bekommt 3,3V?
- ✅ **Pull-up Widerstände**: Bei langen Kabeln evtl. externe Pull-ups (4,7kΩ) zu VCC
- ✅ **I2C-Geschwindigkeit reduzieren**: `Wire.begin(extSDA, extSCL, 50000);` (50 kHz)

### Problem: Ungültige Messwerte (0 oder 65535)

**Symptome**:
- Distanz zeigt immer 0 cm oder 655,35 m
- Genauigkeit unter 50%

**Ursachen & Lösungen**:
1. **Keine Reflexion**:
   - Objekt zu weit entfernt (> 50m)
   - Objekt zu dunkel/absorbierend
   - → Helle, reflektierende Oberfläche verwenden

2. **Transparentes Objekt**:
   - Glas, Wasser reflektieren IR schlecht
   - → Undurchsichtiges Objekt verwenden

3. **Winkel zu steil**:
   - Sensor muss relativ senkrecht auf Objekt zeigen
   - → Sensor ausrichten (±15° optimal)

4. **Umgebungslicht**:
   - Direkte Sonneneinstrahlung stört
   - → Im Schatten messen oder Sensor abschirmen

### Problem: Messungen zu langsam

**Symptome**:
- Im FAST-Modus nur 2-3 Messungen pro Sekunde
- Graph scrollt kaum

**Lösungen**:
```cpp
// I2C auf Fast Mode setzen (400 kHz)
Wire.begin(extSDA, extSCL);
Wire.setClock(400000);  // Statt Standard 100 kHz

// Polling-Intervall reduzieren
if (millis() - lastMeasurement >= 50) {  // 20 Hz statt 10 Hz
  currentDistance = readDistance();
  lastMeasurement = millis();
}
```

- ⚠️ **Achtung**: Zu schnelles Polling kann Sensor überlasten
- ⚠️ **Maximum**: R50-C unterstützt bis zu 50 Hz

### Problem: Messungen springen stark

**Symptome**:
- Distanz schwankt um ±10 cm
- Graph zeigt Zick-Zack-Muster

**Lösung**: Median-Filter implementieren
```cpp
#define FILTER_SIZE 5
uint16_t filterBuffer[FILTER_SIZE];
int filterIndex = 0;

uint16_t getMedianDistance() {
  filterBuffer[filterIndex] = readDistance();
  filterIndex = (filterIndex + 1) % FILTER_SIZE;

  // Sortieren
  uint16_t sorted[FILTER_SIZE];
  memcpy(sorted, filterBuffer, sizeof(filterBuffer));
  for (int i = 0; i < FILTER_SIZE - 1; i++) {
    for (int j = i + 1; j < FILTER_SIZE; j++) {
      if (sorted[i] > sorted[j]) {
        uint16_t temp = sorted[i];
        sorted[i] = sorted[j];
        sorted[j] = temp;
      }
    }
  }

  return sorted[FILTER_SIZE / 2];  // Median
}
```

### Problem: Hoher Stromverbrauch

**Symptome**:
- USB-Port überlastet
- ESP32 resettet sporadisch

**Lösungen**:
- ✅ **Externes Netzteil**: 5V/1A Netzteil verwenden
- ✅ **Power-Saving Mode**: Sensor nur bei Bedarf aktivieren
```cpp
void sleepSensor() {
  // Sensor in Sleep-Mode (falls unterstützt)
  Wire.beginTransmission(0x52);
  Wire.write(0x06);  // Sleep-Register
  Wire.write(0x01);
  Wire.endTransmission();
}
```

## 🎯 Erweiterungsideen

### 1. Objekt-Detektion & Alarmierung

**Konzept**: Automatische Erkennung von Objekten in definierten Bereichen

```cpp
// Alarm-Zonen definieren
#define CRITICAL_ZONE 100    // < 1m: Kritisch
#define WARNING_ZONE 500     // < 5m: Warnung
#define SAFE_ZONE 1000       // < 10m: Hinweis

void checkAlarmZones(uint16_t distance) {
  if (distance < CRITICAL_ZONE) {
    // Kritischer Alarm
    lcd.fillScreen(TFT_RED);
    tone(BUZZER_PIN, 2000, 500);  // Hoher Ton
    Serial.println("!!! KRITISCH: Objekt sehr nah !!!");
  } else if (distance < WARNING_ZONE) {
    // Warnung
    lcd.drawRect(0, 0, 320, 480, TFT_ORANGE);  // Rahmen
    tone(BUZZER_PIN, 1000, 200);
    Serial.println("WARNUNG: Objekt im Warnbereich");
  } else if (distance < SAFE_ZONE) {
    // Hinweis
    lcd.setTextColor(TFT_YELLOW);
    Serial.println("INFO: Objekt erkannt");
  }
}
```

**Anwendungen**:
- Parkplatz-Einparkhilfe
- Kollisionswarnung für Roboter
- Sicherheitsabstand-Monitor

### 2. Füllstandsmessung

**Konzept**: Messen des Füllstands in Tanks/Behältern

```cpp
#define TANK_HEIGHT 300  // Tank-Höhe in cm
#define SENSOR_OFFSET 10 // Sensor-Montage über Tank

void measureFillLevel() {
  uint16_t distance = readDistance();
  uint16_t fillLevel = TANK_HEIGHT - (distance - SENSOR_OFFSET);

  float fillPercent = ((float)fillLevel / TANK_HEIGHT) * 100.0;

  // Visualisierung als vertikaler Tank
  int tankH = 300;  // Pixel
  int fillH = map(fillLevel, 0, TANK_HEIGHT, 0, tankH);

  lcd.drawRect(110, 100, 100, tankH, TFT_WHITE);        // Tank-Umriss
  lcd.fillRect(112, 100 + (tankH - fillH), 96, fillH, TFT_BLUE);  // Füllstand

  lcd.setCursor(120, 50);
  lcd.printf("%.1f %%", fillPercent);

  // Alarm bei niedrigem Füllstand
  if (fillPercent < 20.0) {
    lcd.setTextColor(TFT_RED);
    lcd.print(" - NIEDRIG!");
  }
}
```

**Anwendungen**:
- Wassertank-Überwachung
- Silo-Füllstand
- Regenwasser-Zisterne

### 3. Geschwindigkeitsmessung

**Konzept**: Berechnung der Geschwindigkeit bewegter Objekte

```cpp
struct SpeedMeasurement {
  uint16_t lastDistance;
  unsigned long lastTime;
  float currentSpeed;  // m/s
};

SpeedMeasurement speedMeter = {0, 0, 0.0};

void measureSpeed() {
  uint16_t distance = readDistance();
  unsigned long now = millis();

  if (speedMeter.lastDistance > 0) {
    float deltaDistance = (speedMeter.lastDistance - distance) / 100.0;  // in Meter
    float deltaTime = (now - speedMeter.lastTime) / 1000.0;  // in Sekunden

    speedMeter.currentSpeed = deltaDistance / deltaTime;  // m/s
    float speedKmh = speedMeter.currentSpeed * 3.6;       // km/h

    // Anzeige
    lcd.setCursor(50, 200);
    lcd.setTextSize(3);
    lcd.printf("%.1f km/h", speedKmh);

    // Richtungspfeil
    if (speedMeter.currentSpeed > 0.1) {
      lcd.print(" →");  // Näher kommend
    } else if (speedMeter.currentSpeed < -0.1) {
      lcd.print(" ←");  // Entfernend
    }
  }

  speedMeter.lastDistance = distance;
  speedMeter.lastTime = now;
}
```

**Anwendungen**:
- Verkehrsgeschwindigkeit messen
- Ballgeschwindigkeit (Sport)
- Förderbandgeschwindigkeit

### 4. 3D-Scanning (mit Servo)

**Konzept**: Kombination mit Servo-Motor für Umgebungs-Scanning

```cpp
#include <ESP32Servo.h>

Servo scanServo;
#define SERVO_PIN 15
#define SCAN_STEPS 36  // 360° / 10° = 36 Schritte

struct ScanPoint {
  int angle;         // 0-359°
  uint16_t distance; // cm
};

ScanPoint scanData[SCAN_STEPS];

void performScan() {
  for (int i = 0; i < SCAN_STEPS; i++) {
    int angle = i * 10;  // 10° Schritte
    scanServo.write(angle);
    delay(200);  // Servo bewegen lassen

    uint16_t distance = readDistance();
    scanData[i].angle = angle;
    scanData[i].distance = distance;

    // Live-Anzeige als Polar-Plot
    drawPolarPoint(angle, distance);
  }
}

void drawPolarPoint(int angle, uint16_t distance) {
  int centerX = 160, centerY = 240;
  int maxRadius = 120;  // Pixel

  float rad = angle * PI / 180.0;
  int radius = map(distance, 0, 5000, 0, maxRadius);

  int x = centerX + radius * cos(rad);
  int y = centerY + radius * sin(rad);

  lcd.fillCircle(x, y, 2, TFT_GREEN);
}
```

**Anwendungen**:
- Raumvermessung
- Hindernis-Mapping für autonome Fahrzeuge
- 3D-Modell-Erstellung

### 5. SD-Karten Logging

**Konzept**: Langzeit-Aufzeichnung von Distanzmessungen

```cpp
#include <SD.h>
#include <SPI.h>

#define SD_CS_PIN 5

File logFile;

void setupSDCard() {
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD-Karte Fehler!");
    return;
  }

  // Neue Log-Datei erstellen
  char filename[32];
  sprintf(filename, "/distance_%lu.csv", millis());

  logFile = SD.open(filename, FILE_WRITE);
  if (logFile) {
    // CSV-Header
    logFile.println("Timestamp,Distance_cm,Mode,Accuracy");
    logFile.close();
  }
}

void logMeasurement(uint16_t distance, uint8_t mode, float accuracy) {
  logFile = SD.open("/distance_log.csv", FILE_APPEND);
  if (logFile) {
    // CSV-Zeile
    logFile.print(millis());
    logFile.print(",");
    logFile.print(distance);
    logFile.print(",");
    logFile.print(mode);
    logFile.print(",");
    logFile.println(accuracy, 2);
    logFile.close();
  }
}

// In loop() aufrufen:
// if (distance > 0) logMeasurement(distance, measurementMode, accuracy);
```

**Anwendungen**:
- Langzeit-Monitoring
- Datenanalyse in Excel/Python
- Qualitätssicherung

### 6. MQTT/IoT Integration

**Konzept**: Drahtlose Übertragung von Distanzdaten

```cpp
#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient mqtt(espClient);

#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_TOPIC "cyd/distance"

void setupMQTT() {
  WiFi.begin("SSID", "PASSWORD");
  while (WiFi.status() != WL_CONNECTED) delay(500);

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.connect("CYD-Distance-Sensor");
}

void publishDistance(uint16_t distance) {
  if (mqtt.connected()) {
    char payload[128];
    sprintf(payload, "{\"distance\":%d,\"unit\":\"cm\",\"timestamp\":%lu}",
            distance, millis());

    mqtt.publish(MQTT_TOPIC, payload);
  }
}
```

**Anwendungen**:
- Home Assistant Integration
- Cloud-Datenbank (InfluxDB)
- Smartphone-Benachrichtigungen

### 7. Multi-Sensor Array

**Konzept**: Mehrere R50-C für Weitwinkel-Abdeckung

```cpp
// Mit I2C-Multiplexer TCA9548A (8 Kanäle)
#define MULTIPLEXER_ADDR 0x70
#define NUM_SENSORS 4

uint16_t sensorDistances[NUM_SENSORS];

void selectSensor(uint8_t channel) {
  if (channel > 7) return;

  Wire.beginTransmission(MULTIPLEXER_ADDR);
  Wire.write(1 << channel);  // Kanal aktivieren
  Wire.endTransmission();
}

void readAllSensors() {
  for (uint8_t i = 0; i < NUM_SENSORS; i++) {
    selectSensor(i);
    delay(10);
    sensorDistances[i] = readDistance();
  }

  // Top-View Anzeige
  drawTopView();
}

void drawTopView() {
  // Sensor-Array: 90° Winkel zwischen Sensoren
  int centerX = 160, centerY = 240;
  for (int i = 0; i < NUM_SENSORS; i++) {
    int angle = i * 90;  // 0°, 90°, 180°, 270°
    int radius = map(sensorDistances[i], 0, 5000, 0, 100);

    float rad = angle * PI / 180.0;
    int x = centerX + radius * cos(rad);
    int y = centerY + radius * sin(rad);

    lcd.drawLine(centerX, centerY, x, y, TFT_GREEN);
  }
}
```

**Anwendungen**:
- 360°-Überwachung
- Roboter-Navigation
- Sicherheitssystem

## 📚 Weiterführende Themen

### Time-of-Flight Technologie im Detail

**Phasenverschiebungs-Methode** (alternative zu Direct ToF):
```
Statt direkter Laufzeitmessung:
1. Moduliertes Licht aussenden (z.B. 100 MHz Sinuswelle)
2. Phasenverschiebung des reflektierten Signals messen
3. Aus Phasenverschiebung die Distanz berechnen

Vorteile:
- Höhere Genauigkeit bei kurzen Distanzen
- Geringerer Stromverbrauch

Nachteile:
- Geringere Reichweite (typisch < 10m)
- Teurer in der Herstellung
```

### Laser-Sicherheit

Der R50-C nutzt einen **Class 1 Laser** (905 nm):

- ✅ **Augensicher**: Auch bei direktem Blick in den Strahl
- 💡 **Unsichtbar**: Infrarot (905 nm) ist für das menschliche Auge nicht sichtbar
- ⚠️ **Nicht sichtbare Gefahr**: Nicht bewusst wegschauen → Class 1 wichtig!

**Technische Details**:
- Wellenlänge: 905 nm (Nahes Infrarot)
- Pulsleistung: < 1 mW (kontinuierlich äquivalent)
- Pulsdauer: typisch 10-20 ns
- Strahlaufweitung: ca. 3° (reduziert Intensität mit Distanz)

### Umgebungseinflüsse

**Faktoren, die Messungen beeinflussen**:

1. **Oberflächenreflektivität**:
   - Weiß/Glänzend: 80-90% Reflexion → Max. Reichweite
   - Grau/Matt: 30-50% Reflexion → Reduzierte Reichweite
   - Schwarz/Absorbierend: < 10% Reflexion → Stark reduzierte Reichweite

2. **Umgebungslicht**:
   - Innenraum: Kein Einfluss
   - Schatten/Bewölkt: Minimaler Einfluss
   - Direkte Sonne: 10-30% Reichweitenreduktion
   - → Lösung: Optischer Filter, Sichtschutz

3. **Winkel zur Oberfläche**:
   - 0° (senkrecht): 100% Reflexion zurück
   - 30°: ca. 70% zurück
   - 60°: ca. 30% zurück
   - > 75°: Unreliable Messung

4. **Wetter**:
   - Nebel/Dunst: Streuung → Reichweite -50%
   - Regen: Tropfen reflektieren → Fehlmessungen
   - Schnee: Ähnlich wie Regen
   - → Lösung: Multi-Echo-Algorithmus (komplexere Sensoren)

### Vergleich: R50-C vs. andere Sensoren

**vs. Ultraschall (HC-SR04)**:
- ✅ ToF: Schneller (50 Hz vs. 10 Hz)
- ✅ ToF: Größere Reichweite (50m vs. 4m)
- ✅ ToF: Besser bei Wind/Temperatur
- ❌ ToF: Teurer
- ❌ ToF: Probleme mit transparenten Objekten
- ✅ Ultraschall: Funktioniert bei Glas/Wasser

**vs. Sharp IR (GP2Y0A21YK0F)**:
- ✅ ToF: Viel größere Reichweite (50m vs. 0,8m)
- ✅ ToF: Keine Farb-Abhängigkeit
- ✅ ToF: Höhere Genauigkeit
- ❌ ToF: Höherer Stromverbrauch
- ❌ ToF: Höherer Preis

**vs. VL53L0X ToF (STMicroelectronics)**:
- ✅ R50-C: Größere Reichweite (50m vs. 2m)
- ❌ R50-C: Geringere Genauigkeit (±1% vs. ±3%)
- ❌ R50-C: Größerer Formfaktor
- ✅ VL53L0X: Günstiger
- ✅ VL53L0X: Niedriger Stromverbrauch

## 🛠️ Praktische Tipps

### Optimale Sensor-Positionierung

```
┌─────────────────────────────────────┐
│  OPTIMAL:                            │
│                                      │
│   Sensor ──→ ┆ Objekt               │
│              ┆ (senkrecht)           │
│                                      │
│  AKZEPTABEL:                         │
│                                      │
│   Sensor ──→ ╱ Objekt               │
│             ╱  (< 30° Winkel)        │
│                                      │
│  SCHLECHT:                           │
│                                      │
│   Sensor ──→ ⟋ Objekt               │
│            ⟋   (> 45° Winkel)        │
│                                      │
└─────────────────────────────────────┘
```

### Kalibrierung

Der R50-C benötigt **keine manuelle Kalibrierung**, aber:

```cpp
// Offset-Korrektur bei systematischem Fehler
#define DISTANCE_OFFSET 5  // cm (ermittelt durch Vergleichsmessung)

uint16_t getCalibratedDistance() {
  uint16_t raw = readDistance();
  int16_t calibrated = raw - DISTANCE_OFFSET;

  if (calibrated < 0) calibrated = 0;
  return (uint16_t)calibrated;
}

// Kalibrierung mit bekannter Referenz
void calibrate() {
  Serial.println("Platziere Objekt bei genau 100 cm...");
  delay(5000);

  uint16_t measured = readDistance();
  DISTANCE_OFFSET = measured - 100;  // Sollte 100cm sein

  Serial.printf("Offset bestimmt: %d cm\n", DISTANCE_OFFSET);
}
```

### Stromverbrauch reduzieren

```cpp
// Power-Management
void enterLowPowerMode() {
  // ESP32 in Light Sleep
  esp_sleep_enable_timer_wakeup(1000000);  // 1 Sekunde
  esp_light_sleep_start();

  // Nach Aufwachen: Eine Messung
  uint16_t distance = readDistance();
  publishDistance(distance);  // z.B. via MQTT

  // Wieder schlafen
  enterLowPowerMode();
}

// Batterie-Betrieb: Bis zu mehrere Wochen möglich!
```

## 📖 Ressourcen & Links

### Datenblätter & Dokumentation
- **R50-C Datasheet**: (Hersteller-spezifisch, oft begrenzt verfügbar)
- **I2C Specification**: [NXP I2C-bus specification](https://www.nxp.com/docs/en/user-guide/UM10204.pdf)
- **Laser Safety**: [IEC 60825-1 Standard](https://en.wikipedia.org/wiki/Laser_safety#Class_1)
- **Time-of-Flight Prinzip**: [Wikipedia ToF](https://de.wikipedia.org/wiki/Laufzeitmessung)

### CYD-Spezifisch
- **CYD Display Config**: `lib/CYD_Display_Config/CYD_Display_Config.h`
- **I2C Pins**: extSDA (GPIO 22), extSCL (GPIO 27)
- **Touch Controller**: XPT2046 (SPI)

### Verwandte Beispiele
- **Beispiel 16**: Spirit Level (Erste I2C Verwendung)
- **Beispiel 17**: Lightning Sensor (Interrupt + Polling Pattern)
- **Beispiel 18**: Compass & Orientation (9-DoF Sensor Fusion)

### Arduino Libraries
```cpp
// Verwendete Libraries:
#include <Wire.h>              // I2C (ESP32 Core)
#include <CYD_Display_Config.h> // CYD-Academy Library
```

**Keine externe R50-C Library erforderlich!** Direktes I2C-Protokoll.

## 🎓 Lernziele

Nach Abschluss dieses Beispiels solltest du:

- ✅ **Time-of-Flight Technologie** verstehen
- ✅ **Direkte I2C-Kommunikation** ohne Library beherrschen
- ✅ **Ring-Buffer** für Datenspeicherung implementieren können
- ✅ **Echtzeit-Graphen** mit Scrolling erstellen
- ✅ **Statistik-Tracking** (Min/Max/Avg) umsetzen
- ✅ **Touch-Interaktion** für Modusauswahl nutzen
- ✅ **Fehlerbehandlung** in Sensor-Anwendungen durchführen
- ✅ **Datenvisualisierung** mit Farb-Codierung beherrschen

## 🔜 Nächste Schritte

Weitere Sensor-Beispiele in Planung:
- **Beispiel 20**: Farbsensor TCS34725 (RGB-Erkennung)
- **Beispiel 21**: Luftqualität BME680 (Gas, Temp, Feuchte, Druck)
- **Beispiel 22**: Gesten-Sensor APDS-9960 (Berührungslos)

---

**Viel Erfolg mit deinem Laser Distance Projekt! 🎯📏**

Bei Fragen: CYD-Academy GitHub Issues
