# Sensor Examples

Dieses Verzeichnis enthält **fortgeschrittene Beispiele** mit I2C/SPI-Sensoren für die CYD-Academy. Diese Beispiele demonstrieren die Integration externer Hardware-Sensoren mit dem ESP32 Display.

## 📋 Übersicht

Die Sensor-Beispiele sind nach **Sensoren/Hardware** organisiert und zeigen vollständige Implementierungen mit professioneller Visualisierung.

### 🎯 Voraussetzungen

Für diese Beispiele benötigst du:

- ✅ **Grundlagen-Kenntnisse** aus den [Basic Examples](../examples/)
- ✅ **Externe Sensoren** (siehe jeweiliges Beispiel)
- ✅ **I2C-Verkabelung** (SDA: GPIO 22, SCL: GPIO 27)
- ✅ **Sensor-spezifische Libraries** (siehe README der Beispiele)

## 📚 Sensor-Beispiele

### Bewegungssensoren & IMU

#### 16 - Spirit Level (Wasserwaage)
- **Sensor**: BNO055 oder MPU6050 (9-DOF IMU)
- **Funktion**: Digitale Wasserwaage mit Neigungsmessung
- **Features**:
  - Echtzeit-Neigungsanzeige
  - Blasen-Animation
  - Präzisions-Kalibrierung
- **Varianten**:
  - `16_Spirit_Level` - Klassische Wasserwaage
  - `16b_Spirit_Level_Maze` - Labyrinth-Spiel mit Neigungssteuerung

**Hardware:**
```
BNO055/MPU6050    CYD (ESP32)
--------------    -----------
VCC     ------>   3.3V
GND     ------>   GND
SDA     ------>   GPIO 22 (extSDA)
SCL     ------>   GPIO 27 (extSCL)
```

---

### Wettersensoren

#### 17 - Lightning Sensor (Blitzdetektor)
- **Sensor**: AS3935 Franklin Lightning Sensor
- **Funktion**: Erkennung von Blitzen und Gewittern
- **Features**:
  - Distanzschätzung (0-40 km)
  - Störungsfilterung
  - Echtzeit-Blitzereignisse
  - Alarm-System
- **Varianten**:
  - `17_Lightning_Sensor` - Portrait-Modus
  - `17b_Lightning_Sensor_Landscape` - Landscape-Modus

**Hardware:**
```
AS3935            CYD (ESP32)
------            -----------
VCC     ------>   3.3V (WICHTIG: 5V kann Sensor zerstören!)
GND     ------>   GND
SDA     ------>   GPIO 22
SCL     ------>   GPIO 27
IRQ     ------>   GPIO 35 (optional)
```

#### 22 - Weather Station (Wetterstation)
- **Sensor**: BMP180 Barometric Pressure Sensor
- **Funktion**: Luftdruck, Temperatur, Höhenmessung
- **Features**:
  - Luftdruck-Anzeige (hPa/mbar)
  - Temperatur-Messung
  - Höhen-Berechnung
  - Wettertrend-Vorhersage
- **Varianten**:
  - `22_Weather_Station_BMP180` - Live-Anzeige
  - `22b_BMP180_7Day_History` - 7-Tage Verlauf mit Grafik

**Hardware:**
```
BMP180            CYD (ESP32)
------            -----------
VCC     ------>   3.3V
GND     ------>   GND
SDA     ------>   GPIO 22
SCL     ------>   GPIO 27
```

---

### Orientierungssensoren

#### 18 - Compass Orientation (Kompass)
- **Sensor**: BNO055 oder HMC5883L (Magnetometer)
- **Funktion**: Digitaler Kompass mit Orientierung
- **Features**:
  - 360° Himmelsrichtungs-Anzeige
  - Nord-Markierung
  - Gradzahl-Anzeige
  - Kalibrierungs-Status
  - Smooth Rotation

