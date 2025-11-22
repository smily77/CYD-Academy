/*
 * CYD-Academy - Beispiel 00: Board Configuration Test
 *
 * Dieses Beispiel testet die Board-Konfiguration (Display + Touch) und
 * demonstriert die wichtigsten Funktionen. Ideal zum Validieren einer
 * neuen Board-Definition oder zum Testen verschiedener Display-Größen.
 *
 * Hardware:
 * - ESP32 mit Display (CYD oder kompatibel)
 * - Display: ILI9341 (240x320), ILI9488 (320x480) oder andere
 * - Touch-Controller: XPT2046, FT6236 oder kompatibel
 *
 * Features:
 * - RGB Farb-Test (Rot, Grün, Blau) zur visuellen Kontrolle
 * - Touch-Demonstration mit Paint/Draw Funktion
 * - Rotation-Unterstützung (Portrait ⟷ Landscape)
 * - Automatische Skalierung für verschiedene Display-Größen
 * - Display-Info (Größe, Rotation)
 * - Touch-Rotation-Fix integriert (aus CYD_Display_Config.h)
 *
 * Bedienung:
 * - Touch auf RGB-Bereiche: Farbe wechseln & Farbtest
 * - Touch im Zeichen-Bereich: Zeichnen/Malen
 * - Touch auf "🔄 ROTATE" Button: Display rotieren
 * - Touch auf "CLEAR" Button: Zeichenfläche löschen
 *
 * Verwendung:
 * 1. Upload dieses Sketches auf dein Board
 * 2. Prüfe die RGB-Farben (sollten klar erkennbar sein)
 * 3. Teste Touch-Funktionalität (zeichnen & Buttons)
 * 4. Teste Rotation (Touch sollte in allen Rotationen funktionieren)
 * 5. Prüfe Display-Größe im Info-Bereich
 *
 * Autor: CYD-Academy
 * Lizenz: MIT
 */

#include <CYD_Display_Config.h>

// ========== Display Setup ==========
LGFX tft;

// ========== Konfiguration ==========
// Farben
#define COLOR_BG        0x1082      // Dunkelgrau (Hintergrund)
#define COLOR_HEADER    0x4228      // Mittelgrau (Header)
#define COLOR_TEXT      TFT_WHITE
#define COLOR_RED       TFT_RED
#define COLOR_GREEN     TFT_GREEN
#define COLOR_BLUE      TFT_BLUE
#define COLOR_BTN       0x5AEB      // Hellgrau (Buttons)
#define COLOR_BTN_TEXT  TFT_BLACK

// Layout wird dynamisch berechnet basierend auf Display-Größe
int headerHeight;
int rgbHeight;
int drawAreaY;
int drawAreaHeight;
int footerY;
int footerHeight;

// RGB Bereich Flags
bool showRed = true;
bool showGreen = true;
bool showBlue = true;

// Aktuelle Zeichen-Farbe
uint16_t drawColor = TFT_YELLOW;

// Touch-Tracking für smooth drawing
int16_t lastTouchX = -1;
int16_t lastTouchY = -1;

// ========== Funktions-Deklarationen ==========
void calculateLayout();
void drawHeader();
void drawRGBArea();
void drawDrawArea();
void drawFooter();
void drawUI();
void handleTouch(int16_t x, int16_t y);
void clearDrawArea();
void rotateDisplay();

// ========== Setup ==========
void setup() {
  Serial.begin(115200);
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║  CYD Board Configuration Test          ║");
  Serial.println("║  Testing Display + Touch + Rotation    ║");
  Serial.println("╚════════════════════════════════════════╝\n");

  // Display initialisieren
  tft.init();
  tft.setRotation(0);  // Start mit Portrait
  tft.fillScreen(COLOR_BG);

  Serial.printf("Display initialized: %dx%d pixels\n", tft.width(), tft.height());
  Serial.printf("Rotation: %d (0=Portrait, 1=Landscape)\n", tft.getRotation());
  Serial.println("\n✓ Display OK");
  Serial.println("✓ Touch-Rotation-Fix active (from CYD_Display_Config.h)");
  Serial.println("\nReady! Touch the screen to test.\n");

  // Layout berechnen und UI zeichnen
  calculateLayout();
  drawUI();
}

