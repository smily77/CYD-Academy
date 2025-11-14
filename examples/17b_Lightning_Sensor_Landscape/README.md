# ⚡ Beispiel 17b: Lightning Sensor - Landscape Layout

## 📝 Was macht dieses Beispiel?

Eine **optimierte Landscape-Version** der Blitzwetter-Anzeige mit verbessertem Layout für horizontale Darstellung!

**Unterschiede zu Beispiel 17:**
- ✅ **Landscape-Modus** - Display-Rotation 1 (320x240 statt 240x320)
- ✅ **Optimiertes 2-Spalten Layout** - Radar links, Info-Panel rechts
- ✅ **Bessere Platznutzung** - Kompaktere Darstellung
- ✅ **Größerer Radar** - Mehr Platz für die Visualisierung
- ✅ **Übersichtlicheres Info-Panel** - Vertikale Anordnung rechts

**Features (identisch zu 17):**
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
- **Touch links:** Statistik zurücksetzen
- **Touch rechts:** Sensitivität ändern (0-7)
- **Automatisch:** Blitze werden automatisch erkannt und angezeigt

---

## 🎨 Layout-Übersicht (Landscape 320x240)

```
┌──────────────────────────────────────────────┐
│          LIGHTNING RADAR            17b      │ Header (25px)
├──────────────────────┬───────────────────────┤
│                      │   Distance:           │
│       Radar          │      25 km            │
│    Visualisierung    │                       │
│                      │   Status:             │
│      ●  ●  ●        │      Danger           │
│     ●   ⚡  ●       │                       │
│      ●  ●  ●        │   Count:              │
│                      │      7                │
│    (200x190px)       │                       │
│                      │   Last:               │
│                      │      2m ago           │
│                      │                       │
│                      │   Noise: 3            │
│                      │   Disturb: 1          │
│                      │  (120x190px)          │
├──────────────────────┴───────────────────────┤
│ Sensitivity: 2/7    Left:Reset  Right:Sens  │ Footer (25px)
└──────────────────────────────────────────────┘
```

### Layout-Bereiche:

- **Header (0-25px):** Titel und Version
- **Radar-Bereich (0-200px Breite):** Konzentrische Kreise, Blitz-Positionen
- **Info-Panel (200-320px Breite):** Alle Daten vertikal angeordnet
- **Footer (215-240px):** Einstellungen und Touch-Hilfe

---

## 🎯 Lernziele

Gleiche Lernziele wie Beispiel 17, plus:
1. **Landscape-Layout** - Optimierung für horizontale Darstellung
2. **Responsive UI** - Anpassung an verschiedene Display-Orientierungen
3. **Multi-Column Layout** - Effektive Nutzung des Breitbildformats

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
└─ IRQ    →    GPIO 35 (optional)
```

**ACHTUNG:** Sensor ist **3.3V**!

**I2C Pins (CYD Standard):**
- `extSDA = GPIO 22`
- `extSCL = GPIO 27`

**Interrupt Pin (optional):**
- `IRQ_PIN = GPIO 35` (Input-only Pin, ideal für Interrupts)

### 3. I2C-Adresse

**Standard:** `0x03`

**Alternative:** `0x02` oder `0x01` (über Adress-Jumper am Modul)

Falls der Sensor nicht erkannt wird:
1. I2C Scanner läuft automatisch beim Start
2. Schau dir die Serial Monitor Ausgabe an
3. Ändere `AS3935_I2C_ADDR` in Zeile 85 falls nötig

### 4. Kompilieren & Upload

1. Arduino IDE öffnen
2. Sketch öffnen: `examples/17b_Lightning_Sensor_Landscape/17b_Lightning_Sensor_Landscape.ino`
3. Board: "ESP32 Dev Module"
4. Port: Entsprechenden COM-Port wählen
5. Upload-Button drücken
6. Serial Monitor öffnen (115200 Baud)

---

## 💻 Code-Struktur

### Hauptunterschiede zu Beispiel 17

#### 1. Display-Rotation
```cpp
// Portrait (17):
lcd.setRotation(0);  // 240x320