**Hardware:**
```
BNO055            CYD (ESP32)
------            -----------
VCC     ------>   3.3V
GND     ------>   GND
SDA     ------>   GPIO 22
SCL     ------>   GPIO 27
```

---

### Distanzsensoren

#### 19 - Laser Distance (Laser-Entfernungsmessung)
- **Sensor**: VL53L0X oder R50-C ToF (Time-of-Flight)
- **Funktion**: Präzise Distanzmessung mit Laser
- **Features**:
  - Messbereich: 30-2000mm (je nach Sensor)
  - mm-Genauigkeit
  - Echtzeit-Visualisierung
  - Balken-Diagramm
  - Min/Max-Tracking

**Hardware:**
```
VL53L0X/R50-C     CYD (ESP32)
-------------     -----------
VCC     ------>   3.3V
GND     ------>   GND
SDA     ------>   GPIO 22
SCL     ------>   GPIO 27
```

---

### Luftqualitätssensoren

#### 20 - Air Quality CCS811
- **Sensor**: CCS811 eCO2 & TVOC Sensor
- **Funktion**: Luftqualitäts-Überwachung
- **Features**:
  - eCO2 (equivalent CO2) Messung (400-8192 ppm)
  - TVOC (Total Volatile Organic Compounds) (0-1187 ppb)
  - Luftqualitäts-Index
  - Baseline-Management (EEPROM)
  - Farb-codierte Warnstufen
  - Alarm-System

**Hardware:**
```
CCS811            CYD (ESP32)
------            -----------
VCC     ------>   3.3V
GND     ------>   GND
SDA     ------>   GPIO 22
SCL     ------>   GPIO 27
WAK     ------>   GND (Wake-up, dauerhaft aktiv)
```

**Wichtig:**
- Sensor benötigt **48h Einlaufzeit** für genaue Werte
- Baseline wird in EEPROM gespeichert
- Optimal für Indoor-Luftqualität

---

### Gestensensoren

#### 21 - Gesture Sensor MGC3130
- **Sensor**: MGC3130 3D Gesture Controller
- **Funktion**: Berührungslose Gestensteuerung
- **Features**:
  - 3D-Positionserkennung (X, Y, Z)
  - 6 Gesten-Typen:
    - Swipe (Links, Rechts, Oben, Unten)
    - Kreis (Clockwise, Counter-Clockwise)
  - Touch-Elektroden (5x kapazitiv)
  - AirWheel (Rotations-Geste)
  - Echtzeit-Visualisierung

**Hardware:**
```
MGC3130           CYD (ESP32)
-------           -----------
VCC     ------>   3.3V
GND     ------>   GND
SDA     ------>   GPIO 22
SCL     ------>   GPIO 27
TS      ------>   GPIO 35 (Transfer Status, optional)
RESET   ------>   GPIO 4 (optional)
```

**Anwendungen:**
- Berührungslose UI-Steuerung
- Volume-Control (AirWheel)
- Menü-Navigation
- Spiele-Steuerung

---

## 🔧 Allgemeine Hinweise

### I2C-Verkabelung (Standard)

Alle Sensor-Beispiele nutzen die **gleichen I2C-Pins**:

```
Sensor     CYD Board
------     ---------
VCC  --->  3.3V (WICHTIG: Nicht 5V!)
GND  --->  GND
SDA  --->  GPIO 22 (extSDA)
SCL  --->  GPIO 27 (extSCL)
```

Diese Pins sind in `CYD_Display_Config.h` definiert:
```cpp
#define extSDA 22
#define extSCL 27
```

### I2C-Adressen

| Sensor | Standard I2C-Adresse | Alternative |
|--------|---------------------|-------------|
| BNO055 | 0x28 | 0x29 (ADR Pin) |
| AS3935 | 0x03 | - |
| BMP180 | 0x77 | - |
| HMC5883L | 0x1E | - |
| VL53L0X | 0x29 | konfigurierbar |
| CCS811 | 0x5A | 0x5B (ADR Pin) |
| MGC3130 | 0x42 | - |

