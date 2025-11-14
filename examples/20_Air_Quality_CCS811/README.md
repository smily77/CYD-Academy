# Beispiel 20: Luftqualität mit CCS811 Sensor

Dieses Beispiel demonstriert die Überwachung der Raumluftqualität mit dem **CCS811 Digital Gas Sensor**. Der Sensor misst eCO2 (equivalent CO2) und TVOC (Total Volatile Organic Compounds) und liefert einen umfassenden Luftqualitäts-Index.

## 📸 Features

- **🌡️ eCO2-Messung**: 400 - 8192 ppm (parts per million)
- **💨 TVOC-Messung**: 0 - 1187 ppb (parts per billion)
- **🎨 Luftqualitäts-Index (AQI)**:
  - Ausgezeichnet (< 600 ppm) - Grün
  - Gut (600-800 ppm) - Gelb-Grün
  - Mäßig (800-1000 ppm) - Gelb
  - Ungesund (1000-1500 ppm) - Orange
  - Sehr ungesund (1500-2500 ppm) - Rot
  - Gefährlich (> 2500 ppm) - Lila
- **📊 Dual-Channel Verlaufsdiagramm**: eCO2 (Cyan) und TVOC (Orange)
- **⏱️ Drei Messmodi**:
  - Eco: 60 Sekunden Interval (Stromsparmodus)
  - Normal: 10 Sekunden Interval (Empfohlen)
  - Fast: 1 Sekunde Interval (Echtzeit)
- **🔥 Aufwärmphase-Timer**: Countdown für 20 Minuten Burn-in
- **💾 Baseline-Speicherung**: Automatisches Speichern im EEPROM für konsistente Messungen
- **📈 Statistik**: Min/Max/Durchschnitt für eCO2 und TVOC
- **🎨 Touch-Bedienung**: Modusauswahl und Reset

## 🔌 Hardware-Anforderungen

### Hauptkomponenten
- **ESP32-2432S028R** (CYD - Cheap Yellow Display)
- **CCS811 Air Quality Sensor** (I2C)

### CCS811 Spezifikationen

**Messbereiche**:
- **eCO2**: 400 - 8192 ppm
- **TVOC**: 0 - 1187 ppb
- **Genauigkeit**: ±50 ppm (eCO2), ±15% (TVOC)

**Technische Daten**:
- **Interface**: I2C (Standard: 100 kHz, Fast: 400 kHz)
- **I2C-Adressen**:
  - 0x5A (Standard, ADDR Pin LOW/offen)
  - 0x5B (ADDR Pin HIGH)
- **Spannung**: 1,8V - 3,6V (Logik), 3,3V (Betrieb)
- **Stromaufnahme**:
  - Idle: < 1 µA
  - Mode 1 (1s): ca. 26 mA
  - Mode 2 (10s): ca. 2,8 mA (durchschnittlich)
  - Mode 3 (60s): ca. 0,5 mA (durchschnittlich)
- **Sensor-Typ**: MOX (Metal Oxide Semiconductor)
- **Aufwärmzeit**:
  - 20 Minuten für optimale Genauigkeit
  - 48 Stunden für maximale Stabilität
- **Lebensdauer**: > 5 Jahre (typisch)

### Verbindung (I2C)

```
CCS811         CYD (ESP32-2432S028R)
------         ---------------------
VCC     -----> 3.3V
GND     -----> GND
SDA     -----> GPIO 22 (extSDA)
SCL     -----> GPIO 27 (extSCL)
WAK     -----> GND (Wake-Pin, LOW = aktiv)
RST     -----> (optional, HIGH oder offen)
ADDR    -----> (offen für 0x5A, HIGH für 0x5B)
INT     -----> (optional, für Interrupt-basierte Messung)
```

**Wichtig**:
- WAK (Wake) Pin muss auf GND, sonst ist Sensor im Sleep-Modus
- ADDR Pin bestimmt I2C-Adresse (meist offen = 0x5A)
- Keine externen Pull-ups nötig (bereits auf CYD vorhanden)

## 🔬 Technischer Hintergrund

### Was ist eCO2?

**eCO2 = equivalent CO2** (äquivalentes Kohlendioxid)

Der CCS811 misst **nicht direkt CO2**, sondern berechnet einen **äquivalenten CO2-Wert** basierend auf VOC-Messungen.

#### Wie funktioniert das?

```
1. MOX-Sensor misst VOC-Konzentrationen
   ↓
2. Algorithmus korreliert VOC mit typischen CO2-Werten
   ↓
3. Ausgabe als "eCO2" (entspricht Raumluft-CO2)
```

**Warum nicht echtes CO2?**
- Echte CO2-Sensoren (NDIR) sind **teuer** (> 50€) und **groß**
- CCS811 ist **kompakt** und **günstig** (< 10€)
- Für **Raumluftqualität** ist eCO2 ausreichend genau

**Limitierung**:
- ❌ Nicht für **präzise CO2-Messungen** (z.B. Gewächshaus-Steuerung)
- ❌ Kann durch starke VOC-Quellen verfälscht werden
- ✅ Perfekt für **Lüftungssteuerung** und **Raumklima**

#### eCO2-Werte Interpretation

| eCO2 (ppm) | Bedeutung | Luftqualität | Empfehlung |
|------------|-----------|--------------|------------|
| 400 | Außenluft | Ausgezeichnet | - |
| 400-600 | Sehr gut belüftet | Ausgezeichnet | - |
| 600-800 | Gut belüftet | Gut | Optimal |
| 800-1000 | Mäßig belüftet | Mäßig | Lüften empfohlen |
| 1000-1500 | Schlecht belüftet | Ungesund | Sofort lüften |
| 1500-2500 | Sehr schlecht | Sehr ungesund | Raum verlassen |
| > 2500 | Extrem | Gefährlich | Dringend lüften! |