// Landscape (17b):
lcd.setRotation(1);  // 320x240
```

#### 2. Layout-Konstanten
```cpp
// Layout für Landscape (320x240)
#define HEADER_HEIGHT 25
#define FOOTER_HEIGHT 25
#define MAIN_HEIGHT (240 - HEADER_HEIGHT - FOOTER_HEIGHT)

// Radar (linke Seite)
#define RADAR_AREA_WIDTH 200
#define RADAR_CENTER_X 100
#define RADAR_CENTER_Y (HEADER_HEIGHT + MAIN_HEIGHT / 2)
#define RADAR_MAX_RADIUS 80

// Info-Panel (rechte Seite)
#define INFO_PANEL_X 200
#define INFO_PANEL_WIDTH (320 - INFO_PANEL_X)
```

#### 3. Radar-Zeichnung
```cpp
void drawRadar() {
  // Radar nimmt nur linke Hälfte ein
  lcd.fillRect(0, HEADER_HEIGHT, RADAR_AREA_WIDTH, MAIN_HEIGHT, COLOR_RADAR_BG);

  // Radar zentriert im linken Bereich
  // Center: (100, 120) statt (120, 140)
  // Radius: 80 statt 90

  // Trennlinie rechts
  lcd.drawLine(RADAR_AREA_WIDTH, HEADER_HEIGHT,
               RADAR_AREA_WIDTH, 240 - FOOTER_HEIGHT,
               COLOR_RADAR_GRID);
}
```

#### 4. Info-Panel (komplett neu)
```cpp
void drawInfoPanel() {
  // Rechter Bereich (200-320px)
  lcd.fillRect(INFO_PANEL_X, HEADER_HEIGHT, INFO_PANEL_WIDTH, MAIN_HEIGHT, COLOR_BG);

  // Vertikale Anordnung:
  int x = INFO_PANEL_X + 10;  // 210px
  int y = HEADER_HEIGHT + 10; // Start bei 35px

  // Distance (große Anzeige)
  // Status (farbcodiert)
  // Count (Anzahl Blitze)
  // Last (Zeit seit letztem Blitz)
  // Statistik (Noise, Disturb)
}
```

#### 5. Touch-Bereiche
```cpp
void handleTouch() {
  // Portrait (17): Oben/Unten
  if (y < 100) { /* Reset */ }
  else if (y > 220) { /* Sensitivity */ }

  // Landscape (17b): Links/Rechts
  if (x < 160) { /* Reset */ }
  else { /* Sensitivity */ }
}
```

---

## 🚀 Verwendung

### Normale Bedienung

1. **Display einschalten:**
   - Splash Screen wird angezeigt
   - I2C Scanner sucht nach Sensor
   - Sensor wird initialisiert

2. **Blitze beobachten:**
   - Radar zeigt Entfernungsringe (10, 20, 30, 40km)
   - Erkannte Blitze erscheinen als farbige Punkte
   - Info-Panel zeigt aktuelle Daten

3. **Statistik zurücksetzen:**
   - Touch auf linke Bildschirmhälfte
   - Alle Zähler werden auf 0 gesetzt
   - Historie wird gelöscht

4. **Empfindlichkeit ändern:**
   - Touch auf rechte Bildschirmhälfte
   - Sensitivity: 0-7 (höher = empfindlicher)
   - Hilfreich bei zu vielen Störungen

### Serielle Ausgabe

```
=== Lightning Sensor Demo (Landscape) ===
Initializing I2C (SDA=22, SCL=27)...

=== I2C Scanner ===
Scanning I2C bus (SDA=22, SCL=27)...
✓ Device found at address 0x03
  → AS3935_ADD3 (A0=high, A1=high)

✓ Found 1 device(s)
==================

Using interrupt mode on pin 35
Attempting to initialize AS3935 at address 0x03...
✓ AS3935 initialized successfully!
✓ AS3935 configured!
  Sensitivity: 2/7
  I2C Address: 0x03
  Mode: Indoor

