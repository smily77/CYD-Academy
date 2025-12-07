# Beispiel 24: GPS Monitor - Serielle Schnittstelle

Ein vollständiges Beispiel für die Verwendung der seriellen Schnittstelle (UART2) des CYD mit einem GPS-Modul. Der Display fungiert als scrollender Monitor für NMEA-GPS-Daten vom Quectel L80-R GPS-Modul.

## 📸 Features

- **NMEA-Datenempfang**: Empfang und Anzeige von GPS NMEA 0183 Sentences über serielle Schnittstelle
- **Scrollender Monitor**: Automatisches Scrollen bei neuen GPS-Daten (letzte 100 Zeilen im Puffer)
- **Farbcodierung**: Verschiedene Farben für unterschiedliche NMEA-Sentence-Typen (GGA=Grün, RMC=Gelb, GSA=Orange, GSV=Blau)
- **GPS-Status**: Visuelle Anzeige ob GPS-Modul aktiv ist (grüner/roter Indikator)
- **Touch-Interaktion**: Display durch Berühren löschen und neu starten
- **Statistik**: Anzeige der Anzahl empfangener Zeilen
- **Timeout-Erkennung**: Warnung bei fehlenden GPS-Daten (Verkabelungsprobleme)
- **Debug-Ausgabe**: Parallele Ausgabe aller GPS-Daten über Serial Monitor
- **Robuster Code**: Buffer-Überlauf-Schutz, automatische Fehlerbehandlung

## 🔌 Hardware-Anforderungen

### Hauptkomponenten

- **ESP32-2432S028R (CYD)**
  - Cheap Yellow Display mit ESP32
  - 320x240 TFT Display (ILI9341)
  - Touch-Controller (XPT2046)
  - 3 Hardware-UARTs (Serial, Serial1, Serial2)

- **Quectel L80-R GPS-Modul** (oder kompatibles NMEA GPS)
  - Ultra-kompaktes GPS-Modul
  - NMEA 0183 Protokoll
  - MTK MT3339 Chipsatz
  - 9600 Baud Standard-Baudrate

### Quectel L80-R Spezifikationen

**Technische Daten**:
- **Chipsatz**: MediaTek MT3339
- **Frequenz**: L1, 1575.42 MHz (C/A Code)
- **Kanäle**: 66 Akquisitions-, 22 Tracking-Kanäle
- **Sensitivität**:
  - Tracking: -165 dBm
  - Akquisition: -148 dBm
  - Reacquisition: -160 dBm
- **Genauigkeit**:
  - Position: 2.5m CEP (ohne SBAS/DGPS)
  - Position: <2.0m CEP (mit SBAS)
  - Geschwindigkeit: 0.1 m/s
  - Zeit: 1μs synchronisiert zu GPS-Zeit
- **Startzeiten**:
  - Kaltstart: <32 Sekunden
  - Warmstart: <1 Sekunde
  - Hot Start: <1 Sekunde
- **Update-Rate**: Standard 1 Hz, bis 10 Hz möglich
- **Protokoll**: NMEA 0183 v3.01
- **Baudrate**: 9600 (Standard), konfigurierbar bis 115200
- **Stromverbrauch**:
  - Acquisition: 42 mA @ 3.3V
  - Tracking: 40 mA @ 3.3V
  - Standby: 1 mA
- **Spannung**: 3.0V - 4.3V (optimal 3.3V)
- **Abmessungen**: 16.0 x 16.0 x 6.45 mm
- **Gewicht**: <1g
- **Temperaturbereich**: -40°C bis +85°C

**Unterstützte GNSS-Systeme**:
- **GPS**: Global Positioning System (USA)
- **QZSS**: Quasi-Zenith Satellite System (Japan)
- **SBAS**: Satellite-Based Augmentation System (WAAS, EGNOS, MSAS, GAGAN)

### Verbindung (UART2)

```
Quectel L80-R    CYD (ESP32-2432S028R)
-------------    ---------------------
VCC (3.3V)  ---> 3.3V
GND         ---> GND
TX          ---> GPIO 16 (extRxD)
RX          ---> GPIO 17 (extTxD) [optional]
```

**Pin-Funktionen**:

| Pin Name | CYD GPIO | Funktion | Beschreibung |
|----------|----------|----------|--------------|
| extRxD   | GPIO 16  | UART2 RX | Empfängt Daten vom GPS TX |
| extTxD   | GPIO 17  | UART2 TX | Sendet Kommandos zu GPS RX (optional) |

**Wichtige Hinweise**:

⚠️ **SPANNUNG**: Quectel L80-R läuft mit **3.3V** - NIEMALS 5V anschließen!

⚠️ **TX/RX KREUZUNG**: GPS TX → CYD RX (GPIO 16), GPS RX → CYD TX (GPIO 17)

⚠️ **GPS-ANTENNE**: Für GPS-Fix ist freie Sicht zum Himmel erforderlich (Fenster, Balkon, Außenbereich)

⚠️ **STARTZEIT**: Erster GPS-Fix (Kaltstart) dauert 30-60 Sekunden bei freier Sicht

💡 **TIPP**: GPS-Modul benötigt externen Antennenanschluss oder integrierte Patch-Antenne

### Optionale Hardware

- **Externe GPS-Antenne**: Für besseren Empfang in Gebäuden (Antennenanschluss erforderlich)
- **Backup-Batterie**: CR1220 für schnellere Warmstarts (einige Module haben Batteriehalter)
- **Level-Shifter**: Falls 5V GPS-Modul verwendet wird (nicht empfohlen)

## 🌐 Technischer Hintergrund

### GPS - Global Positioning System

**GPS** (Global Positioning System) ist ein satellitengestütztes Navigationssystem, das vom US-Verteidigungsministerium entwickelt wurde und seit 1995 vollständig operativ ist.

#### Funktionsprinzip

GPS basiert auf **Trilateration** - der Bestimmung der Position durch Abstandsmessungen zu mindestens 4 Satelliten:

1. **Signallaufzeitmessung**: GPS-Satelliten senden kontinuierlich ihre Position und genaue Uhrzeit
2. **Entfernungsberechnung**: Empfänger misst Laufzeit des Signals → Entfernung = Laufzeit × Lichtgeschwindigkeit
3. **Trilateration**: Mit Entfernungen zu 3+ Satelliten lässt sich Position berechnen
4. **Zeitkorrektur**: 4. Satellit kompensiert Ungenauigkeit der Empfängeruhr

**Mathematische Grundlage**:

Für jeden Satelliten *i* gilt:
```
(x - x_i)² + (y - y_i)² + (z - z_i)² = (c × (t - t_i))²
```

Wobei:
- (x, y, z) = Position des Empfängers (gesucht)
- (x_i, y_i, z_i) = Position des Satelliten i (bekannt)
- c = Lichtgeschwindigkeit (≈ 3×10⁸ m/s)
- (t - t_i) = Signallaufzeit

Mit 4 Satelliten ergeben sich 4 Gleichungen mit 4 Unbekannten (x, y, z, t) → eindeutig lösbar!

#### GPS-Satelliten-Konstellation

- **Anzahl**: Mindestens 24 aktive Satelliten (+ Reserven)
- **Orbithöhe**: ~20.200 km über der Erde
- **Umlaufzeit**: 11 Stunden 58 Minuten (2 Umläufe pro Tag)
- **Bahnen**: 6 Bahnebenen, je 4 Satelliten, 55° Inklination
- **Sichtbarkeit**: 6-12 Satelliten von jedem Punkt der Erde sichtbar

#### Genauigkeit & Fehlerquellen

**Typische Genauigkeiten**:
- Zivil (C/A Code): 5-10 Meter (95% der Zeit)
- Mit SBAS (WAAS/EGNOS): 1-3 Meter
- Mit DGPS: <1 Meter
- Militärisch (P/Y Code): ~1 Zentimeter

**Fehlerquellen**:

| Fehlerquelle | Einfluss | Beschreibung |
|--------------|----------|--------------|
| Ionosphäre | ±5m | Signalverzögerung durch Elektronen in 50-1000 km Höhe |
| Troposphäre | ±0.5m | Wasserdampf in unterer Atmosphäre |
| Multipath | ±1m | Signalreflexionen an Gebäuden, Bergen |
| Satellitenuhren | ±2m | Minimale Abweichungen der Atomuhren |
| Ephemeridendaten | ±2.5m | Ungenaue Satellitenbahnvorhersage |
| Empfängerrauschen | ±1m | Elektronisches Rauschen im Empfänger |

**Verbesserungssysteme**:

- **SBAS** (Satellite-Based Augmentation System): Korrektursignale von geostationären Satelliten
  - WAAS (USA): Wide Area Augmentation System
  - EGNOS (Europa): European Geostationary Navigation Overlay Service
  - MSAS (Japan): Multi-functional Satellite Augmentation System
  - GAGAN (Indien): GPS Aided Geo Augmented Navigation

- **DGPS** (Differential GPS): Referenzstation sendet Korrekturen für lokale Fehler

### NMEA 0183 Protokoll

**NMEA 0183** ist das Standard-Kommunikationsprotokoll für GPS-Empfänger, entwickelt von der National Marine Electronics Association.

#### Protokoll-Eigenschaften

- **Format**: ASCII-Text (human-readable)
- **Baudrate**: Typisch 4800 oder 9600 Baud
- **Datenformat**: 8 Datenbits, keine Parität, 1 Stoppbit (8N1)
- **Zeilenende**: Carriage Return + Line Feed (\r\n)
- **Maximale Länge**: 82 Zeichen pro Sentence

#### NMEA Sentence-Struktur

Jede NMEA-Nachricht (Sentence) folgt diesem Format:

```
$AACCC,Feld1,Feld2,Feld3,...,FeldN*CS<CR><LF>
```

**Aufbau**:
- **$**: Startzeichen (Dollar-Zeichen)
- **AA**: Talker ID (GP=GPS, GN=GNSS, GL=GLONASS, GA=Galileo)
- **CCC**: Sentence ID (z.B. GGA, RMC, GSA)
- **Feld1,Feld2,...**: Datenfelder, durch Komma getrennt
- **\*CS**: Checksum (2 Hex-Zeichen, XOR aller Zeichen zwischen $ und \*)
- **<CR><LF>**: Zeilenende (0x0D 0x0A)

**Checksum-Berechnung**:
```cpp
uint8_t checksum = 0;
for (char c : sentence_ohne_$_und_*) {
  checksum ^= c;  // XOR
}
```

#### Wichtigste NMEA Sentence-Typen

##### 1. $GPGGA - Global Positioning System Fix Data

**Wichtigste GPS-Daten**: Position, Höhe, Fix-Qualität, Satelliten

**Beispiel**:
```
$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
```

**Felder**:
1. `123519`: UTC-Zeit (12:35:19)
2. `4807.038,N`: Breitengrad 48°07.038' Nord
3. `01131.000,E`: Längengrad 11°31.000' Ost
4. `1`: Fix-Qualität (0=kein, 1=GPS, 2=DGPS)
5. `08`: Anzahl verwendeter Satelliten
6. `0.9`: HDOP (Horizontal Dilution of Precision)
7. `545.4,M`: Höhe über Meeresspiegel in Metern
8. `46.9,M`: Geoid-Separation (Unterschied WGS84/lokales Geoid)
9. (leer): Zeit seit letztem DGPS-Update
10. (leer): DGPS-Stations-ID
11. `*47`: Checksum

**Verwendung**: Primäre Position, Höhe, Qualitätsinformation

##### 2. $GPRMC - Recommended Minimum Navigation Information

**Kompakte Navigationsdaten**: Position, Geschwindigkeit, Kurs, Datum

**Beispiel**:
```
$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
```

**Felder**:
1. `123519`: UTC-Zeit
2. `A`: Status (A=Active/gültig, V=Void/ungültig)
3. `4807.038,N`: Breitengrad
4. `01131.000,E`: Längengrad
5. `022.4`: Geschwindigkeit über Grund (Knoten)
6. `084.4`: Kurs über Grund (Grad)
7. `230394`: Datum (23. März 1994)
8. `003.1,W`: Magnetische Deklination (3.1° West)
9. `*6A`: Checksum

