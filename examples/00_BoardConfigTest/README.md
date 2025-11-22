# Beispiel 00: Board Configuration Test

Ein einfaches Test-Tool zur Validierung von Board-Konfigurationen (Display + Touch). Dieses Beispiel sollte **als erstes** ausgeführt werden, um sicherzustellen, dass deine Board-Definition korrekt funktioniert.

## 📸 Features

- **RGB Farb-Streifen** - Visuelle Kontrolle von Rot, Grün und Blau
- **Touch-Test** - Zeichne frei auf dem gesamten Bildschirm
- **Display-Info** - Zeigt Größe und aktuelle Rotation
- **Automatische Skalierung** - Funktioniert auf verschiedenen Display-Größen
- **Touch-Rotation-Fix** - Nutzt automatische Touch-Anpassung aus `CYD_Display_Config.h`
- **Einfach & Übersichtlich** - Minimalistisches Design, ~150 Zeilen Code

## 🎯 Zweck

Dieses Test-Tool validiert:

1. ✅ **Display-Initialisierung** - Funktioniert das Display?
2. ✅ **Farb-Ausgabe** - Sind RGB-Farben korrekt?
3. ✅ **Touch-Funktionalität** - Reagiert der Touch-Controller?
4. ✅ **Touch-Genauigkeit** - Sind die Touch-Koordinaten präzise?
5. ✅ **Touch-Rotation** - Funktioniert Touch in verschiedenen Rotationen?
6. ✅ **Display-Größe** - Wird die richtige Auflösung erkannt?

## 🖥️ UI-Layout

```
┌───┬───┬───┬──────────────────┐
│ R │ G │ B │ 240x320  Rot: 0  │ ← Info-Zeile (30px)
├──────────────────────────────┤
│                              │
│                              │
│   Touch anywhere to draw     │
│                              │ ← Zeichenfläche
│                              │   (Rest des Bildschirms)
│                              │
│                              │
└──────────────────────────────┘
```

### Layout-Beschreibung

**Info-Zeile (oben, 30px hoch):**
- **Links**: RGB-Quadrate (ROT, GRÜN, BLAU) zur Farb-Kontrolle
- **Rechts**: Display-Info (`Breite×Höhe Rot:X`)

**Zeichenfläche:**
- Kompletter Rest des Bildschirms
- Touch überall außerhalb der Info-Zeile → Zeichnet gelbe Punkte
- Smooth Drawing (verbindet Punkte zu Linien)

## 🔌 Hardware-Anforderungen

- **ESP32-basiertes Board** mit Display (z.B. CYD ESP32-2432S028R)
- **Display-Controller**: ILI9341, ILI9488 oder kompatibel
- **Touch-Controller**: XPT2046, FT6236 oder kompatibel

### Unterstützte Display-Größen

| Display-Typ | Auflösung | Getestet |
|-------------|-----------|----------|
| **ILI9341** | 240×320 | ✅ |
| **ILI9488** | 320×480 | ✅ |
| **ST7789** | 240×240 | ⚠️ |
| **ILI9486** | 320×480 | ⚠️ |

## 🚀 Installation & Verwendung

### 1. Voraussetzungen

- **LovyanGFX** (Version 1.2.0 oder neuer)
- **CYD_Display_Config.h** (Board-Definition)

### 2. Rotation einstellen

Öffne `00_BoardConfigTest.ino` und ändere die Rotation in `setup()`:

```cpp
void setup() {
  // ...
  tft.init();

  // ========================================
  // ROTATION HIER EINSTELLEN (0, 1, 2, oder 3)
  // ========================================
  tft.setRotation(0);  // ← Ändere diese Zeile

  // ...
}
```

**Rotation-Werte:**
- `0` = Portrait (0°)
- `1` = Landscape (90°)
- `2` = Portrait umgedreht (180°)
- `3` = Landscape umgedreht (270°)

### 3. Upload & Test

