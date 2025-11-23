# Beispiel 22: Wetterstation mit BMP180

Dieses Beispiel demonstriert eine vollständige Wetterstation mit dem **BMP180 Barometric Pressure Sensor**. Der Sensor misst Luftdruck und Temperatur und erstellt daraus eine lokale Wettervorhersage basierend auf Drucktrends.

## 📸 Features

- **🌡️ Luftdruck-Messung**: 300 - 1100 hPa mit ±1 hPa Genauigkeit
- **🌡️ Temperatur-Messung**: -40 bis +85°C mit ±2°C Genauigkeit  
- **☁️ Wettervorhersage**: 6 Zustände basierend auf 3h Drucktrend
  - Sonnig (Hochdruck, steigend)
  - Heiter (Hochdruck, stabil)
  - Bewölkt (Normal, leicht fallend)
  - Regen (Tiefdruck, stabil/fallend)
  - Gewitter (Stark fallend)
  - Schnee (Fallend + Temperatur < 2°C)
- **🎨 Wetter-Icons**: Animierte grafische Darstellung
  - Sonne mit Strahlen
  - Wolken (hell/dunkel)
  - Regentropfen
  - Blitze (Gewitter)
  - Schneeflocken
- **📊 24h Druckverlaufsdiagramm**: 288 Datenpunkte (alle 5 Min)
- **📏 Höhenberechnung**: Aus Luftdruck (barometrische Höhenformel)
- **🎛️ Unit-Wechsel per Touch**:
  - Druck: hPa ↔ mmHg
  - Temperatur: °C ↔ °F
  - Höhe: m ↔ ft
- **📈 Statistik**: Min/Max für Druck und Temperatur

## 🔌 Hardware-Anforderungen

### Hauptkomponenten
- **ESP32-2432S028R** (CYD - Cheap Yellow Display)
- **BMP180** Barometric Pressure Sensor (oder BMP085)

### BMP180 Spezifikationen

**Messbereiche**:
- **Luftdruck**: 300 - 1100 hPa
- **Temperatur**: -40 bis +85°C
- **Höhe**: -500 bis +9000 m (berechnet)

**Genauigkeit**:
- **Druck**: ±1 hPa (@ 25°C)
- **Temperatur**: ±2°C
- **Relative Genauigkeit**: ±0.12 hPa (entspricht ±1 m Höhe)

**Technische Daten**:
- **Interface**: I2C (Standard: 100 kHz, Fast: 400 kHz)
- **I2C-Adressen**:
  - 0x77 (Standard, SDO Pin HIGH oder offen)
  - 0x76 (SDO Pin LOW - selten)
- **Spannung**: 1,8V - 3,6V (VDD), 3,3V empfohlen
- **Stromaufnahme**:
  - Standby: 0,1 µA
  - Ultra Low Power: 3 µA @ 1 Sample/s
  - Standard: 5 µA @ 1 Sample/s
  - Ultra High Resolution: 12 µA @ 1 Sample/s
- **Sensor-Typ**: Piezoresistiver Drucksensor (MEMS)
- **Auflösung**:
  - Druck: 0,01 hPa (Ultra High Resolution)
  - Temperatur: 0,1°C
- **Ansprechzeit**: < 7,5 ms (Druck), < 4,5 ms (Temperatur)
- **Lebensdauer**: > 10 Jahre (typisch)

### Verbindung (I2C)

```
BMP180         CYD (ESP32-2432S028R)
------         ---------------------
VCC     -----> 3.3V
GND     -----> GND
SDA     -----> GPIO 22 (extSDA)
SCL     -----> GPIO 27 (extSCL)
```

**Wichtig**:
- BMP180 benötigt 3,3V (5V kann Sensor beschädigen!)
- I2C Pull-ups sind auf CYD bereits vorhanden (intern im ESP32)
- Keine externen Komponenten erforderlich

## 🔬 Technischer Hintergrund

### Barometrische Druckmessung

Der BMP180 nutzt einen **piezoresistiven MEMS-Sensor** zur Druckmessung:

#### Funktionsprinzip

```
┌──────────────────────────────────────────┐
│  Piezoresistiver Drucksensor (MEMS):    │
│                                           │
│  1. Silizium-Membran (Diaphragm)         │
│     ↓                                     │
│  2. Luftdruck verformt Membran           │
│     ↓                                     │
│  3. Piezoresistoren ändern Widerstand    │
│     ↓                                     │
│  4. Wheatstone-Brücke misst Änderung     │
│     ↓                                     │
│  5. ADC wandelt in digitalen Wert        │
│                                           │
│  ┌────────┐                              │
│  │ Membran│ ← Luftdruck drückt           │
│  └───┬────┘                              │
│      │                                    │
│  ┌───┴───┐                               │
│  │Piez.R.│ Widerstand ändert sich        │
│  └───────┘                               │
│                                           │
└──────────────────────────────────────────┘
```

**Piezoresistiver Effekt**:
```
Wenn ein Material mechanisch verformt wird, 
ändert sich sein elektrischer Widerstand.

ΔR/R = K × ε

Wobei:
ΔR/R = Relative Widerstandsänderung
K = Piezo-Koeffizient (Material-abhängig)
ε = Dehnung (Strain)
```

**Wheatstone-Brücke**:
```
       R1
    ┌──┴──┐
Vin │     │ Vout
    ├──┬──┤
    │ R2  │
    └──┬──┘
       R3
    
Vout = Vin × (R1/(R1+R2) - R3/(R3+R4))

Wenn R1-R4 = Piezoresistoren, dann:
Vout ∝ Luftdruck
```

### Temperatur-Messung

Der BMP180 hat einen **integrierten Temperatursensor** (Band-Gap Referenz):

**Zweck**:
1. **Temperatur-Kompensation**: Piezoresistoren sind temperaturempfindlich
2. **Eigenständige Messung**: Umgebungstemperatur auslesen

**Messung**:
```
1. Band-Gap Spannung messen (temperaturabhängig)
2. ADC-Wert erfassen
3. Kalibrations-Koeffizienten anwenden
4. Temperatur berechnen
```

### Höhenberechnung aus Luftdruck

Die **barometrische Höhenformel** berechnet Höhe aus Luftdruck:

#### Internationale Höhenformel

