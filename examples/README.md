# 📚 CYD Examples - Grundlegende Beispiele

Diese Sammlung enthält 11 grundlegende Beispiele zum Lernen der Display-Programmierung mit dem CYD (Cheap Yellow Display) und der LovyanGFX Library.

## 🎯 Übersicht

Die Beispiele sind progressiv aufgebaut - von einfachen "Hello World" bis zu komplexen Anwendungen wie WiFi-Uhren und Live-Daten-Visualisierung.

**Alle Beispiele sind vollständig skalierbar** und funktionieren auf beiden Display-Größen:
- ✓ CYD 2.8" (320x240px)
- ✓ CYD 3.5" (480x320px)

## 📖 Beispiel-Liste

### 01_HelloWorld
**Schwierigkeit:** ⭐ Anfänger
**Beschreibung:** Einfaches "Hello World" Beispiel - zeigt Text auf dem Display an.

**Was du lernst:**
- Display initialisieren
- Text auf dem Display ausgeben
- Textgröße und -farbe ändern
- Positionierung mit `setCursor()`

**Hardware:** Nur CYD Display

---

### 02_BasicShapes
**Schwierigkeit:** ⭐ Anfänger
**Beschreibung:** Demonstriert das Zeichnen von Grundformen.

**Was du lernst:**
- Rechtecke zeichnen (`drawRect`, `fillRect`)
- Kreise zeichnen (`drawCircle`, `fillCircle`)
- Linien zeichnen (`drawLine`)
- Dreiecke zeichnen (`drawTriangle`, `fillTriangle`)
- Farben verwenden (RGB565 Format)

**Hardware:** Nur CYD Display

---

### 03_TouchDemo
**Schwierigkeit:** ⭐⭐ Fortgeschritten
**Beschreibung:** Zeigt wie man Touch-Input verarbeitet und visualisiert.

**Was du lernst:**
- Touch-Koordinaten auslesen
- Touch-Events verarbeiten
- Visuelle Feedback bei Touch
- Touch-Trails zeichnen
- Debouncing implementieren

**Hardware:** CYD Display mit Touch-Screen

---

### 04_Colors
**Schwierigkeit:** ⭐ Anfänger
**Beschreibung:** Demonstriert verschiedene Farben und Farbverläufe.

**Was du lernst:**
- RGB565 Farbformat verstehen
- Vordefinierte Farben verwenden
- Farbverläufe erstellen
- Farb-Interpolation
- Farbräder zeichnen

**Hardware:** Nur CYD Display

---

### 05_ScrollText
**Schwierigkeit:** ⭐⭐ Fortgeschritten
**Beschreibung:** Zeigt verschiedene Text-Scroll-Effekte.

**Was du lernst:**
- Horizontales Scrollen
- Vertikales Scrollen
- Ticker-Style Scrolling
- Text-Animationen
- Timing mit `millis()`

**Hardware:** Nur CYD Display

---

### 06_BouncingBall
**Schwierigkeit:** ⭐⭐ Fortgeschritten
**Beschreibung:** Physik-Simulation mit springenden Bällen und Touch-Interaktion.

**Was du lernst:**
- Physik-Simulation (Geschwindigkeit, Beschleunigung)
- Kollisionserkennung (Wände, Touch)
- Mehrere Objekte verwalten
- FPS-Messung und Anzeige
- Interaktive Grafik

**Hardware:**
- CYD Display mit Touch-Screen
- Optional: RGB LED für visuelle Effekte

---

### 07_WiFiClock
**Schwierigkeit:** ⭐⭐⭐ Fortgeschritten
**Beschreibung:** Digitale Uhr mit WiFi und NTP (Network Time Protocol).

**Was du lernst:**
- WiFi-Verbindung herstellen
- NTP-Zeit abrufen
- Zeitzonen verwalten (CET/CEST)
- Zeit formatieren und anzeigen
- Datum anzeigen
- Auto-Update jede Stunde

**Hardware:**
- CYD Display
- WiFi-Zugangsdaten erforderlich