**Verwendung**: Minimal-Information für Navigation, enthält auch Datum (wichtig für Zeit-Synchronisation)

##### 3. $GPGSA - GPS DOP and Active Satellites

**Präzisionsdaten**: Welche Satelliten werden verwendet, wie gut ist die Geometrie

**Beispiel**:
```
$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39
```

**Felder**:
1. `A`: Modus (A=Auto, M=Manuell)
2. `3`: Fix-Typ (1=kein, 2=2D, 3=3D)
3. `04,05,09,12,24`: PRNs der verwendeten Satelliten (max. 12)
4. `2.5`: PDOP (Position Dilution of Precision)
5. `1.3`: HDOP (Horizontal DOP)
6. `2.1`: VDOP (Vertical DOP)

**DOP-Werte** (Dilution of Precision):
- **1-2**: Exzellent (ideale Satellitengeometrie)
- **2-5**: Gut
- **5-10**: Moderat
- **10-20**: Schlecht
- **>20**: Sehr schlecht (nicht verwenden)

**Verwendung**: Qualitätsbeurteilung, Satellitenauswahl

##### 4. $GPGSV - GPS Satellites in View

**Satelliten-Sichtbarkeit**: Welche Satelliten sind sichtbar, Signal-Stärke

**Beispiel**:
```
$GPGSV,3,1,11,03,03,111,00,04,15,270,00,06,01,010,00,13,06,292,00*74
```

**Felder**:
1. `3`: Gesamtanzahl GSV-Nachrichten
2. `1`: Nummer dieser Nachricht
3. `11`: Anzahl sichtbarer Satelliten
4-7. Für jeden Satelliten (max. 4 pro Nachricht):
   - `03`: PRN-Nummer
   - `03`: Elevation (Höhenwinkel, 0-90°)
   - `111`: Azimuth (Richtung, 0-359°)
   - `00`: SNR (Signal-to-Noise Ratio, 0-99 dB)

**SNR-Bedeutung**:
- **0**: Kein Signal
- **1-20**: Sehr schwaches Signal
- **20-35**: Schwaches Signal (möglicherweise nicht verwendbar)
- **35-45**: Gutes Signal (verwendbar)
- **45+**: Sehr gutes Signal

**Verwendung**: Satelliten-Tracking, Signal-Qualität, Anzahl sichtbarer Satelliten

##### 5. $GPVTG - Track Made Good and Ground Speed

**Bewegungsdaten**: Kurs und Geschwindigkeit

**Beispiel**:
```
$GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*48
```

**Felder**:
1. `054.7,T`: Kurs über Grund (True North, 54.7°)
2. `034.4,M`: Kurs über Grund (Magnetic, 34.4°)
3. `005.5,N`: Geschwindigkeit (Knoten)
4. `010.2,K`: Geschwindigkeit (km/h)

**Verwendung**: Navigation, Geschwindigkeitsmessung

#### Koordinaten-Format in NMEA

GPS-Koordinaten werden im **Degrees + Decimal Minutes** Format übertragen:

**Format**: `DDMM.MMMM` (Latitude) oder `DDDMM.MMMM` (Longitude)

**Beispiel**: `4807.038,N`
- `48`: Grad (Degrees)
- `07.038`: Minuten (Minutes mit Dezimalstellen)
- `N`: Nord (North)

**Umrechnung zu Dezimalgrad**:
```
Dezimalgrad = Grad + (Minuten / 60)
Beispiel: 48 + (7.038 / 60) = 48.1173°
```

**Code-Beispiel**:
```cpp
float nmeaToDecimal(String coord, char dir) {
  // Beispiel: "4807.038" -> 48.1173
  int dotPos = coord.indexOf('.');
  float degrees = coord.substring(0, dotPos-2).toFloat();
  float minutes = coord.substring(dotPos-2).toFloat();
  float decimal = degrees + (minutes / 60.0);

  // Süd/West sind negativ
  if (dir == 'S' || dir == 'W') {
    decimal = -decimal;
  }

  return decimal;
}
```

### UART - Universal Asynchronous Receiver/Transmitter

#### UART Grundlagen

**UART** ist ein serielles Kommunikationsprotokoll für asynchrone Datenübertragung zwischen zwei Geräten.

**Eigenschaften**:
- **Asynchron**: Kein gemeinsames Taktsignal (nur TX/RX-Leitungen)
- **Full-Duplex**: Gleichzeitiges Senden und Empfangen möglich
- **Punkt-zu-Punkt**: Verbindung zwischen genau 2 Geräten
- **Baudrate**: Beide Geräte müssen gleiche Geschwindigkeit haben

**Standard-Baudraten**: 9600, 19200, 38400, 57600, 115200 (Baud = Bits pro Sekunde)

#### UART-Datenformat

Jedes Byte wird als **Datenrahmen** (Frame) übertragen:

```
  Start  D0 D1 D2 D3 D4 D5 D6 D7 Parity Stop
Idle ___┐  ┌──┐  ┌──┐     ┌──┐     ┌──┐  ┌─────
        │  │  │  │  │     │  │     │  │  │
        └──┘  └──┘  └─────┘  └─────┘  └──┘

   0    1  0  1  0  1  0  1  0  Even  1  (Idle)
```

**Rahmen-Aufbau**:
1. **Start-Bit**: Immer 0 (signalisiert Beginn)
2. **Datenbits**: 5-9 Bits (meist 8)
3. **Parity-Bit**: Optional (Even/Odd/None)
4. **Stop-Bits**: 1 oder 2 Bits (immer 1)

**Typisches GPS-Format**: 8N1 (8 Datenbits, No Parity, 1 Stop-Bit)

#### ESP32 Hardware-UARTs

Der ESP32 hat **3 Hardware-UART-Schnittstellen**:

| UART | Standard-Pins | Verwendung |
|------|---------------|------------|
| UART0 (Serial) | GPIO 1 (TX), GPIO 3 (RX) | USB-Serial (Programmierung, Serial Monitor) |
| UART1 (Serial1) | GPIO 10 (TX), GPIO 9 (RX) | Flash-Speicher (nicht verfügbar) |
| UART2 (Serial2) | Frei wählbar | **Verfügbar für GPS!** |

**UART2 für GPS-Modul**:
```cpp
// UART2 initialisieren mit benutzerdefinierten Pins
Serial2.begin(9600, SERIAL_8N1, extRxD, extTxD);
//             |      |          |       |
//             |      |          |       +-- TX Pin (GPIO 17)
//             |      |          +---------- RX Pin (GPIO 16)
//             |      +--------------------- Datenformat (8N1)
//             +---------------------------- Baudrate (9600)
```

**Vorteile Hardware-UART**:
- **Automatische Pufferung**: Empfangene Daten werden in FIFO gepuffert
- **Kein Interrupt nötig**: Hardware übernimmt Timing
- **Multitasking-fähig**: Empfang läuft parallel zu anderem Code
- **Zuverlässig**: Keine verpassten Zeichen bei korrekter Baudrate

#### TX/RX Kreuzverbindung

⚠️ **WICHTIG**: TX und RX werden **über Kreuz** verbunden!

```
  GPS-Modul          CYD (ESP32)
  ---------          -----------
     TX     -------->    RX (GPIO 16)
     RX     <--------    TX (GPIO 17)
    GND     ----------- GND
```

**Warum über Kreuz?**
- TX (Transmit) = Senden
- RX (Receive) = Empfangen
- Sender (TX) muss mit Empfänger (RX) verbunden sein!

### Vergleich: GPS vs. GNSS

**GPS** ist nur **ein** Satelliten-Navigationssystem. Moderne Empfänger nutzen oft **mehrere**:

| System | Land | Satelliten | Status | Frequenz |
|--------|------|------------|--------|----------|
| **GPS** | USA | 31 | Vollständig operativ (seit 1995) | L1 (1575.42 MHz) |
| **GLONASS** | Russland | 24 | Vollständig operativ | L1 (1602 MHz) |
| **Galileo** | EU | 30 | Operativ (seit 2016) | E1 (1575.42 MHz) |
| **BeiDou** | China | 35 | Vollständig operativ (2020) | B1 (1561.098 MHz) |
| **QZSS** | Japan | 4 | Regional (Asien-Pazifik) | L1 (1575.42 MHz) |
| **NavIC** | Indien | 7 | Regional (Indien) | L5 (1176.45 MHz) |

**GNSS** = Global Navigation Satellite System = Sammelbegriff für **alle** Systeme zusammen

**Vorteile von Multi-GNSS**:
- **Mehr Satelliten**: Bis zu 100+ sichtbare Satelliten (statt nur 6-12 bei GPS allein)
- **Bessere Genauigkeit**: Redundante Messungen verbessern Präzision
- **Schnellerer Fix**: Mehr Satelliten = schnellerer Erstkontakt
- **Bessere Verfügbarkeit**: Weniger tote Winkel in Städten/Bergen

**Quectel L80-R**:
- Unterstützt: GPS + QZSS + SBAS
- Sentence-Präfixe:
  - `$GP...`: Nur GPS-Satelliten
  - `$GN...`: Gemischt (GPS + andere GNSS)

## 📡 Serielle Kommunikation auf dem CYD

### Hardware-UARTs des ESP32

Der ESP32-2432S028R (CYD) nutzt einen ESP32-WROOM-32 mit **3 Hardware-UART-Controllern**:

```
┌─────────────────────────────────────┐
│         ESP32-WROOM-32              │
│                                     │
│  UART0 (Serial)  ← USB-Serial       │
│    └─ GPIO 1 (TX), GPIO 3 (RX)      │
│                                     │
│  UART1 (Serial1) ← Flash SPI        │
│    └─ GPIO 10, GPIO 9 (nicht frei)  │
│                                     │
│  UART2 (Serial2) ← Frei!            │
│    └─ GPIO 16 (RX), GPIO 17 (TX)    │
│         (konfigurierbar)            │
└─────────────────────────────────────┘
```

**UART2** ist die einzige frei verfügbare Hardware-UART und wird für GPS verwendet!

### Pin-Konfiguration in CYD_Display_Config.h

Laut CYD-Academy Repository-Guideline werden die seriellen Pins in der **CYD_Display_Config.h** definiert:

```cpp
// Serielle Schnittstelle (UART2) für GPS, Sensoren, etc.
#define extRxD 16    // GPIO 16 - Empfangen von externen seriellen Geräten
#define extTxD 17    // GPIO 17 - Senden zu externen seriellen Geräten
```

**Initialisierung im Code**:
```cpp
#include <CYD_Display_Config.h>

// Serielle Pins aus Config verwenden (NICHT hardcoden!)
#define extRxD 16
#define extTxD 17

void setup() {
  // GPS mit 9600 Baud, 8 Datenbits, No Parity, 1 Stop-Bit
  Serial2.begin(9600, SERIAL_8N1, extRxD, extTxD);
}
```

**Warum diese Pins?**

| Kriterium | GPIO 16/17 | Andere GPIOs |
|-----------|------------|--------------|
| **Verfügbarkeit** | Frei auf CYD | Oft belegt (Display, Touch, LED) |
| **Boot-Sicherheit** | Kein Pull-Up/Down nötig | GPIO 0, 2, 12, 15 beeinflussen Boot |
| **Standardisierung** | In CYD-Academy definiert | Wechselnde Pin-Belegung |
| **RGB-LED Konflikt** | Kein Konflikt | GPIO 4, 16, 17 = RGB LEDs auf manchen Boards |

**⚠️ Wichtig**: Auf einigen CYD-Boards sind GPIO 16/17 für **RGB-LED** (Grün/Blau) verwendet!
- Wenn RGB-LED vorhanden → Konflikterkennung im Code empfohlen
- Alternative Pins: GPIO 25, 26, 32, 33

### Datenfluss GPS → CYD