```
h = (T₀ / L) × (1 - (P / P₀)^(R × L / (g × M)))

Wobei:
h = Höhe über Meeresspiegel (m)
T₀ = Temperatur auf Meereshöhe (288,15 K = 15°C)
L = Temperatur-Gradient (-0,0065 K/m)
P = Gemessener Luftdruck (Pa)
P₀ = Luftdruck auf Meereshöhe (101325 Pa = 1013,25 hPa)
R = Universelle Gaskonstante (8,314 J/(mol×K))
g = Erdbeschleunigung (9,807 m/s²)
M = Molare Masse von Luft (0,02896 kg/mol)
```

#### Vereinfachte Formel (für < 11 km Höhe)

```
h ≈ 44330 × (1 - (P / P₀)^0,1903)

Oder noch einfacher:
h ≈ 44330 × (1 - (P / P₀)^(1/5,255))
```

#### Praktisches Beispiel

```
Angenommen:
- Gemessener Druck: 950 hPa
- Referenzdruck (Meereshöhe): 1013,25 hPa

h = 44330 × (1 - (950 / 1013,25)^0,1903)
h = 44330 × (1 - 0,9375^0,1903)
h = 44330 × (1 - 0,9879)
h = 44330 × 0,0121
h ≈ 536 m
```

**Genauigkeit**:
- ±1 hPa Druck-Fehler → ±8 m Höhen-Fehler
- ±0,12 hPa Auflösung → ±1 m Höhen-Auflösung

### Wettervorhersage aus Drucktrend

Die lokale Wettervorhersage basiert auf **Luftdruck-Veränderungen**:

#### Warum funktioniert das?

Luftdruck reflektiert **Luftmassen-Bewegungen**:

```
HOCHDRUCK (> 1020 hPa):
- Absinkende Luft
- Wolkenauflösung
- Trockenes, stabiles Wetter
→ SONNIG

TIEFDRUCK (< 1000 hPa):
- Aufsteigende Luft
- Wolkenbildung
- Instabiles Wetter
→ REGEN, GEWITTER

DRUCKÄNDERUNG:
- Stark steigend (+3 hPa/3h): Wetterbesserung
- Stark fallend (-3 hPa/3h): Verschlechterung
```

#### Algorithmus im Code

```cpp
// Druckänderung über 3 Stunden
float pressureChange = currentPressure - pressureTrend[3h_ago];

if (currentPressure > 1023) {
  // Hochdruck
  if (pressureChange > 2) {
    WEATHER_SUNNY;  // Stark steigend → Schönwetter
  } else if (pressureChange > -2) {
    WEATHER_PARTLY_CLOUDY;  // Stabil hoch
  } else {
    WEATHER_CLOUDY;  // Fallend trotz Hochdruck
  }
} else if (currentPressure > 1013) {
  // Normaldruck
  if (pressureChange > 3) {
    WEATHER_PARTLY_CLOUDY;
  } else if (pressureChange > 0) {
    WEATHER_CLOUDY;
  } else if (pressureChange > -3) {
    WEATHER_RAINY;
  } else {
    WEATHER_STORMY;  // Stark fallend → Gewitter
  }
} else {
  // Tiefdruck
  if (pressureChange > 2) {
    WEATHER_CLOUDY;  // Steigend von Tief
  } else if (pressureChange > -2) {
    WEATHER_RAINY;  // Stabil tief
  } else {
    if (temperature < 2°C) {
      WEATHER_SNOW;  // Kalt + fallend
    } else {
      WEATHER_STORMY;  // Warm + fallend
    }
  }
}
```

#### Genauigkeit und Limitierungen

**Genauigkeit**:
- **Kurzfristig (3-6h)**: 70-80% korrekt
- **Mittelfristig (12-24h)**: 60-70% korrekt
- **Lokal**: Funktioniert am besten

**Limitierungen**:
- ❌ Keine großräumigen Wettersysteme berücksichtigt
- ❌ Keine Windrichtung/Windgeschwindigkeit
- ❌ Keine Wolkenbeobachtung
- ❌ Keine Wettermodelle (Fronten, etc.)
- ✅ Aber: Gute lokale Trendvorhersage

**Verbesserungen**:
- Nach 24h Laufzeit: Bessere Trends
- Mehrere Sensoren: Druckgradient berechnen
- Online-Wetterdaten: Kombinierte Vorhersage

### Druckeinheiten Vergleich

| Einheit | Symbol | Faktor zu Pa | Verwendung |
|---------|--------|--------------|------------|
| **Pascal** | Pa | 1 | SI-Einheit |
| **Hektopascal** | hPa | 100 | Meteorologie (Standard) |
| **Millibar** | mbar | 100 | Alt, identisch zu hPa |
| **Millimeter Quecksilber** | mmHg | 133,322 | Medizin, Russland |
| **Inch Quecksilber** | inHg | 3386,39 | USA (Aviation) |
| **Pound per Square Inch** | psi | 6894,76 | USA (Technik) |
| **Bar** | bar | 100000 | Technik |
| **Atmosphäre** | atm | 101325 | Referenz |

**Umrechnung**:
```
1 hPa = 1 mbar
1 hPa = 0,750062 mmHg
1 hPa = 0,02953 inHg
1 hPa = 0,01450 psi

Beispiel:
1013,25 hPa = 760 mmHg = 29,92 inHg = 1 atm
```

## 📡 I2C-Kommunikation

### Adafruit BMP085 Library

Der BMP180 hat ein **komplexes Kalibrierungs-Protokoll**, daher Library verwenden:

```cpp
#include <Adafruit_BMP085.h>

Adafruit_BMP085 bmp;

// Initialisierung
if (!bmp.begin()) {
  Serial.println("BMP180 nicht gefunden!");
}

// Daten auslesen
float pressure = bmp.readPressure() / 100.0;  // Pa → hPa
float temperature = bmp.readTemperature();     // °C
float altitude = bmp.readAltitude(101325);     // m (mit Referenzdruck)
```

**Übersampling-Modi**:
```cpp
// BMP180 unterstützt verschiedene Genauigkeiten:
bmp.begin(BMP085_ULTRALOWPOWER);       // 1 Sample, 4,5 ms, 3 µA
bmp.begin(BMP085_STANDARD);            // 2 Samples, 7,5 ms, 5 µA
bmp.begin(BMP085_HIGHRES);             // 4 Samples, 13,5 ms, 7 µA
bmp.begin(BMP085_ULTRAHIGHRES);        // 8 Samples, 25,5 ms, 12 µA (Default)
```

