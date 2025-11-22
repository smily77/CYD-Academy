# Beispiel 00: Board Configuration Test

Ein umfassendes Test-Tool zur Validierung von Board-Konfigurationen (Display + Touch). Dieses Beispiel sollte **als erstes** ausgeführt werden, um sicherzustellen, dass deine Board-Definition korrekt funktioniert.

## 📸 Features

- **RGB Farb-Test** - Visuelle Kontrolle von Rot, Grün und Blau (Toggle durch Touch)
- **Touch-Demonstration** - Interaktive Zeichenfläche zum Testen der Touch-Genauigkeit
- **Rotation-Unterstützung** - Nahtloser Wechsel zwischen Portrait und Landscape (4 Rotationen)
- **Automatische Skalierung** - Funktioniert auf verschiedenen Display-Größen (240x320, 320x480, etc.)
- **Display-Info** - Zeigt aktuelle Größe und Rotation an
- **Touch-Rotation-Fix** - Nutzt automatische Touch-Anpassung aus `CYD_Display_Config.h`
- **Modernes UI-Design** - Schöne, übersichtliche Benutzeroberfläche

## 🎯 Zweck

Dieses Test-Tool validiert:

1. ✅ **Display-Initialisierung** - Funktioniert das Display?
2. ✅ **Farb-Ausgabe** - Sind RGB-Farben korrekt?
3. ✅ **Touch-Funktionalität** - Reagiert der Touch-Controller?
4. ✅ **Touch-Genauigkeit** - Sind die Touch-Koordinaten präzise?
5. ✅ **Rotation-Mechanismus** - Funktioniert `setRotation()` korrekt?
6. ✅ **Touch-Rotation-Fix** - Bleibt Touch nach Rotation funktional?
7. ✅ **Display-Größe** - Wird die richtige Auflösung erkannt?
8. ✅ **Skalierung** - Passt sich das Layout an verschiedene Größen an?

## 🔌 Hardware-Anforderungen

### Hauptkomponenten

- **ESP32-basiertes Board** mit Display (z.B. CYD ESP32-2432S028R)
- **Display-Controller**: ILI9341 (240x320), ILI9488 (320x480) oder kompatibel
- **Touch-Controller**: XPT2046, FT6236 oder kompatibel

### Unterstützte Display-Größen

Dieses Beispiel skaliert automatisch für verschiedene Auflösungen:

| Display-Typ | Auflösung | Portrait | Landscape | Getestet |
|-------------|-----------|----------|-----------|----------|
| **ILI9341** | 240×320 | 240×320 | 320×240 | ✅ |
| **ILI9488** | 320×480 | 320×480 | 480×320 | ✅ |
| **ST7789** | 240×240 | 240×240 | 240×240 | ⚠️ |
| **ILI9486** | 320×480 | 320×480 | 480×320 | ⚠️ |

✅ = Vollständig getestet | ⚠️ = Sollte funktionieren, aber nicht getestet

## 🖥️ UI-Layout

```
┌─────────────────────────────────┐
│     BOARD CONFIG TEST           │ ← Header mit Titel
├───────────┬───────────┬─────────┤
│    RED    │   GREEN   │  BLUE   │ ← RGB Test-Bereiche
│  (Touch)  │  (Touch)  │ (Touch) │   (Touch zum Toggle)
├─────────────────────────────────┤
│                                 │
│      Touch to Draw              │ ← Zeichen-/Test-Fläche
│   (Test Touch Accuracy)         │   (Freies Zeichnen)
│                                 │
│                                 │
├─────────────────────────────────┤
│ Display: 240x320  Rotation: 0  │ ← Info-Bereich
│ [ROTATE]  [●]  [CLEAR]          │ ← Buttons
└─────────────────────────────────┘
```

### Bereiche im Detail

#### 1. **Header (oben)**
- Zeigt "BOARD CONFIG TEST" als Titel
- Text-Größe passt sich der Display-Breite an

#### 2. **RGB Test-Bereich**
- Drei gleichgroße Blöcke: ROT, GRÜN, BLAU
- **Touch auf Block**: Farbe ein/aus schalten (visueller Test)
- **Farbwechsel**: Wählt Farbe für Zeichen-Bereich
- **Zweck**: Validierung der RGB-Ausgabe