```
┌──────────────────┐
│  GPS-Modul       │
│  (Quectel L80-R) │
│                  │
│  [GPS-Chip]      │
│      │           │
│      ├─ Berechnet Position aus Satellitensignalen
│      ├─ Generiert NMEA-Sentences
│      │           │
│      V           │
│  [UART TX]       │ 9600 Baud, 8N1
│      │           │
└──────┼───────────┘
       │ TX Pin (3.3V)
       │
       V (über Kreuz!)
┌──────────────────┐
│  CYD (ESP32)     │
│                  │
│  [GPIO 16 RX]    │ extRxD
│      │           │
│      V           │
│  [UART2 RX]      │ Hardware-Controller
│      │           │
│      ├─ Empfängt Bytes automatisch
│      ├─ Speichert in FIFO (128 Byte)
│      │           │
│      V           │
│  Serial2.read()  │ Arduino-Code liest FIFO
│      │           │
│      V           │
│  [GPS-Parser]    │ Verarbeitet NMEA-Zeilen
│      │           │
│      V           │
│  [Display]       │ Zeigt Daten an
└──────────────────┘
```

**FIFO-Buffer**:
- ESP32 UART hat 128-Byte Hardware-FIFO
- Empfangene Daten werden automatisch gepuffert
- Bei Überlauf gehen älteste Daten verloren
- `Serial2.available()` zeigt Anzahl verfügbarer Bytes

### Baudrate & Timing

**9600 Baud** = 9600 Bits pro Sekunde

**Byte-Übertragungszeit** (bei 8N1):
```
1 Byte = 1 Start + 8 Data + 0 Parity + 1 Stop = 10 Bits
Zeit pro Byte = 10 Bits / 9600 Baud = 1.04 ms
```

**Typische NMEA-Sentence**:
```
$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47<CR><LF>
└────────────────────── 72 Zeichen ─────────────────────────────────┘
```
- 72 Zeichen × 1.04 ms = **75 ms pro Sentence**

**GPS Update-Rate** (Quectel L80-R):
- Standard: 1 Hz (1 Sentence-Set pro Sekunde)
- 1 Set = typisch 5-8 Sentences (GGA, GSA, GSV×3, RMC, VTG)
- Gesamte Übertragungszeit: ~500 ms pro Set

**Code-Timing**:
```cpp
void loop() {
  while (Serial2.available()) {
    char c = Serial2.read();  // ~10-20 µs
    // ... Verarbeitung
  }
  delay(10);  // 10 ms Pause für Watchdog
}
```

**Watchdog-Timer**: ESP32 hat Watchdog der nach ~5 Sekunden ohne `delay()` System resettet!

## 💻 Code-Struktur

### Hauptkomponenten

#### 1. Initialisierung (setup)

**Display-Initialisierung**:
```cpp
lcd.init();
lcd.setRotation(1);  // Landscape (320x240) für mehr Platz
lcd.fillScreen(COLOR_BG);
lcd.setTextSize(TEXT_SIZE);
```

**Serielle Schnittstelle**:
```cpp
// UART2 mit benutzerdefinierten Pins
Serial2.begin(GPS_BAUD, SERIAL_8N1, extRxD, extTxD);
//             9600      8N1         GPIO16  GPIO17
```

**Erklärung**:
- `GPS_BAUD = 9600`: Standard-Baudrate für Quectel L80-R
- `SERIAL_8N1`: 8 Datenbits, No Parity, 1 Stop-Bit
- `extRxD = 16`: GPIO für Empfang (RX)
- `extTxD = 17`: GPIO für Senden (TX, hier nur für Konfiguration)

#### 2. GPS-Daten Empfang (readGPSData)

**Zeichen-basiertes Lesen**:
```cpp
void readGPSData() {
  while (Serial2.available()) {
    char c = Serial2.read();

    // Debug-Ausgabe parallel
    Serial.write(c);

    // Zeichen zu Buffer hinzufügen
    gpsBuffer += c;

    // Zeilenende erkannt? (\n)
    if (c == '\n' && gpsBuffer.length() > 1) {
      processNMEASentence(gpsBuffer);
      gpsBuffer = "";  // Buffer leeren
    }

    // Überlauf-Schutz
    if (gpsBuffer.length() > GPS_BUFFER_SIZE) {
      gpsBuffer = "";
    }
  }
}
```

**Warum Zeichen-basiert?**
- NMEA-Sentences haben variable Länge
- Zeilenende (`\r\n`) markiert Ende einer Sentence
- Zeichenweises Lesen ermöglicht Echtzeit-Verarbeitung

**Buffer-Strategie**:
- `String gpsBuffer`: Dynamischer String-Puffer
- Max. 256 Zeichen (längste NMEA = 82 Zeichen)
- Überlauf-Schutz verhindert Speicher-Probleme

#### 3. NMEA Sentence Verarbeitung (processNMEASentence)

**Validierung**:
```cpp
void processNMEASentence(String sentence) {
  sentence.trim();  // Whitespace entfernen

  // Leere Zeilen ignorieren
  if (sentence.length() < 5) return;

  // Muss mit $ beginnen
  if (sentence.charAt(0) != '$') return;

  // ... Verarbeitung
}
```

**Farbcodierung nach Sentence-Typ**:
```cpp
uint16_t color = COLOR_OTHER;  // Weiß = Default

if (sentence.startsWith("$GPGGA") || sentence.startsWith("$GNGGA")) {
  color = COLOR_GPGGA;  // Grün = Position
}
else if (sentence.startsWith("$GPRMC") || sentence.startsWith("$GNRMC")) {
  color = COLOR_GPRMC;  // Gelb = Navigation
}
else if (sentence.startsWith("$GPGSA") || sentence.startsWith("$GNGSA")) {
  color = COLOR_GPGSA;  // Orange = Präzision
}
else if (sentence.startsWith("$GPGSV") || sentence.startsWith("$GNGSV")) {
  color = COLOR_GPGSV;  // Blau = Satelliten
}
```

**Warum GP und GN?**
- `GP`: Nur GPS-Satelliten verwendet
- `GN`: Gemischtes GNSS (GPS + GLONASS + Galileo + ...)
- Modernes GPS-Modul kann beides senden

**Ringpuffer-Speicherung**:
```cpp
displayLines[lineIndex] = sentence;
lineColors[lineIndex] = color;
lineIndex = (lineIndex + 1) % 100;  // Modulo = Ringpuffer
totalLines++;
```

**Ringpuffer-Prinzip**:
```
Array: [Zeile0] [Zeile1] [Zeile2] ... [Zeile99]
          ^
       lineIndex

Nach 100 Zeilen:
       lineIndex = (99 + 1) % 100 = 0  → Überschreibt älteste Zeile
```

Vorteil: Feste Speichergröße, älteste Daten werden automatisch überschrieben.

#### 4. Display-Aktualisierung (updateDisplay)

**Scrolling-Logik**:
```cpp
void updateDisplay() {
  // Wie viele Zeilen passen auf Display?
  int availableHeight = SCREEN_HEIGHT - TOP_MARGIN - BOTTOM_MARGIN;
  int maxLines = availableHeight / LINE_HEIGHT;
  // Beispiel: (240 - 20 - 5) / 16 = 13 Zeilen

  // Wenn mehr Zeilen vorhanden → zeige nur neueste
  int displayStart = 0;
  if (totalLines > maxLines) {
    displayStart = totalLines - maxLines;
  }

  // Zeilen anzeigen
  for (int i = displayStart; i < totalLines && i < displayStart + maxLines; i++) {
    int bufferIndex = (lineIndex - totalLines + i + 100) % 100;
    // Ringpuffer-Rückrechnung um richtige Zeile zu finden

    lcd.setTextColor(lineColors[bufferIndex]);
    lcd.setCursor(2, yPos);
    lcd.println(displayLines[bufferIndex]);

    yPos += LINE_HEIGHT;
  }
}
```

**Automatisches Scrollen**:
- Display zeigt immer **neueste** Zeilen
- Alte Zeilen "scrollen nach oben" und verschwinden
- Kein manuelles Scrollen nötig (würde Touch erfordern)

**Text-Kürzung für Display**:
```cpp
String displayText = displayLines[bufferIndex];
if (displayText.length() > 53) {
  displayText = displayText.substring(0, 50) + "...";
}
lcd.println(displayText);
```

**Warum 53 Zeichen?**
- Display-Breite: 320 Pixel
- Textgröße 1: 6 Pixel pro Zeichen (mit Spacing)
- 320 / 6 ≈ 53 Zeichen pro Zeile

#### 5. Header mit Status-Anzeige (drawHeader)

**Status-LED**:
```cpp
if (gpsActive) {
  lcd.fillCircle(300, 8, 5, TFT_GREEN);  // Grün = GPS sendet
} else {
  lcd.fillCircle(300, 8, 5, TFT_RED);    // Rot = Kein Signal
}
```

**Statistik**:
```cpp
lcd.printf("Lines:%d", totalLines);  // Anzahl empfangener Zeilen
```

#### 6. Touch-Interaktion (checkTouch)

**Display löschen per Touch**:
```cpp
void checkTouch() {
  if (lcd.getTouch(&touchX, &touchY)) {
    delay(200);  // Debounce (Mehrfach-Trigger vermeiden)
    clearDisplay();
  }
}
```

**Warum delay(200)?**
- Touch-Controller kann "bouncen" (mehrfache Trigger)
- 200ms Pause verhindert ungewollte Mehrfach-Löschungen
- Alternative: Zustandsvariable für "Touch bereits erkannt"

#### 7. GPS-Timeout-Erkennung (checkGPSTimeout)

**Verbindungs-Überwachung**:
```cpp
void checkGPSTimeout() {
  if (gpsActive && (millis() - lastDataTime > 5000)) {
    gpsActive = false;

    Serial.println("!!! GPS TIMEOUT - Keine Daten seit 5 Sekunden !!!");
    // ... Fehlerdiagnose-Hinweise
  }
}
```

**Warum 5 Sekunden?**
- GPS sendet normalerweise 1× pro Sekunde (1 Hz)
- 5 Sekunden = deutlich zu lange Pause
- Indikator für Verkabelungsfehler, Stromausfall, etc.

### Code-Flow-Diagramm

```
setup()
  │
  ├─ Display initialisieren
  ├─ Serial2 initialisieren (GPS)
  ├─ Header zeichnen
  └─ Warte-Hinweis anzeigen

loop()
  │
  ├─ readGPSData()
  │   │
  │   └─ while (Serial2.available())
  │       │
  │       ├─ Zeichen lesen
  │       ├─ Zu Buffer hinzufügen
  │       └─ Bei '\n': processNMEASentence()
  │           │
  │           ├─ Validierung ($ am Anfang)
  │           ├─ Farbe bestimmen (GGA/RMC/GSA/GSV)
  │           ├─ In Ringpuffer speichern
  │           └─ updateDisplay()
  │               │
  │               ├─ Scrolling-Logik
  │               ├─ Zeilen zeichnen (mit Farbe)
  │               └─ Header aktualisieren
  │
  ├─ checkTouch()
  │   │
  │   └─ Bei Touch: clearDisplay()
  │
  ├─ checkGPSTimeout()
  │   │
  │   └─ Bei >5s keine Daten: Warnung
  │
  └─ delay(10)  // Watchdog
```

## 🚀 Installation & Verwendung

### 1. Erforderliche Libraries

**Keine zusätzlichen Libraries erforderlich!**

Dieses Beispiel verwendet nur ESP32-Standard-Libraries:
- `Serial2` (Hardware-UART, Teil von ESP32 Arduino Core)
- `CYD_Display_Config.h` (aus MyLGFXConfigs Library, bereits für CYD-Academy installiert)

### 2. MyLGFXConfigs Library (falls noch nicht installiert)

Die CYD-Academy benötigt die **MyLGFXConfigs** Library für Display-Konfiguration:

**Installation**:
```
1. Arduino IDE -> Tools -> Manage Libraries
2. Suche: "MyLGFXConfigs" oder "LovyanGFX"
3. Installiere "LovyanGFX" von lovyan03
```