### Kalibrations-Daten

Der BMP180 hat **11 Kalibrations-Koeffizienten** im EEPROM:

```
Register    Wert-Name    Beschreibung
---------------------------------------
0xAA-0xAB   AC1          Druckkoeffizient 1
0xAC-0xAD   AC2          Druckkoeffizient 2
0xAE-0xAF   AC3          Druckkoeffizient 3
0xB0-0xB1   AC4          Druckkoeffizient 4
0xB2-0xB3   AC5          Temperaturkoeffizient 1
0xB4-0xB5   AC6          Temperaturkoeffizient 2
0xB6-0xB7   B1           Druckkoeffizient 5
0xB8-0xB9   B2           Druckkoeffizient 6
0xBA-0xBB   MB           Temperaturkoeffizient 3
0xBC-0xBD   MC           Temperaturkoeffizient 4
0xBE-0xBF   MD           Temperaturkoeffizient 5
```

**Diese Werte sind werks-kalibriert** und unique für jeden Sensor!

### Low-Level I2C-Protokoll (für Fortgeschrittene)

**Temperatur auslesen**:
```cpp
// 1. Temperatur-Messung starten
Wire.beginTransmission(0x77);
Wire.write(0xF4);                 // Control Register
Wire.write(0x2E);                 // Start Temperature Measurement
Wire.endTransmission();

delay(5);                         // Warten (4,5 ms)

// 2. Rohdaten lesen
Wire.beginTransmission(0x77);
Wire.write(0xF6);                 // MSB Register
Wire.endTransmission();

Wire.requestFrom(0x77, 2);
uint16_t UT = (Wire.read() << 8) | Wire.read();  // Unkompensiert

// 3. Mit Kalibrierung kompensieren
int32_t X1 = ((UT - AC6) * AC5) >> 15;
int32_t X2 = (MC << 11) / (X1 + MD);
int32_t B5 = X1 + X2;
float temperature = ((B5 + 8) >> 4) / 10.0;  // °C
```

**Druck auslesen**:
```cpp
// 1. Druck-Messung starten
Wire.beginTransmission(0x77);
Wire.write(0xF4);
Wire.write(0x34 + (oss << 6));    // oss = Oversampling (0-3)
Wire.endTransmission();

delay(2 + (3 << oss));            // Warten (abhängig von oss)

// 2. Rohdaten lesen (3 Bytes!)
Wire.beginTransmission(0x77);
Wire.write(0xF6);
Wire.endTransmission();

Wire.requestFrom(0x77, 3);
uint32_t UP = ((Wire.read() << 16) | (Wire.read() << 8) | Wire.read()) >> (8 - oss);

// 3. Kompensierung (komplex, siehe Datenblatt)
// ... viele Berechnungen mit Kalibrations-Koeffizienten
int32_t pressure = /* result in Pa */;
```

**Empfehlung**: Nutze die Adafruit Library - das Protokoll ist sehr komplex!

## 💻 Code-Struktur

### Hauptkomponenten

#### 1. Sensor-Initialisierung

```cpp
#include <Adafruit_BMP085.h>

Adafruit_BMP085 bmp;

void setup() {
  Wire.begin(extSDA, extSCL);

  if (!bmp.begin()) {
    Serial.println("BMP180 nicht gefunden!");
    return;
  }

  // Erste Messung
  currentPressure = bmp.readPressure() / 100.0;
  currentTemp = bmp.readTemperature();
}
```

#### 2. Daten auslesen

```cpp
void readSensor() {
  float pressure = bmp.readPressure() / 100.0;  // Pa → hPa
  float temp = bmp.readTemperature();            // °C
  float altitude = bmp.readAltitude(seaLevelPressure * 100);  // m

  // Validierung (BMP180 Range: 300-1100 hPa)
  if (pressure >= 300 && pressure <= 1100) {
    currentPressure = pressure;
    currentTemp = temp;
    currentAltitude = altitude;

    // Statistik
    if (pressure < pressureMin) pressureMin = pressure;
    if (pressure > pressureMax) pressureMax = pressure;
  }
}
```

#### 3. Drucktrend aktualisieren

```cpp
#define TREND_SAMPLES 36  // 36 x 5 Min = 3h
float pressureTrend[TREND_SAMPLES];

void updatePressureTrend() {
  pressureTrend[trendIndex] = currentPressure;
  trendIndex = (trendIndex + 1) % TREND_SAMPLES;

  // Auch in 24h Historie
  pressureHistory[historyIndex] = currentPressure;
  historyIndex = (historyIndex + 1) % HISTORY_24H;

  // Wettervorhersage berechnen
  calculateWeatherForecast();
}
```

#### 4. Wettervorhersage

```cpp
void calculateWeatherForecast() {
  // Druckänderung über 3h
  int oldIndex = (trendIndex - TREND_SAMPLES + TREND_SAMPLES) % TREND_SAMPLES;
  float pressureChange = currentPressure - pressureTrend[oldIndex];

  // Algorithmus (siehe oben)
  if (currentPressure > 1023) {
    if (pressureChange > 2) currentWeather = WEATHER_SUNNY;
    // ...
  }
}
```

#### 5. Wetter-Icons zeichnen

```cpp
void drawSunIcon(int x, int y) {
  // Sonne
  lcd.fillCircle(x, y, 20, TFT_YELLOW);

  // Strahlen (8 Stück)
  for (int i = 0; i < 8; i++) {
    float angle = i * PI / 4;
    int x1 = x + cos(angle) * 25;
    int y1 = y + sin(angle) * 25;
    int x2 = x + cos(angle) * 35;
    int y2 = y + sin(angle) * 35;
    lcd.drawLine(x1, y1, x2, y2, TFT_YELLOW);
  }
}

void drawCloudIcon(int x, int y, uint16_t color) {
  lcd.fillCircle(x - 15, y, 10, color);
  lcd.fillCircle(x, y - 5, 12, color);
  lcd.fillCircle(x + 15, y, 10, color);
  lcd.fillRect(x - 20, y, 40, 10, color);
}
```

#### 6. Druckverlaufs-Graph