1. **Upload** auf dein Board
2. **Prüfe Serielle Ausgabe:**
   ```
   ╔════════════════════════════════════════╗
   ║  CYD Board Configuration Test          ║
   ║  Simple Display + Touch Tester         ║
   ╚════════════════════════════════════════╝

   Display initialized: 240x320 pixels
   Rotation: 0 (0=Portrait, 1=Landscape, ...)

   ✓ Display OK
   ✓ Touch-Rotation-Fix active

   Ready! Touch the screen to draw.
   ```

3. **Visueller Test:**
   - RGB-Quadrate sollten klar ROT, GRÜN, BLAU zeigen
   - Info rechts zeigt korrekte Display-Größe
   - "Touch anywhere to draw" Text sichtbar

4. **Touch-Test:**
   - Zeichne mit dem Finger/Stylus auf dem Bildschirm
   - Gelbe Linien sollten smooth und genau sein
   - Keine Sprünge oder Versatz

### 4. Verschiedene Rotationen testen

Um alle 4 Rotationen zu testen:

**Test 1: Portrait (Rotation 0)**
```cpp
tft.setRotation(0);  // Portrait
```
- Upload → Teste Touch → Prüfe Genauigkeit

**Test 2: Landscape (Rotation 1)**
```cpp
tft.setRotation(1);  // Landscape 90°
```
- Upload → Teste Touch → Sollte weiterhin genau sein ✅

**Test 3: Portrait 180° (Rotation 2)**
```cpp
tft.setRotation(2);  // Portrait umgedreht
```
- Upload → Teste Touch

**Test 4: Landscape 270° (Rotation 3)**
```cpp
tft.setRotation(3);  // Landscape umgedreht
```
- Upload → Teste Touch

**Wenn Touch in ALLEN Rotationen funktioniert:** Touch-Rotation-Fix ist aktiv! ✅

## 💻 Code-Struktur

### Überblick

Das Beispiel ist bewusst **sehr einfach** gehalten (~150 Zeilen):

```cpp
// 1. Setup
void setup() {
  tft.init();
  tft.setRotation(0);  // ← Rotation einstellen
  drawUI();
}

// 2. UI zeichnen
void drawUI() {
  // Info-Zeile mit RGB + Display-Info
  // Info-Text "Touch anywhere"
}

// 3. Touch-Loop
void loop() {
  if (tft.getTouch(&x, &y)) {
    // Zeichne Punkt + Linie
  }
}
```

### Hauptkomponenten

#### 1. RGB-Quadrate

```cpp
tft.fillRect(5, 3, rgbWidth, headerHeight-6, TFT_RED);     // ROT
tft.fillRect(5 + rgbWidth + 3, 3, rgbWidth, headerHeight-6, TFT_GREEN);   // GRÜN
tft.fillRect(5 + (rgbWidth + 3) * 2, 3, rgbWidth, headerHeight-6, TFT_BLUE);    // BLAU
```

**Zweck:** Visueller Test ob RGB-Farben korrekt ausgegeben werden

#### 2. Display-Info

```cpp
char info[40];
sprintf(info, "%dx%d  Rot:%d", w, h, tft.getRotation());
tft.drawString(info, w - 5, headerHeight/2);
```

**Zeigt:**
- Breite × Höhe (z.B. `240x320`)
- Aktuelle Rotation (z.B. `Rot:0`)

#### 3. Smooth Drawing

```cpp
// Punkt zeichnen
tft.fillCircle(x, y, 2, COLOR_DRAW);

// Linie zum letzten Punkt (verbindet Punkte zu Linien)
if (lastTouchX >= 0 && lastTouchY >= 0) {
  tft.drawLine(lastTouchX, lastTouchY, x, y, COLOR_DRAW);
}

lastTouchX = x;
lastTouchY = y;
```

**Ergebnis:** Durchgehende Linien statt einzelner Punkte

## 🔧 Troubleshooting

### Problem: Display bleibt schwarz

**Lösung:**