**Alternative (Manuell)**:
```bash
cd ~/Arduino/libraries/
git clone https://github.com/[username]/MyLGFXConfigs.git
```

### 3. Hardware aufbauen

**Schritt 1: GPS-Modul vorbereiten**

- **Antenne anschließen**: Falls externes Antennen-Modul → U.FL/SMA-Connector fest verbinden
- **Batterie einsetzen** (optional): CR1220 für schnellere Warmstarts

**Schritt 2: Verkabelung**

⚠️ **WICHTIG**: CYD **ausgeschaltet** während Verkabelung!

```
┌─────────────────┐          ┌──────────────────────┐
│  Quectel L80-R  │          │  CYD (ESP32-2432S028)│
├─────────────────┤          ├──────────────────────┤
│ VCC (3.3V) ●────┼─ Rot ───→│● 3.3V                │
│ GND        ●────┼─Schwarz─→│● GND                 │
│ TX         ●────┼─ Grün ──→│● GPIO 16 (extRxD)    │
│ RX         ●────┼─ Gelb ──→│● GPIO 17 (extTxD)    │ (optional)
└─────────────────┘          └──────────────────────┘
```

**Kabelfarben** (Vorschlag):
- 🔴 Rot: VCC (3.3V)
- ⚫ Schwarz: GND
- 🟢 Grün: TX (GPS) → RX (CYD)
- 🟡 Gelb: RX (GPS) ← TX (CYD) - optional

**Schritt 3: GPS-Antenne positionieren**

📡 **GPS benötigt freie Sicht zum Himmel**:
- ✅ Fensterbank (Glas dämpft Signal leicht, aber OK)
- ✅ Balkon/Terrasse
- ✅ Außenbereich
- ❌ Innenraum ohne Fenster (kein Fix möglich)
- ❌ Keller/Garage (kein GPS-Signal)

**Schritt 4: Stromversorgung**

- **USB-Stromversorgung**: 5V via USB-C (CYD hat 3.3V Regler)
- **Stromverbrauch**: ~250 mA (CYD Display) + 42 mA (GPS) = ~300 mA gesamt

### 4. Code hochladen

**Arduino IDE Einstellungen**:
```
Board: "ESP32 Dev Module"
Upload Speed: 921600
CPU Frequency: 240 MHz (WiFi/BT)
Flash Frequency: 80 MHz
Flash Mode: QIO
Flash Size: 4MB (32Mb)
Partition Scheme: "Default 4MB with spiffs"
PSRAM: Disabled
Port: [Ihr CYD COM-Port]
```

**Upload-Prozess**:
1. CYD via USB-C an PC anschließen
2. Arduino IDE: Sketch -> Upload (oder Ctrl+U)
3. Warte bis "Hard resetting via RTS pin..."
4. CYD startet automatisch

### 5. Bedienung

**Erstes Einschalten**:
1. Display zeigt: "Warte auf GPS-Daten..."
2. **Kaltstartzeit**: 30-60 Sekunden (bei freier Sicht zum Himmel)
3. Status-LED (oben rechts):
   - 🔴 Rot = Kein Signal
   - 🟢 Grün = GPS sendet Daten

**Normalbetrieb**:
- NMEA-Sentences scrollen automatisch von unten nach oben
- Farbcodierung:
  - 🟢 Grün = $GPGGA (Position, Höhe)
  - 🟡 Gelb = $GPRMC (Navigation, Geschwindigkeit)
  - 🟠 Orange = $GPGSA (Präzision, DOP)
  - 🔵 Blau = $GPGSV (Satelliten)
  - ⚪ Weiß = Andere Sentences

**Touch-Interaktion**:
- **Beliebige Stelle berühren** → Display wird gelöscht und neu gestartet
- Nützlich um "sauberen Start" zu haben oder Statistik zurückzusetzen

**Status-Anzeige** (Header):
- "GPS Monitor" = Titel
- "Lines: XX" = Anzahl empfangener Zeilen
- 🟢/🔴 = GPS-Status (aktiv/inaktiv)

### 6. Serielle Ausgabe

**Serial Monitor öffnen** (Tools -> Serial Monitor, 115200 Baud):

**Startup-Meldung**:
```
=== CYD GPS Monitor ===
Quectel L80-R GPS Module
========================

GPS UART2 initialisiert:
- Baudrate: 9600
- RX Pin (extRxD): GPIO 16
- TX Pin (extTxD): GPIO 17

Warte auf GPS-Daten...
(Kaltstartzeit ca. 30-60 Sekunden bei freier Sicht zum Himmel)

Setup abgeschlossen!
```

**GPS-Daten empfangen**:
```
>>> GPS-Modul sendet Daten! <<<

$GPGGA,081236,0000.0000,N,00000.0000,E,0,00,99.99,,,,,,*6C
$GPGSA,A,1,,,,,,,,,,,,,99.99,99.99,99.99*30
$GPGSV,1,1,00*79
$GPRMC,081236,V,0000.0000,N,00000.0000,E,,,060180,,,N*4D
$GPVTG,,,,,,,,,N*30
```

**Wichtig**: Ohne GPS-Fix (0 Satelliten) sind Position/Zeit **ungültig** (0000.0000).

**Gültiger GPS-Fix**:
```
$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
```
- `123519`: 12:35:19 UTC
- `4807.038,N`: 48°07.038' Nord
- `01131.000,E`: 11°31.000' Ost
- `1`: GPS-Fix vorhanden
- `08`: 8 Satelliten verwendet

### 7. Erwartetes Verhalten

**Timeline nach Einschalten** (Kaltstartbedingungen):

| Zeit | Ereignis | Display | Serial Monitor |
|------|----------|---------|----------------|
| 0s | Start | "Warte auf GPS..." | Setup-Meldungen |
| 1-5s | GPS wacht auf | Keine Änderung | Warte-Meldung |
| 5-10s | Erste NMEA-Daten | Status: 🟢 Grün | NMEA mit Fix=0 |
| 10-30s | Satelliten-Suche | GPGSV Sentences (Blau) | "0 Satelliten verwendet" |
| 30-60s | **Erster Fix!** | GPGGA mit Position (Grün) | Gültige Koordinaten |
| >60s | Kontinuierlicher Betrieb | Alle NMEA-Typen scrollen | 1 Hz Update-Rate |

**Warmstart** (GPS war vor <4 Stunden aktiv):
- Fix in **1-5 Sekunden** (Ephemeridendaten noch im Speicher)

**Hot Start** (GPS war vor <Minuten aktiv):
- Fix in **<1 Sekunde** (alle Daten noch aktuell)

## 🔧 Troubleshooting

### Problem 1: "Warte auf GPS-Daten..." bleibt stehen (Kein Signal)

**Symptome**:
- Display zeigt weiterhin "Warte auf GPS..."
- Status-LED bleibt rot
- Serial Monitor: Keine NMEA-Daten

**Mögliche Ursachen & Lösungen**:

#### Lösung 1.1: Verkabelung prüfen

```
Checkliste:
☐ VCC (GPS) → 3.3V (CYD) - NICHT 5V!
☐ GND (GPS) → GND (CYD)
☐ TX (GPS) → GPIO 16 (CYD) - NICHT GPIO 17!
☐ Kabel fest verbunden (kein Wackelkontakt)
☐ Richtige Pins am CYD (16/17, nicht 22/27 = I2C!)
```

**Test**: Wackel-Test an allen 4 Kabeln durchführen.

#### Lösung 1.2: Stromversorgung prüfen

GPS-Module benötigen stabile 3.3V mit mindestens 50 mA:

**Test mit Multimeter**:
```
Spannung zwischen VCC und GND messen:
- Soll: 3.20 - 3.40V
- Falls <3.0V: Unterspannung! (schwaches USB-Netzteil)
- Falls >3.6V: Überspannung! (GEFAHR für GPS!)
```

**Lösung**:
- Besseres USB-Netzteil verwenden (mindestens 1A)
- Kabel-Widerstand prüfen (dünne/lange Kabel = Spannungsabfall)

#### Lösung 1.3: GPS-Modul-Funktion testen

**Test**: GPS-Modul ohne ESP32 testen (USB-Serial-Adapter):

```
GPS TX → USB-Serial RX
GPS GND → USB-Serial GND
GPS VCC → USB-Serial 3.3V

Serial Monitor: 9600 Baud
```

Wenn NMEA-Daten erscheinen → GPS funktioniert, Fehler ist am CYD-Pinout.

#### Lösung 1.4: Pin-Konflikt ausschließen

**GPIO 16/17** können auf manchen CYD-Boards für **RGB-LED** verwendet sein!

**Test**:
```cpp
// In setup() vor Serial2.begin():
pinMode(16, INPUT);
pinMode(17, INPUT);

Serial.println(digitalRead(16));  // Sollte LOW oder HIGH sein
Serial.println(digitalRead(17));  // Sollte LOW oder HIGH sein
```

**Alternative Pins** testen:
```cpp
#define extRxD 25  // Alternativer RX-Pin
#define extTxD 26  // Alternativer TX-Pin

Serial2.begin(9600, SERIAL_8N1, extRxD, extTxD);
```

Gute Alternativ-Pins: GPIO 25, 26, 32, 33 (meist frei auf CYD).

### Problem 2: GPS sendet Daten, aber kein GPS-Fix (Position = 0000.0000)

**Symptome**:
- NMEA-Daten werden empfangen (Status: 🟢 Grün)
- Aber Position bleibt `0000.0000,N,00000.0000,E`
- Fix-Status in $GPGGA: `0` (kein Fix) oder `V` (void) in $GPRMC

**Mögliche Ursachen & Lösungen**:

#### Lösung 2.1: Antenne/Sicht zum Himmel

**GPS benötigt freie Sicht zum Himmel!**

```
Schlechte Standorte:
❌ Innenraum ohne Fenster
❌ Keller/Garage/Tiefgarage
❌ Metallgehäuse (Faraday-Käfig)
❌ Neben großen Metallflächen (Auto, Heizung)
❌ Dichte Bebauung (Hochhausschlucht)

Gute Standorte:
✅ Fensterbank (nach außen gerichtet)
✅ Balkon/Terrasse
✅ Garten/Außenbereich
✅ Freies Feld
```

**Test**: GPS-Modul nach draußen bringen (mit langem USB-Kabel).

#### Lösung 2.2: Geduld (Kaltstartzeit)

Erster GPS-Fix dauert **30-60 Sekunden** (bis zu 12 Minuten bei sehr schlechten Bedingungen):

**Gründe**:
- GPS muss **Ephemeridendaten** von Satelliten herunterladen (dauert ~12.5 Minuten pro Satellit)
- Almanachdaten (grobe Satellitenpositionen) dauern ~12.5 Minuten
- Mindestens 4 Satelliten mit gutem Signal nötig

**Wartezeiten**:
- **Kaltstart** (GPS war >4h aus): 30-120 Sekunden
- **Warmstart** (GPS war <4h aus): 1-5 Sekunden
- **Hot Start** (GPS war <1min aus): <1 Sekunde

**Lösung**: GPS-Modul **dauerhaft** eingeschaltet lassen oder Backup-Batterie verwenden.

#### Lösung 2.3: Satelliten-Sichtbarkeit prüfen

**$GPGSV** Sentences zeigen sichtbare Satelliten:

```
$GPGSV,3,1,11,03,03,111,00,04,15,270,00,06,01,010,00,13,06,292,00*74
         |  |   |
         |  |   +-- 11 Satelliten sichtbar
         |  +------ Nachricht 1 von 3
         +--------- GPGSV Sentence

Für jeden Satelliten:
- PRN: 03 (Satelliten-ID)
- Elevation: 03° (sehr niedrig am Horizont!)
- Azimuth: 111° (Richtung)
- SNR: 00 (KEIN SIGNAL!)
```

**Auswertung**:
- **SNR = 0**: Satellit sichtbar, aber kein Signal (blockiert)
- **SNR < 20**: Signal zu schwach
- **SNR 20-35**: Schwach, möglicherweise verwendbar
- **SNR > 35**: Gut verwendbar