```cpp
void drawPressureGraph() {
  // Min/Max finden (letzte 24h)
  float minP = 1100, maxP = 300;
  for (int i = 0; i < HISTORY_24H; i++) {
    if (pressureHistory[i] > 0) {
      if (pressureHistory[i] < minP) minP = pressureHistory[i];
      if (pressureHistory[i] > maxP) maxP = pressureHistory[i];
    }
  }

  // Daten zeichnen
  for (int i = 0; i < HISTORY_24H - 1; i++) {
    int idx1 = (historyIndex + i) % HISTORY_24H;
    int idx2 = (historyIndex + i + 1) % HISTORY_24H;

    int x1 = graphX + (i * graphW / HISTORY_24H);
    int x2 = graphX + ((i + 1) * graphW / HISTORY_24H);

    int y1 = graphY + graphH - map(pressureHistory[idx1] * 10, minP * 10, maxP * 10, 0, graphH);
    int y2 = graphY + graphH - map(pressureHistory[idx2] * 10, minP * 10, maxP * 10, 0, graphH);

    lcd.drawLine(x1, y1, x2, y2, TFT_CYAN);
  }
}
```

## 🚀 Installation & Verwendung

### 1. Library installieren

```
Arduino IDE -> Tools -> Manage Libraries
Suche: "Adafruit BMP085"
Installieren: "Adafruit BMP085 Unified" + Abhängigkeiten
```

**Abhängigkeiten** (werden automatisch installiert):
- Adafruit Unified Sensor
- Adafruit Bus IO

### 2. Hardware aufbauen

```
BMP180 mit CYD verbinden:
- VCC → 3.3V (NICHT 5V!)
- GND → GND
- SDA → GPIO 22
- SCL → GPIO 27
```

### 3. Code hochladen

```bash
# In Arduino IDE:
# 1. Board: "ESP32 Dev Module" auswählen
# 2. Datei → Beispiele → CYD-Academy → 22_Weather_Station_BMP180 öffnen
# 3. Hochladen
```

### 4. Bedienung

**Erste Inbetriebnahme**:
1. Sensor startet → Erste Messung
2. Wettervorhersage zunächst "Heiter"
3. Nach 3h: Trend-basierte Vorhersage aktiv
4. Nach 24h: Vollständiger Druckverlauf

**Unit-Wechsel**:
- **hPa ↔ mmHg**: Tippe auf Button unten links
- **°C ↔ °F**: Tippe auf Button unten mitte
- **m ↔ ft**: Tippe auf Button unten rechts

**Seehöhen-Kalibrierung**:
Wenn du deine genaue Höhe kennst, kannst du den Referenzdruck anpassen:
```cpp
// Im Code (Zeile ~78):
float seaLevelPressure = 1013.25;  // Anpassen!

// Berechnung:
// P₀ = P × (1 - h / 44330)^(-5,255)
// 
// Beispiel für 500m Höhe:
// P₀ = 950 × (1 - 500 / 44330)^(-5,255)
// P₀ ≈ 1013,25 hPa
```

### Serielle Ausgabe

```
=== CYD Wetterstation - BMP180 ===
I2C initialisiert auf SDA: 22, SCL: 27
BMP180 gefunden!
Druck: 1013.45 hPa, Temp: 21.30 °C

[10] Druck: 1013.45 hPa, Temp: 21.30 °C, Höhe: 0 m
[20] Druck: 1013.50 hPa, Temp: 21.35 °C, Höhe: -4 m
[300] Drucktrend aktualisiert: 1013.60 hPa (Index: 1)
[310] Druckänderung (3h): 0.00 hPa
[600] Drucktrend aktualisiert: 1013.55 hPa (Index: 2)
[610] Druckänderung (3h): 0.00 hPa
```

## 🔧 Troubleshooting

### Problem: "BMP180 nicht gefunden"

**Symptome**:
- Display zeigt "BMP180 FEHLER!"
- Serielle Ausgabe: "BMP180 nicht gefunden auf 0x77"

**Lösungen**:

1. **I2C-Scanner**:
   ```cpp
   void scanI2C() {
     Serial.println("Scanne I2C-Bus...");
     for (byte addr = 1; addr < 127; addr++) {
       Wire.beginTransmission(addr);
       if (Wire.endTransmission() == 0) {
         Serial.printf("Gefunden: 0x%02X\n", addr);
       }
     }
   }
   ```
   - Erwartetes Ergebnis: `0x77` (oder `0x76`)

2. **Verkabelung prüfen**:
   - SDA und SCL vertauscht?
   - VCC wirklich auf 3,3V (nicht 5V!)
   - Lose Verbindungen?

3. **Alternative Adresse**:
   ```cpp
   // Falls Sensor auf 0x76
   if (!bmp.begin(0x76)) {
     Serial.println("Auch 0x76 fehlgeschlagen");
   }
   ```

4. **Sensor defekt**:
   - BMP180 kann durch 5V beschädigt werden
   - → Neuen Sensor testen

### Problem: Unrealistische Werte

**Symptome**:
- Druck immer bei 300 oder 1100 hPa
- Temperatur springt wild

**Ursachen & Lösungen**:

1. **Sensor nicht bereit**:
   ```cpp
   // Längere Wartezeit nach begin()
   if (!bmp.begin()) {
     Serial.println("Fehler!");
   }
   delay(100);  // BMP180 Zeit geben
   ```

2. **Falsche Kalibrierungs-Daten**:
   - BMP180 EEPROM beschädigt
   - → Sensor defekt, austauschen

3. **I2C-Kommunikations-Fehler**:
   ```cpp
   // I2C-Geschwindigkeit reduzieren
   Wire.setClock(50000);  // 50 kHz statt 100 kHz
   ```

### Problem: Wettervorhersage ungenau

**Symptome**:
- Vorhersage passt nicht zur Realität
- Immer "Heiter" obwohl es regnet

**Gründe**:

1. **Zu kurze Laufzeit**:
   - Wettervorhersage benötigt **mindestens 3h** Daten
   - Nach 24h: Genauigkeit steigt

2. **Lokale vs. großräumige Systeme**:
   - BMP180 sieht nur **lokalen Druck**
   - Kann keine Wetterfront von 100 km Entfernung erkennen
   - → Kombination mit Online-Wetterdaten empfohlen

3. **Geografische Lage**:
   - In Tälern/Bergen: Lokale Effekte überlagern
   - Küstennähe: Meeresbrise verfälscht
   - → Vorhersage in Ebenen am besten

### Problem: Höhenmessung ungenau

