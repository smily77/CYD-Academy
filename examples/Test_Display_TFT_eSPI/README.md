# Test_Display_TFT_eSPI

Alternative Display-Test mit **TFT_eSPI** Library statt LovyanGFX.

## Zweck

Vergleich der Gradient-Darstellung zwischen TFT_eSPI und LovyanGFX, um festzustellen ob das Gradient-Banding Problem bei:
- **Hardware** (2.8" USB-C Panel) liegt, oder
- **LovyanGFX Library** spezifisch ist

## Dateien

### 1. Test_Display_TFT_eSPI_Simple.ino ✅ **EMPFOHLEN**

**Vereinfachte Version** mit allen Einstellungen direkt im Code:

```cpp
#define ILI9341_DRIVER
#define TFT_MISO 12
#define TFT_MOSI 13
// ... usw.
#include <TFT_eSPI.h>
```

**Vorteile:**
- Keine externe Config-Datei nötig
- Nichts in TFT_eSPI Library-Ordner ändern
- Einfach hochladen und testen

**Test-Ablauf:**
1. **Basis-Test**: Rot → Grün → Blau → Weiss Sequenz
2. **Test 1**: 7 Basis-Farben (Schwarz, Rot, Grün, Blau, Gelb, Cyan, Weiss)
3. **Test 2**: Schwarz/Weiss Kontrast
4. **Test 3**: **Farbverläufe** (wie in 10c_TouchButtons_Premium)
   - Rot → Orange
   - Grün → Cyan
   - Violett → Magenta

### 2. Test_Display_TFT_eSPI.ino

Erste Version mit separater Config-Datei `CYD_TFT_Setup.h`.

Funktioniert möglicherweise nicht → **Nutze stattdessen Simple-Version**

## Installation

1. **TFT_eSPI Library** installieren:
   - Arduino IDE: Bibliotheksmanager → "TFT_eSPI" suchen
   - Oder: https://github.com/Bodmer/TFT_eSPI

2. **Simple-Version** hochladen:
   ```
   examples/Test_Display_TFT_eSPI/Test_Display_TFT_eSPI_Simple.ino
   ```

3. **Seriellen Monitor** öffnen (115200 baud)

4. **Display beobachten**:
   - Siehst du die Farbsequenz (Rot, Grün, Blau, Weiss)?
   - Wie sehen die Gradients aus?
   - Vergleiche mit `Test_Display_Colors` (LovyanGFX)

## Zu beachten

### Backlight

Das Programm schaltet die Hintergrundbeleuchtung **automatisch** ein:

```cpp
pinMode(21, OUTPUT);
digitalWrite(21, HIGH);
```

### SPI-Geschwindigkeit

Auf 27 MHz reduziert für Stabilität:

```cpp
#define SPI_FREQUENCY  27000000
```

### Pin-Konfiguration (CYD 2.8" USB-C)

```cpp
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  -1    // Kein Reset-Pin
#define TFT_BL   21    // Backlight
```

## Ergebnis-Interpretation

### Wenn Gradients **glatt** sind:
→ Problem liegt bei **LovyanGFX** oder dessen Konfiguration

### Wenn Gradients **gestreift** sind:
→ Problem liegt bei **Hardware** (2.8" USB-C Panel)
→ RGB565 Limitation (nur 65.536 Farben statt 16.7 Mio)

### Wenn Bildschirm **schwarz** bleibt:
1. Überprüfe TFT_eSPI Library installiert
2. Öffne Seriellen Monitor → Siehst du Ausgabe?
3. Probiere anderen USB-Port
4. Überprüfe Board-Auswahl: ESP32 Dev Module

## Vergleich mit LovyanGFX

### Test_Display_Colors (LovyanGFX)
- Nutzt `LGFX` aus `CYD_Display_Config.h`
- Konfiguration über `Panel_ILI9341` Setup
- Zwei Gradient-Versionen:
  - `drawGradient()` mit Bayer-Dithering
  - `drawGradient2()` ohne Dithering

### Test_Display_TFT_eSPI (TFT_eSPI)
- Nutzt `TFT_eSPI` Library
- Konfiguration inline im Code
- Gradient ohne Dithering (zum Vergleich)

## Nächste Schritte

1. ✅ Simple-Version hochladen und testen
2. ⏳ Ergebnis dokumentieren (glatt oder gestreift?)
3. ⏳ Mit LovyanGFX-Version vergleichen
4. ⏳ Entscheidung treffen:
   - Bei Hardware-Problem: Limitation akzeptieren
   - Bei Software-Problem: LovyanGFX Config anpassen

## Credits

Basiert auf den Gradient-Tests aus `10c_TouchButtons_Premium`.

**Farben (Hex):**
- Momentary Button: `#FF4444` → `#FF8844`
- Toggle Button: `#44FF44` → `#44FFFF`
- Radio A Button: `#8844FF` → `#FF44FF`