**Konfiguration:** Erstelle `Credentials.h`:
```cpp
const char* ssid = "DeinWiFiName";
const char* password = "DeinWiFiPasswort";
```

---

### 08_AnalogClock
**Schwierigkeit:** ⭐⭐⭐ Fortgeschritten
**Beschreibung:** Analoge Uhr mit WiFi und NTP - zeigt Zeit mit klassischen Zeigern.

**Was du lernst:**
- Kreis-Mathematik (sin/cos für Zeiger)
- Analoge Uhr-Darstellung
- Stundenmarkierungen zeichnen
- Zeiger-Animation
- WiFi/NTP Integration
- Zeitzone-Verwaltung

**Hardware:**
- CYD Display
- Optional: Photoresistor für automatische Helligkeitssteuerung
- WiFi-Zugangsdaten erforderlich

**Konfiguration:** Wie 07_WiFiClock

---

### 09_Calculator
**Schwierigkeit:** ⭐⭐⭐ Expert
**Beschreibung:** Voll funktionsfähiger Touch-Taschenrechner im Portrait-Modus.

**Was du lernst:**
- Portrait-Modus (Hochformat)
- Touch-Button-Grid erstellen
- Button-Zustände verwalten
- Mathematische Berechnungen
- Display-Logik (Zahlen, Operatoren)
- Fehlerbehandlung (Division durch 0)
- State Machine implementieren

**Hardware:**
- CYD Display mit Touch-Screen
- Optional: RGB LED für Feedback

**Features:**
- Grundrechenarten (+, -, ×, ÷)
- Dezimalpunkt-Unterstützung
- Clear-Funktion (C)
- Visuelles Button-Feedback
- Farbcodierte Buttons

---

### 10_TouchButtons
**Schwierigkeit:** ⭐⭐⭐ Expert
**Beschreibung:** Demonstriert verschiedene Touch-Button-Verhaltensweisen.

**Was du lernst:**
- Momentary Buttons (wie Klingeltaster)
- Toggle Buttons (wie Lichtschalter)
- Radio Button Groups (nur einer aktiv)
- Button-State-Management
- Visuelle Button-Zustände
- Komplexe Touch-Logik

**Hardware:**
- CYD Display mit Touch-Screen
- Optional: RGB LED für Feedback

**Features:**
- 5 verschiedene Buttons mit unterschiedlichem Verhalten
- Status-Indikatoren
- Doppelter Rahmen für aktive Buttons
- Serial Monitor Output

---

### 11_ScrollingChart
**Schwierigkeit:** ⭐⭐⭐ Expert
**Beschreibung:** Klassisches scrollendes Liniendiagramm für Live-Daten-Visualisierung.

**Was du lernst:**
- Scrolling-Technik (Daten von links nach rechts)
- Achsen und Skala zeichnen
- Grid-Linien für Lesbarkeit
- Farbgradienten basierend auf Werten
- Dynamische Daten-Erfassung
- Echtzeit-Visualisierung

**Hardware:**
- CYD Display
- Optional: Photoresistor für echte Live-Daten
- Optional: RGB LED für Feedback

**Features:**
- Neue Datenpunkte kommen links herein
- Chart scrollt kontinuierlich nach rechts
- Y-Achse mit Skala (0-100)
- Farbgradient: Blau → Cyan → Grün → Rot
- Mit Photoresistor: Echtzeit-Lichtwerte
- Ohne Photoresistor: Simulierte Sinus-Welle

## 🎓 Empfohlener Lernpfad

### Stufe 1: Grundlagen (Anfänger)
1. **01_HelloWorld** - Erste Schritte
2. **02_BasicShapes** - Zeichnen lernen
3. **04_Colors** - Farben verstehen

### Stufe 2: Interaktion (Fortgeschritten)
4. **03_TouchDemo** - Touch-Input
5. **05_ScrollText** - Animationen
6. **06_BouncingBall** - Physik & Gameplay

### Stufe 3: Komplexe Anwendungen (Expert)
7. **10_TouchButtons** - Button-Logik
8. **09_Calculator** - Vollständige App
9. **07_WiFiClock** oder **08_AnalogClock** - Netzwerk
10. **11_ScrollingChart** - Daten-Visualisierung