**Referenzen** (WHO, DIN EN 13779):
- **< 800 ppm**: Hohe Raumluftqualität (Kategorie I)
- **800-1000 ppm**: Mittlere Qualität (Kategorie II)
- **1000-1400 ppm**: Mäßige Qualität (Kategorie III)
- **> 1400 ppm**: Niedrige Qualität (Kategorie IV)

### Was ist TVOC?

**TVOC = Total Volatile Organic Compounds** (flüchtige organische Verbindungen)

TVOC ist die **Summe aller flüchtigen organischen Verbindungen** in der Luft.

#### Beispiele für VOCs:

**Natürliche Quellen**:
- 🌳 Terpene (Holz, Pflanzen)
- 🍎 Fruchtsäuren
- 🌬️ Ausgeatmete Gase (Ethanol, Aceton)

**Künstliche Quellen**:
- 🎨 Farben, Lacke, Klebstoffe
- 🧴 Reinigungsmittel, Parfüm
- 🔥 Kochen, Rauchen
- 🖨️ Drucker, Kopierer (Ozon, Toner-Ausgasungen)
- 🛋️ Möbel (Formaldehyd aus Spanplatten)
- 🚗 Benzin, Lösungsmittel

#### TVOC-Werte Interpretation

| TVOC (ppb) | Bedeutung | Luftqualität | Empfehlung |
|------------|-----------|--------------|------------|
| 0-220 | Sauber | Ausgezeichnet | - |
| 220-660 | Leicht belastet | Gut | Normal |
| 660-1430 | Mäßig belastet | Mäßig | Lüften |
| 1430-2200 | Stark belastet | Ungesund | Quelle finden |
| > 2200 | Extrem belastet | Gefährlich | Sofort handeln |

**Umrechnung**: 1 ppm = 1000 ppb

**Gesundheitseffekte hoher TVOC**:
- Kopfschmerzen
- Reizung der Augen/Atemwege
- Müdigkeit, Konzentrationsprobleme
- Langfristig: Leberschäden (bei sehr hohen Werten)

### MOX-Sensor Technologie

Der CCS811 nutzt einen **MOX (Metal Oxide Semiconductor)** Sensor:

#### Funktionsprinzip

```
┌─────────────────────────────────────┐
│  MOX-Sensor (vereinfacht):          │
│                                      │
│  ┌──────────────┐                   │
│  │  SnO2-Schicht│  ← Metalloxid     │
│  │  (beheizt)   │  (Zinnoxid)       │
│  └──────────────┘                   │
│         │                            │
│    VOCs adsorbieren                 │
│         ↓                            │
│  Widerstand ändert sich             │
│         ↓                            │
│  Messung der Leitfähigkeit          │
│                                      │
└─────────────────────────────────────┘
```

**Schritte**:
1. **Heizen**: Metalloxid-Schicht wird auf ca. 250-400°C erhitzt
2. **Adsorption**: VOC-Moleküle lagern sich an der Oberfläche an
3. **Reaktion**: Chemische Reaktion ändert elektrische Leitfähigkeit
4. **Messung**: Widerstandsänderung wird gemessen
5. **Algorithmus**: Umrechnung in eCO2 und TVOC

**Vorteile**:
- ✅ Sehr empfindlich für VOCs
- ✅ Kompakt und günstig
- ✅ Lange Lebensdauer (> 5 Jahre)

**Nachteile**:
- ❌ Benötigt Aufwärmphase (20 min)
- ❌ Stromverbrauch (durch Heizung)
- ❌ Querempfindlichkeit (reagiert auf viele Gase)
- ❌ Baseline-Drift (regelmäßige Kalibrierung nötig)

### Baseline-Konzept

Der CCS811 nutzt ein **Baseline-System** für konstante Genauigkeit:

#### Was ist die Baseline?

Die Baseline ist ein **Referenzwert**, der die "normale" Luftqualität repräsentiert.

```
Baseline = "Was ist normal für diese Umgebung?"

Beispiel:
- Wohnzimmer: Baseline bei 450 ppm eCO2
- Büro mit Drucker: Baseline bei 500 ppm eCO2
- Küche: Baseline bei 550 ppm eCO2
```

#### Warum wichtig?

MOX-Sensoren unterliegen **Drift**:
- Langfristige Änderungen der Sensitivität
- Verschmutzung der Sensor-Oberfläche
- Alterung der Materialien

**Ohne Baseline**: Werte driften über Wochen/Monate ab
**Mit Baseline**: Sensor "lernt" die normale Umgebung

#### Baseline-Verwaltung im Code

```cpp
// Baseline speichern (alle 24h)
void saveBaseline() {
  uint16_t baseline = ccs.getBaseline();
  EEPROM.write(EEPROM_BASELINE_ADDR, baseline >> 8);
  EEPROM.write(EEPROM_BASELINE_ADDR + 1, baseline & 0xFF);
  EEPROM.commit();
}

// Baseline laden (beim Start)
void loadBaseline() {
  uint8_t high = EEPROM.read(EEPROM_BASELINE_ADDR);
  uint8_t low = EEPROM.read(EEPROM_BASELINE_ADDR + 1);
  uint16_t baseline = (high << 8) | low;
  ccs.setBaseline(baseline);
}
```

**Best Practice**:
- Baseline **täglich** speichern
- Baseline **beim Start** laden
- Nach **ersten 24h Betrieb** erste Baseline speichern
- Bei **Standortwechsel** Baseline zurücksetzen

### Aufwärmphase (Burn-in)

Der CCS811 benötigt **Aufwärmzeit** für genaue Messungen:

| Zeitraum | Genauigkeit | Status |
|----------|-------------|--------|
| 0-1 Min | Nicht nutzbar | Heizung startet |
| 1-20 Min | ±100 ppm | Stabilisierung |
| 20 Min - 48h | ±50 ppm | Gut |
| > 48h | ±30 ppm | Optimal |