Ready! Waiting for lightning...

⚡ LIGHTNING DETECTED! ⚡
Distance: 25 km
Energy: 4567
Total Count: 1
```

---

## 🎨 Visuelle Features

### Farbcodierung nach Entfernung

| Entfernung | Farbe | Status |
|-----------|--------|--------|
| 0-10 km | 🔴 Rot | CRITICAL! |
| 10-20 km | 🟠 Orange | Danger |
| 20-30 km | 🟡 Gelb | Warning |
| 30-40 km | 🟢 Grün | Caution |

### Blitz-Animation

Bei Blitz-Erkennung:
1. **Rahmen blinkt** (200ms)
2. **Blitz-Symbol** in Radar-Mitte (500ms)
3. **Pulsierender Punkt** auf Radar (5 Sekunden)
4. **Linie zum Zentrum** (während Highlight)

### Historische Blitze

- Letzte 20 Blitze werden gespeichert
- Fade-out über 5 Minuten
- Ältere Blitze werden dunkler
- Automatische Bereinigung

---

## 🔧 Troubleshooting

### Problem: Sensor wird nicht erkannt

**Symptome:** "AS3935 Error!" auf Display

**Lösungen:**
1. **I2C Scanner Ausgabe prüfen:**
   ```
   Serial Monitor öffnen (115200 Baud)
   I2C Scanner zeigt gefundene Adressen an
   ```

2. **I2C-Adresse anpassen:**
   ```cpp
   // In Zeile 85 ändern:
   #define AS3935_I2C_ADDR AS3935_ADD1  // für 0x01
   // oder
   #define AS3935_I2C_ADDR AS3935_ADD2  // für 0x02
   ```

3. **Verkabelung prüfen:**
   - SDA → GPIO 22
   - SCL → GPIO 27
   - VCC → 3.3V (NICHT 5V!)
   - GND → GND
   - IRQ → GPIO 35 (optional)

4. **Pull-up Widerstände:**
   - Die meisten AS3935-Module haben bereits Pull-ups
   - Falls nicht: 4.7kΩ Widerstände zwischen SDA/SCL und 3.3V

### Problem: Zu viele Störungen

**Symptome:** Hohe Noise/Disturber-Zähler, keine echten Blitze

**Lösungen:**
1. **Empfindlichkeit reduzieren:**
   - Touch rechts um Sensitivity zu senken
   - Empfehlung: Start mit 2-3

2. **Noise Floor erhöhen:**
   ```cpp
   // In setup(), Zeile ~271:
   sensor.setNoiseFloorLvl(4);  // War: 2, höher = mehr Filterung
   ```

3. **Indoor/Outdoor Modus:**
   ```cpp
   // Indoor (mehr Filterung):
   sensor.setIndoors();

   // Outdoor (empfindlicher):
   sensor.setOutdoors();
   ```

4. **Störquellen entfernen:**
   - Abstand zu Motoren, Schaltnetzteilen
   - Abstand zu WiFi-Routern
   - Keine langen ungeschirmten Kabel

### Problem: Display-Orientierung falsch

**Symptome:** Anzeige ist auf dem Kopf oder seitlich

**Lösung:**
```cpp
// Zeile 251, verschiedene Rotationen testen:
lcd.setRotation(1);  // Standard Landscape
lcd.setRotation(3);  // Landscape umgedreht
```

### Problem: Touch reagiert nicht korrekt

**Symptome:** Touch löst falsche Aktion aus

**Lösung:**
- Touch-Kalibrierung könnte nötig sein
- In `CYD_Display_Config.h` Touch-Parameter anpassen
- Touch-Bereiche in Code prüfen (Zeile ~479)

---

## 💡 Erweiterungsideen

### 1. Automatische Display-Helligkeit

**Konzept:** Helligkeit bei Blitzen erhöhen

```cpp
void handleLightningInterrupt() {
  // ... bestehender Code

  if (intSource == 3) {  // Blitz erkannt
    // Helligkeit kurz erhöhen
    lcd.setBrightness(255);

    // Nach 3 Sekunden wieder normal
    // (in loop() mit Timer implementieren)
  }
}
```

### 2. Blitz-Richtungs-Erkennung

**Konzept:** Echte Winkel statt zufälliger Position

```cpp
// Benötigt 2-3 Sensoren in verschiedenen Positionen
// Triangulation der Blitz-Position
// Zeige echte Himmelsrichtung auf Radar
```

### 3. Historisches Blitz-Diagramm

**Konzept:** Zeitlicher Verlauf als Graph

```cpp
void drawLightningHistory() {
  // Unterhalb des Radars
  // X-Achse: Zeit (letzte 30 Minuten)
  // Y-Achse: Anzahl Blitze pro Minute
  // Balkendiagramm oder Liniengraph
}
```

### 4. Blitz-Dichte-Karte

**Konzept:** Heatmap der Blitz-Aktivität

```cpp
// Teile Radar in 8x8 Grid
// Zähle Blitze pro Sektor
// Färbe Sektoren basierend auf Aktivität
// Zeigt lokale Gewitter-Zentren
```

### 5. Alarm-System

**Konzept:** Warnung bei nahen Blitzen

```cpp
// RGB LED rot bei Blitz < 10km
// Buzzer (falls vorhanden)
// Blinkende Display-Ränder
// MQTT Nachricht an Smart Home
```

### 6. SD-Karten Logging

**Konzept:** Langzeit-Aufzeichnung aller Blitze

```cpp
#include <SD.h>

