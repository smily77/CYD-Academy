# ⚡ Beispiel 17: Lightning Sensor - Professionelle Blitzwetter-Anzeige

## 📝 Was macht dieses Beispiel?

Eine **professionelle Blitzwetter-Anzeige** mit Radar-Visualisierung und umfangreicher Datenanalyse!

**Features:**
- ✅ **AS3935 Lightning Sensor** - Erkennt Blitze bis 40km Entfernung
- ✅ **Radar-Visualisierung** - Konzentrische Ringe für Entfernungsstufen
- ✅ **Blitz-Animation** - Visueller Effekt bei Blitz-Erkennung
- ✅ **Entfernungsmessung** - Distanz zum Blitz in km
- ✅ **Gefahren-Klassifizierung** - Farbcodierte Warnstufen
- ✅ **Historische Anzeige** - Letzte 20 Blitze mit Fade-out
- ✅ **Energie-Messung** - Blitz-Intensität
- ✅ **Störungs-Erkennung** - Unterscheidet Blitze von Störquellen
- ✅ **Statistiken** - Zähler, Zeit seit letztem Blitz
- ✅ **Einstellbar** - Anpassbare Empfindlichkeit

**Steuerung:**
- **Touch oben:** Statistik zurücksetzen
- **Touch unten:** Sensitivität ändern (0-7)
- **Automatisch:** Blitze werden automatisch erkannt und angezeigt

---

## 🎯 Lernziele

1. **I2C-Kommunikation** - Komplexer Sensor über I2C
2. **AS3935 Sensor** - Lightning Detection IC verstehen
3. **Interrupt-Handling** - Asynchrone Event-Verarbeitung
4. **Radar-Visualisierung** - Entfernungs-basierte Darstellung
5. **Animationen** - Dynamische Effekte & Feedback
6. **Daten-Historie** - Zeitbasierte Visualisierung
7. **Klassifizierung** - Gefahrenstufen-Logik
8. **Sensor-Kalibrierung** - Indoor/Outdoor Anpassung

---

## ⚙️ Installation & Setup

### 1. Library installieren

**Arduino IDE:**
1. Library Manager öffnen (Strg+Shift+I)
2. "DFRobot AS3935" suchen
3. Installieren

**PlatformIO:**
```ini
[env:esp32]
lib_deps =
  lovyan03/LovyanGFX @ ^1.1.0
  dfrobot/DFRobot_AS3935_I2C @ ^1.0.2
```

### 2. Hardware anschließen

**SEN0290 (AS3935 Lightning Sensor):**

```
SEN0290          CYD
├─ VCC    →    3.3V
├─ GND    →    GND
├─ SDA    →    GPIO 22 (extSDA)
├─ SCL    →    GPIO 27 (extSCL)
└─ IRQ    →    GPIO 35
```

**ACHTUNG:** Sensor ist **3.3V**!

**I2C Pins (CYD Standard):**
- `extSDA = GPIO 22`
- `extSCL = GPIO 27`

**Interrupt Pin:**
- `IRQ_PIN = GPIO 35` (Input-only Pin, ideal für Interrupts)

### 3. I2C-Adresse

**Standard:** `0x03`

**Alternative:** `0x02` oder `0x01` (über Adress-Jumper am Modul)

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

### 4. Kompilieren & Upload

```
Arduino IDE: Upload (Strg+U)
PlatformIO: pio run -t upload
```

### 5. Testen

**Wichtig:** Der Sensor erkennt **echte Blitze** (Gewitter).

Zum Testen ohne Gewitter:

1. **Elektro-Feuerzeug** - Funken erzeugen (1-2m Entfernung)
2. **Bohrmaschine** - Motor-Funken
3. **Leuchtstoffröhre** - Ein-/Ausschalten
4. **Gewitter abwarten** 😊

**Störquellen vermeiden:**
- WiFi-Router (zu nah)
- Mikrowelle
- Elektromotoren
- Schaltnetzteile