**Im Code**:
```cpp
#define WARMUP_TIME_MS (20UL * 60UL * 1000UL)  // 20 Minuten

if (millis() - sensorStartTime < WARMUP_TIME_MS) {
  // Zeige Countdown
  unsigned long remainingMin = (WARMUP_TIME_MS - elapsed) / 60000;
  lcd.printf("Aufwärmen: %lu min", remainingMin);
}
```

**Tipps**:
- Sensor **dauerhaft** laufen lassen für beste Genauigkeit
- Nach **Stromausfall**: 20 Min warten
- Für **Prototyping**: Erste Werte nach 5 Min nutzbar (geringere Genauigkeit)

## 📡 I2C-Kommunikation

### Adafruit CCS811 Library

Der CCS811 hat ein **komplexes Protokoll**, daher verwenden wir eine Library:

```cpp
#include <Adafruit_CCS811.h>

Adafruit_CCS811 ccs;

// Initialisierung
if (!ccs.begin(0x5A, &Wire)) {
  Serial.println("CCS811 nicht gefunden!");
}

// Messmodus setzen
ccs.setDriveMode(CCS811_DRIVE_MODE_10SEC);

// Daten auslesen
if (ccs.available() && !ccs.readData()) {
  uint16_t eco2 = ccs.geteCO2();
  uint16_t tvoc = ccs.getTVOC();
}
```

### Messmodi

Der CCS811 hat **5 Betriebsmodi**:

| Modus | Beschreibung | Interval | Stromverbrauch |
|-------|--------------|----------|----------------|
| 0 | Idle | - | < 1 µA |
| 1 | 1 Sekunde | 1s | 26 mA |
| 2 | 10 Sekunden | 10s | 2,8 mA (avg) |
| 3 | 60 Sekunden | 60s | 0,5 mA (avg) |
| 4 | 250 Millisekunden | 250ms | 80 mA |

**Im Code**:
```cpp
// Mode 1: Fast (1s)
ccs.setDriveMode(CCS811_DRIVE_MODE_1SEC);

// Mode 2: Normal (10s) - Empfohlen
ccs.setDriveMode(CCS811_DRIVE_MODE_10SEC);

// Mode 3: Eco (60s) - Stromsparmodus
ccs.setDriveMode(CCS811_DRIVE_MODE_60SEC);
```

**Empfehlung**: Mode 2 (10s) als bester Kompromiss zwischen Genauigkeit und Stromverbrauch.

### Temperatur/Feuchtigkeits-Kompensation

Der CCS811 kann mit **Umgebungsdaten** kompensiert werden:

```cpp
// Von BME280 oder DHT22
float temperature = 22.5;  // °C
float humidity = 45.0;     // %

// An CCS811 übergeben
ccs.setEnvironmentalData(humidity, temperature);
```

**Effekt**:
- Verbesserte Genauigkeit um **10-15%**
- Besonders wichtig bei **starken Temperaturschwankungen**

**Mathematisch**:
```
VOC-Sensitivität ist temperaturabhängig:
Resistance = R0 × e^(E_a / (k × T))

Wobei:
R0 = Basis-Widerstand
E_a = Aktivierungsenergie
k = Boltzmann-Konstante
T = Temperatur in Kelvin

→ Kompensation verbessert Linearität
```

## 💻 Code-Struktur

### Hauptkomponenten

#### 1. Sensor-Initialisierung

```cpp
#include <Adafruit_CCS811.h>

Adafruit_CCS811 ccs;

void setup() {
  Wire.begin(extSDA, extSCL);

  if (!ccs.begin(CCS811_ADDR, &Wire)) {
    Serial.println("CCS811 nicht gefunden!");
    return;
  }

  // Warten bis bereit
  while (!ccs.available()) delay(100);

  // Messmodus setzen
  ccs.setDriveMode(CCS811_DRIVE_MODE_10SEC);

  // Baseline laden
  loadBaseline();
}
```

#### 2. Daten auslesen

```cpp
void readSensor() {
  if (!ccs.available()) return;

  if (!ccs.readData()) {  // 0 = Erfolg
    eco2 = ccs.geteCO2();   // ppm
    tvoc = ccs.getTVOC();   // ppb

    // Optional: Mit Temperatur kompensieren
    // ccs.setEnvironmentalData(humidity, temperature);

    updateStatistics();
    addToHistory();

    Serial.printf("eCO2: %d ppm | TVOC: %d ppb\n", eco2, tvoc);
  } else {
    Serial.println("Fehler beim Lesen");
  }
}
```

#### 3. Luftqualitäts-Index

```cpp
String getAQILevel() {
  if (eco2 < 600) return "Ausgezeichnet";
  if (eco2 < 800) return "Gut";
  if (eco2 < 1000) return "Mäßig";
  if (eco2 < 1500) return "Ungesund";
  return "Gefährlich";
}

uint16_t getAQIColor() {
  if (eco2 < 600) return TFT_GREEN;
  if (eco2 < 800) return TFT_GREENYELLOW;
  if (eco2 < 1000) return TFT_YELLOW;
  if (eco2 < 1500) return TFT_ORANGE;
  if (eco2 < 2500) return TFT_RED;
  return TFT_PURPLE;
}
```

#### 4. Baseline-Verwaltung

```cpp
void saveBaseline() {
  uint16_t baseline = ccs.getBaseline();

  // Im EEPROM speichern (2 Bytes)
  EEPROM.write(EEPROM_BASELINE_ADDR, baseline >> 8);
  EEPROM.write(EEPROM_BASELINE_ADDR + 1, baseline & 0xFF);
  EEPROM.commit();

  Serial.printf("Baseline: 0x%04X\n", baseline);
}

void loadBaseline() {
  uint8_t high = EEPROM.read(EEPROM_BASELINE_ADDR);
  uint8_t low = EEPROM.read(EEPROM_BASELINE_ADDR + 1);

  // Validierung
  if (high == 0xFF && low == 0xFF) {
    Serial.println("Keine Baseline");
    return;
  }

  uint16_t baseline = (high << 8) | low;
  ccs.setBaseline(baseline);
}
```