// ========== Layout-Berechnung ==========
void calculateLayout() {
  int h = tft.height();

  // Layout proportional zur Display-Höhe
  headerHeight = h / 8;        // ~12.5% für Header
  rgbHeight = h / 5;           // ~20% für RGB-Bereiche
  drawAreaHeight = h / 2;      // ~50% für Zeichenfläche
  footerHeight = h / 8;        // ~12.5% für Footer

  // Y-Positionen
  drawAreaY = headerHeight + rgbHeight;
  footerY = h - footerHeight;

  Serial.printf("\nLayout calculated for %dx%d:\n", tft.width(), tft.height());
  Serial.printf("  Header: %dpx\n", headerHeight);
  Serial.printf("  RGB: %dpx\n", rgbHeight);
  Serial.printf("  Draw Area: %dpx\n", drawAreaHeight);
  Serial.printf("  Footer: %dpx\n", footerHeight);
}

// ========== Header zeichnen ==========
void drawHeader() {
  int w = tft.width();

  // Header-Hintergrund
  tft.fillRect(0, 0, w, headerHeight, COLOR_HEADER);

  // Titel
  tft.setTextColor(COLOR_TEXT);
  tft.setTextDatum(MC_DATUM);  // Middle Center

  // Größe anpassen je nach Display
  if (w >= 400) {
    tft.setTextSize(3);
  } else if (w >= 300) {
    tft.setTextSize(2);
  } else {
    tft.setTextSize(1);
  }

  tft.drawString("BOARD CONFIG TEST", w/2, headerHeight/2);

  // Linie unter Header
  tft.drawFastHLine(0, headerHeight-1, w, TFT_WHITE);
}

// ========== RGB Test-Bereich zeichnen ==========
void drawRGBArea() {
  int w = tft.width();
  int blockWidth = w / 3;
  int y = headerHeight;

  // RED Block
  tft.fillRect(0, y, blockWidth, rgbHeight, showRed ? COLOR_RED : COLOR_BG);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(w >= 300 ? 2 : 1);
  tft.drawString("RED", blockWidth/2, y + rgbHeight/2);

  // GREEN Block
  tft.fillRect(blockWidth, y, blockWidth, rgbHeight, showGreen ? COLOR_GREEN : COLOR_BG);
  tft.setTextColor(COLOR_TEXT);
  tft.drawString("GREEN", blockWidth + blockWidth/2, y + rgbHeight/2);

  // BLUE Block
  tft.fillRect(blockWidth*2, y, w - blockWidth*2, rgbHeight, showBlue ? COLOR_BLUE : COLOR_BG);
  tft.setTextColor(COLOR_TEXT);
  tft.drawString("BLUE", blockWidth*2 + blockWidth/2, y + rgbHeight/2);

  // Trennlinien
  tft.drawFastVLine(blockWidth, y, rgbHeight, TFT_WHITE);
  tft.drawFastVLine(blockWidth*2, y, rgbHeight, TFT_WHITE);
  tft.drawFastHLine(0, y + rgbHeight - 1, w, TFT_WHITE);
}

// ========== Zeichenfläche initialisieren ==========
void drawDrawArea() {
  int w = tft.width();

  // Zeichenfläche löschen
  tft.fillRect(0, drawAreaY, w, drawAreaHeight, COLOR_BG);

  // Info-Text in der Mitte
  tft.setTextColor(0x7BEF);  // Hellgrau
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(w >= 300 ? 2 : 1);
  tft.drawString("Touch to Draw", w/2, drawAreaY + drawAreaHeight/3);

  if (w >= 300) {
    tft.setTextSize(1);
    tft.drawString("(Test Touch Accuracy)", w/2, drawAreaY + drawAreaHeight/2);
  }

  // Linie unter Zeichenfläche
  tft.drawFastHLine(0, footerY - 1, w, TFT_WHITE);
}

// ========== Footer mit Buttons und Info ==========
void drawFooter() {
  int w = tft.width();
  int h = tft.height();

  // Footer-Hintergrund
  tft.fillRect(0, footerY, w, footerHeight, COLOR_HEADER);

  // Display Info
  tft.setTextColor(COLOR_TEXT);
  tft.setTextDatum(TC_DATUM);  // Top Center
  tft.setTextSize(1);

  char info[50];
  sprintf(info, "Display: %dx%d  Rotation: %d", w, h, tft.getRotation());
  tft.drawString(info, w/2, footerY + 5);

  // Buttons
  int btnY = footerY + footerHeight/2 + 5;
  int btnWidth = w / 3;
  int btnHeight = footerHeight / 3;

  // ROTATE Button (links)
  tft.fillRoundRect(10, btnY, btnWidth - 20, btnHeight, 5, COLOR_BTN);
  tft.setTextColor(COLOR_BTN_TEXT);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("ROTATE", btnWidth/2, btnY + btnHeight/2);

  // CLEAR Button (rechts)
  tft.fillRoundRect(w - btnWidth + 10, btnY, btnWidth - 20, btnHeight, 5, COLOR_BTN);
  tft.drawString("CLEAR", w - btnWidth/2, btnY + btnHeight/2);

  // Color Indicator (Mitte)
  tft.fillRoundRect(w/2 - 20, btnY, 40, btnHeight, 5, drawColor);
  tft.drawRoundRect(w/2 - 20, btnY, 40, btnHeight, 5, TFT_WHITE);
}

