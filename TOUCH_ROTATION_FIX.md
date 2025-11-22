# Touch-Rotation bei Orientierungswechsel (LovyanGFX)

**Version:** 1.0
**Datum:** November 2024
**Für:** CYD-Academy - ESP32 Display-Projekte mit LovyanGFX

---

## 📋 Inhaltsverzeichnis

1. [Problem](#-problem)
2. [Warum nicht die Standard-Kalibrierung?](#-warum-nicht-die-standard-kalibrierung)
3. [Lösung: Erweiterte setRotation() Methode](#-lösung-erweiterte-setrotation-methode)
4. [Technische Details](#-technische-details)
5. [Vorteile dieser Lösung](#-vorteile-dieser-lösung)
6. [Anwendung im Code](#-anwendung-im-code)
7. [Anpassung für andere Boards](#-anpassung-für-andere-boards)
8. [Vollständiges Beispiel](#-vollständiges-beispiel)
9. [Alternative Lösungen](#-alternative-lösungen-nicht-empfohlen)
10. [Troubleshooting](#-troubleshooting)
11. [Getestet mit](#-getestet-mit)

---

## 🔍 Problem

Bei der Verwendung von LovyanGFX mit ILI9341 Display und XPT2046 Touch-Controller tritt ein bekanntes Problem auf: **Display und Touch benötigen unterschiedliche `offset_rotation` Werte für verschiedene Orientierungen**.

### Konkret in unserem Fall (CYD ESP32-2432S028R):

- **Panel:** `cfg.offset_rotation = 2` (konstant)
- **Touch Portrait (rotation 0/2):** `cfg.offset_rotation = 4`
- **Touch Landscape (rotation 1/3):** `cfg.offset_rotation = 6`

### Das Problem in der Praxis

Da die Touch-Konfiguration nur beim Initialisieren gesetzt wird, funktioniert der Touch nach einem Rotationswechsel mit `tft.setRotation()` **nicht mehr korrekt**.

**Beispiel:**
```cpp
LGFX tft;

void setup() {
  tft.init();
  tft.setRotation(0);  // Portrait - Touch funktioniert ✅
}

void loop() {
  if (buttonPressed) {
    tft.setRotation(1);  // Landscape - Touch ist FALSCH ❌
  }
}
```

Nach dem Wechsel zu Landscape sind die Touch-Koordinaten invertiert oder gespiegelt, weil der Touch-Controller immer noch mit `offset_rotation = 4` (Portrait) konfiguriert ist.

---

## ❓ Warum nicht die Standard-Kalibrierung?

LovyanGFX bietet eine `setCalibrate()` Methode zur Touch-Kalibrierung. Diese Lösung hat jedoch **entscheidende Nachteile**:

### Nachteile der manuellen Kalibrierung

| Aspekt | Manuelle Kalibrierung (`setCalibrate`) | Offset-Rotation-Fix |
|--------|----------------------------------------|---------------------|
| **Setup-Aufwand** | Manuelle Kalibrierung für jede Rotation | Einmalig in Board-Definition |
| **Hardware-Abhängigkeit** | Jedes Board braucht individuelle Werte | Gleiche Werte für alle Boards desselben Typs |
| **Wartungsaufwand** | Bei Hardware-Variationen neue Werte nötig | Wartungsfrei |
| **Code-Komplexität** | Kalibrierwerte speichern & bei jedem Wechsel laden | Transparent im Hintergrund |
| **Portabilität** | Code muss Board-spezifisch angepasst werden | Code funktioniert überall gleich |

### Vorteile der Offset-Rotation-Lösung

Die Lösung über `offset_rotation` ist dagegen:

✅ **Hardware-unabhängig** - Gleiche Werte für alle Boards desselben Typs
✅ **Wartungsfrei** - Keine manuellen Kalibrierungen notwendig
✅ **In der Board-Definition gekapselt** - Code bleibt portabel
✅ **Transparent** - Funktioniert automatisch im Hintergrund
✅ **Professionell** - Beste Lösung für wiederverwendbare Board-Definitionen

---

## 🛠️ Lösung: Erweiterte `setRotation()` Methode

Die elegante Lösung besteht darin, die `setRotation()` Methode in der LGFX-Klasse zu überschreiben und den Touch-Offset **automatisch anzupassen**.

### Implementierung in CYD_Display_Config.h

```cpp
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Light_PWM _light_instance;
  lgfx::Touch_XPT2046 _touch_instance;

public:
  LGFX(void)
  {
    // ===== Display Bus (SPI2) =====
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read = 16000000;
      cfg.spi_3wire = false;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = 12;
      cfg.pin_mosi = 11;
      cfg.pin_miso = 13;
      cfg.pin_dc = 9;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    // ===== Display Panel =====
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = 10;
      cfg.pin_rst = 46;
      cfg.offset_rotation = 2;  // Panel bleibt konstant
      // ... weitere Panel-Config
      _panel_instance.config(cfg);
    }

    // ===== Touch (SPI3) =====
    {
      auto cfg = _touch_instance.config();
      cfg.x_min = 200;
      cfg.x_max = 3900;
      cfg.y_min = 200;
      cfg.y_max = 3900;
      cfg.pin_int = 39;
      cfg.bus_shared = true;
      cfg.offset_rotation = 4;  // Standard für Portrait
      cfg.spi_host = SPI3_HOST;
      cfg.freq = 2500000;
      cfg.pin_sclk = 6;
      cfg.pin_mosi = 4;
      cfg.pin_miso = 5;
      cfg.pin_cs = 38;
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }

  // ========================================
  // Erweiterte setRotation() mit automatischer Touch-Anpassung
  // ========================================
  void setRotation(uint8_t rotation)
  {
    // 1. Erst Display-Rotation setzen (Basis-Methode aufrufen)
    lgfx::LGFX_Device::setRotation(rotation);

    // 2. Dann Touch-Offset direkt auf _touch_instance anpassen
    auto cfg = _touch_instance.config();  // Config-Kopie holen

    // Touch-Offset abhängig von Rotation
    if (rotation == 0 || rotation == 2) {
      cfg.offset_rotation = 4;   // Portrait (0° und 180°)
    } else {  // rotation 1 oder 3
      cfg.offset_rotation = 6;   // Landscape (90° und 270°)
    }

    _touch_instance.config(cfg);  // Geänderte Config zurücksetzen
  }
};
```

### Wie funktioniert es?

1. **Display-Rotation setzen:** Die Basis-Methode `lgfx::LGFX_Device::setRotation(rotation)` rotiert das Display
2. **Touch-Config holen:** Eine Kopie der Touch-Konfiguration wird mit `config()` geholt
3. **Offset anpassen:** Je nach Rotation wird `offset_rotation` auf 4 (Portrait) oder 6 (Landscape) gesetzt
4. **Config zurücksetzen:** Die geänderte Konfiguration wird mit `config(cfg)` zurückgeschrieben

---

## 🔬 Technische Details

### Warum direkter Zugriff auf `_touch_instance`?

LovyanGFX 1.2.7 bietet **keine** `setConfig()` Methode für Touch-Instanzen auf der öffentlichen API. Deshalb greifen wir direkt auf die Member-Variable `_touch_instance` zu:

```cpp
// ❌ Funktioniert NICHT (Methode existiert nicht):
auto touch = getTouch();
touch->setConfig(cfg);

// ✅ Funktioniert (direkter Zugriff):
_touch_instance.config(cfg);
```

Da `_touch_instance` ein **protected Member** der LGFX-Klasse ist, haben wir als abgeleitete Klasse direkten Zugriff darauf.

### Warum kein `override` Keyword?

Die `setRotation()` Methode in `lgfx::LGFX_Device` ist **nicht** als `virtual` deklariert. Deshalb:

- **Kein `override` möglich** (würde Compiler-Fehler erzeugen)
- **Name-Hiding:** Unsere Methode "versteckt" die Basis-Methode (C++ Name Hiding)
- **Basis-Methode aufrufen:** Via explizitem Namespace `lgfx::LGFX_Device::setRotation(rotation)`

```cpp
// Das ist Name-Hiding, NICHT Polymorphismus (kein virtual)
void setRotation(uint8_t rotation)  // Kein override!
{
  // Basis-Methode explizit aufrufen
  lgfx::LGFX_Device::setRotation(rotation);

  // Eigene Erweiterung
  auto cfg = _touch_instance.config();
  // ...
}
```

### Config-Pattern in LovyanGFX

LovyanGFX verwendet ein spezifisches Pattern für Konfigurationsänderungen:

**"Config-Kopie holen → ändern → zurücksetzen"**

```cpp
auto cfg = _touch_instance.config();  // Gibt Kopie zurück (per value)
cfg.offset_rotation = 6;              // Kopie ändern
_touch_instance.config(cfg);          // Geänderte Kopie zurücksetzen
```

#### Wichtig: `config()` gibt eine Kopie zurück!

Die `config()` Methode gibt eine **Kopie** zurück, keine Referenz! Deshalb funktioniert direktes Ändern **nicht**:

```cpp
// ❌ Funktioniert NICHT (Compiler-Fehler "using rvalue as lvalue"):
_touch_instance.config().offset_rotation = 6;

// ✅ Funktioniert (Kopie in Variable, ändern, zurücksetzen):
auto cfg = _touch_instance.config();  // Kopie holen
cfg.offset_rotation = 6;               // Ändern
_touch_instance.config(cfg);           // Zurücksetzen
```

#### Warum gibt `config()` eine Kopie zurück?

LovyanGFX nutzt das "Copy-Modify-Apply" Pattern aus mehreren Gründen:

1. **Thread-Safety:** Keine Race-Conditions beim gleichzeitigen Zugriff
2. **Atomare Updates:** Alle Änderungen werden gemeinsam angewendet
3. **Validierung:** Die `config(cfg)` Methode kann die Werte validieren, bevor sie übernommen werden
4. **Rollback-Möglichkeit:** Alte Config bleibt erhalten, falls etwas schiefgeht

---

## ✅ Vorteile dieser Lösung

### 1. **Board-spezifisch gekapselt**

Die Touch-Anpassung ist Teil der Board-Definition (in `CYD_Display_Config.h` bzw. MyLGFXConfigs). Jedes Projekt, das diese Board-Definition verwendet, profitiert automatisch vom Fix.

### 2. **Transparente Verwendung**

Der Hauptcode bleibt **unverändert** und muss nichts von der Touch-Rotation-Problematik wissen:

```cpp
// Hauptcode bleibt einfach und portabel:
tft.setRotation(1);  // Touch-Offset wird automatisch angepasst ✅
```

### 3. **Portabel**

Code funktioniert mit allen Board-Definitionen **ohne Änderungen**. Wechselst du von einem CYD zu einem anderen Board, ändert sich nur die `#include <CYD_Display_Config.h>` Zeile.

### 4. **Wartungsfrei**

Keine manuellen Kalibrierungen notwendig. Die Offset-Werte sind einmalig in der Board-Definition festgelegt.

### 5. **Hardware-unabhängig**

Gleiche Offset-Werte für alle Boards desselben Typs (z.B. alle ESP32-2432S028R mit ILI9341 + XPT2046).

### 6. **Professionell**

Beste Lösung für wiederverwendbare Board-Definitionen. Wird auch von LovyanGFX-Community empfohlen.

---

## 🚀 Anwendung im Code

### Einfaches Beispiel

```cpp
#include <CYD_Display_Config.h>  // Enthält LGFX mit Touch-Fix

LGFX tft;

void setup() {
  tft.init();
  tft.setRotation(0);  // Portrait - Touch-Offset automatisch auf 4 ✅

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Touch me!", tft.width()/2, tft.height()/2);
}

void loop() {
  int16_t x, y;
  if (tft.getTouch(&x, &y)) {
    // Touch funktioniert korrekt in Portrait ✅
    tft.fillCircle(x, y, 5, TFT_RED);
  }
}
```

### Mit Rotationswechsel

```cpp
#include <CYD_Display_Config.h>

LGFX tft;
bool isLandscape = false;

void setup() {
  tft.init();
  tft.setRotation(0);  // Portrait
}

void loop() {
  // Touch-Eingabe prüfen
  int16_t x, y;
  if (tft.getTouch(&x, &y)) {
    //Zeichne Touch-Punkt
    tft.fillCircle(x, y, 3, TFT_WHITE);

    // Wenn in oberer Ecke getoucht: Rotation wechseln
    if (x < 50 && y < 50) {
      isLandscape = !isLandscape;
      tft.setRotation(isLandscape ? 1 : 0);  // Touch-Offset wird automatisch angepasst ✅
      tft.fillScreen(TFT_BLACK);

      // Hinweis anzeigen
      tft.setTextDatum(MC_DATUM);
      tft.drawString(isLandscape ? "Landscape" : "Portrait",
                     tft.width()/2, tft.height()/2);
    }
  }
}
```

### In komplexeren Projekten

```cpp
// UI-Manager Klasse
class UIManager {
  LGFX* display;
  uint8_t currentRotation = 0;

public:
  UIManager(LGFX* tft) : display(tft) {}

  void rotateLeft() {
    currentRotation = (currentRotation + 3) % 4;  // -90°
    display->setRotation(currentRotation);  // Touch-Fix automatisch ✅
    redrawUI();
  }

  void rotateRight() {
    currentRotation = (currentRotation + 1) % 4;  // +90°
    display->setRotation(currentRotation);  // Touch-Fix automatisch ✅
    redrawUI();
  }

  void handleTouch() {
    int16_t x, y;
    if (display->getTouch(&x, &y)) {
      // Touch funktioniert in allen Rotationen korrekt ✅
      processTouch(x, y);
    }
  }
};
```

---

## 🔧 Anpassung für andere Boards

Falls dein Board andere `offset_rotation` Werte benötigt (z.B. andere Display-/Touch-Kombination), passe einfach die Werte in der `setRotation()` Methode an:

### Beispiel: Andere Offset-Werte

```cpp
void setRotation(uint8_t rotation)
{
  lgfx::LGFX_Device::setRotation(rotation);

  auto cfg = _touch_instance.config();

  // Anpassen für deine Hardware:
  if (rotation == 0 || rotation == 2) {
    cfg.offset_rotation = 2;   // Dein Wert für Portrait
  } else {
    cfg.offset_rotation = 0;   // Dein Wert für Landscape
  }

  _touch_instance.config(cfg);
}
```

### Wie finde ich die richtigen Werte?

1. **Experimentell:** Probiere Werte 0-7 aus für jede Rotation
2. **Systematisch:** Teste alle Kombinationen in einer Tabelle:

| Display Rotation | Offset 0 | Offset 1 | Offset 2 | Offset 3 | Offset 4 | Offset 5 | Offset 6 | Offset 7 |
|------------------|----------|----------|----------|----------|----------|----------|----------|----------|
| 0 (Portrait)     | ❌       | ❌       | ❌       | ❌       | ✅       | ❌       | ❌       | ❌       |
| 1 (Landscape 90°)| ❌       | ❌       | ❌       | ❌       | ❌       | ❌       | ✅       | ❌       |
| 2 (Portrait 180°)| ❌       | ❌       | ❌       | ❌       | ✅       | ❌       | ❌       | ❌       |
| 3 (Landscape 270°)| ❌      | ❌       | ❌       | ❌       | ❌       | ❌       | ✅       | ❌       |

3. **Test-Sketch:** Verwende diesen einfachen Sketch zum Testen:

```cpp
#include <CYD_Display_Config.h>

LGFX tft;

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(0);  // Ändere dies für verschiedene Rotationen

  // TESTE verschiedene offset_rotation Werte hier:
  auto cfg = tft.getTouch()->config();
  cfg.offset_rotation = 4;  // ÄNDERE DIESEN WERT (0-7)
  tft.getTouch()->config(cfg);

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Touch test", tft.width()/2, 50);
}

void loop() {
  int16_t x, y;
  if (tft.getTouch(&x, &y)) {
    Serial.printf("Touch: x=%d, y=%d\n", x, y);
    tft.fillCircle(x, y, 5, TFT_GREEN);

    // Check: Ist der grüne Punkt am richtigen Ort?
    // Wenn ja: offset_rotation ist korrekt! ✅
    // Wenn nein: probiere nächsten Wert
  }
}
```

---

## 📦 Vollständiges Beispiel

Hier ist eine vollständige Board-Definition mit dem Touch-Rotation-Fix für **ESP32-2432S028R** (CYD):

```cpp
// CYD_Display_Config.h oder MyLGFXConfigs Library
#ifndef CYD_DISPLAY_CONFIG_H
#define CYD_DISPLAY_CONFIG_H

#include <LovyanGFX.hpp>

// ========================================
// LGFX Klassen-Definition für ESP32-2432S028R
// ========================================
class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Light_PWM _light_instance;
  lgfx::Touch_XPT2046 _touch_instance;

public:
  LGFX(void)
  {
    // ===== Display Bus (SPI2) =====
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read = 16000000;
      cfg.spi_3wire = false;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = 12;
      cfg.pin_mosi = 11;
      cfg.pin_miso = 13;
      cfg.pin_dc = 9;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    // ===== Display Panel =====
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = 10;
      cfg.pin_rst = 46;
      cfg.pin_busy = -1;
      cfg.memory_width = 240;
      cfg.memory_height = 320;
      cfg.panel_width = 240;
      cfg.panel_height = 320;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 2;  // Panel-Offset bleibt konstant
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = true;
      cfg.invert = false;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;
      _panel_instance.config(cfg);
    }

    // ===== Backlight =====
    {
      auto cfg = _light_instance.config();
      cfg.pin_bl = 45;
      cfg.invert = false;
      cfg.freq = 44100;
      cfg.pwm_channel = 7;
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    // ===== Touch (SPI3) =====
    {
      auto cfg = _touch_instance.config();

      // Touch Kalibrierung (rohe ADC-Werte, nicht Pixel!)
      cfg.x_min = 200;
      cfg.x_max = 3900;
      cfg.y_min = 200;
      cfg.y_max = 3900;

      cfg.pin_int = 39;
      cfg.bus_shared = true;
      cfg.offset_rotation = 4;  // Standard für Portrait

      // Touch SPI (SPI3)
      cfg.spi_host = SPI3_HOST;
      cfg.freq = 2500000;
      cfg.pin_sclk = 6;
      cfg.pin_mosi = 4;
      cfg.pin_miso = 5;
      cfg.pin_cs = 38;

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }

  // ========================================
  // Erweiterte setRotation() mit automatischer Touch-Anpassung
  // ========================================
  void setRotation(uint8_t rotation)
  {
    // Erst Display-Rotation setzen
    lgfx::LGFX_Device::setRotation(rotation);

    // Dann Touch-Offset anpassen
    auto cfg = _touch_instance.config();
    if (rotation == 0 || rotation == 2) {
      cfg.offset_rotation = 4;   // Portrait
    } else {
      cfg.offset_rotation = 6;   // Landscape
    }
    _touch_instance.config(cfg);
  }
};

// ========================================
// GPIO Pin-Definitionen für CYD
// ========================================

// RGB LED
#define rgbLedR 4
#define rgbLedG 16
#define rgbLedB 17

// Photoresistor (LDR)
#define photoPin 34

// I2C für Sensoren & Erweiterungen
#define extSDA 22
#define extSCL 27

#endif  // CYD_DISPLAY_CONFIG_H
```

### Verwendung im Sketch:

```cpp
#include <CYD_Display_Config.h>

LGFX tft;

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(0);  // Portrait - Touch funktioniert ✅

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Touch me!", tft.width()/2, tft.height()/2);
}

void loop() {
  int16_t x, y;
  if (tft.getTouch(&x, &y)) {
    // Touch funktioniert korrekt in beiden Orientierungen ✅
    tft.fillCircle(x, y, 3, TFT_RED);
    Serial.printf("Touch: x=%d, y=%d\n", x, y);
  }
}
```

---

## 🔀 Alternative Lösungen (nicht empfohlen)

Es gibt weitere Ansätze zur Lösung des Touch-Rotation-Problems, die jedoch **Nachteile** haben:

### 1. Manuelle Koordinaten-Transformation

**Idee:** Touch-Koordinaten im Haupt-Code manuell umrechnen.

```cpp
bool getAdjustedTouch(int16_t &x, int16_t &y) {
  if (!tft.getTouch(&x, &y)) return false;

  // Manuelle Transformation abhängig von Rotation
  uint8_t rotation = tft.getRotation();
  if (rotation == 1) {  // Landscape 90°
    x = tft.width() - x;
    y = tft.height() - y;
  } else if (rotation == 3) {  // Landscape 270°
    // Andere Transformation
    int16_t temp = x;
    x = y;
    y = tft.height() - temp;
  }

  return true;
}

void loop() {
  int16_t x, y;
  if (getAdjustedTouch(x, y)) {
    tft.fillCircle(x, y, 3, TFT_WHITE);
  }
}
```

**Nachteile:**
- ❌ Muss in **jedem Projekt** implementiert werden
- ❌ Nicht in Board-Definition gekapselt
- ❌ Fehleranfällig (unterschiedliche Transformationen für verschiedene Rotationen)
- ❌ Nicht portabel (muss für jedes Board angepasst werden)
- ❌ Code-Duplikation über Projekte hinweg

### 2. `setCalibrate()` verwenden

**Idee:** Touch-Kalibrierung mit festen Werten für jede Rotation.

```cpp
void setupTouchCalibration(uint8_t rotation) {
  uint16_t parameters[8];

  if (rotation == 0) {  // Portrait
    parameters[0] = 239;   // x0
    parameters[1] = 3926;  // y0
    parameters[2] = 3926;  // x1
    parameters[3] = 239;   // y1
    parameters[4] = 230;   // x2
    parameters[5] = 230;   // y2
    parameters[6] = 3896;  // x3
    parameters[7] = 3896;  // y3
  } else if (rotation == 1) {  // Landscape 90°
    parameters[0] = 3926;  // Andere Werte...
    parameters[1] = 239;
    // ... weitere 6 Werte
  }
  // ... für rotation 2 und 3

  tft.setCalibrate(parameters);
}

void setup() {
  tft.init();
  tft.setRotation(0);
  setupTouchCalibration(0);
}

void rotateDisplay(uint8_t rotation) {
  tft.setRotation(rotation);
  setupTouchCalibration(rotation);  // Kalibrierung nachziehen
}
```

**Nachteile:**
- ❌ **Manuelle Kalibrierung pro Board nötig** (8 Werte pro Rotation × 4 Rotationen = 32 Werte!)
- ❌ **Wartungsintensiv** - Bei neuen Boards müssen alle Werte neu ermittelt werden
- ❌ **Nicht wiederverwendbar** - Jedes Display-Modul kann leicht unterschiedliche Werte haben
- ❌ **Komplex** - Schwer zu debuggen wenn Werte nicht passen

### 3. Touch-Controller neu initialisieren

**Idee:** Touch bei jedem Rotationswechsel komplett neu initialisieren.

```cpp
void setRotationWithTouch(uint8_t rotation) {
  // Display rotieren
  tft.setRotation(rotation);

  // Touch-Controller komplett neu initialisieren
  // (würde Zugriff auf private Touch-Instance erfordern)
  // => Nicht möglich ohne Änderung an LovyanGFX selbst!
}
```

**Nachteile:**
- ❌ **Nicht möglich** ohne LovyanGFX-Library zu patchen
- ❌ **Ineffizient** - Vollständige Reinitialisierung dauert Zeit
- ❌ **Glitches** - Touch könnte kurzzeitig nicht verfügbar sein

---

## 🛑 Troubleshooting

### Problem: Touch reagiert nach Rotation falsch

**Symptome:**
- Touch-Koordinaten sind nach `setRotation()` invertiert oder gespiegelt
- Touch funktioniert in Portrait, aber nicht in Landscape (oder umgekehrt)

**Diagnose:**
```cpp
void loop() {
  int16_t x, y;
  if (tft.getTouch(&x, &y)) {
    Serial.printf("Rotation: %d, Touch: x=%d, y=%d, Display: %dx%d\n",
                  tft.getRotation(), x, y, tft.width(), tft.height());

    // Ist x außerhalb von [0, width] oder y außerhalb von [0, height]?
    if (x < 0 || x > tft.width() || y < 0 || y > tft.height()) {
      Serial.println("❌ Touch außerhalb Display-Bereich!");
    }
  }
}
```

**Lösungen:**

1. **Überprüfe die `offset_rotation` Werte** - Möglicherweise sind die Werte 4 und 6 bei deinem Board vertauscht:
   ```cpp
   // Versuche vertauschte Werte:
   if (rotation == 0 || rotation == 2) {
     cfg.offset_rotation = 6;   // Statt 4
   } else {
     cfg.offset_rotation = 4;   // Statt 6
   }
   ```

2. **Teste alle Offset-Werte systematisch** - Nutze den Test-Sketch oben, um die richtigen Werte zu finden

3. **Prüfe Panel `offset_rotation`** - Manche Boards benötigen einen anderen Panel-Offset:
   ```cpp
   // Im Panel-Config:
   cfg.offset_rotation = 0;  // Statt 2 testen
   ```

### Problem: Compiler-Fehler: "using rvalue as lvalue"

**Symptome:**
```cpp
// Dieser Code produziert einen Compiler-Fehler:
_touch_instance.config().offset_rotation = 6;  // ❌ ERROR
```

**Fehler-Meldung:**
```
error: using rvalue as lvalue
```

**Ursache:**
`config()` gibt eine **temporäre Kopie** (rvalue) zurück, keine Referenz. Du kannst keine temporären Objekte modifizieren.

**Lösung:**
Config-Kopie in Variable speichern, ändern, zurücksetzen:
```cpp
// ✅ RICHTIG:
auto cfg = _touch_instance.config();  // Kopie in Variable
cfg.offset_rotation = 6;               // Variable ändern
_touch_instance.config(cfg);           // Zurücksetzen
```

### Problem: Touch funktioniert nur bei einer Rotation

**Symptome:**
- Touch funktioniert perfekt in Portrait
- Nach Wechsel zu Landscape funktioniert Touch überhaupt nicht mehr

**Diagnose:**
```cpp
void setup() {
  Serial.begin(115200);
  tft.init();

  // Teste beide Rotationen:
  tft.setRotation(0);
  Serial.printf("Portrait: Display=%dx%d\n", tft.width(), tft.height());
  delay(2000);

  tft.setRotation(1);
  Serial.printf("Landscape: Display=%dx%d\n", tft.width(), tft.height());
}

void loop() {
  int16_t x, y;
  if (tft.getTouch(&x, &y)) {
    Serial.printf("Touch: x=%d, y=%d (Rot=%d)\n", x, y, tft.getRotation());
  }
}
```

**Mögliche Ursachen:**

1. **Touch-Kalibrierungswerte sind Pixel statt ADC-Werte:**
   ```cpp
   // ❌ FALSCH (Pixel-Koordinaten):
   cfg.x_min = 0;
   cfg.x_max = 240;
   cfg.y_min = 0;
   cfg.y_max = 320;

   // ✅ RICHTIG (ADC-Rohwerte):
   cfg.x_min = 200;    // Typischer ADC-Wert bei Touch links
   cfg.x_max = 3900;   // Typischer ADC-Wert bei Touch rechts
   cfg.y_min = 200;
   cfg.y_max = 3900;
   ```

2. **`setRotation()` Fix nicht implementiert:**
   - Stelle sicher, dass die erweiterte `setRotation()` Methode in deiner LGFX-Klasse vorhanden ist
   - Prüfe, ob `#include <CYD_Display_Config.h>` die richtige Version lädt

3. **Touch-Config wird überschrieben:**
   - Stelle sicher, dass nirgendwo anders im Code die Touch-Config geändert wird
   - Suche nach anderen `_touch_instance.config()` Aufrufen

### Problem: Touch-Offset wird nicht angewendet

**Symptome:**
- Die `setRotation()` Methode wird aufgerufen
- Aber Touch-Koordinaten ändern sich nicht

**Diagnose:**
```cpp
void setRotation(uint8_t rotation)
{
  lgfx::LGFX_Device::setRotation(rotation);

  auto cfg = _touch_instance.config();

  // Debug-Ausgabe VOR Änderung:
  Serial.printf("Before: offset_rotation=%d\n", cfg.offset_rotation);

  if (rotation == 0 || rotation == 2) {
    cfg.offset_rotation = 4;
  } else {
    cfg.offset_rotation = 6;
  }

  // Debug-Ausgabe NACH Änderung:
  Serial.printf("After: offset_rotation=%d\n", cfg.offset_rotation);

  _touch_instance.config(cfg);

  // Verify:
  auto verify = _touch_instance.config();
  Serial.printf("Verified: offset_rotation=%d\n", verify.offset_rotation);
}
```

**Lösungen:**

1. **Config wird nicht zurückgeschrieben:**
   - Stelle sicher, dass `_touch_instance.config(cfg);` aufgerufen wird
   - Prüfe, ob es ein `return` vor diesem Aufruf gibt

2. **Falsche Touch-Instance:**
   - Manche Boards haben mehrere Touch-Controller
   - Prüfe, ob die richtige Instance verwendet wird

3. **LovyanGFX Version zu alt:**
   - Dieser Fix benötigt LovyanGFX 1.2.0 oder neuer
   - Update via Library Manager

### Problem: Sketch kompiliert nicht ("setRotation already defined")

**Symptome:**
```
error: 'void LGFX::setRotation(uint8_t)' cannot be overloaded with 'void lgfx::LGFX_Device::setRotation(uint8_t)'
```

**Ursache:**
Möglicherweise hast du `override` Keyword verwendet, aber die Basis-Methode ist nicht `virtual`.

**Lösung:**
Entferne `override`:
```cpp
// ❌ FALSCH (override für nicht-virtuelle Methode):
void setRotation(uint8_t rotation) override { ... }

// ✅ RICHTIG (Name-Hiding, kein override):
void setRotation(uint8_t rotation) { ... }
```

---

## ✅ Getestet mit

Diese Lösung wurde erfolgreich getestet mit:

| Komponente | Details |
|------------|---------|
| **LovyanGFX** | Version 1.2.7 |
| **MCU** | ESP32-S3, ESP32 |
| **Display** | ILI9341 (240×320) |
| **Touch** | XPT2046 |
| **Board** | ESP32-2432S028R (CYD - Cheap Yellow Display) |
| **IDE** | Arduino IDE 2.x, PlatformIO |
| **Framework** | Arduino, ESP-IDF |

### Kompatibilität

**Funktioniert mit:**
- ✅ ESP32 Classic
- ✅ ESP32-S2
- ✅ ESP32-S3
- ✅ ESP32-C3
- ✅ ILI9341, ILI9488 Displays
- ✅ XPT2046, FT6236 Touch-Controller
- ✅ Arduino Framework
- ✅ ESP-IDF Framework

**Nicht getestet, sollte aber funktionieren:**
- ⚠️ Andere ESP32-Varianten (C6, H2)
- ⚠️ Andere Display-Controller mit Touch-Rotation-Problemen

---

## 📚 Ressourcen & Links

### LovyanGFX Dokumentation
- **GitHub:** https://github.com/lovyan03/LovyanGFX
- **Wiki:** https://github.com/lovyan03/LovyanGFX/wiki
- **Discussions:** https://github.com/lovyan03/LovyanGFX/discussions

### CYD-Academy
- **Repository:** https://github.com/smily77/CYD-Academy
- **Board-Config:** Siehe MyLGFXConfigs Library
- **Installation:** `INSTALL_LIBRARY.md`
- **Repo-Guideline:** `Repo_Guideline.txt`

### Verwandte Themen
- **Display-Konfiguration:** Siehe `CYD_Display_Config.h` Dokumentation
- **I2C Sensoren:** Beispiele 17-21 in `examples/`
- **Touch-Kalibrierung:** LovyanGFX Wiki - Touch Calibration

---

## 🎯 Zusammenfassung

### Das Problem
- Display und Touch benötigen unterschiedliche `offset_rotation` Werte für verschiedene Orientierungen
- Standard-Lösung (manuelle Kalibrierung) ist wartungsintensiv und nicht portabel

### Die Lösung
- Überschreibe `setRotation()` in der LGFX-Klasse
- Passe `offset_rotation` des Touch-Controllers automatisch an
- Kapselung in Board-Definition (CYD_Display_Config.h)

### Vorteile
- ✅ Transparent und portabel
- ✅ Wartungsfrei
- ✅ Hardware-unabhängig
- ✅ Professionell

### Verwendung
```cpp
#include <CYD_Display_Config.h>  // Enthält Fix

LGFX tft;

void setup() {
  tft.init();
  tft.setRotation(0);  // Touch funktioniert automatisch ✅
}
```

---

## 🙏 Credits

**Entwickelt für die CYD-Academy** - ESP32 Display-Projekte mit LovyanGFX

**Community-Beiträge:**
- LovyanGFX Framework: lovyan03
- CYD-Academy: smily77
- Testing & Feedback: CYD Community

---

## 📄 Lizenz

Dieses Dokument und die Code-Beispiele stehen unter **MIT-Lizenz** und können frei verwendet werden.

```
MIT License

Copyright (c) 2024 CYD-Academy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

**Letzte Aktualisierung:** November 2024
**Autor:** CYD-Academy Team
**Version:** 1.0

---

**Viel Erfolg mit Touch-Rotation in LovyanGFX!** 🎮📱

Bei Fragen: [CYD-Academy GitHub Issues](https://github.com/smily77/CYD-Academy/issues)