#### 5. Dual-Channel Graph

```cpp
void drawGraph() {
  for (int i = 0; i < MAX_HISTORY - 1; i++) {
    // eCO2 (Cyan) - Skala 400-2000 ppm
    int y1_eco2 = map(eco2History[i], 400, 2000, graphH, 0);
    int y2_eco2 = map(eco2History[i+1], 400, 2000, graphH, 0);
    lcd.drawLine(x1, y1_eco2, x2, y2_eco2, TFT_CYAN);

    // TVOC (Orange) - Skala 0-1000 ppb
    int y1_tvoc = map(tvocHistory[i], 0, 1000, graphH, 0);
    int y2_tvoc = map(tvocHistory[i+1], 0, 1000, graphH, 0);
    lcd.drawLine(x1, y1_tvoc, x2, y2_tvoc, TFT_ORANGE);
  }
}
```

#### 6. Aufwärmphase-Anzeige

```cpp
void updateWarmupStatus() {
  unsigned long elapsed = millis() - sensorStartTime;

  if (elapsed < WARMUP_TIME_MS) {
    unsigned long remainingMin = (WARMUP_TIME_MS - elapsed) / 60000;
    lcd.printf("Aufwärmen: %lu min", remainingMin);
    lcd.setTextColor(TFT_YELLOW);
  } else {
    lcd.print("Bereit");
    lcd.setTextColor(TFT_GREEN);
    sensorWarmedUp = true;
  }
}
```

## 🚀 Installation & Verwendung

### 1. Library installieren

```
Arduino IDE -> Tools -> Manage Libraries
Suche: "Adafruit CCS811"
Installieren: "Adafruit CCS811" + Abhängigkeiten
```

**Abhängigkeiten** (werden automatisch installiert):
- Adafruit Unified Sensor
- Adafruit Bus IO

### 2. Hardware aufbauen

```
CCS811 mit CYD verbinden:
- VCC → 3.3V
- GND → GND
- SDA → GPIO 22
- SCL → GPIO 27
- WAK → GND (wichtig!)
```

### 3. Code hochladen

```bash
# In Arduino IDE:
# 1. Board: "ESP32 Dev Module" auswählen
# 2. Datei → Beispiele → CYD-Academy → 20_Air_Quality_CCS811 öffnen
# 3. Hochladen
```

### 4. Bedienung

**Erste Inbetriebnahme**:
1. Sensor startet → "Aufwärmen: 20 min" wird angezeigt
2. Warte mindestens 20 Minuten
3. Nach 24h wird erste Baseline gespeichert

**Messmodi**:
- **ECO**: Tippe auf "ECO" → Messung alle 60s (Stromsparmodus)
- **NORMAL**: Tippe auf "NORMAL" → Messung alle 10s (Empfohlen)
- **FAST**: Tippe auf "FAST" → Messung jede Sekunde (Echtzeit)

**Reset**:
- Tippe auf "RESET" → Statistik und Verlauf werden gelöscht

### Serielle Ausgabe

```
=== CYD Air Quality Monitor - CCS811 ===
I2C initialisiert auf SDA: 22, SCL: 27
CCS811 gefunden!
CCS811 bereit!
Messmodus: NORMAL (10s)
Baseline geladen: 0x847B
Sensor benötigt 20 Minuten Aufwärmzeit

[120] eCO2: 456 ppm | TVOC: 12 ppb | AQI: Ausgezeichnet
[130] eCO2: 478 ppm | TVOC: 18 ppb | AQI: Ausgezeichnet
[140] eCO2: 501 ppm | TVOC: 25 ppb | AQI: Ausgezeichnet
[150] eCO2: 623 ppm | TVOC: 45 ppb | AQI: Gut
```

## 🔧 Troubleshooting

### Problem: "CCS811 nicht gefunden"

**Symptome**:
- Display zeigt "CCS811 FEHLER!"
- Serielle Ausgabe: "CCS811 nicht gefunden auf Adresse 0x5A"

**Lösungen**:

1. **Verkabelung prüfen**:
   ```cpp
   // I2C-Scanner
   void scanI2C() {
     for (byte addr = 1; addr < 127; addr++) {
       Wire.beginTransmission(addr);
       if (Wire.endTransmission() == 0) {
         Serial.printf("Gefunden: 0x%02X\n", addr);
       }
     }
   }
   ```
   - Erwartetes Ergebnis: `0x5A` oder `0x5B`

2. **WAK-Pin prüfen**:
   - ⚠️ WAK **muss auf GND**!
   - Wenn WAK offen/HIGH → Sensor im Sleep-Modus

3. **I2C-Adresse prüfen**:
   ```cpp
   // Falls Sensor auf 0x5B
   if (!ccs.begin(0x5B, &Wire)) {
     Serial.println("Auch 0x5B fehlgeschlagen");
   }
   ```

4. **Pull-up Widerstände**:
   - Bei langen Kabeln (> 20cm): 4,7kΩ Pull-ups zu 3,3V

### Problem: Sensor bleibt bei "Warte auf Sensor..."

**Ursache**: `ccs.available()` liefert nie `true`

**Lösungen**:

1. **Timeout einbauen**:
   ```cpp
   unsigned long startWait = millis();
   while (!ccs.available()) {
     if (millis() - startWait > 5000) {
       Serial.println("Timeout! Sensor reagiert nicht");
       break;
     }
     delay(100);
   }
   ```

2. **Firmware-Version prüfen**:
   ```cpp
   // Manche CCS811-Module haben alte Firmware
   // → Sensor-Reset durchführen
   // RST Pin für 1s auf LOW, dann HIGH
   ```

### Problem: Werte bleiben bei 400 ppm / 0 ppb