---

## 🔍 Technische Konzepte

### 1. AS3935 Lightning Sensor

**Was ist der AS3935?**

Der **AS3935** ist ein spezialisierter IC zur Blitz-Erkennung von AMS (Austria Microsystems).

#### Funktionsweise

```
Blitz-Entladung
      ↓
Elektromagnetisches Feld (EM)
      ↓
Antenne im AS3935
      ↓
Signal-Analyse (DSP)
      ↓
Pattern-Erkennung
      ↓
Entfernungs-Schätzung
      ↓
Interrupt (IRQ)
```

**Erkennungs-Algorithmus:**

Der AS3935 analysiert das **elektromagnetische Signal** und vergleicht es mit typischen Blitz-Mustern:

```
Blitz-Signal:
  ─╮     ╭─╮
    ╰─────╯ ╰──  (charakteristische Form)

Störung (z.B. Motor):
  ╭╮╭╮╭╮╭╮╭╮
  ╰╯╰╯╰╯╰╯╰╯  (gleichmäßiges Rauschen)
```

**Entfernungs-Berechnung:**

```cpp
// Basiert auf Signal-Stärke
// Stärkeres Signal = näher
// Reichweite: 1 - 40 km

uint8_t distance = sensor.getLightningDistKm();

// Mögliche Werte:
// 1, 5, 6, 8, 10, 12, 14, 17, 20, 24, 27, 31, 34, 37, 40 (km)
// Oder: 0 = "Overhead" (direkt über Sensor, < 1km!)
```

**Energie-Messung:**

```cpp
uint32_t energy = sensor.getStrikeEnergyRaw();

// Dimensionslose Einheit (0 - 0x1FFFFF)
// Höher = stärkerer Blitz
// Relative Vergleiche möglich
```

---

### 2. I2C-Kommunikation

#### AS3935 Register

Der AS3935 hat mehrere interne Register für Konfiguration:

```cpp
// Beispiel: Noise Floor Level setzen
// Register 0x01, Bits [6:4]

sensor.setNoiseFloorLvl(2);

// Intern:
// 1. I2C-Write zu Register 0x01
// 2. Bits [6:4] auf 0b010 setzen (Wert 2)
// 3. Andere Bits unverändert lassen (Read-Modify-Write)
```

**Wichtige Register:**

| Register | Funktion |
|----------|----------|
| 0x00 | Power Down, Indoor/Outdoor |
| 0x01 | Noise Floor Level, Watchdog |
| 0x02 | Spike Rejection, Disturber |
| 0x03 | Lightning Interrupt, Distance |
| 0x04-0x05 | Energy (MSB/LSB) |
| 0x08 | Calibration |

---

### 3. Interrupt-Handling

**Warum Interrupts?**

```
Polling (schlecht):
  loop() {
    if (sensor.hasLightning()) { ... }  // Prüft ständig
    // Problem: Verschwendet CPU, könnte Event verpassen
  }

Interrupt (gut):
  ISR() {
    lightningDetected = true;  // Sofortige Reaktion
  }
  loop() {
    if (lightningDetected) { ... }
  }
```

**AS3935 Interrupt-Pin (IRQ):**

```
Normal: LOW (0V)
Event:  HIGH (3.3V) für ~2ms

ESP32 Interrupt:
  pinMode(IRQ_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), ISR, RISING);
```

**ISR (Interrupt Service Routine):**

```cpp
void IRAM_ATTR lightningISR() {
  // WICHTIG: Minimaler Code!
  // Keine Serial.print, lcd.print, delay!

  interruptDetected = true;  // Flag setzen
}
```

**Warum `IRAM_ATTR`?**