#### 3. **Zeichen-Bereich (Mitte)**
- Große Fläche zum freien Zeichnen
- **Touch & Drag**: Zeichnet mit aktueller Farbe
- **Smooth Drawing**: Verbindet Punkte zu Linien
- **Zweck**: Touch-Genauigkeit und -Responsiveness testen

#### 4. **Footer (unten)**
- **Display-Info**: Zeigt `Breite × Höhe` und aktuelle Rotation (0-3)
- **ROTATE Button**: Wechselt zur nächsten Rotation (0→1→2→3→0)
- **Farb-Indikator**: Zeigt aktuell gewählte Zeichen-Farbe
- **CLEAR Button**: Löscht die Zeichenfläche

## 💻 Code-Struktur

### Hauptkomponenten

#### 1. Layout-System

Das Beispiel verwendet ein **dynamisches Layout-System**, das sich automatisch an verschiedene Display-Größen anpasst:

```cpp
void calculateLayout() {
  int h = tft.height();

  // Proportionale Berechnung
  headerHeight = h / 8;        // 12.5% für Header
  rgbHeight = h / 5;           // 20% für RGB-Bereiche
  drawAreaHeight = h / 2;      // 50% für Zeichenfläche
  footerHeight = h / 8;        // 12.5% für Footer

  // Y-Positionen berechnen
  drawAreaY = headerHeight + rgbHeight;
  footerY = h - footerHeight;
}
```

**Warum proportional?**
- Funktioniert auf 240×320 ebenso wie auf 320×480
- Bereiche bleiben im gleichen Verhältnis zueinander
- Keine hardcoded Pixel-Werte

#### 2. UI-Rendering

Jeder UI-Bereich hat eine eigene Zeichenfunktion:

```cpp
void drawUI() {
  tft.fillScreen(COLOR_BG);
  drawHeader();      // Titel
  drawRGBArea();     // RGB Blöcke
  drawDrawArea();    // Zeichenfläche
  drawFooter();      // Info & Buttons
}
```

**Modular & wiederverwendbar:**
- Einzelne Bereiche können separat neu gezeichnet werden
- Bei Rotation: Komplettes UI wird neu berechnet & gezeichnet

#### 3. Touch-Handling

Touch-Events werden nach **Y-Position** kategorisiert:

```cpp
void handleTouch(int16_t x, int16_t y) {
  // Header-Bereich
  if (y < headerHeight) {
    // Keine Aktion
  }
  // RGB-Bereich
  else if (y >= headerHeight && y < headerHeight + rgbHeight) {
    // Toggle RGB & wähle Farbe
    int blockWidth = tft.width() / 3;
    if (x < blockWidth) drawColor = COLOR_RED;
    // ...
  }
  // Zeichenfläche
  else if (y >= drawAreaY && y < footerY) {
    // Zeichne Punkt/Linie
    tft.fillCircle(x, y, 3, drawColor);
  }
  // Footer (Buttons)
  else if (y >= footerY) {
    // ROTATE oder CLEAR
  }
}
```

**Hierarchisches System:**
1. Y-Position bestimmt Bereich
2. X-Position bestimmt Aktion innerhalb des Bereichs

#### 4. Smooth Drawing

Für natürliches Zeichengefühl werden Touch-Punkte verbunden:

```cpp
// Aktueller Punkt
tft.fillCircle(x, y, 3, drawColor);

// Linie zum letzten Punkt (falls vorhanden)
if (lastTouchX >= 0 && lastTouchY >= 0) {
  tft.drawLine(lastTouchX, lastTouchY, x, y, drawColor);
}

// Punkte speichern für nächste Iteration
lastTouchX = x;
lastTouchY = y;
```

**Ergebnis:** Durchgehende Linien statt einzelner Punkte

#### 5. Rotation mit Reflow

Bei Rotation wird das komplette Layout neu berechnet:

```cpp
void rotateDisplay() {
  // Rotation ändern
  uint8_t newRotation = (tft.getRotation() + 1) % 4;
  tft.setRotation(newRotation);

  // Layout neu berechnen (neue Breite/Höhe!)
  calculateLayout();

  // UI komplett neu zeichnen
  drawUI();

  // Touch-Tracking zurücksetzen
  lastTouchX = -1;
  lastTouchY = -1;
}
```

**Wichtig:** Touch-Offset wird automatisch durch `setRotation()` angepasst (siehe `TOUCH_ROTATION_FIX.md`)

## 🚀 Installation & Verwendung

### 1. Voraussetzungen

Stelle sicher, dass folgende Libraries installiert sind:

- **LovyanGFX** (Version 1.2.0 oder neuer)
  - Arduino IDE: `Tools` → `Manage Libraries` → "LovyanGFX" suchen & installieren
  - PlatformIO: `lib_deps = lovyan03/LovyanGFX@^1.2.0`

- **CYD_Display_Config.h** (Board-Definition)
  - Entweder aus MyLGFXConfigs Library
  - Oder im Sketch-Verzeichnis (siehe unten)

### 2. Board-Konfiguration

Das Beispiel erwartet eine Board-Definition in `CYD_Display_Config.h`.

**Option A: MyLGFXConfigs Library verwenden**

```bash
# Clone Library ins Arduino Libraries Verzeichnis
cd ~/Arduino/libraries/
git clone https://github.com/[username]/MyLGFXConfigs.git
```

**Option B: Lokale Board-Definition**

Erstelle `CYD_Display_Config.h` im gleichen Ordner wie das `.ino` File:

```cpp
// CYD_Display_Config.h
#ifndef CYD_DISPLAY_CONFIG_H
#define CYD_DISPLAY_CONFIG_H

#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device
{
  // ... deine Board-Konfiguration
  // Siehe TOUCH_ROTATION_FIX.md für vollständiges Beispiel
};

#endif
```

### 3. Upload & Test

1. **Upload:**
   - Arduino IDE: `Sketch` → `Upload`
   - PlatformIO: `platformio run --target upload`

2. **Serielle Ausgabe prüfen:**
   ```
   ╔════════════════════════════════════════╗
   ║  CYD Board Configuration Test          ║
   ║  Testing Display + Touch + Rotation    ║
   ╚════════════════════════════════════════╝

   Display initialized: 240x320 pixels
   Rotation: 0 (0=Portrait, 1=Landscape)

   ✓ Display OK
   ✓ Touch-Rotation-Fix active

   Ready! Touch the screen to test.
   ```

3. **Visueller Test:**
   - Display sollte die UI anzeigen
   - RGB-Bereiche sollten klar ROT, GRÜN, BLAU zeigen
   - Kein Flackern oder Artefakte

### 4. Test-Durchführung

**Test 1: RGB Farben**
- ✅ Touch auf RED Block → Block sollte an/aus gehen
- ✅ Touch auf GREEN Block → Block sollte an/aus gehen
- ✅ Touch auf BLUE Block → Block sollte an/aus gehen
- ✅ Farben sollten klar erkennbar und nicht verfälscht sein

**Test 2: Touch-Genauigkeit**
- ✅ Touch & Drag in Zeichenfläche → Sollte smooth zeichnen
- ✅ Gezeichnete Linien sollten genau dort sein, wo du touchst
- ✅ Keine Sprünge oder Versatz

**Test 3: Rotation**
- ✅ Touch auf ROTATE Button → Display dreht sich
- ✅ UI passt sich an neue Orientierung an
- ✅ Touch funktioniert weiterhin korrekt (WICHTIG!)
- ✅ Teste alle 4 Rotationen (0, 1, 2, 3)

**Test 4: Display-Größe**
- ✅ Prüfe Info-Bereich: Zeigt korrekte Pixel-Größe?
- ✅ UI-Elemente sind proportional und nicht abgeschnitten

**Wenn alle Tests ✅ sind:** Deine Board-Konfiguration ist korrekt! 🎉

## 🔧 Troubleshooting

### Problem: Display bleibt schwarz/weiß

**Symptome:**
- Display zeigt nur schwarzen oder weißen Bildschirm
- Keine UI sichtbar

**Mögliche Ursachen & Lösungen:**

1. **Falsche Pin-Konfiguration**
   ```cpp
   // In CYD_Display_Config.h prüfen:
   cfg.pin_cs = 10;   // Chip Select
   cfg.pin_dc = 9;    // Data/Command
   cfg.pin_rst = 46;  // Reset
   // Pins müssen zu deinem Board passen!
   ```

2. **Falscher Display-Controller**
   ```cpp
   // Prüfe ob der richtige Panel verwendet wird:
   lgfx::Panel_ILI9341 _panel_instance;  // Für 240x320
   // oder
   lgfx::Panel_ILI9488 _panel_instance;  // Für 320x480
   ```

3. **SPI-Bus nicht korrekt**
   ```cpp
   // SPI-Host prüfen:
   cfg.spi_host = SPI2_HOST;  // Oder SPI3_HOST
   ```