**Symptome**:
- eCO2 immer 400 ppm
- TVOC immer 0 ppb
- Keine Reaktion auf VOC-Quellen

**Ursachen & Lösungen**:

1. **Noch in Aufwärmphase**:
   - Warte mindestens 20 Minuten
   - Erste 5 Minuten: Sensor liefert Standardwerte

2. **Sensor-Modus falsch**:
   ```cpp
   // Prüfen ob Modus gesetzt ist
   ccs.setDriveMode(CCS811_DRIVE_MODE_10SEC);
   delay(100);

   // Erzwinge neue Messung
   if (ccs.available()) {
     ccs.readData();
   }
   ```

3. **Baseline falsch**:
   ```cpp
   // Baseline zurücksetzen
   void resetBaseline() {
     EEPROM.write(EEPROM_BASELINE_ADDR, 0xFF);
     EEPROM.write(EEPROM_BASELINE_ADDR + 1, 0xFF);
     EEPROM.commit();

     ESP.restart();  // Neustart
   }
   ```

4. **Sensor defekt**:
   - Nach > 5 Jahren Betrieb möglich
   - MOX-Schicht erschöpft
   - → Sensor ersetzen

### Problem: Unplausible Werte (> 8000 ppm)

**Symptome**:
- eCO2 springt auf > 8000 ppm
- TVOC > 1000 ppb
- Werte instabil

**Ursachen**:

1. **Starke VOC-Quelle**:
   - Lösungsmittel, Reiniger, Alkohol
   - Nagellack, Parfüm
   - Frisch gedruckte Dokumente (Toner)
   - → Normal! Werte sinken nach Entfernung der Quelle

2. **Defekte Baseline**:
   ```cpp
   // Baseline-Reset wie oben
   resetBaseline();
   ```

3. **Temperatur zu hoch**:
   - MOX-Sensor bei > 50°C ungenau
   - → Sensor kühlen, Umgebung prüfen

### Problem: Hoher Stromverbrauch / ESP32 resettet

**Symptome**:
- ESP32 startet sporadisch neu
- USB-Port meldet Überlast

**Ursache**: CCS811 zieht bis zu 26 mA im Fast-Mode

**Lösungen**:

1. **Externes Netzteil**: 5V/1A USB-Netzteil verwenden
2. **Eco-Modus**: Auf 60s-Modus umschalten
   ```cpp
   measurementMode = MODE_ECO;
   ccs.setDriveMode(CCS811_DRIVE_MODE_60SEC);
   ```

### Problem: Baseline geht verloren

**Symptome**:
- Nach Neustart wieder bei 400 ppm
- Baseline muss neu gelernt werden

**Lösung**: EEPROM-Größe prüfen
```cpp
void checkEEPROM() {
  EEPROM.begin(512);  // Mindestens 512 Bytes

  // Test-Write
  EEPROM.write(0, 0xAA);
  EEPROM.commit();

  if (EEPROM.read(0) != 0xAA) {
    Serial.println("EEPROM Fehler!");
  }
}
```

## 🎯 Erweiterungsideen

### 1. Temperatur/Feuchtigkeits-Sensor Integration

**Konzept**: Kombination mit BME280 für bessere Genauigkeit

```cpp
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;
Adafruit_CCS811 ccs;

void setup() {
  bme.begin(0x76);
  ccs.begin(0x5A);
}

void loop() {
  // Umgebungsdaten lesen
  float temp = bme.readTemperature();      // °C
  float hum = bme.readHumidity();          // %
  float pressure = bme.readPressure();     // Pa

  // An CCS811 übergeben
  ccs.setEnvironmentalData(hum, temp);

  // Messungen
  if (!ccs.readData()) {
    uint16_t eco2 = ccs.geteCO2();
    uint16_t tvoc = ccs.getTVOC();

    // Erweitertes Display
    lcd.printf("Temp: %.1f°C\n", temp);
    lcd.printf("Feuchte: %.1f%%\n", hum);
    lcd.printf("Druck: %.0f hPa\n", pressure / 100.0);
    lcd.printf("eCO2: %d ppm\n", eco2);
    lcd.printf("TVOC: %d ppb\n", tvoc);
  }
}
```

**Vorteile**:
- ±10-15% bessere Genauigkeit
- Vollständiges Raumklima-Monitoring
- Taupunkt-Berechnung möglich

### 2. Automatische Lüftersteuerung

**Konzept**: Relais schaltet Lüfter bei schlechter Luftqualität

```cpp
#define RELAY_PIN 25
#define CO2_THRESHOLD 1000  // ppm

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // Lüfter aus
}

void controlVentilation() {
  static bool fanRunning = false;

  if (eco2 > CO2_THRESHOLD && !fanRunning) {
    // Lüfter einschalten
    digitalWrite(RELAY_PIN, HIGH);
    fanRunning = true;
    Serial.println("Lüfter AN");

    lcd.fillRect(0, 0, 320, 30, TFT_ORANGE);
    lcd.drawString("LÜFTER AKTIV", 160, 10);
  } else if (eco2 < CO2_THRESHOLD - 100 && fanRunning) {
    // Lüfter ausschalten (mit Hysterese)
    digitalWrite(RELAY_PIN, LOW);
    fanRunning = false;
    Serial.println("Lüfter AUS");
  }
}
```

**Hardware**:
- 5V Relais-Modul
- Lüfter (12V DC oder 230V AC)
- Externe Stromversorgung

**Erweiterung**: PWM-Steuerung für variable Lüftergeschwindigkeit
```cpp
void controlFanSpeed() {
  // PWM: 0-255
  int speed = map(constrain(eco2, 600, 1500), 600, 1500, 0, 255);
  analogWrite(FAN_PWM_PIN, speed);
}
```

### 3. Home Assistant Integration (MQTT)

**Konzept**: Daten an Home Assistant senden