```
IRAM = Internal RAM (schneller Speicher)

Ohne IRAM_ATTR:
  - ISR liegt im Flash (langsam)
  - Bei Flash-Zugriff (z.B. WiFi) → ISR blockiert!

Mit IRAM_ATTR:
  - ISR liegt im RAM (schnell)
  - Immer verfügbar, auch während Flash-Zugriff
```

**Event-Verarbeitung:**

```cpp
void loop() {
  if (interruptDetected) {
    interruptDetected = false;

    delay(2);  // Datasheet: 2ms warten

    uint8_t intSource = sensor.getInterruptSrc();

    switch (intSource) {
      case 1:  // Disturber (Störung)
        break;
      case 2:  // Noise (Rauschen)
        break;
      case 3:  // Lightning!
        handleLightning();
        break;
    }
  }
}
```

---

### 4. Radar-Visualisierung

#### Konzept

```
Radar-Display:
  - Zentrum = Eigener Standort
  - Konzentrische Kreise = Entfernungsstufen (10, 20, 30, 40 km)
  - Blitz-Symbole = Erkannte Blitze

  40km ─╮
  30km ──┤
  20km ───┤     ⚡  ← Blitz bei ~25km
  10km ────┤
        ───┼─── Standort
```

#### Mathematik

**Entfernung zu Radius:**

```cpp
int radius = map(distance, 0, 40, 0, RADAR_MAX_RADIUS);

// Beispiel:
// distance = 20 km
// RADAR_MAX_RADIUS = 90 px
// → radius = 45 px (Mitte zwischen Zentrum und Rand)
```

**Winkel für Visualisierung:**

```cpp
// Problem: Sensor gibt keine Richtung!
// Lösung: Zufälliger Winkel basierend auf Zeit

float angle = (millis() % 360) * 0.01745;  // zu Radiant (PI/180)

// Kartesische Koordinaten:
int x = RADAR_CENTER_X + radius * cos(angle);
int y = RADAR_CENTER_Y + radius * sin(angle);

lcd.fillCircle(x, y, 5, color);
```

**Warum `millis() % 360`?**

```
Pseudo-Zufall für visuelle Verteilung:
  - Jeder Blitz bekommt scheinbar andere Richtung
  - Basiert auf Erkennungszeit
  - Nicht echt, aber optisch ansprechend!

Echt wäre:
  - Zweiter Sensor (Triangulation)
  - Oder: Lightning-Netzwerk (mehrere Standorte)
```

---

### 5. Historische Daten & Fade-out

#### Ring-Buffer

```cpp
const int MAX_HISTORY = 20;
LightningEvent history[MAX_HISTORY];
int historyIndex = 0;

void addToHistory(uint8_t distance, uint32_t energy) {
  history[historyIndex].distance = distance;
  history[historyIndex].time = millis();
  history[historyIndex].energy = energy;

  historyIndex = (historyIndex + 1) % MAX_HISTORY;  // Ring!
}
```

**Ring-Buffer Prinzip:**

```
Index:  0   1   2   3   4  (MAX_HISTORY = 5)
Data: [A] [B] [C] [D] [E]
        ↑
    historyIndex = 0

Nach neuem Event (F):
Data: [F] [B] [C] [D] [E]
            ↑
    historyIndex = 1

Ältestes Element (B) wird überschrieben!
```

#### Fade-out

```cpp
unsigned long age = now - event.time;  // in ms

// Alpha-Wert (0 = unsichtbar, 255 = voll sichtbar)
uint8_t alpha = 255 - (age * 255 / 300000);  // 300000ms = 5min

if (age > 300000) continue;  // Zu alt, nicht anzeigen

// Farbe dimmen
if (alpha < 128) {
  color = COLOR_TEXT_DIM;  // Dunkler
}
```

**Visualisierung:**

```
Zeit:     0s     60s    120s   180s   240s   300s
Alpha:   255    212    170    127     85      0
Farbe:  ████   ████   ███    ██      █       (invisible)
```

---

### 6. Gefahren-Klassifizierung