4. **Backlight nicht aktiviert**
   ```cpp
   // Backlight-Pin prüfen:
   cfg.pin_bl = 45;      // Backlight Pin
   cfg.invert = false;   // Oder true, je nach Board
   ```

**Diagnose:**
```cpp
void setup() {
  Serial.begin(115200);
  tft.init();

  Serial.println("Display initialized");
  Serial.printf("Display size: %dx%d\n", tft.width(), tft.height());

  // Test: Sollte roten Bildschirm zeigen
  tft.fillScreen(TFT_RED);
  delay(2000);
}
```

### Problem: Touch reagiert nicht

**Symptome:**
- Display zeigt UI korrekt
- Touch-Eingaben werden nicht erkannt
- Keine Reaktion beim Berühren

**Lösungen:**

1. **Touch-Pins prüfen**
   ```cpp
   // In CYD_Display_Config.h:
   auto cfg = _touch_instance.config();
   cfg.pin_cs = 38;      // Touch CS Pin
   cfg.pin_int = 39;     // Touch Interrupt Pin (optional)
   cfg.spi_host = SPI3_HOST;  // Oft separater SPI-Bus!
   ```

2. **Touch-Controller-Typ prüfen**
   ```cpp
   // Richtiger Touch-Controller:
   lgfx::Touch_XPT2046 _touch_instance;  // Für resistive Touch
   // oder
   lgfx::Touch_FT6x36 _touch_instance;   // Für kapazitive Touch
   ```

3. **Touch-Kalibrierung prüfen**
   ```cpp
   // Rohe ADC-Werte (nicht Pixel!):
   cfg.x_min = 200;
   cfg.x_max = 3900;
   cfg.y_min = 200;
   cfg.y_max = 3900;
   ```

**Diagnose:**
```cpp
void loop() {
  int16_t x, y;
  if (tft.getTouch(&x, &y)) {
    Serial.printf("Touch: x=%d, y=%d\n", x, y);
  }
  delay(100);
}
// Keine Ausgabe = Touch-Hardware Problem
// Ausgabe mit seltsamen Werten = Kalibrierung falsch
```

### Problem: Touch funktioniert nur in Portrait ODER Landscape

**Symptome:**
- Touch funktioniert perfekt in einer Orientierung
- Nach ROTATE Button-Press: Touch ist falsch oder invertiert

**Ursache:**
Touch-Rotation-Fix fehlt oder ist falsch konfiguriert.

**Lösung:**

Stelle sicher, dass `CYD_Display_Config.h` die erweiterte `setRotation()` Methode enthält:

```cpp
class LGFX : public lgfx::LGFX_Device
{
  // ... member variables

public:
  LGFX(void) {
    // ... normal setup
  }

  // WICHTIG: Erweiterte setRotation() Methode
  void setRotation(uint8_t rotation)
  {
    lgfx::LGFX_Device::setRotation(rotation);

    auto cfg = _touch_instance.config();
    if (rotation == 0 || rotation == 2) {
      cfg.offset_rotation = 4;   // Portrait
    } else {
      cfg.offset_rotation = 6;   // Landscape
    }
    _touch_instance.config(cfg);
  }
};
```

**Details:** Siehe `TOUCH_ROTATION_FIX.md` im Repository.

### Problem: RGB-Farben sind falsch (z.B. Rot ist Blau)

**Symptome:**
- RED Block zeigt Blau
- BLUE Block zeigt Rot
- Farben sind vertauscht

**Ursache:**
RGB-Order in Panel-Config ist falsch.

**Lösung:**
```cpp
// In Panel-Config:
auto cfg = _panel_instance.config();
cfg.rgb_order = false;  // Versuche true
// oder
cfg.rgb_order = true;   // Versuche false

_panel_instance.config(cfg);
```

### Problem: UI ist abgeschnitten oder falsch skaliert

**Symptome:**
- Buttons sind außerhalb des sichtbaren Bereichs
- Text ist abgeschnitten
- Layout sieht komisch aus

**Ursache:**
Display-Größe wird falsch erkannt.

**Lösung:**
```cpp
// Prüfe Panel-Konfiguration:
cfg.memory_width = 240;   // Physikalische Breite
cfg.memory_height = 320;  // Physikalische Höhe
cfg.panel_width = 240;    // Sichtbare Breite
cfg.panel_height = 320;   // Sichtbare Höhe
cfg.offset_x = 0;         // X-Offset
cfg.offset_y = 0;         // Y-Offset
```