1. **Prüfe Pin-Konfiguration** in `CYD_Display_Config.h`
2. **Prüfe Display-Controller** (ILI9341 vs ILI9488)
3. **Prüfe Backlight-Pin**

**Diagnose-Sketch:**
```cpp
void setup() {
  Serial.begin(115200);
  tft.init();
  tft.fillScreen(TFT_RED);  // Sollte roten Bildschirm zeigen
  delay(2000);
  tft.fillScreen(TFT_GREEN);
  delay(2000);
  tft.fillScreen(TFT_BLUE);
}
```

Wenn Farben nicht sichtbar: Hardware-Problem oder falsche Pins.

### Problem: RGB-Farben sind falsch (z.B. Rot ist Blau)

**Ursache:** RGB-Order in Panel-Config ist falsch.

**Lösung:**
```cpp
// In CYD_Display_Config.h, Panel-Config:
cfg.rgb_order = false;  // Versuche true
// oder
cfg.rgb_order = true;   // Versuche false
```

### Problem: Touch reagiert nicht

**Symptome:**
- Display zeigt UI korrekt
- Touch-Eingaben werden nicht erkannt
- Keine gelben Linien beim Berühren

**Lösungen:**

1. **Touch-Pins prüfen** in `CYD_Display_Config.h`:
   ```cpp
   cfg.pin_cs = 38;      // Touch CS
   cfg.pin_int = 39;     // Touch Interrupt
   cfg.spi_host = SPI3_HOST;  // Oft separater SPI-Bus
   ```

2. **Touch-Controller-Typ prüfen:**
   ```cpp
   lgfx::Touch_XPT2046 _touch_instance;  // Für resistive
   // oder
   lgfx::Touch_FT6x36 _touch_instance;   // Für kapazitive
   ```

3. **Diagnose:**
   ```cpp
   void loop() {
     int16_t x, y;
     if (tft.getTouch(&x, &y)) {
       Serial.printf("Touch: x=%d, y=%d\n", x, y);
     }
   }
   ```
   Keine Ausgabe = Touch-Hardware-Problem

### Problem: Touch funktioniert nur in einer Rotation

**Symptome:**
- Touch ist OK in Portrait (Rotation 0)
- Nach Wechsel zu Landscape (Rotation 1): Touch falsch oder invertiert

**Ursache:** Touch-Rotation-Fix fehlt in `CYD_Display_Config.h`

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

**Siehe:** `TOUCH_ROTATION_FIX.md` im Repository für Details.

### Problem: Touch ist ungenau oder hat Versatz

**Symptome:**
- Touch-Punkt erscheint 10-50 Pixel versetzt
- Zeichnen ist möglich, aber nicht an der richtigen Stelle

**Lösungen:**

1. **Touch-Kalibrierung anpassen:**
   ```cpp
   // In CYD_Display_Config.h, Touch-Config:
   cfg.x_min = 200;   // Anpassen
   cfg.x_max = 3900;  // Anpassen
   cfg.y_min = 200;
   cfg.y_max = 3900;
   ```

2. **Offset-Rotation prüfen:**
   ```cpp
   // Versuche andere Werte (0-7):
   cfg.offset_rotation = 4;  // Versuche 0, 2, 4, 6
   ```

3. **Test-Sketch für Kalibrierung:**
   ```cpp
   void loop() {
     uint16_t rawX, rawY;
     if (tft.getTouchRaw(&rawX, &rawY)) {
       Serial.printf("Raw: x=%d, y=%d\n", rawX, rawY);

       int16_t x, y;
       tft.getTouch(&x, &y);
       Serial.printf("Calibrated: x=%d, y=%d\n\n", x, y);
     }
   }
   ```

### Problem: Display-Größe ist falsch

**Symptome:**
- Info zeigt falsche Größe (z.B. `0x0` oder `128x160`)
- UI ist abgeschnitten

**Lösung:**
```cpp
// In CYD_Display_Config.h, Panel-Config:
cfg.memory_width = 240;
cfg.memory_height = 320;
cfg.panel_width = 240;
cfg.panel_height = 320;
```