```cpp
uint16_t getDistanceColor(uint8_t distance) {
  if (distance == 0) return COLOR_CRITICAL;   // Overhead!
  if (distance <= 10) return COLOR_CRITICAL;  // 0-10km: ROT
  if (distance <= 20) return COLOR_DANGER;    // 10-20km: ORANGE
  if (distance <= 30) return COLOR_WARNING;   // 20-30km: GELB
  return COLOR_SAFE;                          // 30-40km: GRÜN
}
```

**Sicherheits-Empfehlungen:**

| Entfernung | Farbe | Status | Empfehlung |
|------------|-------|--------|------------|
| 0 km (Overhead) | 🔴 Rot | **KRITISCH** | Sofort Schutz suchen! |
| 1-10 km | 🔴 Rot | **KRITISCH** | Gebäude/Auto aufsuchen |
| 10-20 km | 🟠 Orange | **GEFAHR** | Aktivitäten beenden |
| 20-30 km | 🟡 Gelb | **WARNUNG** | Wetter beobachten |
| 30-40 km | 🟢 Grün | **VORSICHT** | Gewitter nähert sich |

**30-30 Regel:**

```
Wenn Blitz-Donner < 30 Sekunden:
  → Blitz ist < 10 km entfernt
  → Gefahr!

Warte 30 Minuten nach letztem Blitz:
  → Dann erst wieder raus
```

---

### 7. Sensor-Kalibrierung

#### Indoor vs. Outdoor

```cpp
sensor.setIndoors();   // Indoor-Modus
// oder
sensor.setOutdoors();  // Outdoor-Modus
```

**Unterschied:**

```
Indoor:
  - Mehr Störquellen (Elektronik)
  - Höherer Noise Threshold
  - Konservativere Erkennung

Outdoor:
  - Weniger Störungen
  - Niedrigerer Threshold
  - Empfindlichere Erkennung
```

#### Noise Floor Level

```cpp
sensor.setNoiseFloorLvl(2);  // 0 (niedrig) - 7 (hoch)
```

**Was ist der Noise Floor?**

```
Signal-Amplitude
      ↑
      │         Blitz!
      │          /\
      │         /  \
      ├────────/────\──────  ← Detection Threshold
      │   ~~~~/~~~~~~\~~~~  ← Noise Floor
      │   ~~~~~~~~~~~~
      └──────────────────→ Zeit

Noise Floor = Basis-Rausch-Level
  - Zu niedrig: Viele Fehlalarme
  - Zu hoch: Blitze werden verpasst
```

#### Watchdog Threshold (Sensitivität)

```cpp
sensor.setWatchdogThreshold(2);  // 0 (niedrig) - 7 (hoch)
```

**Empfindlichkeit:**

```
Threshold:   0   1   2   3   4   5   6   7
Sensitivity: ██████████████████████████  (max)
             ↓
             █████████  (min)

Höher = Empfindlicher = Mehr Erkennungen (+ mehr Fehlalarme)
Niedriger = Weniger empfindlich = Nur starke Blitze
```

#### Spike Rejection

```cpp
sensor.setSpikeRejection(2);  // 0 - 15
```

**Funktion:**

Filtert kurze Störimpulse (< 2µs) heraus.

```
Spike (Störung):
  ─╮╭─  < 2µs
   ╰╯

Lightning:
  ─╮   ╭─  > 2µs
   ╰───╯
```

#### Manual Calibration

```cpp
sensor.manualCal(96, 64, 64);  // Tuning Caps: LCO, SRCO, TRCO
```

**Was wird kalibriert?**

```
Interne Oszillatoren:
  - LCO = LC Oscillator (500 kHz)
  - SRCO = SRCO Oscillator (1.1 MHz)
  - TRCO = Timer RC Oscillator (32 kHz)

Werte (0-127) tunen die Frequenzen.
Werkseinstellungen meist OK (96, 64, 64).
```

---