```cpp
#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient mqtt(espClient);

#define MQTT_SERVER "192.168.1.100"
#define MQTT_PORT 1883
#define MQTT_TOPIC_ECO2 "homeassistant/sensor/cyd/eco2"
#define MQTT_TOPIC_TVOC "homeassistant/sensor/cyd/tvoc"

void setup() {
  WiFi.begin("SSID", "PASSWORD");
  while (WiFi.status() != WL_CONNECTED) delay(500);

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.connect("CYD-AirQuality");

  // Home Assistant Auto-Discovery
  publishDiscovery();
}

void publishDiscovery() {
  // eCO2 Sensor
  String config = R"(
  {
    "name": "CYD eCO2",
    "state_topic": "homeassistant/sensor/cyd/eco2",
    "unit_of_measurement": "ppm",
    "device_class": "carbon_dioxide",
    "icon": "mdi:molecule-co2"
  }
  )";
  mqtt.publish("homeassistant/sensor/cyd_eco2/config", config.c_str(), true);

  // TVOC Sensor
  config = R"(
  {
    "name": "CYD TVOC",
    "state_topic": "homeassistant/sensor/cyd/tvoc",
    "unit_of_measurement": "ppb",
    "device_class": "volatile_organic_compounds",
    "icon": "mdi:air-filter"
  }
  )";
  mqtt.publish("homeassistant/sensor/cyd_tvoc/config", config.c_str(), true);
}

void publishData() {
  char payload[32];

  sprintf(payload, "%d", eco2);
  mqtt.publish(MQTT_TOPIC_ECO2, payload);

  sprintf(payload, "%d", tvoc);
  mqtt.publish(MQTT_TOPIC_TVOC, payload);
}

// In loop():
// if (millis() - lastPublish >= 60000) {
//   publishData();
//   lastPublish = millis();
// }
```

**Home Assistant Automations**:
```yaml
# automation.yaml
- alias: "Luftqualität Warnung"
  trigger:
    platform: numeric_state
    entity_id: sensor.cyd_eco2
    above: 1000
  action:
    service: notify.mobile_app
    data:
      message: "Hoher CO2-Wert! Bitte lüften."
      title: "Luftqualität"
```

### 4. SD-Karten Datenlogger

**Konzept**: Langzeit-Aufzeichnung für Analyse

```cpp
#include <SD.h>
#include <SPI.h>

#define SD_CS 5

File logFile;

void setupSD() {
  if (!SD.begin(SD_CS)) {
    Serial.println("SD-Karte Fehler!");
    return;
  }

  // CSV-Header
  logFile = SD.open("/airquality_log.csv", FILE_WRITE);
  if (logFile) {
    logFile.println("Timestamp,eCO2_ppm,TVOC_ppb,Temp_C,Humidity_%,AQI");
    logFile.close();
  }
}

void logData() {
  logFile = SD.open("/airquality_log.csv", FILE_APPEND);
  if (logFile) {
    // Unix-Timestamp (von NTP)
    logFile.print(getUnixTimestamp());
    logFile.print(",");
    logFile.print(eco2);
    logFile.print(",");
    logFile.print(tvoc);
    logFile.print(",");
    logFile.print(temperature, 1);
    logFile.print(",");
    logFile.print(humidity, 1);
    logFile.print(",");
    logFile.println(getAQILevel());
    logFile.close();

    Serial.println("Daten geloggt");
  }
}

// Alle 10 Minuten loggen
if (millis() - lastLog >= 600000) {
  logData();
  lastLog = millis();
}
```

**Analyse in Python**:
```python
import pandas as pd
import matplotlib.pyplot as plt

# CSV laden
df = pd.read_csv('airquality_log.csv')
df['Timestamp'] = pd.to_datetime(df['Timestamp'], unit='s')

# Diagramm
fig, ax1 = plt.subplots(figsize=(12, 6))

ax1.plot(df['Timestamp'], df['eCO2_ppm'], label='eCO2', color='cyan')
ax1.set_ylabel('eCO2 (ppm)', color='cyan')
ax1.tick_params(axis='y', labelcolor='cyan')

ax2 = ax1.twinx()
ax2.plot(df['Timestamp'], df['TVOC_ppb'], label='TVOC', color='orange')
ax2.set_ylabel('TVOC (ppb)', color='orange')
ax2.tick_params(axis='y', labelcolor='orange')

plt.title('Luftqualität über Zeit')
plt.show()
```

### 5. Alarmsystem mit Buzzer

**Konzept**: Akustische Warnung bei kritischen Werten

```cpp
#define BUZZER_PIN 26

struct AlarmLevel {
  uint16_t threshold;
  uint16_t frequency;
  uint16_t duration;
  String message;
};

AlarmLevel alarms[] = {
  {1000, 1000, 200, "Bitte lüften"},      // Mäßig
  {1500, 2000, 500, "DRINGEND lüften!"},  // Ungesund
  {2500, 3000, 1000, "GEFAHR! Raum verlassen!"}  // Gefährlich
};

void checkAlarms() {
  for (int i = 2; i >= 0; i--) {  // Von höchster Priorität
    if (eco2 >= alarms[i].threshold) {
      // Buzzer
      tone(BUZZER_PIN, alarms[i].frequency, alarms[i].duration);

      // Display-Warnung
      lcd.fillRect(0, 0, 320, 40, TFT_RED);
      lcd.setTextColor(TFT_WHITE);
      lcd.setTextSize(2);
      lcd.drawString(alarms[i].message, 160, 15);

      // Blink-Effekt
      for (int j = 0; j < 3; j++) {
        lcd.fillScreen(TFT_RED);
        delay(200);
        lcd.fillScreen(COLOR_BACKGROUND);
        delay(200);
      }

      break;
    }
  }
}
```