**Symptome**:
- Angezeigte Höhe ±50m daneben
- Höhe ändert sich mit Wetter

**Ursache**: Luftdruck ändert sich mit Wetter!

**Lösung**: Seehöhen-Referenzdruck anpassen

```cpp
// Wenn du auf 500m Höhe bist:
// Aktuellen Druck ablesen (z.B. 955 hPa)
// Seehöhen-Druck berechnen:

float currentPressure = 955.0;  // Aktuell gemessen
float knownAltitude = 500.0;     // Bekannte Höhe

// Seehöhen-Druck berechnen
float seaLevelPressure = currentPressure / pow(1.0 - (knownAltitude / 44330.0), 5.255);

// Im Code eintragen (Zeile 78)
```

**Alternative**: Online-Wetterdaten verwenden (siehe Erweiterungsidee 1)

### Problem: Druck weicht von Wetterdienst ab

**Ursache**: Wetterdienste geben **reduzierten Luftdruck** (auf Meereshöhe) an!

**Beispiel**:
```
Deine Höhe: 500 m
BMP180 misst: 955 hPa (absoluter Druck)
Wetterdienst: 1013 hPa (reduziert auf Meereshöhe)

Umrechnung:
P₀ = P × (1 - h / 44330)^(-5,255)
P₀ = 955 × (1 - 500 / 44330)^(-5,255)
P₀ ≈ 1013 hPa ✓
```

**Lösung**: Code berechnet bereits reduzierten Druck:
```cpp
float seaLevelPressure = 1013.25;  // Referenz
float altitude = bmp.readAltitude(seaLevelPressure * 100);
```

## 🎯 Erweiterungsideen

### 1. Online-Wetterdaten Integration (WiFi + API)

**Konzept**: Kombiniere lokale Messungen mit Online-Wetterdaten

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Credentials.h"  // WiFi-Zugangsdaten

// OpenWeatherMap API
const char* apiKey = "DEIN_API_KEY";
const char* city = "Zürich";

struct OnlineWeather {
  float pressure_sea;      // Luftdruck auf Meereshöhe
  float temperature;
  float humidity;
  String description;
  String icon;
};

OnlineWeather onlineWeather;

void fetchOnlineWeather() {
  if (WiFi.status() != WL_CONNECTED) return;

  String url = "http://api.openweathermap.org/data/2.5/weather?q=" + String(city) + "&appid=" + apiKey + "&units=metric";

  HTTPClient http;
  http.begin(url);

  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);

    onlineWeather.pressure_sea = doc["main"]["pressure"];  // hPa
    onlineWeather.temperature = doc["main"]["temp"];
    onlineWeather.humidity = doc["main"]["humidity"];
    onlineWeather.description = doc["weather"][0]["description"].as<String>();

    Serial.printf("Online: %.1f hPa, %.1f °C, %s\n",
                  onlineWeather.pressure_sea,
                  onlineWeather.temperature,
                  onlineWeather.description.c_str());

    // Seehöhen-Druck kalibrieren
    seaLevelPressure = onlineWeather.pressure_sea;
  }

  http.end();
}

// Alle 30 Minuten aktualisieren
if (millis() - lastOnlineUpdate >= 1800000) {
  fetchOnlineWeather();
  lastOnlineUpdate = millis();
}
```

**Vorteile**:
- Präziser Seehöhen-Druck
- Vergleich lokal vs. großräumig
- Bessere Wettervorhersage

**API-Keys**:
- OpenWeatherMap: https://openweathermap.org/api (Gratis bis 60 Calls/Min)
- WeatherAPI: https://www.weatherapi.com/ (Gratis bis 1M Calls/Monat)

### 2. BME280 Upgrade (mit Luftfeuchtigkeit)

**Konzept**: BMP180 → BME280 für zusätzliche Feuchtigkeit

```cpp
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;  // Statt BMP180

void setup() {
  if (!bme.begin(0x76)) {  // BME280 oft auf 0x76
    Serial.println("BME280 nicht gefunden!");
  }
}

void readSensor() {
  float pressure = bme.readPressure() / 100.0;
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();  // NEU!

  // Taupunkt berechnen
  float dewPoint = temperature - ((100 - humidity) / 5.0);

  // Gefühlte Temperatur (Hitzeindex)
  float heatIndex = calculateHeatIndex(temperature, humidity);

  // Anzeige erweitern
  lcd.printf("Feuchte: %.1f %%\n", humidity);
  lcd.printf("Taupunkt: %.1f °C\n", dewPoint);
}

float calculateHeatIndex(float T, float RH) {
  // Rothfusz Gleichung
  float HI = -42.379 +
             2.04901523 * T +
             10.14333127 * RH -
             0.22475541 * T * RH -
             6.83783e-3 * T * T -
             5.481717e-2 * RH * RH +
             1.22874e-3 * T * T * RH +
             8.5282e-4 * T * RH * RH -
             1.99e-6 * T * T * RH * RH;

  // Umrechnung in Celsius
  HI = (HI - 32) * 5 / 9;
  return HI;
}
```

**BME280 vs. BMP180**:
| Feature | BMP180 | BME280 |
|---------|--------|--------|
| Druck | ✓ | ✓ |
| Temperatur | ✓ | ✓ |
| Luftfeuchtigkeit | ✗ | ✓ (0-100%) |
| Preis | ~3€ | ~5€ |
| Genauigkeit (Druck) | ±1 hPa | ±1 hPa |
| Genauigkeit (Feuchte) | - | ±3% |

### 3. SD-Karten Logging für Langzeit-Analyse

**Konzept**: Wetterdaten auf SD speichern, später analysieren

```cpp
#include <SD.h>
#include <SPI.h>
#include <TimeLib.h>  // Für Timestamp

#define SD_CS 5

void setupSD() {
  if (!SD.begin(SD_CS)) {
    Serial.println("SD-Karte Fehler!");
    return;
  }

  // CSV-Header
  File logFile = SD.open("/weather_log.csv", FILE_WRITE);
  if (logFile) {
    logFile.println("Timestamp,Pressure_hPa,Temperature_C,Altitude_m,Weather_Forecast");
    logFile.close();
  }
}