## 🚀 Experimente & Erweiterungen

### 1. 🌐 Blitz-Netzwerk (Multi-Sensor)

**Idee:** Mehrere CYDs mit Lightning Sensor → Triangulation für echte Richtung!

```cpp
// Sensor 1 (Standort A)
uint8_t distanceA = sensor.getLightningDistKm();
unsigned long timeA = millis();

// Sensor 2 (Standort B) - via ESP-NOW
uint8_t distanceB = receivedData.distance;
unsigned long timeB = receivedData.time;

// Sensor 3 (Standort C)
uint8_t distanceC = receivedData2.distance;

// Triangulation (vereinfacht)
// Mit 3 bekannten Standorten (A, B, C) und Entfernungen:
// → Schnittpunkt der Kreise = Blitz-Position!

float blitzX, blitzY;
triangulate(distanceA, distanceB, distanceC,
            &blitzX, &blitzY);

Serial.printf("Blitz-Position: %.2f, %.2f\n", blitzX, blitzY);
```

**Triangulation-Mathematik:**

```
     A (0, 0)
      ●────────○ distanceA
      │       ╱
      │      ╱
      │     ╱
      │    ╱  ⚡ Lightning
      │   ●
      │  ╱ distanceB
      │ ╱
      │╱
      B

Mit 3 Sensoren:
  (x - Ax)² + (y - Ay)² = distanceA²
  (x - Bx)² + (y - By)² = distanceB²
  (x - Cx)² + (y - Cy)² = distanceC²

→ Löse Gleichungssystem für (x, y)
```

---

### 2. 📊 Langzeit-Statistik (SD-Karte)

**Idee:** Blitz-Daten auf SD-Karte speichern für Analyse.

```cpp
#include <SD.h>
#include <SPI.h>

void saveLightningToSD(uint8_t distance, uint32_t energy) {
  File file = SD.open("/lightning_log.csv", FILE_APPEND);

  if (file) {
    // Timestamp (Unix-Zeit via NTP)
    unsigned long timestamp = getUnixTime();

    // CSV-Format: timestamp,distance,energy
    file.printf("%lu,%d,%lu\n", timestamp, distance, energy);

    file.close();

    Serial.println("Saved to SD");
  }
}

// Später: CSV in Excel/Python analysieren
// → Blitz-Häufigkeit pro Tag
// → Durchschnitts-Entfernung
// → Stärkste Blitze
```

**Analyse-Beispiele:**

```python
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('lightning_log.csv',
                 names=['timestamp', 'distance', 'energy'])

df['date'] = pd.to_datetime(df['timestamp'], unit='s')

# Blitze pro Tag
daily = df.groupby(df['date'].dt.date).size()
daily.plot(kind='bar')
plt.title('Lightning Strikes per Day')

# Entfernungs-Verteilung
df['distance'].hist(bins=20)
plt.title('Distance Distribution')

# Energie-Verteilung
df['energy'].hist(bins=30, log=True)
plt.title('Energy Distribution (log scale)')
```

---

### 3. 🔊 Audio-Warnung

**Idee:** Ton-Ausgabe bei kritischen Blitzen.

```cpp
#define BUZZER_PIN 26

void setup() {
  ledcSetup(0, 5000, 8);
  ledcAttachPin(BUZZER_PIN, 0);
}

void playWarningSound() {
  // Sirenen-Ton (auf/ab)
  for (int i = 0; i < 3; i++) {
    for (int freq = 400; freq < 1200; freq += 50) {
      ledcWriteTone(0, freq);
      delay(10);
    }
    for (int freq = 1200; freq > 400; freq -= 50) {
      ledcWriteTone(0, freq);
      delay(10);
    }
  }
  ledcWriteTone(0, 0);  // Aus
}

void handleLightning() {
  if (lastDistance <= 10) {
    // Kritisch!
    playWarningSound();
  }
}
```

---