**Diagnose:**
```cpp
void setup() {
  // ...
  Serial.printf("Detected size: %dx%d\n", tft.width(), tft.height());
  // Sollte deine erwartete Größe zeigen (z.B. 240x320)
}
```

### Problem: Display ist gespiegelt oder invertiert

**Symptome:**
- Display zeigt alles spiegelverkehrt
- Oder auf dem Kopf

**Lösung:**
```cpp
// Ändere offset_rotation im Panel-Config:
cfg.offset_rotation = 2;  // Versuche 0, 1, 2, oder 3
// Bis das Display richtig herum ist
```

### Problem: Zeichnen ist nicht smooth (Sprünge)

**Symptome:**
- Beim Zeichnen entstehen Sprünge
- Linien sind nicht durchgehend
- Touch scheint zu "springen"

**Lösungen:**

1. **Touch-Polling-Rate erhöhen**
   ```cpp
   void loop() {
     // ... touch handling
     delay(10);  // Versuche 5 oder 1
   }
   ```

2. **Touch-Frequenz erhöhen**
   ```cpp
   // In Touch-Config:
   cfg.freq = 2500000;  // Versuche höhere Frequenz (z.B. 5000000)
   ```

3. **Bus-Sharing prüfen**
   ```cpp
   cfg.bus_shared = true;  // Sollte true sein wenn Display & Touch gleichen SPI-Bus nutzen
   ```

## 🎯 Verwendung für verschiedene Boards

### Anpassung für eigene Hardware

Um dieses Test-Tool für dein eigenes Board zu verwenden:

**Schritt 1: Erstelle Board-Definition**

Siehe `TOUCH_ROTATION_FIX.md` für eine vollständige Anleitung.

**Schritt 2: Pin-Mapping**

Erstelle eine Pin-Tabelle für dein Board:

| Signal | GPIO | Notiz |
|--------|------|-------|
| TFT_CS | 10 | Display Chip Select |
| TFT_DC | 9 | Display Data/Command |
| TFT_RST | 46 | Display Reset |
| TFT_MOSI | 11 | SPI MOSI |
| TFT_MISO | 13 | SPI MISO |
| TFT_SCK | 12 | SPI Clock |
| TFT_BL | 45 | Backlight |
| TOUCH_CS | 38 | Touch Chip Select |
| TOUCH_IRQ | 39 | Touch Interrupt |

**Schritt 3: Teste systematisch**

1. Display-Init (fillScreen Test)
2. RGB-Farben
3. Touch-Raw-Werte
4. Touch-Kalibrierung
5. Touch-Rotation

### Beispiel: Anpassung für ILI9488 (320x480)

```cpp
// CYD_Display_Config.h für größeres Display:

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ILI9488 _panel_instance;  // ← Anderer Panel-Typ
  // ... rest bleibt ähnlich

public:
  LGFX(void)
  {
    // Panel-Config:
    auto cfg = _panel_instance.config();
    cfg.memory_width = 320;   // ← Größere Breite
    cfg.memory_height = 480;  // ← Größere Höhe
    cfg.panel_width = 320;
    cfg.panel_height = 480;
    // ... rest der Config
  }

  // setRotation() Fix (gleich wie bei ILI9341)
  void setRotation(uint8_t rotation) {
    lgfx::LGFX_Device::setRotation(rotation);
    auto cfg = _touch_instance.config();
    if (rotation == 0 || rotation == 2) {
      cfg.offset_rotation = 4;
    } else {
      cfg.offset_rotation = 6;
    }
    _touch_instance.config(cfg);
  }
};
```

**Dann:** Sketch ohne Änderungen hochladen - Layout skaliert automatisch!

## 🎓 Lernziele

Nach Abschluss dieses Beispiels solltest du:

- ✅ Verstehen, wie LovyanGFX initialisiert wird
- ✅ Wissen, wie Touch-Controller in LovyanGFX eingebunden werden
- ✅ Das Konzept von Display-Rotationen verstehen (0-3)
- ✅ Den Touch-Rotation-Fix kennen und anwenden können
- ✅ Proportionale Layouts für verschiedene Display-Größen erstellen können
- ✅ Touch-Events hierarchisch nach Position verarbeiten können
- ✅ Smooth Drawing mit Touch-Tracking implementieren können
- ✅ Board-Definitionen erstellen und debuggen können