## 🔧 Hardware-Setup

### Basis (für alle Beispiele):
- CYD Display (2.8" oder 3.5")
- ESP32 Board
- USB-Kabel für Upload

### Erweitert (für spezielle Beispiele):
- **Touch-Screen:** Für 03, 06, 09, 10
- **WiFi-Zugang:** Für 07, 08
- **RGB LED (optional):** Visuelle Effekte
- **Photoresistor (optional):** Helligkeitssteuerung & Live-Daten

## 📊 Beispiel-Statistiken

| Beispiel | Zeilen Code | Schwierigkeit | Hardware | Lernzeit |
|----------|-------------|---------------|----------|----------|
| 01_HelloWorld | ~50 | ⭐ | Minimal | 10 min |
| 02_BasicShapes | ~100 | ⭐ | Minimal | 20 min |
| 03_TouchDemo | ~120 | ⭐⭐ | Touch | 30 min |
| 04_Colors | ~80 | ⭐ | Minimal | 20 min |
| 05_ScrollText | ~150 | ⭐⭐ | Minimal | 40 min |
| 06_BouncingBall | ~200 | ⭐⭐ | Touch | 60 min |
| 07_WiFiClock | ~180 | ⭐⭐⭐ | WiFi | 60 min |
| 08_AnalogClock | ~250 | ⭐⭐⭐ | WiFi | 90 min |
| 09_Calculator | ~350 | ⭐⭐⭐ | Touch | 120 min |
| 10_TouchButtons | ~280 | ⭐⭐⭐ | Touch | 90 min |
| 11_ScrollingChart | ~220 | ⭐⭐⭐ | Optional | 90 min |

**Gesamt:** ~1.980 Zeilen Code zum Lernen!

## 💡 Tipps für Anfänger

1. **Starte einfach:** Beginne mit 01_HelloWorld
2. **Experimentiere:** Ändere Farben, Größen, Positionen
3. **Serial Monitor:** Nutze `Serial.println()` zum Debuggen
4. **Dokumentation:** Lese die Kommentare im Code
5. **LovyanGFX Docs:** Schaue in die LovyanGFX Dokumentation für mehr Funktionen
6. **Schritt für Schritt:** Verstehe jedes Beispiel bevor du weiter gehst

## 🐛 Häufige Probleme & Lösungen

**Problem:** Display bleibt schwarz
- **Lösung:** Prüfe Upload, Board-Einstellungen, USB-Verbindung

**Problem:** Touch funktioniert nicht
- **Lösung:** CYD Display hat Touch - prüfe Code, `lcd.getTouch(&x, &y)`

**Problem:** WiFi verbindet nicht (07, 08)
- **Lösung:** Prüfe SSID/Passwort in `Credentials.h`, WiFi-Reichweite

**Problem:** RGB LED funktioniert nicht
- **Lösung:** LED ist optional - Code funktioniert ohne LED

**Problem:** Code kompiliert nicht
- **Lösung:** LovyanGFX Library installiert? Board "ESP32 Dev Module" gewählt?

## 📚 Weiterführende Ressourcen

- [Haupt-README](../README.md) - Übersicht über gesamtes Repository
- [Arcade Games](../arcade-games/README.md) - Vollständige Spielesammlung
- [LovyanGFX GitHub](https://github.com/lovyan03/LovyanGFX) - Library Dokumentation
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32) - ESP32 Dokumentation

## 🚀 Nächste Schritte

Nach den Beispielen kannst du:
1. **Arcade Games spielen:** Siehe [arcade-games/](../arcade-games/)
2. **Eigene Projekte bauen:** Kombiniere gelernte Techniken
3. **CYD_Games Library nutzen:** Eigene Spiele entwickeln
4. **Community beitreten:** Teile deine Projekte!

## 🎉 Viel Erfolg beim Lernen!

Die Beispiele sind der perfekte Einstieg in die CYD Display-Programmierung. Nimm dir Zeit, experimentiere und hab Spaß!

**Happy Coding! 💻**