## 💡 Erweiterungsideen

### 1. **Touch-Drucksensitivität anzeigen**

```cpp
void loop() {
  int16_t x, y;
  if (tft.getTouch(&x, &y)) {
    // Bei resistiven Touchscreens: Druck auslesen
    uint16_t pressure = tft.getTouchRaw(&x, &y);

    // Größerer Kreis = mehr Druck
    int radius = map(pressure, 0, 4095, 1, 8);
    tft.fillCircle(x, y, radius, COLOR_DRAW);
  }
}
```

### 2. **Mehrere Farben per RGB-Touch**

```cpp
uint16_t drawColor = TFT_YELLOW;

void loop() {
  int16_t x, y;
  if (tft.getTouch(&x, &y)) {
    // Touch auf RGB-Quadrate = Farbe wechseln
    if (y <= headerHeight) {
      if (x < rgbWidth * 2) drawColor = TFT_RED;
      else if (x < rgbWidth * 4) drawColor = TFT_GREEN;
      else if (x < rgbWidth * 6) drawColor = TFT_BLUE;
    } else {
      // Zeichne mit gewählter Farbe
      tft.fillCircle(x, y, 2, drawColor);
    }
  }
}
```

### 3. **Clear-Funktion per Touch**

```cpp
unsigned long lastTouchTime = 0;

void loop() {
  int16_t x, y;
  if (tft.getTouch(&x, &y)) {
    // Doppel-Touch in Header = Clear
    if (y <= headerHeight) {
      if (millis() - lastTouchTime < 500) {
        drawUI();  // Bildschirm löschen
      }
      lastTouchTime = millis();
    }
  }
}
```

### 4. **Touch-Heatmap**

```cpp
uint8_t heatmap[240][320] = {0};

void loop() {
  int16_t x, y;
  if (tft.getTouch(&x, &y) && y > headerHeight) {
    heatmap[x][y]++;

    // Farbe basierend auf Häufigkeit
    uint16_t intensity = min(heatmap[x][y] * 10, 255);
    uint16_t color = tft.color565(intensity, 0, 255 - intensity);
    tft.drawPixel(x, y, color);
  }
}
```

### 5. **FPS Counter**

```cpp
unsigned long frameCount = 0;
unsigned long lastFPSUpdate = 0;

void loop() {
  frameCount++;

  if (millis() - lastFPSUpdate >= 1000) {
    int fps = frameCount;
    frameCount = 0;
    lastFPSUpdate = millis();

    // FPS in Header anzeigen
    char fpsStr[20];
    sprintf(fpsStr, "FPS:%d", fps);
    tft.drawString(fpsStr, tft.width()/2, 5);
  }

  // ... rest der loop
}
```

## 📚 Weiterführende Themen

### Touch-Rotation-Fix verstehen

Der Touch-Rotation-Fix ist in `CYD_Display_Config.h` integriert und passt automatisch den Touch-Offset an, wenn `setRotation()` aufgerufen wird.

**Warum notwendig?**
- Display und Touch benötigen unterschiedliche `offset_rotation` Werte
- Portrait: `offset_rotation = 4`
- Landscape: `offset_rotation = 6`

**Wie funktioniert es?**
```cpp
void setRotation(uint8_t rotation) {
  lgfx::LGFX_Device::setRotation(rotation);  // Display rotieren

  auto cfg = _touch_instance.config();
  if (rotation == 0 || rotation == 2) {
    cfg.offset_rotation = 4;   // Portrait
  } else {
    cfg.offset_rotation = 6;   // Landscape
  }
  _touch_instance.config(cfg);  // Touch anpassen
}
```

**Mehr Details:** Siehe `TOUCH_ROTATION_FIX.md` im Repository-Root.

### Display-Skalierung

Dieses Beispiel funktioniert auf verschiedenen Display-Größen ohne Code-Änderungen:

```cpp
int w = tft.width();   // Breite abholen (z.B. 240 oder 320)
int h = tft.height();  // Höhe abholen (z.B. 320 oder 480)

// Info rechts positionieren (funktioniert bei allen Größen)
tft.drawString(info, w - 5, headerHeight/2);

// Text zentrieren (funktioniert bei allen Größen)
tft.drawString("Touch anywhere", w/2, h/2 - 20);
```

**Ergebnis:** Code funktioniert auf 240×320 ebenso wie auf 320×480!

## 🛠️ Praktische Tipps

### Tipp 1: Serielle Ausgabe nutzen

Die serielle Ausgabe ist sehr hilfreich beim Debuggen:

```cpp
Serial.printf("Touch: x=%d, y=%d\n", x, y);
```

Öffne den Serial Monitor (115200 baud) und sehe Touch-Koordinaten in Echtzeit.

### Tipp 2: Farben anpassen

Möchtest du andere Zeichen-Farben?

```cpp
#define COLOR_DRAW TFT_YELLOW   // Original
#define COLOR_DRAW TFT_WHITE    // Weiß
#define COLOR_DRAW 0x07FF       // Cyan
#define COLOR_DRAW 0xF81F       // Magenta
```

Oder eigene RGB565-Farbe erstellen:

```cpp
uint16_t myColor = tft.color565(255, 128, 0);  // Orange (R, G, B)
```

### Tipp 3: Größere Touch-Punkte

Für bessere Sichtbarkeit:

```cpp
tft.fillCircle(x, y, 2, COLOR_DRAW);  // Original (Radius 2)
tft.fillCircle(x, y, 5, COLOR_DRAW);  // Größer (Radius 5)
```

### Tipp 4: Verschiedene Hintergründe testen

```cpp
#define COLOR_BG 0x0000  // Schwarz (Original)
#define COLOR_BG 0xFFFF  // Weiß
#define COLOR_BG 0x001F  // Dunkelblau
```

## 📖 Ressourcen & Links

### CYD-Academy
- **Repository:** https://github.com/smily77/CYD-Academy
- **Touch-Rotation-Fix:** `TOUCH_ROTATION_FIX.md`
- **Repo-Guideline:** `Repo_Guideline.txt`

### LovyanGFX Dokumentation
- **GitHub:** https://github.com/lovyan03/LovyanGFX
- **Wiki:** https://github.com/lovyan03/LovyanGFX/wiki

### Verwandte Beispiele
- **Beispiel 01 - HelloWorld:** Einfachster Display-Test
- **Beispiel 02 - BasicShapes:** Grafik-Funktionen
- **Beispiel 03 - TouchDemo:** Erweiterte Touch-Features

## 🎓 Lernziele

Nach Abschluss dieses Beispiels solltest du:

- ✅ Wissen, wie man eine Board-Konfiguration testet
- ✅ RGB-Farben visuell validieren können
- ✅ Touch-Funktionalität und Genauigkeit prüfen können
- ✅ Verschiedene Display-Rotationen testen können
- ✅ Verstehen, warum der Touch-Rotation-Fix wichtig ist
- ✅ Einfache Touch-basierte Zeichenanwendungen erstellen können

## 🔜 Nächste Schritte

Wenn deine Board-Konfiguration erfolgreich getestet wurde:

1. **Beispiel 01 - HelloWorld:** Grundlegende Display-Funktionen
2. **Beispiel 02 - BasicShapes:** Grafik-Primitiven
3. **Beispiel 03 - TouchDemo:** Erweiterte Touch-Programmierung
4. **Eigene Projekte:** Nutze `CYD_Display_Config.h` in deinen Projekten

---

**Viel Erfolg beim Testen deiner Board-Konfiguration! 🎨**

Bei Fragen: [CYD-Academy GitHub Issues](https://github.com/smily77/CYD-Academy/issues)

---

**Version:** 1.1 (Simplified)
**Autor:** CYD-Academy Team
**Lizenz:** MIT