## 💡 Erweiterungsideen

### 1. **Multi-Touch-Test**

**Konzept:** Teste Multi-Touch-Fähigkeiten (falls Hardware unterstützt).

```cpp
void loop() {
  uint8_t touchCount = tft.getTouchCount();

  for (uint8_t i = 0; i < touchCount; i++) {
    int16_t x, y;
    if (tft.getTouchPoint(i, x, y)) {
      // Zeichne verschiedene Farben für verschiedene Finger
      uint16_t colors[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW};
      tft.fillCircle(x, y, 5, colors[i]);
    }
  }
}
```

### 2. **Touch-Druck-Test**

**Konzept:** Zeige Touch-Drucksensitivität (bei resistiven Touchscreens).

```cpp
void loop() {
  int16_t x, y;
  if (tft.getTouch(&x, &y)) {
    // Bei XPT2046: Druck aus Z-Wert
    uint16_t pressure = tft.getTouchRaw(&x, &y);

    // Größerer Kreis = mehr Druck
    int radius = map(pressure, 0, 4095, 2, 10);
    tft.fillCircle(x, y, radius, drawColor);
  }
}
```

### 3. **Performance-Messung**

**Konzept:** Messe und zeige FPS (Frames Per Second).

```cpp
unsigned long frameCount = 0;
unsigned long lastFPSUpdate = 0;
int fps = 0;

void loop() {
  frameCount++;

  if (millis() - lastFPSUpdate >= 1000) {
    fps = frameCount;
    frameCount = 0;
    lastFPSUpdate = millis();

    // FPS in Footer anzeigen
    tft.setTextColor(COLOR_TEXT);
    tft.drawString("FPS: " + String(fps), tft.width() - 50, footerY + 5);
  }

  // ... rest der loop
}
```

### 4. **Touch-Heatmap**

**Konzept:** Visualisiere Touch-Aktivität als Heatmap.

```cpp
uint8_t heatmap[240][320] = {0};  // Touch-Zähler für jedes Pixel

void loop() {
  int16_t x, y;
  if (tft.getTouch(&x, &y)) {
    // Inkrementiere Heatmap
    if (x >= 0 && x < 240 && y >= 0 && y < 320) {
      heatmap[x][y]++;

      // Zeichne mit Intensität
      uint16_t intensity = map(heatmap[x][y], 0, 255, 0, 31);
      uint16_t color = tft.color565(intensity * 8, 0, 0);
      tft.drawPixel(x, y, color);
    }
  }
}
```

### 5. **Benchmark-Modus**

**Konzept:** Automatischer Test von Display-Performance.

```cpp
void runBenchmark() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);

  // Test 1: Fill Speed
  unsigned long start = millis();
  for (int i = 0; i < 10; i++) {
    tft.fillScreen(TFT_RED);
    tft.fillScreen(TFT_BLACK);
  }
  unsigned long fillTime = millis() - start;

  tft.drawString("Fill: " + String(fillTime) + "ms", 10, 20);

  // Test 2: Line Drawing Speed
  start = millis();
  for (int i = 0; i < 100; i++) {
    tft.drawLine(0, 0, tft.width(), tft.height(), TFT_WHITE);
  }
  unsigned long lineTime = millis() - start;

  tft.drawString("Lines: " + String(lineTime) + "ms", 10, 50);

  // Test 3: Circle Drawing
  start = millis();
  for (int i = 0; i < 100; i++) {
    tft.drawCircle(tft.width()/2, tft.height()/2, 50, TFT_GREEN);
  }
  unsigned long circleTime = millis() - start;

  tft.drawString("Circles: " + String(circleTime) + "ms", 10, 80);
}
```

### 6. **Gesten-Erkennung**

**Konzept:** Erkenne Touch-Gesten (Swipe, Pinch, etc.).

```cpp
int16_t touchStartX = -1, touchStartY = -1;

void detectGestures() {
  int16_t x, y;

  if (tft.getTouch(&x, &y)) {
    if (touchStartX < 0) {
      touchStartX = x;
      touchStartY = y;
    }
  } else {
    if (touchStartX >= 0) {
      // Swipe erkennen
      int dx = x - touchStartX;
      int dy = y - touchStartY;

      if (abs(dx) > 50 && abs(dx) > abs(dy)) {
        if (dx > 0) {
          Serial.println("Swipe RIGHT");
        } else {
          Serial.println("Swipe LEFT");
        }
      }

      touchStartX = -1;
      touchStartY = -1;
    }
  }
}
```