**Erweiterung**: SMS-Benachrichtigung
```cpp
// Mit SIM800L GSM-Modul
void sendSMSAlert() {
  Serial2.println("AT+CMGS=\"+49123456789\"");
  delay(100);
  Serial2.print("WARNUNG: Hoher CO2-Wert (");
  Serial2.print(eco2);
  Serial2.println(" ppm) im Wohnzimmer!");
  delay(100);
  Serial2.write(26);  // Ctrl+Z
}
```

### 6. Web-Dashboard

**Konzept**: Live-Daten über Web-Interface

```cpp
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

void setupWebServer() {
  // HTML-Seite
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = R"(
    <!DOCTYPE html>
    <html>
    <head>
      <title>CYD Air Quality</title>
      <meta http-equiv="refresh" content="5">
      <style>
        body { font-family: Arial; text-align: center; background: #1a1a1a; color: white; }
        .value { font-size: 48px; margin: 20px; }
        .good { color: #00ff00; }
        .moderate { color: #ffff00; }
        .bad { color: #ff0000; }
      </style>
    </head>
    <body>
      <h1>🌡️ Luftqualität Monitor</h1>
      <div class="value )" + getColorClass() + R"(">
        )" + String(eco2) + R"( ppm
      </div>
      <p>eCO2 (equivalent CO2)</p>
      <div class="value">
        )" + String(tvoc) + R"( ppb
      </div>
      <p>TVOC (flüchtige Verbindungen)</p>
      <h2>Status: )" + getAQILevel() + R"(</h2>
    </body>
    </html>
    )";
    request->send(200, "text/html", html);
  });

  // JSON API
  server.on("/api/data", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{";
    json += "\"eco2\":" + String(eco2) + ",";
    json += "\"tvoc\":" + String(tvoc) + ",";
    json += "\"aqi\":\"" + getAQILevel() + "\"";
    json += "}";
    request->send(200, "application/json", json);
  });

  server.begin();
}

String getColorClass() {
  if (eco2 < 800) return "good";
  if (eco2 < 1500) return "moderate";
  return "bad";
}
```

### 7. Kalibrierung mit Outdoor-Referenz

**Konzept**: Automatische Baseline-Kalibrierung mit Außenluft

```cpp
#define OUTDOOR_CO2 400  // Außenluft typisch 400 ppm

void calibrateWithOutdoor() {
  lcd.fillScreen(COLOR_BACKGROUND);
  lcd.drawString("KALIBRIERUNG", 160, 100);
  lcd.drawString("Sensor nach draußen", 160, 150);
  lcd.drawString("halten für 5 Minuten", 160, 170);

  // 5 Minuten warten
  unsigned long startTime = millis();
  while (millis() - startTime < 300000) {
    unsigned long remaining = (300000 - (millis() - startTime)) / 1000;
    lcd.fillRect(100, 200, 120, 30, COLOR_BACKGROUND);
    lcd.drawString(String(remaining) + " s", 160, 210);
    delay(1000);
  }

  // Baseline setzen
  if (!ccs.readData()) {
    uint16_t measuredCO2 = ccs.geteCO2();
    Serial.printf("Gemessen: %d ppm\n", measuredCO2);

    // Offset berechnen und Baseline anpassen
    // (Vereinfachte Methode - echte Kalibrierung ist komplexer)
    uint16_t currentBaseline = ccs.getBaseline();
    Serial.printf("Alte Baseline: 0x%04X\n", currentBaseline);

    // Neue Baseline speichern
    saveBaseline();

    lcd.drawString("Kalibrierung OK!", 160, 250);
  }
}
```

## 📚 Weiterführende Themen

### Vergleich: CCS811 vs. andere Luftqualitätssensoren

| Sensor | eCO2 | TVOC | Echtes CO2 | Preis | Genauigkeit |
|--------|------|------|------------|-------|-------------|
| **CCS811** | ✅ 400-8192 ppm | ✅ 0-1187 ppb | ❌ | ~10€ | ±50 ppm |
| **BME680** | ✅ berechnet | ✅ IAQ-Index | ❌ | ~15€ | Relativ |
| **SGP30** | ✅ 400-60000 ppm | ✅ 0-60000 ppb | ❌ | ~12€ | ±50 ppm |
| **MH-Z19B** | ❌ | ❌ | ✅ 0-5000 ppm | ~20€ | ±50 ppm |
| **SCD40** | ❌ | ❌ | ✅ 0-40000 ppm | ~50€ | ±30 ppm |

**Empfehlung**:
- **Raumluft-Monitoring**: CCS811 oder BME680
- **Präzise CO2-Messung**: SCD40 oder MH-Z19B
- **Kombination**: CCS811 + MH-Z19B für eCO2 + echtes CO2

### MOX vs. NDIR Sensoren

**MOX (Metal Oxide)** - wie CCS811:
```
Vorteile:
✅ Günstig (< 10€)
✅ Kompakt
✅ Misst VOCs direkt
✅ Lange Lebensdauer

Nachteile:
❌ Kein echtes CO2
❌ Querempfindlich
❌ Baseline-Drift
❌ Stromverbrauch (Heizung)
```

**NDIR (Non-Dispersive Infrared)** - wie MH-Z19B:
```
Vorteile:
✅ Echtes CO2
✅ Sehr genau (±30 ppm)
✅ Keine Baseline-Drift
✅ Selektiv (nur CO2)

Nachteile:
❌ Teurer (> 20€)
❌ Größer
❌ Misst keine VOCs
❌ Stromverbrauch (IR-Lampe)
```

### Luftqualitäts-Standards

**WHO (World Health Organization)**:
- **< 1000 ppm**: Akzeptabel
- **> 1000 ppm**: Lüften empfohlen
- **> 2000 ppm**: Gesundheitsrisiko

**DIN EN 13779** (Raumluft-Kategorien):
- **IDA 1** (High): < 800 ppm
- **IDA 2** (Medium): 800-1000 ppm
- **IDA 3** (Moderate): 1000-1400 ppm
- **IDA 4** (Low): > 1400 ppm

