# CYD Academy - Arduino Beispiele

Dieses Repository enthält Arduino IDE Beispiele für das **CYD (Cheap Yellow Display)** mit der **LovyanGFX** Library.

## Hardware
- CYD 2.8" ESP32 Display (USB-C Variante) - 320x240px
- CYD 3.5" ESP32 Display (USB-C Variante) - 480x320px
- ESP32 basiertes Board mit TFT Display und Touch-Screen
- Optionale Hardware: RGB LED und Photoresistor

## Setup

### Benötigte Libraries
1. **LovyanGFX** - Installiere über den Arduino Library Manager

### Display-Konfiguration
Die Beispiele verwenden `CYD_Display_Config.h` für die Display-Konfiguration. Diese Datei enthält:
- LGFX Display-Setup für CYD Boards
- Optionale Pin-Definitionen:
  - `LED_PIN_R`, `LED_PIN_G`, `LED_PIN_B` (RGB LED)
  - `photoR_PIN` (Photoresistor)

Die Pins werden automatisch initialisiert, wenn sie definiert sind. Um Features zu deaktivieren, kommentiere die entsprechenden `#define` Zeilen in `CYD_Display_Config.h` aus.

### Automatische Display-Anpassung
Alle Beispiele erkennen automatisch die Display-Auflösung und passen sich an:
- ✓ Vollständig skalierbar: **Alle 11 Beispiele**
- Funktionieren auf 2.8" (320x240) und 3.5" (480x320) Displays
- Alle Koordinaten und Größen werden dynamisch berechnet

### Installation
1. Öffne die Arduino IDE
2. Installiere die ESP32 Board-Unterstützung über den Board Manager
3. Installiere die LovyanGFX Library über den Library Manager
4. Öffne die gewünschte Beispieldatei (.ino)
5. Wähle das richtige Board: "ESP32 Dev Module"
6. Kompiliere und lade das Programm hoch

## Beispiele

### 01_HelloWorld
Einfaches "Hello World" Beispiel - zeigt Text auf dem Display an.

### 02_BasicShapes
Demonstriert das Zeichnen von Grundformen (Rechtecke, Kreise, Linien, Dreiecke).

### 03_TouchDemo
Zeigt wie man Touch-Input verarbeitet und auf dem Display visualisiert.

### 04_Colors
Demonstriert verschiedene Farben und Farbverläufe.

### 05_ScrollText
Zeigt verschiedene Text-Scroll-Effekte (horizontal, vertikal, Ticker-Style).

### 06_BouncingBall
Physik-Simulation mit springenden Bällen, Touch-Interaktion und FPS-Anzeige.

### 07_WiFiClock
Digitale Uhr mit WiFi und NTP (Network Time Protocol) - zeigt die aktuelle Zeit vom Internet.

**Wichtig:** Erstelle eine `Credentials.h` Datei mit deinen WiFi-Zugangsdaten:
```cpp
const char* ssid = "DeinWiFiName";
const char* password = "DeinWiFiPasswort";
```

Oder kopiere `Credentials.h.example` zu `Credentials.h` und trage deine Daten ein.

### 08_AnalogClock
Analoge Uhr mit WiFi und NTP - zeigt die aktuelle Zeit mit klassischen Zeigern (Stunden, Minuten, Sekunden).

Features:
- Analoges Zifferblatt mit Stundenmarkierungen
- Automatische Zeit-Synchronisation über WiFi/NTP
- Timezone: Zürich (CET/CEST mit automatischer Sommer-/Winterzeit)
- Optional: Automatische Helligkeitssteuerung über Photoresistor

**Wichtig:** Benötigt ebenfalls eine `Credentials.h` Datei (siehe 07_WiFiClock).

### 09_Calculator
Taschenrechner mit Touch-Eingabe im Portrait-Modus (Hochformat).

Features:
- Touch-Bedienung mit Button-Grid
- Grundrechenarten: Addition, Subtraktion, Multiplikation, Division
- Dezimalpunkt-Unterstützung
- Clear-Funktion (C)
- Display-Bereich für Anzeige von Zahlen und Ergebnissen
- Farbcodierte Buttons (Ziffern, Operatoren, Clear, Equals)
- Visuelles Feedback bei Button-Druck
- Division durch Null Fehlerbehandlung
- Optional: LED-Feedback bei Berechnungen und Fehlern

### 10_TouchButtons
Demonstriert verschiedene Touch-Button-Verhaltensweisen.

Features:
- **Button 1 (Momentary):** Aktiv solange gedrückt (wie Klingeltaster)
- **Button 2 (Toggle):** Ein/Aus beim Drücken (wie Lichtschalter)
- **Button 3-5 (Radio Group):** Nur einer kann aktiv sein (wie Radio-Buttons)
- Visuelle Rückmeldung: Grün = aktiv, Grau = inaktiv
- Status-Indikator auf jedem Button (gelb/rot)
- Doppelter Rahmen für aktive Buttons
- Serial Monitor Output für Debugging
- Optional: RGB LED Feedback (Rot = Momentary, Grün = Toggle, Blau = Radio)

### 11_ScrollingChart
Klassisches scrollendes Liniendiagramm für Live-Daten-Visualisierung.

Features:
- Neue Datenpunkte kommen links herein
- Chart scrollt kontinuierlich nach rechts
- Y-Achse mit Skala (0-100)
- Horizontale Grid-Linien für bessere Lesbarkeit
- Farbgradient: Blau (niedrig) → Cyan → Grün → Rot (hoch)
- Neuester Punkt (links) wird hervorgehoben
- Datenquelle:
  - Mit Photoresistor: Echtzeit-Lichtwerte
  - Ohne Photoresistor: Simulierte Sinus-Welle mit Rauschen
- Dynamische Chart-Breite: Passt sich an Display-Größe an
  - 2.8" Display: ~280 Datenpunkte
  - 3.5" Display: ~430 Datenpunkte
- Update-Rate anzeigbar
- Optional: RGB LED Feedback (Blau = niedrig, Grün = mittel, Rot = hoch)

## Verwendung

Jedes Beispiel befindet sich in einem eigenen Verzeichnis mit dem gleichnamigen .ino File:
```
examples/
  01_HelloWorld/
    01_HelloWorld.ino
  02_BasicShapes/
    02_BasicShapes.ino
  ...
```

Öffne die .ino Datei in der Arduino IDE und lade sie auf dein CYD hoch.

## Konfiguration

Alle Beispiele verwenden die gemeinsame Konfiguration:
```cpp
#include <CYD_Display_Config.h>
```

Die Display-Auflösung wird dynamisch ermittelt:
```cpp
int SCREEN_WIDTH = lcd.width();
int SCREEN_HEIGHT = lcd.height();
```

## Lizenz

MIT License