### 7. **Touch-Koordinaten-Debugger**

**Konzept:** Zeige rohe und kalibrierte Touch-Werte an.

```cpp
void loop() {
  int16_t x, y;

  // Rohe Werte
  uint16_t rawX, rawY;
  tft.getTouchRaw(&rawX, &rawY);

  // Kalibrierte Werte
  if (tft.getTouch(&x, &y)) {
    // Display im Footer
    tft.fillRect(0, footerY, tft.width(), 20, TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE);

    char buf[60];
    sprintf(buf, "Raw: %d,%d  Cal: %d,%d", rawX, rawY, x, y);
    tft.drawString(buf, 10, footerY + 5);
  }
}
```

## 📚 Weiterführende Themen

### Display-Rotation im Detail

LovyanGFX unterstützt 4 Rotationen:

| Rotation | Wert | Beschreibung | Typische Nutzung |
|----------|------|--------------|------------------|
| **0** | 0 | Portrait (0°) | Standard, Hochformat |
| **1** | 1 | Landscape (90° CW) | Querformat rechts |
| **2** | 2 | Portrait (180°) | Umgekehrtes Hochformat |
| **3** | 3 | Landscape (270° CW / 90° CCW) | Querformat links |

**CW** = Clockwise (im Uhrzeigersinn) | **CCW** = Counter-Clockwise (gegen Uhrzeigersinn)

**Koordinatensystem:**
- `(0, 0)` ist **immer** oben links (nach Rotation)
- `width()` und `height()` **tauschen** sich bei 90°/270°

```cpp
// Beispiel für 240×320 Display:

tft.setRotation(0);  // Portrait
// width() = 240, height() = 320

tft.setRotation(1);  // Landscape
// width() = 320, height() = 240  ← getauscht!
```

### Touch-Kalibrierung verstehen

Touch-Controller liefern **rohe ADC-Werte** (0-4095), nicht Pixel-Koordinaten!

**Kalibrierungs-Parameter:**

```cpp
cfg.x_min = 200;   // ADC-Wert bei Touch links
cfg.x_max = 3900;  // ADC-Wert bei Touch rechts
cfg.y_min = 200;   // ADC-Wert bei Touch oben
cfg.y_max = 3900;  // ADC-Wert bei Touch unten
```

LovyanGFX macht automatisch Mapping:

```
Raw ADC (200-3900) → Pixel-Koordinaten (0-239 oder 0-319)
```

**Warum nicht 0-4095?**
- Touch-Sensoren haben toten Rand
- Extremwerte (0, 4095) sind oft ungenau
- 200-3900 ist der "sichere" Bereich

### SPI-Bus-Sharing

Viele Boards nutzen **einen SPI-Bus** für Display UND Touch:

```cpp
// Display (SPI2):
cfg.spi_host = SPI2_HOST;
cfg.pin_mosi = 11;
cfg.pin_miso = 13;
cfg.pin_sclk = 12;

// Touch (auch SPI2!):
cfg.spi_host = SPI2_HOST;  // ← Gleicher Bus
cfg.pin_mosi = 11;         // ← Gleiche Pins
cfg.pin_miso = 13;
cfg.pin_sclk = 12;
```

**Wichtig:** `bus_shared = true` setzen!

```cpp
cfg.bus_shared = true;  // Display & Touch teilen SPI-Bus
```

LovyanGFX verwendet dann **Chip-Select (CS)** Pins zur Unterscheidung:

```cpp
cfg.pin_cs = 10;   // Display CS
cfg.pin_cs = 38;   // Touch CS (in Touch-Config)
```

## 🛠️ Praktische Tipps

### Tipp 1: Serielle Ausgabe nutzen

Die serielle Ausgabe zeigt hilfreiche Debug-Infos:

```
╔════════════════════════════════════════╗
║  CYD Board Configuration Test          ║
╚════════════════════════════════════════╝

Display initialized: 240x320 pixels
Rotation: 0

Layout calculated for 240x320:
  Header: 40px
  RGB: 64px
  Draw Area: 160px
  Footer: 40px

Touch detected: x=120, y=180
RED block toggled
ROTATE button pressed
Rotation changed to: 1
New dimensions: 320x240
```

### Tipp 2: Farbkonstanten definieren

Für eigene Farben:

```cpp
// RGB565 Format (5-bit Rot, 6-bit Grün, 5-bit Blau)
#define MY_COLOR 0xF81F  // Magenta

// Oder mit Helper-Funktion:
uint16_t myColor = tft.color565(255, 128, 0);  // Orange (R, G, B)
```

### Tipp 3: Text-Alignment nutzen

LovyanGFX hat praktische Text-Alignment Modi:

```cpp
tft.setTextDatum(TL_DATUM);  // Top-Left
tft.setTextDatum(TC_DATUM);  // Top-Center
tft.setTextDatum(TR_DATUM);  // Top-Right
tft.setTextDatum(ML_DATUM);  // Middle-Left
tft.setTextDatum(MC_DATUM);  // Middle-Center (Standard für dieses Beispiel)
tft.setTextDatum(MR_DATUM);  // Middle-Right
tft.setTextDatum(BL_DATUM);  // Bottom-Left
tft.setTextDatum(BC_DATUM);  // Bottom-Center
tft.setTextDatum(BR_DATUM);  // Bottom-Right
```

### Tipp 4: Anti-Aliased Fonts

Für schönere Text-Ausgabe:

```cpp
#include "lgfx/v1/misc/fonts/FreeSans9pt7b.h"  // Font-Header

void setup() {
  tft.setFont(&fonts::FreeSans9pt7b);  // Anti-aliased Font
  tft.setTextSize(1);
  // Jetzt ist Text viel schärfer!
}
```

### Tipp 5: Double-Buffering für flüssige Animationen

```cpp
LGFX_Sprite sprite(&tft);

void setup() {
  sprite.createSprite(tft.width(), tft.height());
}

void loop() {
  // In Sprite zeichnen (Off-Screen)
  sprite.fillScreen(TFT_BLACK);
  sprite.drawString("Hello", 100, 100);

  // Sprite auf Display kopieren (schnell, kein Flackern)
  sprite.pushSprite(0, 0);
}
```

## 📖 Ressourcen & Links

### LovyanGFX Dokumentation
- **GitHub:** https://github.com/lovyan03/LovyanGFX
- **Wiki:** https://github.com/lovyan03/LovyanGFX/wiki
- **Discussions:** https://github.com/lovyan03/LovyanGFX/discussions
- **Examples:** https://github.com/lovyan03/LovyanGFX/tree/master/examples

### CYD-Academy
- **Repository:** https://github.com/smily77/CYD-Academy
- **Touch-Rotation-Fix:** `TOUCH_ROTATION_FIX.md`
- **Repo-Guideline:** `Repo_Guideline.txt`
- **Installation:** `INSTALL_LIBRARY.md`

### Verwandte Beispiele
- **Beispiel 01 - HelloWorld:** Einfachster Display-Test
- **Beispiel 02 - BasicShapes:** Grafik-Funktionen kennenlernen
- **Beispiel 03 - TouchDemo:** Erweiterte Touch-Funktionen
- **Beispiel 04 - Colors:** Farbpaletten und Gradienten

### Hardware-Dokumentation
- **ILI9341 Datasheet:** https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf
- **XPT2046 Datasheet:** https://www.buydisplay.com/download/ic/XPT2046.pdf
- **ESP32 Pinout:** https://randomnerdtutorials.com/esp32-pinout-reference-gpios/

## 🔜 Nächste Schritte

Wenn deine Board-Konfiguration erfolgreich getestet wurde:

1. **Beispiel 01 - HelloWorld:** Lerne grundlegende Display-Funktionen
2. **Beispiel 02 - BasicShapes:** Erkunde Grafik-Primitiven
3. **Beispiel 03 - TouchDemo:** Vertiefte Touch-Programmierung
4. **Beispiel 10 - TouchButtons:** Erstelle interaktive UIs
5. **Eigene Projekte:** Nutze `CYD_Display_Config.h` in deinen Projekten!

### Community & Support

- **GitHub Issues:** https://github.com/smily77/CYD-Academy/issues
- **Diskussionen:** https://github.com/smily77/CYD-Academy/discussions
- **LovyanGFX Community:** https://github.com/lovyan03/LovyanGFX/discussions

---

**Viel Erfolg beim Testen deiner Board-Konfiguration! 🎨📱**

Bei Fragen oder Problemen: [CYD-Academy GitHub Issues](https://github.com/smily77/CYD-Academy/issues)

---

**Version:** 1.0
**Autor:** CYD-Academy Team
**Lizenz:** MIT
