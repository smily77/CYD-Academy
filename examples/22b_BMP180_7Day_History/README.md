# 📊 Beispiel 22b: BMP180 7-Tage Verlauf

Einfache, übersichtliche Wetterstation mit Fokus auf **Langzeit-Datenerfassung** und **Verlaufsvisualisierung**. Im Vergleich zu Beispiel 22 (komplexe Wettervorhersage mit animierten Icons) konzentriert sich dieses Beispiel auf die klare Darstellung historischer Temperatur- und Luftdruck-Daten über 7 Tage.

![BMP180](https://via.placeholder.com/400x300.png?text=BMP180+7-Tage+Verlauf)

## 🎯 Features

### Datenerfassung
- ⏱️ **Automatische Messung** alle 2 Stunden
- 📅 **7-Tage-Speicher**: 84 Datenpunkte (12 Messungen/Tag × 7 Tage)
- 💾 **Persistente Speicherung** in ESP32 NVS (Preferences)
- 🕐 **NTP-Zeitstempel** für genaue Zeitmarkierungen
- 🔄 **Überlebt Neustarts**: Daten bleiben erhalten

### Visualisierung
- 📈 **Zwei Liniendiagramme**: Temperatur und Luftdruck
- 📊 **Min/Max Markierungen** in jedem Graphen
- 🎨 **Übersichtliches Design** ohne Ablenkung
- 📅 **Zeitachse** mit Datum (ältester/neuester Punkt)
- 🔍 **Detailansicht** für einzelne Messpunkte

### Interaktion
- 👆 **Kurzer Touch**: Manuelle Messung (sofort)
- ⏱️ **Long Press** (>1s): Detailansicht öffnen
- ◀️▶️ **Navigation**: Durch historische Datenpunkte blättern
- 📊 **Live-Update**: Display-Refresh jede Minute

## 🔧 Hardware

### Komponenten
- **ESP32-2432S028R** (CYD - Cheap Yellow Display)
- **BMP180** Barometer/Temperatur-Sensor (oder BMP085)
- WiFi-Verbindung (für NTP-Zeitstempel)

### I2C Verkabelung

```
BMP180         CYD (ESP32)
------         -----------
VCC     →      3.3V
GND     →      GND
SDA     →      GPIO 22 (extSDA)
SCL     →      GPIO 27 (extSCL)
```

## 📚 Bibliotheken

Installiere über den Arduino Library Manager:

1. **Adafruit BMP085 Library** (funktioniert mit BMP180)
   - Autor: Adafruit
   - Einfache API: `readTemperature()`, `readPressure()`, `readAltitude()`

2. **LovyanGFX** (via CYD_Display_Config.h)
   - Hochperformante Grafik-Bibliothek

3. **WiFi** (ESP32 Core integriert)
   - Für NTP-Synchronisierung

4. **Preferences** (ESP32 Core integriert)
   - Persistente Datenspeicherung im Flash (NVS)

5. **Time** (ESP32 Core integriert)
   - NTP und RTC-Funktionen

## 📁 WiFi-Konfiguration

Erstelle eine Datei `Credentials.h` im gleichen Ordner:

```cpp
// Credentials.h
const char* ssid = "DeinWiFiName";
const char* password = "DeinWiFiPasswort";
```

**Wichtig**: Nutze `const char*` Variablen (nicht `#define`).

Die Datei wird automatisch von `.gitignore` ausgeschlossen (`**/Credentials.h`).

## 🚀 Schnellstart

1. **Hardware verbinden**: BMP180 an I2C (SDA=22, SCL=27)
2. **Credentials.h erstellen**: WiFi-Zugangsdaten eintragen
3. **Sketch hochladen**: 22b_BMP180_7Day_History.ino
4. **WiFi-Verbindung**: Wartet auf Verbindung und NTP-Sync
5. **Erste Messung**: Startet automatisch
6. **Warten**: Nach 2 Stunden kommt die nächste automatische Messung

### Zeitzonen-Anpassung

Im Code ist Deutschland (GMT+1 + Sommerzeit) eingestellt:

```cpp
const long gmtOffset_sec = 3600;        // GMT+1
const int daylightOffset_sec = 3600;    // Sommerzeit
```

**Andere Zeitzonen**:
```cpp
// Schweiz/Österreich (gleich wie Deutschland)
const long gmtOffset_sec = 3600;        // GMT+1
const int daylightOffset_sec = 3600;

// London
const long gmtOffset_sec = 0;           // GMT+0
const int daylightOffset_sec = 0;

// New York (EST)
const long gmtOffset_sec = -5 * 3600;   // GMT-5
const int daylightOffset_sec = 3600;

// Tokyo
const long gmtOffset_sec = 9 * 3600;    // GMT+9
const int daylightOffset_sec = 0;
```

## 💡 Bedienung

### Normal-Ansicht
- **Aktuelle Werte**: Große Anzeige oben (Temperatur links, Luftdruck rechts)
- **Zwei Graphen**: Temperaturverlauf (orange) und Druckverlauf (cyan)
- **Statistiken**: Min/Max Werte unter jedem Graphen
- **Datenpunkte**: Anzahl der gespeicherten Messungen (unten rechts)

### Touch-Steuerung
- **Kurzer Touch**: Sofortige neue Messung (unabhängig vom 2h-Intervall)
- **Long Press** (>1s): Öffnet Detailansicht

### Detail-Ansicht
- **Zeitstempel**: Datum und Uhrzeit der Messung
- **Große Werte**: Temperatur und Luftdruck gut lesbar
- **Navigation**:
  - Links Touch: Älterer Datenpunkt
  - Rechts Touch: Neuerer Datenpunkt
  - Mitte Touch: Zurück zur Normal-Ansicht

## 🧠 Technische Details

### Datenspeicherung (ESP32 Preferences / NVS)

Der ESP32 besitzt einen **NVS (Non-Volatile Storage)** Bereich im Flash-Speicher, der Daten auch nach einem Neustart erhält.

#### Was wird gespeichert?

```cpp
struct DataPoint {
  time_t timestamp;      // Unix-Zeitstempel (4 Bytes)
  float temperature;     // Temperatur in °C (4 Bytes)
  float pressure;        // Luftdruck in hPa (4 Bytes)
};                       // Gesamt: 12 Bytes pro Datenpunkt

DataPoint historyData[84];  // 84 × 12 = 1008 Bytes
```

#### Preferences API

```cpp
// Speichern
preferences.begin("weather", false);          // Namespace "weather"
preferences.putInt("dataCount", dataCount);    // Anzahl Datenpunkte
preferences.putInt("currentIdx", currentIndex); // Ring-Buffer Index
preferences.putBytes("historyData", historyData, sizeof(historyData));
preferences.end();

// Laden
preferences.begin("weather", true);  // read-only
dataCount = preferences.getInt("dataCount", 0);
currentIndex = preferences.getInt("currentIdx", 0);
preferences.getBytes("historyData", historyData, sizeof(historyData));
preferences.end();
```

**Vorteile**:
- ✅ Daten überleben Neustarts und Stromausfälle
- ✅ Kein externes EEPROM oder SD-Karte nötig
- ✅ Wear-Leveling (Flash-Schonung) integriert
- ✅ Namespaces vermeiden Konflikte mit anderen Projekten

**Limitierungen**:
- ⚠️ NVS-Partition: Typisch 12-16 KB verfügbar
- ⚠️ Schreibzyklen: Flash hat begrenzte Lebensdauer (~100.000 Zyklen)
- ⚠️ Speicherung alle 2h = ~4 Schreibvorgänge/Tag = ~68 Jahre Lebensdauer 😊

### Ring-Buffer Logik

Um die 7-Tage-Daten zu speichern, wird ein **zirkulärer Buffer** verwendet:

```
Beispiel mit 5 Datenpunkten (statt 84):
┌───┬───┬───┬───┬───┐
│ 0 │ 1 │ 2 │ 3 │ 4 │  Array-Indizes
└───┴───┴───┴───┴───┘

Start:
┌───┬───┬───┬───┬───┐
│   │   │   │   │   │  leer
└───┴───┴───┴───┴───┘
currentIndex = 0, dataCount = 0

Nach 3 Messungen:
┌───┬───┬───┬───┬───┐
│ A │ B │ C │   │   │
└───┴───┴───┴───┴───┘
currentIndex = 3, dataCount = 3

Buffer voll (5 Messungen):
┌───┬───┬───┬───┬───┐
│ A │ B │ C │ D │ E │
└───┴───┴───┴───┴───┘
currentIndex = 0, dataCount = 5

6. Messung überschreibt ältesten Wert:
┌───┬───┬───┬───┬───┐
│ F │ B │ C │ D │ E │  <- F überschreibt A
└───┴───┴───┴───┴───┘
currentIndex = 1, dataCount = 5

7. Messung:
┌───┬───┬───┬───┬───┐
│ F │ G │ C │ D │ E │  <- G überschreibt B
└───┴───┴───┴───┴───┘
currentIndex = 2, dataCount = 5
```

**Implementierung**:
```cpp
historyData[currentIndex] = newDataPoint;
currentIndex = (currentIndex + 1) % MAX_DATA_POINTS;  // Modulo für Wrap-Around
if (dataCount < MAX_DATA_POINTS) {
  dataCount++;
}
```

### NTP-Zeitstempel

**Warum NTP?**
- ESP32 hat keine RTC (Real-Time Clock) mit Batterie
- Nach Neustart ist die Zeit zurückgesetzt
- NTP (Network Time Protocol) synchronisiert die Zeit über WiFi

**Ablauf**:
```cpp
1. WiFi verbinden
2. configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");
3. Warten (2 Sekunden)
4. getLocalTime(&timeinfo)  // Prüfen ob erfolgreich
5. time(nullptr)            // Unix-Timestamp (Sekunden seit 1970)
```

**Unix-Timestamp**:
- Sekunden seit 1. Januar 1970, 00:00 UTC
- Beispiel: `1704067200` = 1. Januar 2024, 00:00 UTC
- Passt in 4 Bytes (`time_t` = `long`)

**Zeitstempel formatieren**:
```cpp
time_t timestamp = historyData[i].timestamp;
struct tm timeinfo;
localtime_r(&timestamp, &timeinfo);  // Konvertierung

char timeStr[64];
strftime(timeStr, sizeof(timeStr), "%d.%m.%Y %H:%M:%S", &timeinfo);
// Output: "14.01.2024 15:30:00"
```

### Messintervall

```cpp
const int MEASUREMENT_INTERVAL = 2 * 60 * 60;  // 2 Stunden = 7200 Sekunden

// Im Loop:
time_t now = time(nullptr);
if (now - lastMeasurement >= MEASUREMENT_INTERVAL) {
  performMeasurement();
  lastMeasurement = now;
}
```

**7-Tage-Berechnung**:
- 7 Tage = 168 Stunden
- Messung alle 2 Stunden = 168 / 2 = **84 Messpunkte**
- Bei 12 Bytes/Punkt = 1008 Bytes Speicher

### Graph-Skalierung

Die Y-Achse wird dynamisch an Min/Max der Daten angepasst:

```cpp
// Min/Max finden
float minVal = 9999.0, maxVal = -9999.0;
for (int i = 0; i < dataCount; i++) {
  float val = isTemperature ? data[i].temperature : data[i].pressure;
  if (val < minVal) minVal = val;
  if (val > maxVal) maxVal = val;
}

// Padding hinzufügen (10% auf jeder Seite)
float range = maxVal - minVal;
minVal -= range * 0.1;
maxVal += range * 0.1;

// Wert zu Y-Koordinate konvertieren
int y = graphY + graphHeight - ((value - minVal) / (maxVal - minVal) * graphHeight);
```

**Beispiel**:
- Temperatur-Daten: 18°C - 24°C
- Mit Padding: 17.4°C - 24.6°C
- Wertebereich: 7.2°C
- Graph-Höhe: 80 Pixel
- 21°C → y = 80 - ((21 - 17.4) / 7.2 * 80) = 80 - 40 = 40 Pixel vom oberen Rand

## 📊 Vergleich mit Beispiel 22

| Feature | Beispiel 22 (komplex) | Beispiel 22b (einfach) |
|---------|----------------------|------------------------|
| **Fokus** | Wettervorhersage | Langzeit-Datenerfassung |
| **Zeitraum** | 24 Stunden | 7 Tage |
| **Datenpunkte** | 288 (5-Min-Intervall) | 84 (2h-Intervall) |
| **Speicher** | RAM (geht bei Neustart verloren) | NVS/Flash (persistent) |
| **Wettervorhersage** | ✅ 6 Zustände | ❌ Keine |
| **Animierte Icons** | ✅ Sonne, Wolken, Regen, Blitz, Schnee | ❌ Keine |
| **Graphen** | 1 (Druckverlauf) | 2 (Druck + Temperatur) |
| **Einheiten-Umschaltung** | ✅ hPa/mmHg, °C/°F, m/ft | ❌ Keine |
| **WiFi/NTP** | ❌ Optional | ✅ Erforderlich |
| **Daten-Persistenz** | ❌ Nein | ✅ Ja (NVS) |
| **Komplexität (LoC)** | ~780 | ~650 |
| **Use Case** | Echtzeitwetter, Vorhersage | Trends, historische Analyse |

**Wann welches Beispiel?**
- **Beispiel 22**: Wenn du eine "richtige" Wetterstation mit Vorhersage und schöner Visualisierung willst
- **Beispiel 22b**: Wenn du Langzeit-Trends analysieren willst (z.B. saisonale Veränderungen)

## 🔍 Troubleshooting

### WiFi verbindet nicht
```
Symptom: Display bleibt bei "Verbinde WiFi..." hängen

Lösung:
1. Credentials.h prüfen (ssid/password korrekt?)
2. SSID darf keine Sonderzeichen haben (ä, ö, ü vermeiden)
3. 2.4 GHz WiFi erforderlich (5 GHz wird nicht unterstützt)
4. Timeout: Nach 30 Versuchen fährt das System ohne WiFi fort
```

### NTP-Sync schlägt fehl
```
Symptom: "Zeit-Sync fehlgeschlagen"

Lösung:
1. Router-Firewall: NTP-Port 123 (UDP) muss offen sein
2. Alternativer NTP-Server:
   - time.google.com
   - time.nist.gov
   - de.pool.ntp.org (für Deutschland)
3. Zeitzone korrekt? (gmtOffset_sec)
4. Ohne NTP: Zeitachse fehlt, aber Messung funktioniert weiter
```

### Daten gehen verloren
```
Symptom: Nach Neustart sind Daten weg

Mögliche Ursachen:
1. Preferences nicht richtig gespeichert
   → Serial Monitor prüfen: "Daten gespeichert" muss erscheinen
   
2. NVS-Partition voll
   → Mit Partition Tool prüfen: Tools > Partition Scheme > "Default 4MB with spiffs"
   
3. Flash-Fehler (selten)
   → preferences.clear() im setup() und neu starten
```

### BMP180 nicht gefunden
```
Symptom: Roter Bildschirm "BMP180 nicht gefunden"

Verkabelung prüfen:
┌─────────────┬──────────┐
│ BMP180 Pin  │ CYD Pin  │
├─────────────┼──────────┤
│ VCC         │ 3.3V     │  ← NICHT 5V!
│ GND         │ GND      │
│ SDA         │ GPIO 22  │
│ SCL         │ GPIO 27  │
└─────────────┴──────────┘

Test:
1. I2C-Scanner ausführen (Example > Wire > i2c_scanner)
2. BMP180 sollte an Adresse 0x77 erscheinen
3. Wenn nicht: Verkabelung/Sensor defekt
```

### Graph zeigt komische Werte
```
Symptom: Temperatur zeigt 85°C oder -40°C

Ursache: BMP180 Lesefehler

Lösung:
1. Sensor-Reset:
   digitalWrite(extSDA, LOW);
   delay(10);
   Wire.begin(extSDA, extSCL);
   
2. Überspring fehlerhafte Messungen:
   if (temp < -30 || temp > 50) return;  // Plausibilitätsprüfung
```

## 💡 Erweiterungs-Ideen

### 1. Export-Funktion (CSV über Serial)

Daten im CSV-Format ausgeben für Excel/LibreOffice:

```cpp
void exportCSV() {
  Serial.println("Timestamp,Temperature,Pressure");
  
  for (int i = 0; i < dataCount; i++) {
    DataPoint& dp = historyData[i];
    
    // Unix-Timestamp
    Serial.print(dp.timestamp);
    Serial.print(",");
    
    // Temperatur
    Serial.print(dp.temperature, 2);
    Serial.print(",");
    
    // Luftdruck
    Serial.println(dp.pressure, 2);
  }
}

// Aufruf bei bestimmtem Touch-Pattern oder Serial-Befehl
```

### 2. SD-Karten Backup

Langzeit-Archivierung auf microSD:

```cpp
#include <SD.h>
#include <SPI.h>

void saveToSD() {
  File file = SD.open("/weather_history.csv", FILE_WRITE);
  
  for (int i = 0; i < dataCount; i++) {
    char line[64];
    sprintf(line, "%ld,%.2f,%.2f\n", 
            historyData[i].timestamp,
            historyData[i].temperature,
            historyData[i].pressure);
    file.print(line);
  }
  
  file.close();
}
```

### 3. Konfigurierbares Messintervall

Touch-Menu für 15min / 30min / 1h / 2h Intervalle:

```cpp
enum MeasurementInterval {
  INTERVAL_15MIN = 15 * 60,
  INTERVAL_30MIN = 30 * 60,
  INTERVAL_1HOUR = 60 * 60,
  INTERVAL_2HOURS = 2 * 60 * 60
};

MeasurementInterval currentInterval = INTERVAL_2HOURS;

// Touch-Menu zum Umschalten
void drawIntervalMenu() {
  // [15min] [30min] [1h] [2h]
  // Aktuell gewähltes Intervall hervorheben
}
```

### 4. Alarm bei Druck-Extremwerten

Warnung bei plötzlichen Druckänderungen (Unwetter):

```cpp
void checkAlarm() {
  if (dataCount < 2) return;
  
  float pressureChange = currentPressure - historyData[dataCount-2].pressure;
  
  // Schneller Druckabfall (>5 hPa in 2h) → Sturm
  if (pressureChange < -5.0) {
    lcd.fillRect(0, 0, 320, 30, TFT_RED);
    lcd.setTextColor(TFT_WHITE, TFT_RED);
    lcd.drawString("WARNUNG: Starker Druckabfall!", 160, 15);
    
    // Optional: Piezo-Summer oder LED
  }
}
```

### 5. Vergleich mit Vorwoche

Zeige Differenz zu vor 7 Tagen:

```cpp
void drawWeekComparison() {
  if (dataCount < 84) return;  // Nicht genug Daten
  
  float tempDiff = currentTemp - historyData[0].temperature;
  float pressureDiff = currentPressure - historyData[0].pressure;
  
  lcd.setTextColor(tempDiff > 0 ? TFT_RED : TFT_BLUE);
  lcd.drawString(String(tempDiff, 1) + " C vs. vor 7d", 160, 50);
  
  lcd.setTextColor(pressureDiff > 0 ? TFT_GREEN : TFT_YELLOW);
  lcd.drawString(String(pressureDiff, 1) + " hPa vs. vor 7d", 160, 70);
}
```

### 6. WebServer mit Chart.js

Daten per Web-Interface abrufen:

```cpp
#include <WebServer.h>

WebServer server(80);

void handleRoot() {
  String html = "<html><head>";
  html += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>";
  html += "</head><body>";
  html += "<canvas id='chart'></canvas>";
  html += "<script>";
  html += "const ctx = document.getElementById('chart');";
  html += "new Chart(ctx, { type: 'line', data: { labels: [";
  
  // Zeitstempel
  for (int i = 0; i < dataCount; i++) {
    html += "'" + String(historyData[i].timestamp) + "'";
    if (i < dataCount - 1) html += ",";
  }
  
  html += "], datasets: [{ label: 'Temperatur', data: [";
  
  // Temperaturdaten
  for (int i = 0; i < dataCount; i++) {
    html += String(historyData[i].temperature, 2);
    if (i < dataCount - 1) html += ",";
  }
  
  html += "] }] } });";
  html += "</script></body></html>";
  
  server.send(200, "text/html", html);
}

void setup() {
  // ...
  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
  // ...
}
```

### 7. MQTT für Home Assistant

Integration ins Smart Home:

```cpp
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient mqtt(espClient);

void publishToMQTT() {
  if (!mqtt.connected()) {
    mqtt.connect("CYD_Weather");
  }
  
  // Home Assistant Auto-Discovery Format
  String topic = "homeassistant/sensor/cyd_temp/state";
  String payload = "{\"temperature\":" + String(currentTemp, 2) + 
                   ",\"pressure\":" + String(currentPressure, 2) + "}";
  
  mqtt.publish(topic.c_str(), payload.c_str());
}
```

### 8. BME280 Upgrade (+ Luftfeuchtigkeit)

Erweitere auf BME280 für zusätzliche Luftfeuchtigkeit:

```cpp
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;

struct DataPoint {
  time_t timestamp;
  float temperature;
  float pressure;
  float humidity;  // ← NEU
};

void performMeasurement() {
  // ...
  historyData[currentIndex].humidity = bme.readHumidity();
}

// Dritter Graph für Luftfeuchtigkeit
void drawHumidityGraph() {
  // 0-100% Skalierung
}
```

## 📖 Meteorologie-Grundlagen

### Barometrische Höhenformel

Die Luftdruck-Messung ermöglicht Höhenberechnung:

```
h ≈ 44330 × (1 - (P / P₀)^0.1903)

Wobei:
- h  = Höhe über dem Meeresspiegel [m]
- P  = Gemessener Luftdruck [hPa]
- P₀ = Luftdruck auf Meereshöhe [hPa] (Standard: 1013.25)
```

**Beispiel**:
- Meereshöhe: P = 1013 hPa → h = 0 m
- München (ca. 520m): P ≈ 955 hPa
- Zugspitze (2962m): P ≈ 700 hPa

### Druckeinheiten

| Einheit | Faktor | Typischer Bereich |
|---------|--------|-------------------|
| hPa (Hektopascal) | 1.0 | 950 - 1050 hPa |
| mbar (Millibar) | 1.0 | 950 - 1050 mbar (identisch mit hPa) |
| mmHg (Torr) | 0.750062 | 713 - 788 mmHg |
| inHg (Zoll) | 0.02953 | 28 - 31 inHg |
| atm (Atmosphären) | 0.000987 | 0.94 - 1.04 atm |

**Umrechnung**:
```
1013.25 hPa = 1013.25 mbar = 760 mmHg = 29.92 inHg = 1 atm
```

### Langzeit-Trends

**7-Tage-Beobachtungen**:
- **Stabile Hochdrucklage**: Druck >1020 hPa über mehrere Tage → Schönwetter
- **Tiefdruckgebiet**: Druck <990 hPa → Schlechtwetter
- **Sinusförmiger Verlauf**: Tages-/Nacht-Schwankungen (±2-3 hPa)
- **Jahreszeiten**: Winter höherer Durchschnittsdruck als Sommer

**Temperatur-Trends**:
- **Tag/Nacht-Zyklus**: 5-10°C Schwankung
- **Wochentrends**: Kalt-/Warmfronten erkennbar
- **Saisonale Muster**: Wärmster Monat Juli, kältester Januar

## 📚 Weiterführende Informationen

### BMP180 Datenblatt
- Hersteller: Bosch Sensortec
- Druckbereich: 300 - 1100 hPa
- Temperaturbereich: -40°C bis +85°C
- I2C-Adresse: 0x77 (Standard)
- Stromverbrauch: 5 μA (Low-Power)

### ESP32 NVS
- [ESP-IDF Preferences Documentation](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/preferences.html)
- Namespace: Max. 15 Zeichen
- Key: Max. 15 Zeichen
- Maximale Größe: Abhängig von Partition Table (typisch 12-16 KB)

### NTP Protocol
- RFC 5905: Network Time Protocol Version 4
- Genauigkeit: Typisch < 100ms über Internet
- Hierarchie: Stratum 0 (Atomuhr) bis Stratum 16

## ⚖️ Lizenz

MIT License - Frei verwendbar für private und kommerzielle Projekte.

---

**CYD-Academy** - Lerne ESP32-Entwicklung mit praktischen Beispielen!  
*Beispiel 22b: Fokus auf Langzeit-Datenerfassung und persistente Speicherung*
