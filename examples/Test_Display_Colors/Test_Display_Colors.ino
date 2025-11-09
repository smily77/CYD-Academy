/*
  Display Color Test - Ohne LVGL

  Testet direkt mit LovyanGFX:
  - Schwarz / Weiß
  - Primärfarben (Rot, Grün, Blau)
  - Farbverläufe wie in 10c_TouchButtons_Premium

  Hilft zu diagnostizieren ob Problem bei:
  - Display-Treiber / Hardware
  - LVGL Library
*/

#include <CYD_Display_Config.h>

LGFX lcd;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Display Color Test ===\n");

  lcd.init();
  lcd.setRotation(1);  // Landscape
  lcd.setColorDepth(16);  // 16-bit Farben

  Serial.printf("Display: %dx%d\n", lcd.width(), lcd.height());
  Serial.println("\nStarte Farbtests...\n");

  delay(1000);

  // Test 1: Basis-Farben
  testBasicColors();
  delay(3000);
 
  // Test 2: Schwarz-Weiß Test
  testBlackWhite();
  delay(3000);

  // Test 3: Farbverläufe (wie 10c Buttons)
  testGradients();
  delay(3000);

  // Test 4: Farbverläufe (wie 10c Buttons)
  testGradients2();
  delay(3000);

  Serial.println("\n=== Test abgeschlossen ===");
  Serial.println("Schaue dir das Display an:");
  Serial.println("- Sind Schwarz/Weiß korrekt?");
  Serial.println("- Sind Verläufe glatt oder haben sie Striche?");
  Serial.println("- Sind Farben kräftig oder verwaschen?");
}

void loop() {
  // Nichts - statisches Testbild
}

// Test 1: Grundfarben
void testBasicColors() {
  Serial.println("Test 1: Basis-Farben");

  int w = lcd.width();
  int h = lcd.height();
  int blockW = w / 7;

  // Schwarz
  lcd.fillRect(0, 0, blockW, h, TFT_BLACK);
  Serial.println("- Schwarz (sollte RICHTIG schwarz sein)");

  // Rot
  lcd.fillRect(blockW, 0, blockW, h, TFT_RED);
  Serial.println("- Rot");

  // Grün
  lcd.fillRect(blockW * 2, 0, blockW, h, TFT_GREEN);
  Serial.println("- Grün");

  // Blau
  lcd.fillRect(blockW * 3, 0, blockW, h, TFT_BLUE);
  Serial.println("- Blau");

  // Gelb
  lcd.fillRect(blockW * 4, 0, blockW, h, TFT_YELLOW);
  Serial.println("- Gelb");

  // Cyan
  lcd.fillRect(blockW * 5, 0, blockW, h, TFT_CYAN);
  Serial.println("- Cyan");

  // Weiß
  lcd.fillRect(blockW * 6, 0, blockW, h, TFT_WHITE);
  Serial.println("- Weiß (sollte RICHTIG weiß sein)");
}

// Test 2: Farbverläufe (wie in 10c)
void testGradients() {
  Serial.println("\nTest 2: Farbverläufe (wie 10c Buttons)");

  int w = lcd.width();
  int h = lcd.height();
  int barH = h / 3;

  // Verlauf 1: Rot → Orange (Momentary Button)
  Serial.println("- Gradient 1: Rot → Orange");
  drawGradient(0, 0, w, barH, 0xFF4444, 0xFF8844);

  // Verlauf 2: Grün → Cyan (Toggle Button)
  Serial.println("- Gradient 2: Grün → Cyan");
  drawGradient(0, barH, w, barH, 0x44FF44, 0x44FFFF);

  // Verlauf 3: Violett → Magenta (Radio A Button)
  Serial.println("- Gradient 3: Violett → Magenta");
  drawGradient(0, barH * 2, w, barH, 0x8844FF, 0xFF44FF);
}