### 4. 🌡️ Wetterstation-Integration

**Idee:** Kombiniere mit anderen Sensoren (BME280, Windmesser).

```cpp
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;

void readWeatherData() {
  float temp = bme.readTemperature();      // °C
  float humidity = bme.readHumidity();     // %
  float pressure = bme.readPressure() / 100.0;  // hPa

  Serial.printf("Temp: %.1f°C, Humid: %.1f%%, Press: %.1f hPa\n",
                temp, humidity, pressure);

  // Gewitter-Wahrscheinlichkeit?
  if (pressure < 1000 && humidity > 70) {
    Serial.println("Conditions favor thunderstorms!");
  }
}

// Korrelation:
// Blitze + Luftdruck-Abfall → Sturmfront!
```

---

### 5. 🗺️ GPS-Integration (Standort-Logging)

**Idee:** GPS-Koordinaten zu jedem Blitz speichern.

```cpp
#include <TinyGPS++.h>

TinyGPSPlus gps;
HardwareSerial gpsSerial(1);  // UART1

void setup() {
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);  // RX, TX
}

void saveLightningWithGPS(uint8_t distance) {
  if (gps.location.isValid()) {
    float lat = gps.location.lat();
    float lng = gps.location.lng();

    Serial.printf("Lightning at %.6f, %.6f - Distance: %d km\n",
                  lat, lng, distance);

    // Speichern auf SD:
    // timestamp,lat,lng,distance,energy
    // → Heatmap auf Google Maps!
  }
}
```

---

### 6. 🌩️ Blitz-Vorhersage (ML)

**Idee:** Machine Learning für Blitz-Vorhersage.

```cpp
// Sammle Daten:
// - Luftdruck (Trend: steigend/fallend)
// - Temperatur
// - Luftfeuchtigkeit
// - Blitz-Aktivität (letzte Stunde)

// Trainiere Modell (z.B. TensorFlow Lite):
// Input: [pressure, temp, humidity, lightning_count_1h]
// Output: [probability_lightning_next_30min]

#include <TensorFlowLite_ESP32.h>

float predictLightning() {
  float input[] = {pressure, temp, humidity, lightningCount1h};

  // Inference
  interpreter->Invoke();

  float* output = interpreter->output(0)->data.f;
  return output[0];  // 0.0 - 1.0 (Wahrscheinlichkeit)
}
```

---

### 7. 📱 Web-Dashboard (WiFi)

**Idee:** Echtzeit-Daten über WiFi zugänglich machen.

```cpp
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

void setup() {
  WiFi.begin(ssid, password);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{";
    json += "\"distance\":" + String(lastDistance) + ",";
    json += "\"count\":" + String(totalLightningCount) + ",";
    json += "\"energy\":" + String(lastEnergy) + ",";
    json += "\"time\":" + String(millis() - lastLightningTime);
    json += "}";

    request->send(200, "application/json", json);
  });

  server.begin();
}

// Web-App:
// fetch('http://cyd.local/').then(r => r.json()).then(data => {
//   document.getElementById('distance').innerText = data.distance + ' km';
// });
```

---

## 🐛 Troubleshooting

### Problem: "AS3935 not found!"

**Symptom:** Sensor wird nicht erkannt.

**Ursachen:**
1. Falsche Verkabelung
2. Falsche I2C-Adresse
3. Defektes Modul
4. SDA/SCL vertauscht

**Debug:**

```cpp
void scanI2C() {
  Wire.begin(extSDA, extSCL);
  Serial.println("Scanning...");

  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("Found: 0x%02X\n", addr);
    }
  }
}

// Erwartete Adresse: 0x03 (oder 0x02, 0x01)
```

**Lösung:**
- Überprüfe VCC (3.3V), GND, SDA (GPIO 22), SCL (GPIO 27)
- Teste mit anderem Modul
- Überprüfe I2C-Adresse (Jumper am Modul?)