void logLightning(uint8_t distance, uint32_t energy) {
  File dataFile = SD.open("lightning.csv", FILE_APPEND);

  if (dataFile) {
    // Zeitstempel,Entfernung,Energie
    dataFile.printf("%lu,%d,%lu\n", millis(), distance, energy);
    dataFile.close();
  }
}
```

### 7. Mehrere Info-Seiten

**Konzept:** Blättern zwischen verschiedenen Ansichten

```cpp
// Seite 1: Aktueller Blitz (wie jetzt)
// Seite 2: Statistiken & Graphen
// Seite 3: Sensor-Konfiguration
// Touch oben/unten zum Blättern
```

---

## 📚 Weiterführende Themen

### Vergleich: Portrait vs. Landscape

| Aspekt | Portrait (17) | Landscape (17b) |
|--------|---------------|-----------------|
| **Auflösung** | 240x320 | 320x240 |
| **Radar-Größe** | 90px Radius | 80px Radius |
| **Layout** | Vertikal gestapelt | 2-Spalten |
| **Info-Dichte** | Kompakter | Verteilter |
| **Lesbarkeit** | Gut für viel Text | Gut für Überblick |
| **Touch-Bereiche** | Oben/Unten | Links/Rechts |
| **Beste Nutzung** | Tisch/vertikal | Wand/horizontal |

**Empfehlung:**
- **Portrait (17):** Wenn du viele Details auf einmal sehen willst
- **Landscape (17b):** Wenn du den Radar im Fokus haben möchtest

### AS3935 Sensor-Details

**Erkennungsbereich:**
- Distanz: 1-40 km (in 14 Stufen)
- Update-Rate: ~1 Sekunde
- Blitz-Typen: Cloud-to-Ground, Cloud-to-Cloud

**Störungs-Filterung:**
- Frequenz-basiert (500kHz)
- Pattern-Erkennung
- Energie-Schwellwerte
- Zeitliche Korrelation

**Kalibrierung:**
- Indoor: Mehr Filterung, weniger Reichweite
- Outdoor: Weniger Filterung, mehr Reichweite
- Automatische Antennen-Tuning

---

## 🛠️ Praktische Tipps

### Optimale Platzierung

1. **Sensor-Position:**
   - Möglichst hoch platzieren
   - Freie Sicht zum Himmel (wenn möglich)
   - Abstand zu Störquellen

2. **Display-Ausrichtung:**
   - Landscape: Ideal für Wandmontage
   - Gut lesbar aus Entfernung
   - Radar-Fokus

3. **Kabel-Management:**
   - Kurze I2C-Kabel (< 30cm empfohlen)
   - Verdrillte Kabel für SDA/SCL
   - IRQ-Leitung kann länger sein

### Performance-Optimierung

1. **Frame-Rate:**
   ```cpp
   // Aktuell: 20 FPS (delay(50))
   // Für weniger Flackern: delay(33)  // 30 FPS
   // Für Energie-Sparen: delay(100)   // 10 FPS
   ```

2. **Selektives Redraw:**
   ```cpp
   // Nur ändern was sich ändert
   // Nicht bei jedem Frame alles neu zeichnen
   // Radar-Hintergrund cachen
   ```

3. **Double-Buffering:**
   ```cpp
   // LovyanGFX unterstützt Sprites
   // Render in Sprite, dann auf Display
   // Eliminiert Flackern komplett
   ```

---

## 📖 Ressourcen & Links

### Datenblätter & Dokumentation
- [AS3935 Datasheet (AMS)](https://ams.com/as3935)
- [DFRobot SEN0290 Wiki](https://wiki.dfrobot.com/Gravity_Lightning_Sensor_SKU_SEN0290)
- [DFRobot AS3935 Library](https://github.com/DFRobot/DFRobot_AS3935)

### CYD-Spezifisch
- CYD Display Config: `MyLGFXConfigs/CYD_Display_Config.h`
- I2C Pins: extSDA (GPIO 22), extSCL (GPIO 27)
- Standard-Resolution: 320x240 (ILI9341)

### Verwandte Beispiele
- **Beispiel 17:** Lightning Sensor (Portrait-Version)
- **Beispiel 18:** Compass Orientation (anderer I2C Sensor)
- **Beispiel 20:** Air Quality CCS811 (I2C mit Baseline-Management)

---

## 📊 Vergleich mit anderen Sensor-Beispielen

| Feature | Beispiel 17/17b | Beispiel 18 | Beispiel 20 |
|---------|----------------|-------------|-------------|
| **Sensor** | AS3935 Lightning | BNO055 Compass | CCS811 Air Quality |
| **I2C-Adresse** | 0x03 (0x01-0x03) | 0x28/0x29 | 0x5A/0x5B |
| **Update-Rate** | ~1s | ~100Hz | ~1s |
| **Visualisierung** | Radar | 3D Kompass | Gauge Charts |
| **Komplexität** | Mittel | Hoch | Hoch |
| **Kalibrierung** | Auto | Manuell | Auto + EEPROM |

---

## 🎓 Lernziele

Nach Abschluss dieses Beispiels solltest du:
- ✅ **Landscape-Layouts** für CYD erstellen können
- ✅ **AS3935 Lightning Sensor** über I2C ansteuern können
- ✅ **Multi-Column Layouts** effektiv nutzen können
- ✅ **Responsive UI-Design** für verschiedene Orientierungen verstehen
- ✅ **Radar-Visualisierungen** implementieren können
- ✅ **Historische Daten** mit Fade-out darstellen können
- ✅ **Touch-Interaktion** in verschiedenen Orientierungen umsetzen können
- ✅ **I2C-Debugging** mit Scanner durchführen können

---

## 🔜 Nächste Schritte

Weitere Beispiele mit optimierten Layouts:
- **Beispiel 18:** Compass Orientation - 3D Visualisierung
- **Beispiel 19:** Laser Distance - ToF Sensor mit Graph
- **Beispiel 20:** Air Quality - CCS811 mit FRAM Baseline

Experimentiere mit:
- Eigenen Layouts (Portrait, Landscape, verschiedene Größen)
- Verschiedenen Visualisierungen (Graphen, Gauges, etc.)
- Touch-Gesten (Swipe, Long-Press, etc.)

---

**Viel Erfolg mit deiner Blitzwetter-Station! ⚡**

Bei Fragen: CYD-Academy GitHub Issues