**Lösung**: Wenn viele Satelliten SNR=0 haben → Antenne/Sicht verbessern.

#### Lösung 2.4: A-GPS Daten (Advanced)

**A-GPS** (Assisted GPS) beschleunigt GPS-Fix durch Vorab-Download der Ephemeridendaten:

**Für Quectel L80-R**:
- Ephemeridendaten via UART senden (benötigt Internet-Verbindung)
- Quectel bietet PC-Tool für A-GPS-Update

**Implementierung** (in zukünftigem Beispiel):
```cpp
// WiFi-Verbindung herstellen
// Ephemeris-Daten von Quectel-Server laden
// Via Serial2 an GPS senden
```

Reduziert Kaltstartzeit auf **5-10 Sekunden**.

### Problem 3: Display zeigt "GPS TIMEOUT" (Verbindung bricht ab)

**Symptome**:
- GPS sendete Daten, dann plötzlich Timeout
- Status-LED wechselt von 🟢 Grün zu 🔴 Rot
- Serial Monitor: "GPS TIMEOUT - Keine Daten seit 5 Sekunden"

**Mögliche Ursachen & Lösungen**:

#### Lösung 3.1: Wackelkontakt

**Symptom**: Unregelmäßige Timeouts, besonders bei Bewegung/Vibration.

**Test**: CYD/GPS-Modul leicht bewegen während Betrieb.

**Lösung**:
- Dupont-Kabel durch **Löten** ersetzen
- Kabel mit Heißkleber fixieren
- Bessere Steckverbinder verwenden (JST, Molex)

#### Lösung 3.2: Stromunterbrechung

**Symptom**: Timeout immer zur gleichen Zeit (z.B. wenn Display viel zeichnet).

**Grund**: Display-Operationen erhöhen Stromverbrauch → GPS bekommt Unterspannung.

**Test**: Multimeter zwischen GPS VCC/GND anschließen, Spannung beobachten während Display zeichnet.

**Lösung**:
- **100μF Kondensator** zwischen GPS VCC/GND (Puffer-Kapazität)
- Besseres USB-Netzteil (>1A)
- Separate Stromversorgung für GPS

#### Lösung 3.3: Software-Hänger

**Symptom**: Nach längerer Laufzeit (Stunden) friert GPS ein.

**Grund**: Buffer-Überlauf, Memory-Leak, Watchdog-Reset.

**Debugging**:
```cpp
void loop() {
  static unsigned long lastHeapPrint = 0;
  if (millis() - lastHeapPrint > 10000) {
    Serial.printf("Free Heap: %d Bytes\n", ESP.getFreeHeap());
    lastHeapPrint = millis();
  }

  // ... normaler Code
}
```

**Lösung**:
- Heap-Verbrauch überwachen (sollte stabil bleiben)
- String-Operationen minimieren (besser char-Arrays)
- Regelmäßiger Watchdog-Feed (`delay(10)`)

### Problem 4: Display zeigt falsche Zeichen / Kauderwelsch

**Symptome**:
- GPS-Daten werden empfangen, aber unleserlich
- Zeichen wie "�", "▯", oder zufällige Symbole
- NMEA-Struktur ($GPxxx) noch erkennbar

**Mögliche Ursachen & Lösungen**:

#### Lösung 4.1: Falsche Baudrate

**Grund**: CYD und GPS verwenden unterschiedliche Baudraten.

**Test**: Verschiedene Baudraten probieren:
```cpp
Serial2.begin(4800, SERIAL_8N1, extRxD, extTxD);  // Statt 9600
```

**Häufige GPS-Baudraten**: 4800, 9600, 19200, 38400, 57600, 115200

**Quectel L80-R Standard**: 9600 Baud

**Lösung**: Korrekte Baudrate aus Datenblatt des GPS-Moduls prüfen.

#### Lösung 4.2: Falsches Datenformat

**Standard**: 8N1 (8 Datenbits, No Parity, 1 Stop-Bit)

**Alternative Formate** testen:
```cpp
Serial2.begin(9600, SERIAL_7E1, extRxD, extTxD);  // 7 Bits, Even Parity
Serial2.begin(9600, SERIAL_8E1, extRxD, extTxD);  // 8 Bits, Even Parity
```

**NMEA Standard**: 8N1 (fast alle GPS-Module)

#### Lösung 4.3: Signal-Störung

**Symptom**: Einzelne Zeichen falsch, aber nicht alle.

**Grund**: Elektromagnetische Störungen (EMI) von Display/WiFi.

**Lösung**:
- Kabel kürzer halten
- Verdrillte Kabel (Twisted Pair) verwenden
- Kabel von Display-Kabel fernhalten
- Pull-Up Widerstände (1-10 kΩ) an RX/TX

### Problem 5: Zu viele Daten - Display scrollt zu schnell

**Symptome**:
- Daten scrollen so schnell, dass Lesen unmöglich ist
- GPS sendet mehr Sentences als erwartet

**Mögliche Ursachen & Lösungen**:

#### Lösung 5.1: Update-Rate reduzieren

Viele GPS-Module erlauben Konfiguration der Update-Rate via PMTK-Kommandos:

**1 Hz (Standard)**:
```cpp
Serial2.println("$PMTK220,1000*1F");  // 1000ms = 1 Hz
```

**0.5 Hz** (langsamer, besser lesbar):
```cpp
Serial2.println("$PMTK220,2000*1C");  // 2000ms = 0.5 Hz
```

**Für Quectel L80-R**: Siehe Datenblatt für spezifische Kommandos.

#### Lösung 5.2: Sentence-Typen filtern

Nur wichtige Sentences anzeigen:

```cpp
void processNMEASentence(String sentence) {
  // Nur GGA und RMC anzeigen (Position + Navigation)
  if (!(sentence.startsWith("$GPGGA") ||
        sentence.startsWith("$GPRMC"))) {
    return;  // Andere Sentences ignorieren
  }

  // ... normale Verarbeitung
}
```

**Reduziert Datenmenge** von ~8 Sentences/s auf ~2 Sentences/s.

#### Lösung 5.3: Längere Scroll-Pause

Display-Update verzögern:

```cpp
void loop() {
  readGPSData();
  checkTouch();
  checkGPSTimeout();

  delay(50);  // Statt delay(10) → 5× langsamer
}
```

**Nachteil**: Buffer kann überlaufen bei zu langer Pause.

### Problem 6: GPS funktioniert, aber Position ist falsch/springt

**Symptome**:
- GPS-Fix vorhanden (Fix-Status = 1)
- Position weicht stark ab (mehrere Kilometer)
- Position "springt" wild umher

**Mögliche Ursachen & Lösungen**:

#### Lösung 6.1: Schlechte DOP-Werte

**DOP** (Dilution of Precision) zeigt Geometrie der Satelliten:

```
$GPGSA,A,3,04,05,09,12,24,,,,,,,15.2,10.5,11.0*3E
                               |    |    |
                               PDOP HDOP VDOP
```

**DOP-Bewertung**:
- **1-2**: Exzellent → Genauigkeit ±2-5m
- **2-5**: Gut → Genauigkeit ±5-10m
- **5-10**: Moderat → Genauigkeit ±10-50m
- **>10**: Schlecht → Position unbrauchbar

**Lösung**: Bessere Satelliten-Sichtbarkeit (mehr Satelliten, höherer Elevationswinkel).

#### Lösung 6.2: Multipath-Effekte

**Multipath** = Signale werden an Gebäuden/Bergen reflektiert → verfälschte Laufzeitmessungen.

**Symptom**: Position "springt" zwischen 2-3 Positionen hin und her.

**Lösung**:
- Standort mit freier Sicht wählen (weg von Gebäuden)
- GPS-Antenne höher positionieren
- Elevation Mask erhöhen (nur Satelliten >10° Elevation verwenden)

#### Lösung 6.3: Falsche Koordinaten-Umrechnung

**NMEA-Format**: Degrees + Decimal Minutes (`DDMM.MMMM`)

**Fehlerhafte Umrechnung**:
```cpp
// FALSCH:
float lat = sentence.substring(18, 28).toFloat();  // 4807.038 = 4807.038°
```

**Korrekte Umrechnung**:
```cpp
// RICHTIG:
String coord = "4807.038";
float degrees = coord.substring(0, 2).toFloat();  // 48
float minutes = coord.substring(2).toFloat();      // 07.038
float decimal = degrees + (minutes / 60.0);        // 48.1173°
```

**Debugging**: Position mit Google Maps / GPS-Koordinaten-Tool vergleichen.

## 🎯 Erweiterungsideen

### 1. NMEA-Parser mit Position-Anzeige

**Konzept**: Statt rohe NMEA-Daten anzeigen → GPS-Daten parsen und formatiert ausgeben.

**Implementierung**:

```cpp
struct GPSData {
  float latitude = 0.0;
  float longitude = 0.0;
  float altitude = 0.0;
  int satellites = 0;
  float speed = 0.0;        // km/h
  float course = 0.0;       // Grad
  int hour = 0, minute = 0, second = 0;
  bool fix = false;
  float hdop = 99.99;
};

GPSData gpsData;

void parseGPGGA(String sentence) {
  // Beispiel: $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47

  int idx = 0;
  String fields[15];

  // Sentence in Felder aufteilen
  int start = 0;
  for (int i = 0; i < sentence.length(); i++) {
    if (sentence[i] == ',' || sentence[i] == '*') {
      fields[idx++] = sentence.substring(start, i);
      start = i + 1;
    }
  }

  // UTC-Zeit parsen (123519 = 12:35:19)
  if (fields[1].length() == 6) {
    gpsData.hour = fields[1].substring(0, 2).toInt();
    gpsData.minute = fields[1].substring(2, 4).toInt();
    gpsData.second = fields[1].substring(4, 6).toInt();
  }

  // Breitengrad parsen (4807.038 N)
  if (fields[2].length() > 0) {
    float deg = fields[2].substring(0, 2).toFloat();
    float min = fields[2].substring(2).toFloat();
    gpsData.latitude = deg + (min / 60.0);
    if (fields[3] == "S") gpsData.latitude = -gpsData.latitude;
  }

  // Längengrad parsen (01131.000 E)
  if (fields[4].length() > 0) {
    float deg = fields[4].substring(0, 3).toFloat();
    float min = fields[4].substring(3).toFloat();
    gpsData.longitude = deg + (min / 60.0);
    if (fields[5] == "W") gpsData.longitude = -gpsData.longitude;
  }

  // Fix-Status
  gpsData.fix = (fields[6].toInt() > 0);

  // Satelliten
  gpsData.satellites = fields[7].toInt();

  // HDOP
  gpsData.hdop = fields[8].toFloat();

  // Höhe
  gpsData.altitude = fields[9].toFloat();
}

void parseGPRMC(String sentence) {
  // $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A

  // ... Felder parsen (siehe oben)

  // Geschwindigkeit (Knoten → km/h)
  float speedKnots = fields[7].toFloat();
  gpsData.speed = speedKnots * 1.852;  // 1 Knoten = 1.852 km/h

  // Kurs
  gpsData.course = fields[8].toFloat();
}

void displayGPSData() {
  lcd.fillScreen(TFT_BLACK);

  int y = 10;
  lcd.setCursor(10, y); y += 20;
  lcd.setTextColor(TFT_CYAN);
  lcd.println("=== GPS DATA ===");

  // Position
  lcd.setTextColor(TFT_WHITE);
  lcd.setCursor(10, y); y += 20;
  lcd.printf("Lat: %.6f°", gpsData.latitude);
  lcd.setCursor(10, y); y += 20;
  lcd.printf("Lon: %.6f°", gpsData.longitude);

  // Höhe
  lcd.setCursor(10, y); y += 20;
  lcd.printf("Alt: %.1f m", gpsData.altitude);

  // Satelliten & Genauigkeit
  lcd.setCursor(10, y); y += 20;
  lcd.printf("Sats: %d  HDOP: %.1f", gpsData.satellites, gpsData.hdop);

  // Geschwindigkeit & Kurs
  lcd.setCursor(10, y); y += 20;
  lcd.printf("Speed: %.1f km/h", gpsData.speed);
  lcd.setCursor(10, y); y += 20;
  lcd.printf("Course: %.1f°", gpsData.course);

  // Zeit
  lcd.setCursor(10, y); y += 20;
  lcd.printf("UTC: %02d:%02d:%02d", gpsData.hour, gpsData.minute, gpsData.second);

  // Fix-Status
  lcd.setCursor(10, y); y += 20;
  if (gpsData.fix) {
    lcd.setTextColor(TFT_GREEN);
    lcd.println("GPS FIX: OK");
  } else {
    lcd.setTextColor(TFT_RED);
    lcd.println("GPS FIX: NO");
  }
}
```