void testGradients2() {
  Serial.println("\nTest 2: Farbverläufe (wie 10c Buttons)");

  int w = lcd.width();
  int h = lcd.height();
  int barH = h / 3;

  // Verlauf 1: Rot → Orange (Momentary Button)
  Serial.println("- Gradient 1: Rot → Orange");
  drawGradient2(0, 0, w, barH, 0xFF4444, 0xFF8844);

  // Verlauf 2: Grün → Cyan (Toggle Button)
  Serial.println("- Gradient 2: Grün → Cyan");
  drawGradient2(0, barH, w, barH, 0x44FF44, 0x44FFFF);

  // Verlauf 3: Violett → Magenta (Radio A Button)
  Serial.println("- Gradient 3: Violett → Magenta");
  drawGradient2(0, barH * 2, w, barH, 0x8844FF, 0xFF44FF);
}

// Test 3: Schwarz-Weiß Streifen
void testBlackWhite() {
  Serial.println("\nTest 3: Schwarz-Weiß Test");

  int w = lcd.width();
  int h = lcd.height();

  // Linke Hälfte: Schwarz
  lcd.fillRect(0, 0, w / 2, h, TFT_BLACK);
  Serial.println("- Links: SCHWARZ");

  // Rechte Hälfte: Weiß
  lcd.fillRect(w / 2, 0, w / 2, h, TFT_WHITE);
  Serial.println("- Rechts: WEISS");

  // Text in der Mitte
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.setTextDatum(CC_DATUM);
  lcd.drawString("SCHWARZ", w / 4, h / 2);

  lcd.setTextColor(TFT_BLACK, TFT_WHITE);
  lcd.drawString("WEISS", w * 3 / 4, h / 2);
}

// Zeichnet einen vertikalen Farbverlauf mit Dithering
void drawGradient(int x, int y, int w, int h, uint32_t color1, uint32_t color2) {
  // Bayer 4x4 Dithering Matrix
  const int bayer4x4[4][4] = {
    {0,  8,  2,  10},
    {12, 4,  14, 6},
    {3,  11, 1,  9},
    {15, 7,  13, 5}
  };

  // Farben extrahieren
  uint8_t r1 = (color1 >> 16) & 0xFF;
  uint8_t g1 = (color1 >> 8) & 0xFF;
  uint8_t b1 = color1 & 0xFF;

  uint8_t r2 = (color2 >> 16) & 0xFF;
  uint8_t g2 = (color2 >> 8) & 0xFF;
  uint8_t b2 = color2 & 0xFF;

  lcd.startWrite();

  // Gradient Zeile für Zeile zeichnen
  for (int i = 0; i < h; i++) {
    // Interpolation
    float ratio = (float)i / (float)h;

    for (int j = 0; j < w; j++) {
      // Basis-Farbe berechnen
      int r = r1 + (r2 - r1) * ratio;
      int g = g1 + (g2 - g1) * ratio;
      int b = b1 + (b2 - b1) * ratio;

      // Dithering: Bayer-Matrix Wert holen
      int threshold = bayer4x4[i % 4][j % 4];

      // Dithering anwenden (±4 basierend auf Matrix)
      r = constrain(r + (threshold - 8) / 2, 0, 255);
      g = constrain(g + (threshold - 8) / 2, 0, 255);
      b = constrain(b + (threshold - 8) / 2, 0, 255);

      // RGB888 → RGB565 konvertieren
      uint16_t color565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

      // Pixel zeichnen
      lcd.writePixel(x + j, y + i, color565);
    }
  }

  lcd.endWrite();
}

void drawGradient2(int x, int y, int w, int h, uint32_t color1, uint32_t color2) {
  lcd.startWrite();  // Batch-Modus starten
  
  uint8_t r1 = (color1 >> 16) & 0xFF;
  uint8_t g1 = (color1 >> 8) & 0xFF;
  uint8_t b1 = color1 & 0xFF;

  uint8_t r2 = (color2 >> 16) & 0xFF;
  uint8_t g2 = (color2 >> 8) & 0xFF;
  uint8_t b2 = color2 & 0xFF;

  for (int i = 0; i < h; i++) {
    float ratio = (float)i / (float)h;

    uint8_t r = r1 + (r2 - r1) * ratio;
    uint8_t g = g1 + (g2 - g1) * ratio;
    uint8_t b = b1 + (b2 - b1) * ratio;

    // WICHTIG: Nutze lcd.color565() für korrekte Konvertierung!
    uint16_t color = lcd.color565(r, g, b);

    lcd.drawFastHLine(x, y + i, w, color);
  }
  
  lcd.endWrite();  // Batch-Modus beenden
}