**ASHRAE 62.1** (USA):
- **< 1000 ppm**: Gut
- **1000-1500 ppm**: Akzeptabel
- **> 1500 ppm**: Schlecht

### VOC-Quellen im Detail

**Indoor VOC-Quellen** (typische Konzentrationen):

| Quelle | TVOC (ppb) | Hauptkomponenten |
|--------|------------|------------------|
| Frische Farbe | 1000-5000 | Toluol, Xylol, Ethylbenzol |
| Reinigungsmittel | 200-800 | Isopropanol, Ethanol, Terpene |
| Möbel (neu) | 100-500 | Formaldehyd, Acetaldehyd |
| Drucker/Kopierer | 50-200 | Ozon, Toner-Partikel |
| Parfüm/Deo | 500-2000 | Ethanol, Ester, Aldehyde |
| Kochen | 200-1000 | Acrolein, Formaldehyd, Fette |

## 🛠️ Praktische Tipps

### Optimale Sensor-Platzierung

```
OPTIMAL:
✅ Mittlere Raumhöhe (1,5m)
✅ Gute Luftzirkulation
✅ Nicht in direkter Sonneneinstrahlung
✅ Nicht neben Türen/Fenstern
✅ Nicht direkt über Heizung

SCHLECHT:
❌ Direkt an der Decke (zu hoch)
❌ In Ecken (schlechte Zirkulation)
❌ Neben VOC-Quellen (Drucker, etc.)
❌ Im Luftzug (verfälschte Werte)
```

### Langzeit-Betrieb

**Best Practices**:
1. **Dauerbetrieb**: Sensor permanent laufen lassen (bessere Genauigkeit)
2. **Baseline-Backup**: Alle 24h speichern
3. **Jährliche Kalibrierung**: Outdoor-Referenz verwenden
4. **Reinigung**: Staub vom Sensor entfernen (vorsichtig mit Druckluft)

**Stromverbrauch** (bei Dauerbetrieb):
```
Mode 2 (10s): ca. 2,8 mA durchschnittlich
+ ESP32: ca. 80 mA
+ Display: ca. 50 mA
= ca. 133 mA gesamt

Bei 5V: ca. 0,67 W
Pro Tag: ca. 16 Wh
Pro Jahr: ca. 5,8 kWh (ca. 2€ Stromkosten)
```

### Troubleshooting-Checkliste

```
☐ Verkabelung korrekt? (SDA, SCL, VCC, GND)
☐ WAK Pin auf GND?
☐ I2C-Adresse korrekt? (0x5A oder 0x5B)
☐ Library installiert? (Adafruit CCS811)
☐ Aufwärmzeit abgewartet? (20 Min)
☐ Baseline gespeichert? (nach 24h)
☐ Messmodus gesetzt? (Drive Mode 1/2/3)
☐ Stromversorgung stabil? (min. 500 mA)
```

## 📖 Ressourcen & Links

### Datenblätter & Dokumentation
- **CCS811 Datasheet**: [ams.com CCS811](https://ams.com/ccs811)
- **Adafruit CCS811 Library**: [GitHub](https://github.com/adafruit/Adafruit_CCS811)
- **Application Note**: [ams CCS811 App Note](https://ams.com/documents/20143/36005/CCS811_AN000370_2-00.pdf)

### Standards & Richtlinien
- **WHO Air Quality Guidelines**: [who.int](https://www.who.int/news-room/fact-sheets/detail/ambient-(outdoor)-air-quality-and-health)
- **DIN EN 13779**: Lüftung von Nichtwohngebäuden
- **ASHRAE 62.1**: Ventilation for Acceptable Indoor Air Quality

### CYD-Spezifisch
- **CYD Display Config**: `lib/CYD_Display_Config/CYD_Display_Config.h`
- **I2C Pins**: extSDA (GPIO 22), extSCL (GPIO 27)

### Verwandte Beispiele
- **Beispiel 16**: Spirit Level (I2C Einführung)
- **Beispiel 18**: Compass & Orientation (9-DoF Sensor)
- **Beispiel 19**: Laser Distance (ToF Sensor)

### Arduino Libraries
```cpp
// Erforderliche Libraries:
#include <Wire.h>              // I2C (ESP32 Core)
#include <Adafruit_CCS811.h>   // CCS811 Sensor
#include <EEPROM.h>            // Baseline-Speicherung
#include <CYD_Display_Config.h> // CYD Display
```

**Installation**:
```
Arduino IDE -> Tools -> Manage Libraries
- "Adafruit CCS811"
- "Adafruit Unified Sensor"
- "Adafruit Bus IO"
```

## 🎓 Lernziele

Nach Abschluss dieses Beispiels solltest du:

- ✅ **eCO2 vs. echtes CO2** unterscheiden können
- ✅ **TVOC-Konzept** und VOC-Quellen verstehen
- ✅ **MOX-Sensor Technologie** kennen
- ✅ **Baseline-Verwaltung** implementieren können
- ✅ **Luftqualitäts-Standards** (WHO, DIN) anwenden
- ✅ **Dual-Channel Graphen** erstellen
- ✅ **EEPROM-Speicherung** nutzen
- ✅ **Adafruit-Libraries** verwenden
- ✅ **Aufwärmphase-Management** durchführen
- ✅ **Umgebungskompensation** (Temp/Feuchte) verstehen

## 🔜 Nächste Schritte

Weitere Sensor-Beispiele:
- **Beispiel 21**: Umwelt-Multi-Sensor BME680 (Temp, Feuchte, Druck, Gas)
- **Beispiel 22**: Gesten-Sensor APDS-9960 (Berührungslos)
- **Beispiel 23**: Farbsensor TCS34725 (RGB-Erkennung)

---

**Viel Erfolg mit deinem Luftqualitäts-Monitor! 🌬️🌡️**

Bei Fragen: CYD-Academy GitHub Issues