void logWeatherData() {
  File logFile = SD.open("/weather_log.csv", FILE_APPEND);
  if (logFile) {
    // Unix-Timestamp (von NTP)
    logFile.print(now());
    logFile.print(",");
    logFile.print(currentPressure, 2);
    logFile.print(",");
    logFile.print(currentTemp, 2);
    logFile.print(",");
    logFile.print(currentAltitude, 1);
    logFile.print(",");
    logFile.println(weatherText[currentWeather]);
    logFile.close();

    Serial.println("Wetterdaten geloggt");
  }
}

// Alle 5 Minuten loggen (wie Trend-Update)
if (millis() - lastLog >= 300000) {
  logWeatherData();
  lastLog = millis();
}
```

**Python-Analyse**:
```python
import pandas as pd
import matplotlib.pyplot as plt
from datetime import datetime

# CSV laden
df = pd.read_csv('weather_log.csv')
df['Timestamp'] = pd.to_datetime(df['Timestamp'], unit='s')

# Druckverlauf über mehrere Tage
plt.figure(figsize=(12, 6))
plt.plot(df['Timestamp'], df['Pressure_hPa'], label='Luftdruck')
plt.axhline(y=1013.25, color='r', linestyle='--', label='Standard')
plt.xlabel('Zeit')
plt.ylabel('Luftdruck (hPa)')
plt.title('Langzeit-Druckverlauf')
plt.legend()
plt.grid(True)
plt.show()

# Statistik
print(df.describe())
print(f"Anzahl Tage: {(df['Timestamp'].max() - df['Timestamp'].min()).days}")
```

### 4. MQTT / Home Assistant Integration

**Konzept**: Wetterdaten in Smart Home integrieren

```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include "Credentials.h"

WiFiClient espClient;
PubSubClient mqtt(espClient);

#define MQTT_SERVER "192.168.1.100"
#define MQTT_PORT 1883
#define MQTT_TOPIC_PREFIX "homeassistant/sensor/cyd_weather"

void setupMQTT() {
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.connect("CYD-Weather-Station");

  // Home Assistant Auto-Discovery
  publishDiscovery();
}

void publishDiscovery() {
  // Luftdruck
  String config = R"(
  {
    "name": "CYD Luftdruck",
    "state_topic": "homeassistant/sensor/cyd_weather/pressure",
    "unit_of_measurement": "hPa",
    "device_class": "pressure",
    "icon": "mdi:gauge"
  }
  )";
  mqtt.publish("homeassistant/sensor/cyd_pressure/config", config.c_str(), true);

  // Temperatur
  config = R"(
  {
    "name": "CYD Temperatur",
    "state_topic": "homeassistant/sensor/cyd_weather/temperature",
    "unit_of_measurement": "°C",
    "device_class": "temperature",
    "icon": "mdi:thermometer"
  }
  )";
  mqtt.publish("homeassistant/sensor/cyd_temperature/config", config.c_str(), true);
}

void publishWeatherData() {
  char payload[32];

  sprintf(payload, "%.2f", currentPressure);
  mqtt.publish("homeassistant/sensor/cyd_weather/pressure", payload);

  sprintf(payload, "%.2f", currentTemp);
  mqtt.publish("homeassistant/sensor/cyd_weather/temperature", payload);

  // Wettervorhersage als Text
  mqtt.publish("homeassistant/sensor/cyd_weather/forecast", weatherText[currentWeather].c_str());
}

// Alle 60 Sekunden publishen
if (millis() - lastPublish >= 60000) {
  publishWeatherData();
  lastPublish = millis();
}
```

**Home Assistant Automation**:
```yaml
# automation.yaml
- alias: "Wetter-Warnung"
  trigger:
    platform: numeric_state
    entity_id: sensor.cyd_luftdruck
    below: 980
  action:
    service: notify.mobile_app
    data:
      message: "Tiefdruck! Schlechtes Wetter erwartet."
      title: "Wetterwarnung"

- alias: "Fenster schließen Warnung"
  trigger:
    platform: state
    entity_id: sensor.cyd_weather_forecast
    to: "Regen"
  condition:
    condition: state
    entity_id: binary_sensor.fenster_offen
    state: "on"
  action:
    service: notify.mobile_app
    data:
      message: "Regen erwartet und Fenster ist offen!"
      title: "Fenster schließen"
```

### 5. Multi-Sensor Netzwerk (Druckgradient)

**Konzept**: Mehrere BMP180 an verschiedenen Orten → Druckgradient berechnen

```cpp
// Station 1 (ESP32 #1): Wohnzimmer
// Station 2 (ESP32 #2): Garten
// Station 3 (ESP32 #3): Dachboden

struct RemoteSensor {
  String location;
  float pressure;
  float temperature;
  unsigned long lastUpdate;
};

RemoteSensor sensors[3] = {
  {"Wohnzimmer", 0, 0, 0},
  {"Garten", 0, 0, 0},
  {"Dachboden", 0, 0, 0}
};

// Via MQTT empfangen
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = String((char*)payload).substring(0, length);

  if (String(topic).indexOf("wohnzimmer") >= 0) {
    sensors[0].pressure = message.toFloat();
    sensors[0].lastUpdate = millis();
  } else if (String(topic).indexOf("garten") >= 0) {
    sensors[1].pressure = message.toFloat();
    sensors[1].lastUpdate = millis();
  }
  // ...
}

// Druckgradient berechnen
float calculatePressureGradient() {
  // Horizontaler Gradient (Wohnzimmer → Garten)
  float distance = 50.0;  // 50 Meter
  float pressureDiff = sensors[1].pressure - sensors[0].pressure;
  float gradient = pressureDiff / distance;  // hPa/m

  // Gradient > 0: Druck steigt in Richtung Garten
  // → Wind von Wohnzimmer zu Garten
  Serial.printf("Druckgradient: %.3f hPa/m\n", gradient);

  return gradient;
}
```

**Anwendungen**:
- Windrichtung abschätzen
- Lokale Wetterphänomene erkennen
- Präzisere Vorhersage

### 6. Wetteralarm-System (Buzzer + LED)

**Konzept**: Akustischer/Visueller Alarm bei Wetteränderung

```cpp
#define BUZZER_PIN 26
#define LED_RED 4
#define LED_GREEN 16

struct WeatherAlert {
  float pressureDropThreshold;  // hPa/3h
  uint16_t buzzerFreq;
  uint16_t buzzerDuration;
  String message;
};