**Anwendungen**:
- GPS-Logger für Wandern/Radfahren
- Koordinaten-Anzeige für Geocaching
- Geschwindigkeitsmesser
- Echtzeit-Uhr (UTC-Zeit von GPS)

**Bibliothek**: TinyGPSPlus (Arduino Library) vereinfacht NMEA-Parsing erheblich!

### 2. Karten-Anzeige mit GPS-Position

**Konzept**: Zeige eine Karte auf dem Display mit aktuellem GPS-Standort.

**Implementierung (Offline-Karte)**:

```cpp
// Karte als Bitmap (320x240) im Flash speichern
#include "map_munich.h"  // Enthält uint8_t mapData[320*240*2]

// Geo-Bounds der Karte
const float MAP_LAT_MIN = 48.0;
const float MAP_LAT_MAX = 48.3;
const float MAP_LON_MIN = 11.3;
const float MAP_LON_MAX = 11.8;

void drawMap() {
  // Karte auf Display zeichnen (aus Flash)
  lcd.pushImage(0, 0, 320, 240, (uint16_t*)mapData);
}

void drawGPSMarker(float lat, float lon) {
  // GPS-Position auf Karten-Koordinaten umrechnen
  int x = map(lon, MAP_LON_MIN, MAP_LON_MAX, 0, 320);
  int y = map(lat, MAP_LAT_MAX, MAP_LAT_MIN, 0, 240);  // Y ist invertiert!

  // Marker zeichnen (roter Kreis)
  lcd.fillCircle(x, y, 5, TFT_RED);
  lcd.drawCircle(x, y, 6, TFT_WHITE);
}

void loop() {
  // GPS-Daten lesen & parsen (siehe Erweiterung 1)

  if (gpsDataUpdated) {
    drawMap();
    drawGPSMarker(gpsData.latitude, gpsData.longitude);
  }
}
```

**Karten-Quelle**:
- **OpenStreetMap**: Screenshot als Bitmap speichern
- **Bildkonverter**: GIMP / ImageMagick → RGB565 Raw
- **LVGL Image Converter**: Online-Tool für Bitmap → C-Array

**Anwendungen**:
- Navigationsgerät
- Wanderkarte mit GPS-Tracking
- Geo-Caching

**Limitierung**: Display-Größe (320x240) → nur kleiner Kartenausschnitt

### 3. GPS-Datenlogger auf SD-Karte

**Konzept**: GPS-Track auf SD-Karte speichern (GPX-Format).

**Hardware**:
- **Micro-SD-Karten-Modul** (SPI-Interface)
- Verbindung: MISO/MOSI/SCK/CS an freie GPIOs

**GPX-Format** (GPS Exchange Format):
```xml
<?xml version="1.0"?>
<gpx version="1.1">
  <trk>
    <name>CYD GPS Track</name>
    <trkseg>
      <trkpt lat="48.1173" lon="11.5167">
        <ele>545.4</ele>
        <time>2024-01-15T12:35:19Z</time>
      </trkpt>
      <trkpt lat="48.1175" lon="11.5170">
        <ele>546.2</ele>
        <time>2024-01-15T12:35:20Z</time>
      </trkpt>
    </trkseg>
  </trk>
</gpx>
```

**Implementierung**:

```cpp
#include <SD.h>
#include <SPI.h>

#define SD_CS 5  // Chip-Select für SD-Karte

File gpxFile;
unsigned long lastLogTime = 0;

void setup() {
  // ... GPS-Setup

  // SD-Karte initialisieren
  if (SD.begin(SD_CS)) {
    Serial.println("SD-Karte bereit");

    // GPX-Datei erstellen
    gpxFile = SD.open("/track.gpx", FILE_WRITE);
    gpxFile.println("<?xml version=\"1.0\"?>");
    gpxFile.println("<gpx version=\"1.1\">");
    gpxFile.println("  <trk><name>CYD Track</name><trkseg>");
    gpxFile.close();
  }
}

void logGPSPoint() {
  if (!gpsData.fix) return;  // Nur bei gültigem Fix loggen

  // Alle 5 Sekunden einen Punkt loggen
  if (millis() - lastLogTime < 5000) return;
  lastLogTime = millis();

  gpxFile = SD.open("/track.gpx", FILE_APPEND);

  gpxFile.print("    <trkpt lat=\"");
  gpxFile.print(gpsData.latitude, 6);
  gpxFile.print("\" lon=\"");
  gpxFile.print(gpsData.longitude, 6);
  gpxFile.println("\">");

  gpxFile.print("      <ele>");
  gpxFile.print(gpsData.altitude, 1);
  gpxFile.println("</ele>");

  // Zeit (ISO 8601 Format)
  gpxFile.printf("      <time>2024-01-15T%02d:%02d:%02dZ</time>\n",
                 gpsData.hour, gpsData.minute, gpsData.second);

  gpxFile.println("    </trkpt>");

  gpxFile.close();

  Serial.println("GPS-Punkt geloggt");
}

void finishGPX() {
  // Beim Ausschalten: GPX-Datei schließen
  gpxFile = SD.open("/track.gpx", FILE_APPEND);
  gpxFile.println("  </trkseg></trk>");
  gpxFile.println("</gpx>");
  gpxFile.close();
}
```

**Anwendungen**:
- Fahrrad-Computer
- Wanderrouten-Aufzeichnung
- Fahrzeug-Tracker

**GPX-Datei öffnen**:
- Google Earth: File → Open
- Google Maps: My Maps → Import
- GPS Visualizer: https://www.gpsvisualizer.com/

### 4. Geschwindigkeitsmesser mit Graf

**Konzept**: GPS-Geschwindigkeit als großer Tacho + Verlaufs-Graf anzeigen.

**Implementierung**:

```cpp
#define GRAPH_SIZE 160  // 160 Datenpunkte (160 Pixel Breite)
float speedHistory[GRAPH_SIZE];
int graphIndex = 0;

void displaySpeedometer() {
  lcd.fillScreen(TFT_BLACK);

  // --- Großanzeige Geschwindigkeit ---
  lcd.setTextSize(4);
  lcd.setTextColor(TFT_GREEN);
  lcd.setCursor(50, 30);
  lcd.printf("%.1f", gpsData.speed);

  lcd.setTextSize(2);
  lcd.setTextColor(TFT_WHITE);
  lcd.setCursor(200, 50);
  lcd.println("km/h");

  // --- Richtungs-Pfeil ---
  drawCompass(160, 120, 30, gpsData.course);

  // --- Geschwindigkeits-Graf ---
  drawSpeedGraph();
}

void drawCompass(int cx, int cy, int radius, float bearing) {
  // Kreis
  lcd.drawCircle(cx, cy, radius, TFT_WHITE);

  // Himmelsrichtungen
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_YELLOW);
  lcd.setCursor(cx - 3, cy - radius - 12);
  lcd.print("N");
  lcd.setCursor(cx + radius + 5, cy - 3);
  lcd.print("E");
  lcd.setCursor(cx - 3, cy + radius + 5);
  lcd.print("S");
  lcd.setCursor(cx - radius - 10, cy - 3);
  lcd.print("W");

  // Richtungs-Pfeil
  float rad = (bearing - 90) * PI / 180.0;
  int x2 = cx + radius * cos(rad);
  int y2 = cy + radius * sin(rad);

  lcd.drawLine(cx, cy, x2, y2, TFT_RED);
  lcd.fillCircle(x2, y2, 3, TFT_RED);
}

void drawSpeedGraph() {
  // Geschwindigkeit in History speichern
  speedHistory[graphIndex] = gpsData.speed;
  graphIndex = (graphIndex + 1) % GRAPH_SIZE;

  int graphY = 180;
  int graphHeight = 50;

  // Graf-Rahmen
  lcd.drawRect(0, graphY, GRAPH_SIZE, graphHeight, TFT_WHITE);

  // Maximale Geschwindigkeit finden (für Skalierung)
  float maxSpeed = 50.0;  // Mindestens 50 km/h
  for (int i = 0; i < GRAPH_SIZE; i++) {
    if (speedHistory[i] > maxSpeed) maxSpeed = speedHistory[i];
  }

  // Graf zeichnen
  for (int i = 1; i < GRAPH_SIZE; i++) {
    int idx1 = (graphIndex + i - 1) % GRAPH_SIZE;
    int idx2 = (graphIndex + i) % GRAPH_SIZE;

    int y1 = graphY + graphHeight - (speedHistory[idx1] / maxSpeed * graphHeight);
    int y2 = graphY + graphHeight - (speedHistory[idx2] / maxSpeed * graphHeight);

    lcd.drawLine(i - 1, y1, i, y2, TFT_GREEN);
  }

  // Max-Speed Label
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_YELLOW);
  lcd.setCursor(165, graphY);
  lcd.printf("%.0f", maxSpeed);
}
```

**Features**:
- **Große Zahlen**: Geschwindigkeit gut ablesbar
- **Kompass**: Zeigt Fahrtrichtung (Kurs über Grund)
- **Verlaufs-Graf**: Letzte 160 Sekunden Geschwindigkeit

**Anwendungen**:
- Fahrrad-Tacho
- Auto-HUD (Head-Up Display)
- Boot-Geschwindigkeitsmesser

### 5. Satelliten-Skyplot (Polar-Diagramm)

**Konzept**: Zeige Satelliten-Positionen als Polar-Plot (Azimuth + Elevation).

**Implementierung**:

```cpp
struct Satellite {
  int prn;        // Satelliten-ID
  int elevation;  // Höhenwinkel (0-90°)
  int azimuth;    // Richtung (0-359°)
  int snr;        // Signal-Stärke (0-99)
};

Satellite satellites[20];
int satCount = 0;

void parseGPGSV(String sentence) {
  // $GPGSV,3,1,11,03,03,111,00,04,15,270,00,06,01,010,00,13,06,292,00*74

  // ... Felder parsen

  int totalSats = fields[3].toInt();

  // 4 Satelliten pro GPGSV-Nachricht
  for (int i = 0; i < 4; i++) {
    int offset = 4 + i * 4;

    if (fields[offset].length() == 0) break;

    satellites[satCount].prn = fields[offset].toInt();
    satellites[satCount].elevation = fields[offset + 1].toInt();
    satellites[satCount].azimuth = fields[offset + 2].toInt();
    satellites[satCount].snr = fields[offset + 3].toInt();

    satCount++;
    if (satCount >= 20) break;
  }
}

void drawSkyplot() {
  lcd.fillScreen(TFT_BLACK);

  int centerX = 160;
  int centerY = 120;
  int maxRadius = 100;

  // Ringe (0°, 30°, 60°, 90° Elevation)
  for (int elev = 0; elev <= 90; elev += 30) {
    int r = maxRadius * (90 - elev) / 90;
    lcd.drawCircle(centerX, centerY, r, TFT_DARKGREY);
  }

  // Himmelsrichtungen
  lcd.setTextSize(2);
  lcd.setTextColor(TFT_YELLOW);
  lcd.setCursor(centerX - 8, centerY - maxRadius - 20);
  lcd.print("N");
  lcd.setCursor(centerX + maxRadius + 10, centerY - 8);
  lcd.print("E");
  lcd.setCursor(centerX - 8, centerY + maxRadius + 10);
  lcd.print("S");
  lcd.setCursor(centerX - maxRadius - 20, centerY - 8);
  lcd.print("W");

  // Satelliten zeichnen
  for (int i = 0; i < satCount; i++) {
    // Polar → Kartesisch
    float rad = (satellites[i].azimuth - 90) * PI / 180.0;
    float dist = maxRadius * (90 - satellites[i].elevation) / 90.0;

    int x = centerX + dist * cos(rad);
    int y = centerY + dist * sin(rad);

    // Farbe nach SNR
    uint16_t color;
    if (satellites[i].snr == 0) color = TFT_DARKGREY;
    else if (satellites[i].snr < 20) color = TFT_RED;
    else if (satellites[i].snr < 35) color = TFT_ORANGE;
    else color = TFT_GREEN;

    // Satelliten-Kreis
    lcd.fillCircle(x, y, 5, color);

    // PRN-Nummer
    lcd.setTextSize(1);
    lcd.setTextColor(TFT_WHITE);
    lcd.setCursor(x - 6, y - 12);
    lcd.print(satellites[i].prn);
  }

  // Legende
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_WHITE);
  lcd.setCursor(10, 220);
  lcd.printf("Satelliten: %d", satCount);
}
```

