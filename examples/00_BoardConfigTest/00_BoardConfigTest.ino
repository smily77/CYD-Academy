/*
 * CYD-Academy - Beispiel 00: Board Configuration Test
 *
 * Einfaches Test-Tool zur Validierung von Board-Konfigurationen (Display + Touch).
 * Zeigt RGB-Farben zur Kontrolle und ermöglicht freies Zeichnen auf dem ganzen Bildschirm.
 *
 * Hardware:
 * - ESP32 mit Display (CYD oder kompatibel)
 * - Display: ILI9341 (240x320), ILI9488 (320x480) oder andere
 * - Touch-Controller: XPT2046, FT6236 oder kompatibel
 *
 * Features:
 * - RGB Farb-Streifen (Rot, Grün, Blau) zur visuellen Kontrolle
 * - Touch-Test über den gesamten Bildschirm (Paint/Draw)
 * - Display-Info (Größe, Rotation)
 * - Automatische Skalierung für verschiedene Display-Größen
 * - Touch-Rotation-Fix integriert (aus CYD_Display_Config.h)
 *
 * Verwendung:
 * 1. Setze gewünschte Rotation in setup() (siehe Zeile mit tft.setRotation)
 * 2. Upload auf dein Board
 * 3. Prüfe RGB-Farben in der Info-Zeile (sollten klar erkennbar sein)
 * 4. Teste Touch durch Zeichnen auf dem Bildschirm
 * 5. Zum Testen anderer Rotationen: Ändere Rotation in setup() und upload erneut
 *
 * Rotation-Werte:
 * - 0 = Portrait (0°)
 * - 1 = Landscape (90°)
 * - 2 = Portrait umgedreht (180°)
 * - 3 = Landscape umgedreht (270°)
 *
 * Autor: CYD-Academy
 * Lizenz: MIT
 */

#include <CYD_Display_Config.h>

// ========== Display Setup ==========
LGFX tft;

// ========== Konfiguration ==========
#define COLOR_BG        0x0000      // Schwarz (Hintergrund)
#define COLOR_HEADER    0x2104      // Dunkelgrau (Header)
#define COLOR_TEXT      TFT_WHITE
#define COLOR_DRAW      TFT_YELLOW  // Farbe für Touch-Zeichnung

// Layout
int headerHeight = 30;  // Feste Höhe für Info-Zeile
int rgbWidth = 25;      // Breite für jedes RGB-Quadrat

// Touch-Tracking für smooth drawing
int16_t lastTouchX = -1;
int16_t lastTouchY = -1;

// ========== Setup ==========
void setup() {
  Serial.begin(115200);
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║  CYD Board Configuration Test          ║");
  Serial.println("║  Simple Display + Touch Tester         ║");
  Serial.println("╚════════════════════════════════════════╝\n");

  // Display initialisieren
  tft.init();

  // ========================================
  // ROTATION HIER EINSTELLEN (0, 1, 2, oder 3)
  // ========================================
  tft.setRotation(0);  // ← Ändere diese Zeile um verschiedene Rotationen zu testen

  Serial.printf("Display initialized: %dx%d pixels\n", tft.width(), tft.height());
  Serial.printf("Rotation: %d (0=Portrait, 1=Landscape, 2=Portrait 180°, 3=Landscape 270°)\n",
                tft.getRotation());
  Serial.println("\n✓ Display OK");
  Serial.println("✓ Touch-Rotation-Fix active (from CYD_Display_Config.h)");
  Serial.println("\nReady! Touch the screen to draw.\n");

  // UI zeichnen
  drawUI();
}

// ========== UI zeichnen ==========
void drawUI() {
  int w = tft.width();
  int h = tft.height();

  // Schwarzer Hintergrund
  tft.fillScreen(COLOR_BG);

  // ===== Header-Zeile (oben) =====
  tft.fillRect(0, 0, w, headerHeight, COLOR_HEADER);

  // RGB-Quadrate (links)
  tft.fillRect(5, 3, rgbWidth, headerHeight-6, TFT_RED);     // ROT
  tft.fillRect(5 + rgbWidth + 3, 3, rgbWidth, headerHeight-6, TFT_GREEN);   // GRÜN
  tft.fillRect(5 + (rgbWidth + 3) * 2, 3, rgbWidth, headerHeight-6, TFT_BLUE);    // BLAU

  // Labels für RGB
  tft.setTextColor(COLOR_TEXT);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(1);
  tft.drawString("R", 5 + rgbWidth/2, headerHeight/2);
  tft.drawString("G", 5 + rgbWidth + 3 + rgbWidth/2, headerHeight/2);
  tft.drawString("B", 5 + (rgbWidth + 3) * 2 + rgbWidth/2, headerHeight/2);

  // Display-Info (rechts)
  tft.setTextDatum(MR_DATUM);  // Middle Right
  char info[40];
  sprintf(info, "%dx%d  Rot:%d", w, h, tft.getRotation());
  tft.drawString(info, w - 5, headerHeight/2);

  // Linie unter Header
  tft.drawFastHLine(0, headerHeight, w, TFT_DARKGREY);

  // Info-Text in der Mitte
  tft.setTextColor(0x4208);  // Dunkelgrau
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  tft.drawString("Touch anywhere", w/2, h/2 - 20);
  tft.setTextSize(1);
  tft.drawString("to test touch accuracy", w/2, h/2 + 10);

  Serial.println("UI drawn - ready for touch test");
}

// ========== Main Loop ==========
void loop() {
  int16_t x, y;

  if (tft.getTouch(&x, &y)) {
    // Nur zeichnen wenn Touch außerhalb Header
    if (y > headerHeight) {
      // Punkt zeichnen
      tft.fillCircle(x, y, 2, COLOR_DRAW);

      // Linie zum letzten Punkt für smooth drawing
      if (lastTouchX >= 0 && lastTouchY >= 0) {
        tft.drawLine(lastTouchX, lastTouchY, x, y, COLOR_DRAW);
      }

      lastTouchX = x;
      lastTouchY = y;

      // Serielle Ausgabe für Debug
      Serial.printf("Touch: x=%d, y=%d\n", x, y);
    }
  } else {
    // Touch losgelassen - Reset für smooth drawing
    if (lastTouchX >= 0 || lastTouchY >= 0) {
      lastTouchX = -1;
      lastTouchY = -1;
    }
  }

  delay(10);  // Kleine Pause für Watchdog
}