WeatherAlert alerts[] = {
  {-3.0, 1000, 200, "Wetteränderung"},    // Moderate
  {-5.0, 2000, 500, "Starke Änderung"},   // Stark
  {-8.0, 3000, 1000, "STURM-WARNUNG!"}    // Extrem
};

void checkWeatherAlerts() {
  // Druckänderung über 3h
  int oldIndex = (trendIndex - TREND_SAMPLES + TREND_SAMPLES) % TREND_SAMPLES;
  float pressureChange = currentPressure - pressureTrend[oldIndex];

  // Prüfe Schwellenwerte
  for (int i = 2; i >= 0; i--) {  // Von höchster Priorität
    if (pressureChange <= alerts[i].pressureDropThreshold) {
      // Buzzer
      tone(BUZZER_PIN, alerts[i].buzzerFreq, alerts[i].buzzerDuration);

      // LED Blink
      for (int j = 0; j < 5; j++) {
        digitalWrite(LED_RED, HIGH);
        delay(100);
        digitalWrite(LED_RED, LOW);
        delay(100);
      }

      // Display-Warnung
      lcd.fillRect(0, 0, 320, 40, TFT_RED);
      lcd.setTextColor(TFT_WHITE);
      lcd.setTextSize(2);
      lcd.drawString(alerts[i].message, 160, 15);

      Serial.printf("ALARM: %s (%.2f hPa/3h)\n", alerts[i].message.c_str(), pressureChange);

      break;
    }
  }

  // Grüne LED wenn stabiles Wetter
  if (abs(pressureChange) < 1.0) {
    digitalWrite(LED_GREEN, HIGH);
  } else {
    digitalWrite(LED_GREEN, LOW);
  }
}
```

### 7. Historische Wetterdaten Viewer

**Konzept**: Auf SD gespeicherte Daten auf Display anzeigen

```cpp
#include <SD.h>

struct HistoricalData {
  time_t timestamp;
  float pressure;
  float temperature;
};

#define MAX_HISTORICAL 100
HistoricalData historicalData[MAX_HISTORICAL];
int historicalCount = 0;

void loadHistoricalData(time_t startDate, time_t endDate) {
  File logFile = SD.open("/weather_log.csv");
  if (!logFile) return;

  historicalCount = 0;

  // Header überspringen
  logFile.readStringUntil('\n');

  while (logFile.available() && historicalCount < MAX_HISTORICAL) {
    String line = logFile.readStringUntil('\n');

    // CSV parsen
    int comma1 = line.indexOf(',');
    int comma2 = line.indexOf(',', comma1 + 1);
    int comma3 = line.indexOf(',', comma2 + 1);

    time_t timestamp = line.substring(0, comma1).toInt();
    float pressure = line.substring(comma1 + 1, comma2).toFloat();
    float temperature = line.substring(comma2 + 1, comma3).toFloat();

    // Zeitraum filtern
    if (timestamp >= startDate && timestamp <= endDate) {
      historicalData[historicalCount].timestamp = timestamp;
      historicalData[historicalCount].pressure = pressure;
      historicalData[historicalCount].temperature = temperature;
      historicalCount++;
    }
  }

  logFile.close();
  Serial.printf("Geladen: %d Datenpunkte\n", historicalCount);
}

void drawHistoricalGraph() {
  // Ähnlich wie drawPressureGraph(), aber mit historicalData[]
  // ...
}

// Touch für Zeitraum-Auswahl
// "Letzte Woche", "Letzter Monat", "Benutzerdefiniert"
```

## 📚 Weiterführende Themen

### Meteorologie Basics

**Luftdruck und Wetter** - Warum korrelieren sie?

```
HOCHDRUCKGEBIET (Antizyklone):
- Luftmassen sinken ab
- Beim Absinken: Erwärmung (adiabatisch)
- Erwärmte Luft: Wolken lösen sich auf
- → Schönes, trockenes Wetter

TIEFDRUCKGEBIET (Zyklone):
- Luftmassen steigen auf
- Beim Aufsteigen: Abkühlung (adiabatisch)
- Abgekühlte Luft: Wasserdampf kondensiert
- → Wolken, Regen, Wind

DRUCKGRADIENT:
- Große Druckunterschiede → starker Wind
- Gradient = ΔP / Δx
- Coriolis-Kraft lenkt Wind ab (Nordhalbkugel: rechts)
```

**Adiabatische Prozesse**:
```
Trockenadiabatischer Gradient: -10°C / 1000m
Feuchtadiabatischer Gradient: -6°C / 1000m

Beispiel:
Luft steigt von 0m auf 2000m:
T₀ = 20°C (am Boden)
T₂₀₀₀ = 20 - (2 × 10) = 0°C (oben)

→ Kälter → Kondensation → Wolken → Regen
```

### Druckmessung in der Geschichte

**Evangelista Torricelli (1643)**:
- Erfinder des Barometers
- Quecksilbersäule: 760 mm bei Normaldruck
- "Torr" Einheit benannt nach ihm

**Blaise Pascal (1648)**:
- Experiment auf Puy-de-Dôme (1465m)
- Beweis: Luftdruck nimmt mit Höhe ab
- "Pascal" Einheit benannt nach ihm

**Moderne Sensoren**:
- 1960er: Erste piezoresistive Sensoren
- 1980er: MEMS-Technologie
- 2000er: Bosch BMP-Serie (BMP085, BMP180, BMP280, etc.)

### Barometer-Typen

| Typ | Prinzip | Genauigkeit | Verwendung |
|-----|---------|-------------|------------|
| **Quecksilber** | Hg-Säule | ±0,1 hPa | Labor (veraltet) |
| **Aneroid** | Metallkapsel | ±2 hPa | Analog-Barometer |
| **Piezoelektrisch** | Kristall | ±1 hPa | Drucktransmitter |
| **Piezoresistiv (BMP180)** | MEMS | ±1 hPa | Consumer |
| **Kapazitiv** | Kondensator | ±0,01 hPa | Präzisionsmessung |

### Höhenmessung in der Luftfahrt

**Flight Levels**:
```
In der Luftfahrt: Höhe in "Flight Levels" (FL)

FL = Höhe in Fuß / 100

Beispiel:
FL330 = 33.000 Fuß ≈ 10.000 m

Referenzdruck:
- QNH: Meereshöhe (für Flughäfen)
- QNE: Standard (1013,25 hPa, für Reiseflug)
- QFE: Flughafenhöhe
```

**Höhenmesser-Einstellung**:
```
Pilot erhält QNH vom Tower (z.B. 1020 hPa)
→ Stellt Höhenmesser auf 1020 hPa
→ Zeigt Höhe über Meeresspiegel korrekt