### Wichtige Hinweise

⚠️ **Spannung**: Die meisten Sensoren benötigen **3.3V** (nicht 5V!)
⚠️ **Pull-Up-Widerstände**: I2C benötigt Pull-Ups (oft on-board bei Breakout-Boards)
⚠️ **Kabel-Länge**: Kurze Kabel verwenden (< 30cm für stabile Kommunikation)
⚠️ **Mehrere Sensoren**: Bei gleicher I2C-Adresse: Nur einer gleichzeitig oder Multiplexer verwenden

### Troubleshooting

**Problem: Sensor nicht gefunden**
```cpp
// I2C-Scanner verwenden:
void scanI2C() {
  for (byte i = 1; i < 127; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.printf("I2C device found at 0x%02X\n", i);
    }
  }
}
```

**Problem: Falsche Werte**
- Verkabelung prüfen (VCC, GND, SDA, SCL)
- I2C-Adresse prüfen
- Sensor-Kalibrierung durchführen
- Pull-Up-Widerstände prüfen (4.7kΩ empfohlen)

**Problem: Verbindung bricht ab**
- Kabel zu lang → Kürzer als 30cm
- Zu viele Sensoren am Bus → Multiplexer verwenden
- Störungen → Geschirmte Kabel, Kondensatoren (100nF) an VCC/GND

## 📖 Weitere Ressourcen

### Sensor-Datenblätter
- **AS3935**: https://www.mouser.com/datasheet/2/588/ams_AS3935_Datasheet_EN_v5-1214568.pdf
- **BNO055**: https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bno055-ds000.pdf
- **BMP180**: https://cdn-shop.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf
- **CCS811**: https://www.sciosense.com/wp-content/uploads/2020/01/CCS811-Datasheet.pdf
- **VL53L0X**: https://www.st.com/resource/en/datasheet/vl53l0x.pdf
- **MGC3130**: https://ww1.microchip.com/downloads/en/DeviceDoc/40001667D.pdf

### I2C-Grundlagen
- **I2C-Protokoll**: https://www.i2c-bus.org/
- **ESP32 I2C**: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html

### CYD-Academy
- **Basic Examples**: [../examples/](../examples/)
- **Arcade Games**: [../arcade-games/](../arcade-games/)
- **Display-Konfiguration**: `CYD_Display_Config.h`

## 🎓 Lernpfad

### Empfohlene Reihenfolge

**Anfänger** (Start mit einfachen Sensoren):
1. **22 - Weather Station (BMP180)** - Einfach, zuverlässig, viele Libraries
2. **18 - Compass (BNO055)** - Gut dokumentiert, vielseitig
3. **19 - Laser Distance (VL53L0X)** - Schnelle Ergebnisse

**Fortgeschritten**:
4. **20 - Air Quality (CCS811)** - Baseline-Management, EEPROM
5. **16 - Spirit Level** - IMU-Fusion, komplexe Mathematik
6. **17 - Lightning Sensor** - Interrupt-basiert, Event-Handling

**Experte**:
7. **21 - Gesture Sensor** - 3D-Tracking, komplexe Protokolle
8. **22b - 7-Day History** - Daten-Persistenz, Graphen

## 🚀 Nächste Schritte

Nach Abschluss der Sensor-Beispiele:
- **Eigene Sensor-Projekte** erstellen
- **Mehrere Sensoren kombinieren** (z.B. Wetterstation + Luftqualität)
- **IoT-Integration** (MQTT, Cloud-Upload)
- **Master Class**: [Arcade Games](../arcade-games/) für komplexe Anwendungen

---

**Viel Erfolg mit deinen Sensor-Projekten! 🔬📊**

Bei Fragen: [CYD-Academy GitHub Issues](https://github.com/smily77/CYD-Academy/issues)