---

### Problem: Keine Interrupts

**Symptom:** `lightningISR()` wird nie aufgerufen.

**Ursachen:**
1. IRQ-Pin nicht verbunden
2. Falscher Pin (nicht interrupt-fähig)
3. Interrupt nicht aktiviert

**Debug:**

```cpp
void loop() {
  Serial.printf("IRQ Pin State: %d\n", digitalRead(IRQ_PIN));
  delay(1000);
}

// Sollte normalerweise 0 (LOW) sein
// Bei Event kurz 1 (HIGH)
```

**Lösung:**
- Verbinde IRQ mit GPIO 35
- Verwende nur interrupt-fähige Pins (0-39)
- Überprüfe `attachInterrupt()` Aufruf

---

### Problem: Zu viele Fehlalarme

**Symptom:** Ständig "Disturber" oder "Noise" Meldungen.

**Ursachen:**
1. Störquellen in der Nähe (WiFi, Motoren)
2. Noise Floor zu niedrig
3. Indoor/Outdoor falsch

**Lösung:**

```cpp
// Höherer Noise Floor
sensor.setNoiseFloorLvl(4);  // War: 2 → Jetzt: 4

// Weniger empfindlich
sensor.setWatchdogThreshold(1);  // War: 2 → Jetzt: 1

// Störungen ignorieren
sensor.disturberDis();  // Deaktiviert Disturber-Meldungen

// Spike Rejection erhöhen
sensor.setSpikeRejection(3);  // War: 2 → Jetzt: 3
```

---

### Problem: Blitze werden nicht erkannt

**Symptom:** Gewitter draußen, aber Sensor zeigt nichts.

**Ursachen:**
1. Zu hoher Noise Floor
2. Zu unempfindlich
3. Indoor-Modus bei Outdoor-Nutzung
4. Antenne zu klein/schlecht

**Lösung:**

```cpp
// Niedrigerer Noise Floor
sensor.setNoiseFloorLvl(1);

// Empfindlicher
sensor.setWatchdogThreshold(4);

// Outdoor-Modus
sensor.setOutdoors();
```

**Antenna-Verbesserung:**

```
Standard: PCB-Antenne (klein)

Verbessert: Draht-Antenne (20-30cm)
  - Verbinde mit AS3935 Antenna Pin
  - Besser für größere Reichweite
```

---

### Problem: Entfernungen ungenau

**Symptom:** Angezeigte Distanz passt nicht zu realem Gewitter.

**Ursachen:**
1. Sensor schätzt nur (kein GPS)
2. Indirekter Blitz (Reflexion)
3. Kalibrierung falsch

**Realität:**

```
AS3935 Genauigkeit:
  ±5 km (typisch)
  Nur grobe Schätzung!

Für präzise Daten:
  → Triangulation (3+ Sensoren)
  → Professionelle Lightning-Netzwerke (blitzortung.org)
```

**Tipp:**

Nutze AS3935 für **Trend** (nähert sich / entfernt sich), nicht exakte Distanz!

---

### Problem: "Overhead" (distance = 0) immer angezeigt

**Symptom:** Alle Blitze zeigen 0 km.

**Ursachen:**
1. Sehr nahe Störquelle
2. Register-Fehler

**Debug:**

```cpp
void loop() {
  if (interruptDetected) {
    delay(2);

    uint8_t intSrc = sensor.getInterruptSrc();
    uint8_t dist = sensor.getLightningDistKm();

    Serial.printf("IntSrc: %d, Distance Register: %d\n", intSrc, dist);

    // Wenn dist immer 0:
    // → Prüfe I2C-Kommunikation
    // → Reset Sensor
  }
}
```

**Lösung:**

```cpp
// Sensor komplett zurücksetzen
sensor.reset();
delay(100);

// Neu konfigurieren
sensor.manualCal(96, 64, 64);
sensor.setIndoors();
// ...
```

---