Für Reiseflug (> FL180):
→ Alle Flugzeuge auf QNE (1013,25 hPa)
→ Gleiches Referenzniveau für alle
```

## 🛠️ Praktische Tipps

### Optimale Sensor-Platzierung

```
OPTIMAL:
✅ Innenraum (geschützt vor direktem Wetter)
✅ Auf normaler Raumhöhe (nicht Keller/Dachboden)
✅ Nicht in direkter Sonneneinstrahlung
✅ Nicht neben Heizung/Klimaanlage
✅ Stabile Umgebung (keine Zugluft)

SCHLECHT:
❌ Direkt am Fenster (Temperatur-Gradient)
❌ In der Küche (Dampf, Temperaturschwankungen)
❌ Im Badezimmer (Feuchtigkeit)
❌ Neben PC/Server (Abwärme)
```

### Kalibrierung

Der BMP180 ist **werks-kalibriert** und benötigt normalerweise keine Nachkalibrierung.

**Aber**:
- Seehöhen-Referenzdruck anpassen (siehe Erweiterungsidee 1)
- Temperatur-Offset möglich (falls systematischer Fehler)

```cpp
// Temperatur-Offset (falls nötig)
#define TEMP_OFFSET -1.5  // °C

float temp = bmp.readTemperature() + TEMP_OFFSET;
```

### Langzeit-Genauigkeit

**Drift**:
- BMP180: < ±1 hPa / Jahr (typisch)
- Nach 5-10 Jahren: Eventuell merkbare Drift

**Lösung**:
- Regelmäßiger Vergleich mit Referenz (Wetterdienst)
- Bei > 5 hPa Abweichung: Sensor ersetzen

### Stromverbrauch optimieren

```cpp
// Power-Saving für Batterie-Betrieb

// 1. Messintervall vergrößern
#define MEASUREMENT_INTERVAL 60000  // 60s statt 2s

// 2. Display dimmen/ausschalten
void enterLowPowerMode() {
  lcd.setBrightness(0);  // Display aus
  lcd.sleep();           // Display Sleep

  // ESP32 in Light Sleep
  esp_sleep_enable_timer_wakeup(60000000);  // 60s
  esp_light_sleep_start();

  // Nach Aufwachen
  lcd.wakeup();
  lcd.setBrightness(128);
}

// 3. BMP180 zwischen Messungen in Standby
// (automatisch in Library)
```

**Stromverbrauch**:
```
Normal-Betrieb:
- ESP32: 80 mA
- Display: 50 mA (mit Hintergrundlicht)
- BMP180: 0,01 mA (Standby)
= ca. 130 mA gesamt

Low-Power-Modus:
- ESP32 (Light Sleep): 0,8 mA
- Display (aus): 0 mA
- BMP180: 0,0001 mA
= ca. 1 mA gesamt

→ Mit 2000 mAh Akku: ~2000h Laufzeit (83 Tage!)
```

## 📖 Ressourcen & Links

### Datenblätter & Dokumentation
- **BMP180 Datasheet**: [Bosch Sensortec](https://www.bosch-sensortec.com/products/environmental-sensors/pressure-sensors/bmp180/)
- **Adafruit BMP085 Library**: [GitHub](https://github.com/adafruit/Adafruit-BMP085-Library)
- **Barometrische Höhenformel**: [Wikipedia](https://de.wikipedia.org/wiki/Barometrische_H%C3%B6henformel)

### Meteorologie
- **Luftdruck und Wetter**: [DWD Wetterlexikon](https://www.dwd.de/DE/service/lexikon/lexikon_node.html)
- **Internationale Standardatmosphäre**: [ISA](https://de.wikipedia.org/wiki/Normatmosph%C3%A4re)

### CYD-Spezifisch
- **CYD Display Config**: `lib/CYD_Display_Config/CYD_Display_Config.h`
- **I2C Pins**: extSDA (GPIO 22), extSCL (GPIO 27)

### Verwandte Beispiele
- **Beispiel 18**: Compass & Orientation (9-DoF Sensor Fusion)
- **Beispiel 19**: Laser Distance (ToF Sensor)
- **Beispiel 20**: Air Quality (MOX Gas Sensor)
- **Beispiel 21**: Gesture Sensor (3D E-Field)

### Arduino Libraries
```cpp
// Erforderliche Libraries:
#include <Wire.h>              // I2C (ESP32 Core)
#include <Adafruit_BMP085.h>   // BMP180 Sensor
#include <CYD_Display_Config.h> // CYD Display
```

**Installation**:
```
Arduino IDE -> Tools -> Manage Libraries
- "Adafruit BMP085 Unified"
- "Adafruit Unified Sensor"
- "Adafruit Bus IO"
```

## 🎓 Lernziele

Nach Abschluss dieses Beispiels solltest du:

- ✅ **Barometrische Druckmessung** verstehen (piezoresistiver Sensor, MEMS)
- ✅ **Höhenberechnung** aus Luftdruck durchführen (barometrische Höhenformel)
- ✅ **Wettervorhersage** aus Drucktrend erstellen
- ✅ **Luftdruck und Wetter** Zusammenhang kennen (Hoch-/Tiefdruck)
- ✅ **Druckeinheiten** umrechnen können (hPa, mmHg, inHg, etc.)
- ✅ **Adafruit Library** verwenden (BMP085 für BMP180)
- ✅ **24h Historie** mit Ring-Buffer implementieren
- ✅ **Wetter-Icons** grafisch darstellen
- ✅ **Seehöhen-Kalibrierung** verstehen und anwenden
- ✅ **Meteorologie Basics** kennen (adiabatische Prozesse, etc.)

## 🔜 Nächste Schritte

Weitere Sensor-Beispiele:
- **Beispiel 23**: Multi-Sensor Umweltstation BME680 (Temp, Druck, Feuchte, Gas)
- **Beispiel 24**: Farbsensor TCS34725 (RGB-Erkennung)
- **Beispiel 25**: MEMS-Mikrofon mit FFT-Spektrum

---

**Viel Erfolg mit deiner Wetterstation! ☀️🌧️⛈️**

Bei Fragen: CYD-Academy GitHub Issues