// ========== Komplettes UI zeichnen ==========
void drawUI() {
  tft.fillScreen(COLOR_BG);
  drawHeader();
  drawRGBArea();
  drawDrawArea();
  drawFooter();

  Serial.println("UI drawn successfully");
}

// ========== Touch-Handling ==========
void handleTouch(int16_t x, int16_t y) {
  int w = tft.width();
  int h = tft.height();

  Serial.printf("Touch detected: x=%d, y=%d\n", x, y);

  // Header-Bereich (keine Aktion)
  if (y < headerHeight) {
    return;
  }

  // RGB-Bereich
  else if (y >= headerHeight && y < headerHeight + rgbHeight) {
    int blockWidth = w / 3;

    // RED Block
    if (x < blockWidth) {
      showRed = !showRed;
      drawColor = COLOR_RED;
      Serial.println("RED block toggled");
    }
    // GREEN Block
    else if (x < blockWidth * 2) {
      showGreen = !showGreen;
      drawColor = COLOR_GREEN;
      Serial.println("GREEN block toggled");
    }
    // BLUE Block
    else {
      showBlue = !showBlue;
      drawColor = COLOR_BLUE;
      Serial.println("BLUE block toggled");
    }

    drawRGBArea();
    drawFooter();  // Update color indicator
  }

  // Zeichenfläche
  else if (y >= drawAreaY && y < footerY) {
    // Punkt zeichnen
    tft.fillCircle(x, y, 3, drawColor);

    // Linie zum letzten Punkt (für smooth drawing)
    if (lastTouchX >= 0 && lastTouchY >= 0) {
      tft.drawLine(lastTouchX, lastTouchY, x, y, drawColor);
    }

    lastTouchX = x;
    lastTouchY = y;
  }

  // Footer-Bereich (Buttons)
  else if (y >= footerY) {
    int btnY = footerY + footerHeight/2 + 5;
    int btnHeight = footerHeight / 3;
    int btnWidth = w / 3;

    // Check if y is in button area
    if (y >= btnY && y <= btnY + btnHeight) {
      // ROTATE Button (links)
      if (x < btnWidth) {
        Serial.println("ROTATE button pressed");
        rotateDisplay();
      }
      // CLEAR Button (rechts)
      else if (x > w - btnWidth) {
        Serial.println("CLEAR button pressed");
        clearDrawArea();
      }
    }
  }
}

// ========== Zeichenfläche löschen ==========
void clearDrawArea() {
  drawDrawArea();
  lastTouchX = -1;
  lastTouchY = -1;

  Serial.println("Draw area cleared");
}

// ========== Display rotieren ==========
void rotateDisplay() {
  // Nächste Rotation (0 → 1 → 2 → 3 → 0)
  uint8_t newRotation = (tft.getRotation() + 1) % 4;
  tft.setRotation(newRotation);

  Serial.printf("Rotation changed to: %d\n", newRotation);
  Serial.printf("New dimensions: %dx%d\n", tft.width(), tft.height());

  // Layout neu berechnen und UI neu zeichnen
  calculateLayout();
  drawUI();

  lastTouchX = -1;
  lastTouchY = -1;
}

// ========== Main Loop ==========
void loop() {
  // Touch prüfen
  int16_t x, y;
  if (tft.getTouch(&x, &y)) {
    handleTouch(x, y);

    // Kurze Verzögerung für Debouncing
    delay(10);
  } else {
    // Touch losgelassen - Reset für smooth drawing
    if (lastTouchX >= 0 || lastTouchY >= 0) {
      lastTouchX = -1;
      lastTouchY = -1;
    }
  }

  delay(10);  // Kleine Pause für Watchdog
}