**Interpretation**:
- **Mitte**: Direkt über Kopf (90° Elevation)
- **Rand**: Horizont (0° Elevation)
- **Farbe**:
  - Grau: Kein Signal
  - Rot: Schwaches Signal
  - Orange: Mittleres Signal
  - Grün: Starkes Signal

**Anwendungen**:
- GPS-Diagnose (warum kein Fix?)
- Satelliten-Tracking
- Optimale Antennen-Positionierung finden

### 6. Geo-Fence Alarm (Virtueller Zaun)

**Konzept**: Alarm auslösen, wenn GPS-Position einen definierten Bereich verlässt.

**Implementierung**:

```cpp
// Geo-Fence Zentrum
const float FENCE_LAT = 48.1173;
const float FENCE_LON = 11.5167;
const float FENCE_RADIUS = 100.0;  // Meter

bool insideFence = true;

float haversineDistance(float lat1, float lon1, float lat2, float lon2) {
  // Haversine-Formel: Entfernung zwischen 2 GPS-Punkten

  const float R = 6371000.0;  // Erdradius in Metern

  float dLat = (lat2 - lat1) * PI / 180.0;
  float dLon = (lon2 - lon1) * PI / 180.0;

  float a = sin(dLat / 2) * sin(dLat / 2) +
            cos(lat1 * PI / 180.0) * cos(lat2 * PI / 180.0) *
            sin(dLon / 2) * sin(dLon / 2);

  float c = 2 * atan2(sqrt(a), sqrt(1 - a));

  return R * c;  // Distanz in Metern
}

void checkGeoFence() {
  if (!gpsData.fix) return;

  float distance = haversineDistance(
    gpsData.latitude, gpsData.longitude,
    FENCE_LAT, FENCE_LON
  );

  bool nowInside = (distance <= FENCE_RADIUS);

  // Status-Wechsel erkannt?
  if (insideFence && !nowInside) {
    // GEO-FENCE VERLASSEN!
    triggerAlarm();
  }

  insideFence = nowInside;

  // Anzeige
  lcd.setCursor(10, 200);
  lcd.printf("Abstand: %.1f m", distance);

  if (insideFence) {
    lcd.setTextColor(TFT_GREEN);
    lcd.println(" [IN]");
  } else {
    lcd.setTextColor(TFT_RED);
    lcd.println(" [OUT]");
  }
}

void triggerAlarm() {
  // Akustischer Alarm (Piezo-Buzzer an GPIO)
  tone(BUZZER_PIN, 2000, 1000);  // 2 kHz, 1 Sekunde

  // Visueller Alarm
  for (int i = 0; i < 5; i++) {
    lcd.fillScreen(TFT_RED);
    delay(200);
    lcd.fillScreen(TFT_BLACK);
    delay(200);
  }

  // Benachrichtigung senden (WiFi/MQTT)
  sendPushNotification("Geo-Fence verlassen!");
}
```

**Haversine-Formel** (Großkreis-Entfernung):
```
a = sin²(Δlat/2) + cos(lat1) × cos(lat2) × sin²(Δlon/2)
c = 2 × atan2(√a, √(1−a))
d = R × c
```

Wobei R = Erdradius (6371 km).

**Anwendungen**:
- Kinder-Tracker (Warnung bei Verlassen des Schulgeländes)
- Haustier-Tracker
- Diebstahl-Schutz (Fahrzeug/Boot)
- Wanderung (Warnung bei Abweichung von Route)

### 7. WiFi-Integration: GPS-Daten über MQTT senden

**Konzept**: GPS-Position in Echtzeit über WiFi an Home Assistant / MQTT-Broker senden.

**Implementierung**:

```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include "Credentials.h"  // const char* ssid, password, mqtt_server

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

void setup() {
  // ... GPS-Setup

  // WiFi verbinden
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi verbunden");

  // MQTT-Broker
  mqtt.setServer(mqtt_server, 1883);
}

void publishGPSData() {
  if (!gpsData.fix) return;
  if (!mqtt.connected()) {
    mqtt.connect("CYD-GPS");
  }

  // JSON-Format für Home Assistant
  String payload = "{";
  payload += "\"latitude\":" + String(gpsData.latitude, 6) + ",";
  payload += "\"longitude\":" + String(gpsData.longitude, 6) + ",";
  payload += "\"altitude\":" + String(gpsData.altitude, 1) + ",";
  payload += "\"speed\":" + String(gpsData.speed, 1) + ",";
  payload += "\"satellites\":" + String(gpsData.satellites) + ",";
  payload += "\"hdop\":" + String(gpsData.hdop, 1);
  payload += "}";

  mqtt.publish("homeassistant/sensor/cyd_gps/state", payload.c_str());

  Serial.println("GPS-Daten via MQTT gesendet");
}

void loop() {
  // GPS-Daten lesen...

  // Alle 10 Sekunden via MQTT senden
  static unsigned long lastPublish = 0;
  if (millis() - lastPublish > 10000) {
    publishGPSData();
    lastPublish = millis();
  }

  mqtt.loop();
}
```

**Home Assistant Integration**:

```yaml
# configuration.yaml
sensor:
  - platform: mqtt
    name: "CYD GPS Position"
    state_topic: "homeassistant/sensor/cyd_gps/state"
    value_template: "{{ value_json.latitude }},{{ value_json.longitude }}"
    json_attributes_topic: "homeassistant/sensor/cyd_gps/state"
```

**Anwendungen**:
- Echtzeit-Tracking auf Home Assistant
- Automatisierungen (z.B. Licht einschalten bei Annäherung)
- GPS-Logger in InfluxDB
- Bewegungsprofile analysieren

## 📚 Weiterführende Themen

### GPS-Technologien im Vergleich

**A-GPS** (Assisted GPS):
- Ephemeridendaten via Internet vorab laden
- Reduziert Kaltstartzeit von 30s auf <5s
- Benötigt Mobilfunk- oder WiFi-Verbindung

**D-GPS** (Differential GPS):
- Referenzstation sendet Korrekturdaten für lokale Fehler
- Genauigkeit: <1 Meter (statt 5-10m)
- Echtzeit-Korrektur (RTK = Real-Time Kinematic)

**Multi-GNSS**:
- Kombination GPS + GLONASS + Galileo + BeiDou
- Bis zu 100 sichtbare Satelliten (statt 6-12)
- Bessere Genauigkeit, schnellerer Fix

**RTK-GPS** (Real-Time Kinematic):
- Zentimeter-Genauigkeit (1-2 cm!)
- Benötigt Basisstation mit bekannter Position
- Anwendung: Landwirtschaft, Vermessung

### NMEA vs. UBX Protokoll

**NMEA 0183** (verwendet in diesem Beispiel):
- ✅ ASCII-Text (human-readable)
- ✅ Universell unterstützt (alle GPS-Module)
- ✅ Einfach zu parsen (String-Operationen)
- ❌ Langsam (9600 Baud typisch)
- ❌ Viel Overhead (XML-ähnlich)

**UBX-Protokoll** (u-blox proprietär):
- ✅ Binär-Format (kompakt, schnell)
- ✅ Höhere Baudrate möglich (115200+)
- ✅ Mehr Konfigurationsoptionen
- ❌ Komplexer zu parsen
- ❌ Nur u-blox Module

**SiRF-Protokoll**:
- Binär-Format (veraltet)
- Nur alte GPS-Module (SiRFstar III)

### GPS-Genauigkeit verbessern

**1. SBAS aktivieren** (Satellite-Based Augmentation):
```cpp
// PMTK-Kommando für SBAS (WAAS/EGNOS)
Serial2.println("$PMTK313,1*2E");  // SBAS aktivieren
Serial2.println("$PMTK301,2*2E");  // SBAS + GPS
```

Verbessert Genauigkeit auf 1-3 Meter.

**2. Update-Rate erhöhen**:
```cpp
Serial2.println("$PMTK220,100*2F");  // 10 Hz (100ms)
```

Mehr Messpunkte = bessere Durchschnittsbildung.

**3. Satellite-Based Filtering**:
```cpp
// Nur Satelliten mit SNR > 35 verwenden
if (satellites[i].snr < 35) continue;
```

Schwache Satelliten tragen mehr Fehler bei.

**4. Kalman-Filter** (Software-Filter):
```cpp
// Einfacher Kalman-Filter für GPS-Position
float kalmanLat = gpsData.latitude;
float kalmanLon = gpsData.longitude;

void updateKalman() {
  const float Q = 0.01;  // Process Noise
  const float R = 0.1;   // Measurement Noise

  // Prediction (keine Bewegung angenommen)
  float predLat = kalmanLat;
  float predLon = kalmanLon;

  // Update
  float K = 1.0 / (1.0 + R);  // Kalman Gain
  kalmanLat = predLat + K * (gpsData.latitude - predLat);
  kalmanLon = predLon + K * (gpsData.longitude - predLon);
}
```

Glättet "springende" GPS-Positionen.

### Zeitzonen & UTC-Konvertierung

GPS sendet **immer UTC-Zeit** (Coordinated Universal Time).

**Umrechnung zu Lokalzeit**:
```cpp
int TIMEZONE_OFFSET = +1;  // MEZ (Mitteleuropäische Zeit)
// Sommerzeit (MESZ): +2

int localHour = (gpsData.hour + TIMEZONE_OFFSET + 24) % 24;
```

**Bessere Lösung**: NTP + Zeitzonen-Library
```cpp
#include <TimeLib.h>
#include <Timezone.h>

// Mitteleuropäische Zeit
TimeChangeRule CEST = {"MESZ", Last, Sun, Mar, 2, 120};  // Sommerzeit
TimeChangeRule CET = {"MEZ", Last, Sun, Oct, 3, 60};     // Winterzeit
Timezone CE(CEST, CET);

void displayLocalTime() {
  time_t utc = makeTime({gpsData.second, gpsData.minute, gpsData.hour, 1, 1, 2024});
  time_t local = CE.toLocal(utc);

  lcd.printf("%02d:%02d:%02d", hour(local), minute(local), second(local));
}
```

### Dead Reckoning (Koppelnavigation)

**Problem**: GPS-Signal geht verloren (Tunnel, Tiefgarage, Gebäude).

**Lösung**: Position schätzen basierend auf:
- Letzter bekannter Position
- Geschwindigkeit
- Kurs

**Implementierung**:
```cpp
unsigned long lastGPSTime = 0;

void deadReckoning() {
  if (gpsData.fix) {
    // GPS verfügbar → Position übernehmen
    lastGPSTime = millis();
  } else {
    // GPS nicht verfügbar → Position schätzen
    unsigned long elapsed = (millis() - lastGPSTime) / 1000;  // Sekunden

    // Entfernung = Geschwindigkeit × Zeit
    float distance = gpsData.speed / 3.6 * elapsed;  // km/h → m/s

    // Neue Position = Alte Position + Bewegung in Kurs-Richtung
    float rad = gpsData.course * PI / 180.0;
    float dLat = (distance / 111320.0) * cos(rad);  // 1° Lat ≈ 111.32 km
    float dLon = (distance / (111320.0 * cos(gpsData.latitude * PI / 180.0))) * sin(rad);

    gpsData.latitude += dLat;
    gpsData.longitude += dLon;

    // Warnung anzeigen
    lcd.setTextColor(TFT_ORANGE);
    lcd.println("[GESCHÄTZT - KEIN GPS]");
  }
}
```