## 📚 Weiterführende Konzepte

### 1. Time-of-Arrival (TOA) Triangulation

**Professionelle Lightning-Detection:**

```cpp
// Mit mehreren Sensoren:
// - Messe Zeit des Blitz-Signals
// - Lichtgeschwindigkeit c ≈ 300,000 km/s
// - Zeitunterschied → Entfernungs-Unterschied

float timeA = 0.000123;  // Sekunden
float timeB = 0.000156;
float timeC = 0.000098;

// Hyperbolisches Gleichungssystem:
// Schnittpunkte der Hyperbeln = Blitz-Position
```

**Netzwerke wie blitzortung.org nutzen dies!**

---

### 2. Electromagnetic Field (EMF) Theorie

**Blitz-Signal:**

```
Blitz-Kanal:
  - Plasma-Kanal (ionisierte Luft)
  - Strom: 10,000 - 200,000 Ampere!
  - Dauer: ~0.2 ms

Elektromagnetisches Feld:
  - VLF (Very Low Frequency): 3-30 kHz
  - LF (Low Frequency): 30-300 kHz
  - Reichweite: 100+ km

AS3935 empfängt:
  - LF-Band (~500 kHz)
  - Analysiert Puls-Form
```

---

### 3. Schumann-Resonanzen

**Globale EM-Resonanzen:**

```
Erde + Ionosphäre = Resonator

Frequenzen:
  - 7.83 Hz (fundamental)
  - 14.3 Hz
  - 20.8 Hz
  - ...

Ursache: Gewitter weltweit!
  → ~50 Blitze/Sekunde global
  → Ständige Anregung der Resonanzen

AS3935 filtert diese aus (zu niederfrequent).
```

---

### 4. Lightning-Strike Density Maps

**Heatmap-Erstellung:**

```python
import folium
from folium.plugins import HeatMap

# Lade GPS + Lightning Daten
data = [[lat, lng, intensity], ...]

# Erstelle Karte
m = folium.Map(location=[50.0, 10.0], zoom_start=6)

HeatMap(data).add_to(m)

m.save('lightning_heatmap.html')
```

---

## 🎓 Was hast du gelernt?

Nach diesem Beispiel verstehst du:

- ✅ **AS3935 Lightning Sensor** - Funktionsweise & Konfiguration
- ✅ **I2C-Kommunikation** - Komplexe Register-Operationen
- ✅ **Interrupt-Handling** - Asynchrone Event-Verarbeitung
- ✅ **Radar-Visualisierung** - Entfernungs-basierte Darstellung
- ✅ **Daten-Historie** - Ring-Buffer & Fade-out
- ✅ **Klassifizierung** - Gefahrenstufen-Logik
- ✅ **Animationen** - Dynamische UI-Effekte
- ✅ **Sensor-Kalibrierung** - Indoor/Outdoor, Noise, Sensitivity

**Nächste Schritte:**
1. Experimentiere mit Empfindlichkeit
2. Baue Blitz-Netzwerk (Multi-Sensor)
3. Füge Wetterstation-Daten hinzu
4. Erstelle Langzeit-Statistiken (SD-Karte)

---

## 🔗 Siehe auch

- [AS3935 Datasheet](https://www.mouser.com/datasheet/2/588/ams_AS3935_Datasheet_EN_v5-1214568.pdf)
- [DFRobot SEN0290 Wiki](https://wiki.dfrobot.com/Gravity:%20Lightning%20Sensor%20SKU:%20SEN0290)
- [Lightning Physics](https://www.weather.gov/safety/lightning-science)

**Externe Ressourcen:**
- [Blitzortung.org](https://www.blitzortung.org/) - Weltweites Lightning-Netzwerk
- [Lightning Detection Networks](https://www.vaisala.com/en/products/lightning-detection)

---

**Beeindruckende Blitz-Erkennung mit Wissenschaft! ⚡📡**