**Verwendung**: Automotive-Navigation (Tunnel), Indoor-Tracking mit IMU-Fusion.

## 🛠️ Praktische Tipps

### Tipp 1: GPS-Fix beschleunigen

**Problem**: Kaltstartzeit 30-60 Sekunden ist zu lang.

**Lösungen**:
1. **Backup-Batterie**: CR1220 Batterie speichert Ephemeridendaten (Warmstart <5s)
2. **A-GPS**: Ephemeridendaten via WiFi vorab laden
3. **GPS immer eingeschaltet lassen**: Batterie-Betrieb mit Sleep-Mode
4. **Multi-GNSS**: GPS + GLONASS → mehr Satelliten

### Tipp 2: Stromverbrauch reduzieren

**Problem**: GPS + Display = ~300 mA → Batterie leer nach 3-4 Stunden (1000 mAh Akku).

**Lösungen**:
1. **Display dimmen**:
   ```cpp
   ledcSetup(0, 5000, 8);  // PWM für Display-Backlight
   ledcAttachPin(BACKLIGHT_PIN, 0);
   ledcWrite(0, 50);  // 50% Helligkeit
   ```

2. **GPS Update-Rate reduzieren**:
   ```cpp
   Serial2.println("$PMTK220,5000*1B");  // 0.2 Hz (5 Sekunden)
   ```

3. **GPS Power-Saving Mode**:
   ```cpp
   Serial2.println("$PMTK161,0*28");  // Standby-Mode
   // Aufwecken via Serial2.write() oder RTC-Alarm
   ```

4. **Deep Sleep** zwischen Messungen:
   ```cpp
   esp_sleep_enable_timer_wakeup(60 * 1000000);  // 60 Sekunden
   esp_deep_sleep_start();
   ```

### Tipp 3: Bessere Antenne für Indoor-Empfang

**Problem**: GPS funktioniert nur am Fenster, nicht im Innenraum.

**Lösungen**:
1. **Aktive Antenne**: Mit eingebautem LNA (Low-Noise Amplifier)
2. **Externe Antenne**: Auf Dach/Balkon, langes Kabel zu CYD
3. **Repeater**: GPS-Antenne außen + Indoor-Rebroadcast (teuer)

**Tipp**: Patch-Antennen sind besser als Chip-Antennen (größere Fläche = mehr Signal).

### Tipp 4: NMEA-Checksum validieren

**Problem**: Fehlerhafte Daten durch Störungen auf serieller Leitung.

**Lösung**: Checksum prüfen:
```cpp
bool validateNMEA(String sentence) {
  int asterisk = sentence.indexOf('*');
  if (asterisk < 0) return false;  // Keine Checksum

  // Checksum extrahieren
  String checksumStr = sentence.substring(asterisk + 1, asterisk + 3);
  uint8_t expectedChecksum = strtol(checksumStr.c_str(), NULL, 16);

  // Checksum berechnen (XOR aller Zeichen zwischen $ und *)
  uint8_t calculatedChecksum = 0;
  for (int i = 1; i < asterisk; i++) {  // Start bei 1 (nach $)
    calculatedChecksum ^= sentence[i];
  }

  return (calculatedChecksum == expectedChecksum);
}
```

**Verwendung**:
```cpp
if (!validateNMEA(gpsBuffer)) {
  Serial.println("FEHLER: Ungültige Checksum!");
  return;
}
```

### Tipp 5: GPS-Modul konfigurieren via PMTK

**PMTK** (Packet Type) = Konfigurations-Kommandos für MediaTek-Chipsätze.

**Wichtige Kommandos**:

```cpp
// Baudrate ändern (9600 → 115200)
Serial2.println("$PMTK251,115200*1F");
delay(100);
Serial2.end();
Serial2.begin(115200, SERIAL_8N1, extRxD, extTxD);

// Update-Rate ändern (1 Hz → 10 Hz)
Serial2.println("$PMTK220,100*2F");

// Nur GGA + RMC senden (reduziert Datenflut)
Serial2.println("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28");
//                         GGA RMC (1 = an, 0 = aus)

// SBAS aktivieren (WAAS/EGNOS)
Serial2.println("$PMTK313,1*2E");

// GPS-Modul resetten (Cold Start)
Serial2.println("$PMTK104*37");
```

**Checksum berechnen**:
```cpp
String calculatePMTKChecksum(String cmd) {
  uint8_t checksum = 0;
  for (int i = 1; i < cmd.length(); i++) {
    checksum ^= cmd[i];
  }
  return String(checksum, HEX);
}

// Verwendung:
String cmd = "$PMTK220,100";
cmd += "*" + calculatePMTKChecksum(cmd);
Serial2.println(cmd);
```

## 📖 Ressourcen & Links

### Datenblätter & Dokumentation

**Quectel L80-R GPS-Modul**:
- [Quectel L80-R Datasheet (PDF)](https://www.quectel.com/product/gnss-l80-r)
- [Quectel L80-R Hardware Design Guide](https://www.quectel.com/download/quectel_l80-r_hardware_design_v1-0/)
- [L80-R NMEA Messages User Manual](https://www.quectel.com/download/quectel_l80-r_nmea_messages_v1-0/)

**NMEA 0183 Protokoll**:
- [NMEA 0183 Standard Specification](https://www.nmea.org/content/STANDARDS/NMEA_0183_Standard)
- [NMEA Sentence Reference (gpsinformation.org)](http://www.gpsinformation.org/dale/nmea.htm)
- [NMEA Parser Tutorial](https://www.sparkfun.com/datasheets/GPS/NMEA%20Reference%20Manual-Rev2.1-Dec07.pdf)

**GPS-Technologie**:
- [GPS.gov - Official U.S. Government GPS Information](https://www.gps.gov/)
- [How GPS Works (NASA)](https://www.nasa.gov/directorates/heo/scan/communications/policy/GPS_how_it_works.html)
- [GPS Signal Structure](https://gssc.esa.int/navipedia/index.php/GPS_Signal_Plan)

**MediaTek PMTK Kommandos**:
- [PMTK Command Packet (Adafruit)](https://cdn-shop.adafruit.com/datasheets/PMTK_A11.pdf)
- [MediaTek MT3339 GPS Chipset](https://labs.mediatek.com/en/chipset/MT3339)

### ESP32 Hardware-UART

- [ESP32 UART Documentation (Espressif)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html)
- [Arduino ESP32 Serial Reference](https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/HardwareSerial.h)

### CYD-Spezifisch

- **CYD Display Config**: `libraries/MyLGFXConfigs/CYD_Display_Config.h`
- **Serielle Pins**:
  - extRxD (GPIO 16) - Empfang von GPS TX
  - extTxD (GPIO 17) - Senden zu GPS RX
- **I2C Pins** (für andere Sensoren): extSDA (GPIO 22), extSCL (GPIO 27)

### Arduino Libraries

**TinyGPSPlus** (empfohlen für NMEA-Parsing):
```
Arduino IDE -> Tools -> Manage Libraries -> "TinyGPSPlus"
```
- [TinyGPSPlus GitHub](https://github.com/mikalhart/TinyGPSPlus)
- [TinyGPSPlus Dokumentation](http://arduiniana.org/libraries/tinygpsplus/)

**Alternativen**:
- **NeoGPS**: Schneller, komplexer
- **Adafruit GPS Library**: Einfach, für Adafruit-Module optimiert

### Verwandte Beispiele

**CYD-Academy Beispiele**:
- **Beispiel 07**: WiFi Clock (WiFi-Verbindung, Zeitserver)
- **Beispiel 11**: Scrolling Chart (Daten-Visualisierung)
- **Beispiel 18**: Compass Orientation (BNO055 IMU, Kompass)
- **Beispiel 22**: Weather Station (BMP180, Sensor-Logging)

**Kombinations-Ideen**:
- GPS + BNO055 → Dead Reckoning Navigation
- GPS + WiFi → Echtzeit-Tracking via MQTT
- GPS + SD-Card → GPX-Logger

### Online-Tools

**GPS-Koordinaten konvertieren**:
- [GPS Coordinates Converter](https://www.latlong.net/Show-Latitude-Longitude.html)
- [Decimal to DMS Converter](https://www.fcc.gov/media/radio/dms-decimal)

**GPX-Dateien anzeigen**:
- [GPS Visualizer](https://www.gpsvisualizer.com/)
- [MyGPSFiles](https://www.mygpsfiles.com/en/)

**Satelliten-Sichtbarkeit**:
- [GPS Satellite Visibility](https://www.gnssplanning.com/)
- [N2YO GPS Satellite Tracker](https://www.n2yo.com/)

### Video-Tutorials

- [How GPS Works (YouTube - National Geographic)](https://www.youtube.com/watch?v=FU_pY2sTwTA)
- [ESP32 GPS Tutorial (YouTube - Andreas Spiess)](https://www.youtube.com/watch?v=TwhCX0c8Xe0)
- [NMEA Sentence Parsing Explained](https://www.youtube.com/watch?v=2AjfOT-zB0A)

## 🎓 Lernziele

Nach Abschluss dieses Beispiels solltest du:

- ✅ **Hardware-UART des ESP32 verstehen**: Serial, Serial1, Serial2 und ihre Unterschiede
- ✅ **Serielle Kommunikation konfigurieren**: Baudrate, Datenformat (8N1), RX/TX-Pins
- ✅ **GPS-Funktionsprinzip kennen**: Trilateration, Satelliten, Fehlerquellen
- ✅ **NMEA 0183 Protokoll verstehen**: Sentence-Struktur, wichtigste Typen (GGA, RMC, GSA, GSV)
- ✅ **NMEA-Daten parsen können**: String-Operationen, Feldaufteilung, Koordinatenumrechnung
- ✅ **Scrollenden Monitor implementieren**: Ringpuffer, automatisches Scrollen, Farbcodierung
- ✅ **GPS-Probleme diagnostizieren**: Verkabelung, Antennenposition, Kaltstartzeit, DOP-Werte
- ✅ **GPS-Genauigkeit interpretieren**: HDOP, Satellitenzahl, SNR, Fix-Status
- ✅ **Erweiterungen umsetzen können**: GPS-Logger, Karten-Anzeige, Geo-Fence, MQTT-Integration

## 🔜 Nächste Schritte

Weitere GPS-Beispiele in der CYD-Academy:

- **Beispiel 24b**: GPS Navigator mit Karten-Anzeige (Offline-Karten auf SD-Card)
- **Beispiel 24c**: GPS-Logger (GPX-Format auf SD-Card, Track-Visualisierung)
- **Beispiel 24d**: GPS + BNO055 IMU Fusion (Dead Reckoning für Tunnel-Navigation)
- **Beispiel 24e**: GPS Speed Tracker (Geschwindigkeits-Graf, Max-Speed, Durchschnitt)

**Kombinationen mit anderen Sensoren**:
- GPS + BMP180 → Höhenmesser-Vergleich (GPS vs. Barometer)
- GPS + BNO055 → Fahrzeug-Navigation mit Kompass
- GPS + WiFi → Echtzeit-Tracking auf Karte

**Fortgeschrittene Themen**:
- UBX-Protokoll (binär, schneller als NMEA)
- RTK-GPS (Zentimeter-Genauigkeit)
- Multi-GNSS (GPS + GLONASS + Galileo)
- Kalman-Filter für GPS-Positionsglättung

---

**Viel Erfolg mit GPS-Navigation auf dem CYD! 🌍📡**

Bei Fragen: CYD-Academy GitHub Issues
