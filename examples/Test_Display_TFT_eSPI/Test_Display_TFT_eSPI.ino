/*
  Display Color Test - Mit TFT_eSPI

  Testet ob Gradient-Banding Problem bei LovyanGFX oder Hardware liegt.

  WICHTIG: TFT_eSPI Library muss installiert sein!

  Alle Konfiguration ist INLINE - keine Änderungen in TFT_eSPI Library nötig!
*/

// ============================================
// TFT_eSPI Setup - MUSS VOR #include stehen!
// ============================================
/*
#define USER_SETUP_INFO "CYD_28_USBC"

// Display Treiber
#define ILI9341_DRIVER
//#define ILI9341_2_DRIVER

// ESP32 CYD 2.8" USB-C Pin-Konfiguration
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  -1   // Kein Reset-Pin verbunden
#define TFT_BL   21   // Backlight

// SPI Frequenz
#define SPI_FREQUENCY  27000000
#define SPI_READ_FREQUENCY 16000000

// Touch (wird hier nicht genutzt, aber für Vollständigkeit)
#define TOUCH_CS 33

// Farb-Reihenfolge
#define TFT_RGB_ORDER TFT_BGR

// Font Support
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT
*/
#define ILI9341_DRIVER            // Die meisten 2.8" CYD nutzen ILI9341
//#define ST7789_DRIVER            // <-- nur aktivieren, falls dein Board ST7789 hat

// --- Auflösung ---
#define TFT_WIDTH   240
#define TFT_HEIGHT  320

// --- ESP32 CYD 2.8" (USB-C) SPI-Pins ---
#define TFT_MOSI 13
#define TFT_MISO 12          // statt 12 (Boot-Strap-Probleme), 19 ist sicher
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  4           // WICHTIG: echter Reset-Pin am CYD

// --- Backlight ---
#define TFT_BL   21
#define TFT_BACKLIGHT_ON 1    // CYD: BL aktiv HIGH

// --- SPI-Frequenzen ---
#define SPI_FREQUENCY       40000000   // 40 MHz läuft stabil mit ILI9341
#define SPI_READ_FREQUENCY  16000000

// --- (Optional) Touch (XPT2046 auf manchen CYD-Varianten) ---
// #define TOUCH_CS 33
// #define TOUCH_IRQ 36          // nur wenn vorhanden

// --- Farben ---
#define TFT_RGB_ORDER TFT_BGR

// --- Fonts ---
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// ============================================
// Jetzt erst Library einbinden!
// ============================================
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n=================================");
  Serial.println("Display Color Test (TFT_eSPI)");
  Serial.println("=================================\n");

  // WICHTIG: Backlight ZUERST einschalten!
  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH);
  Serial.println("Backlight eingeschaltet");

  delay(100);

  // Display initialisieren
  tft.init();
  Serial.println("Display initialisiert");

  tft.setRotation(1);  // Landscape
  Serial.printf("Rotation gesetzt - Auflösung: %dx%d\n", tft.width(), tft.height());

  // Basis-Test Sequenz: Zeigt ob Display grundsätzlich funktioniert
  Serial.println("\n>>> Basis-Test: Farbsequenz");

  tft.fillScreen(TFT_RED);
  Serial.println("- ROT");
  delay(2000);

  tft.fillScreen(TFT_GREEN);
  Serial.println("- GRÜN");
  delay(2000);

  tft.fillScreen(TFT_BLUE);
  Serial.println("- BLAU");
  delay(2000);

  tft.fillScreen(TFT_WHITE);
  Serial.println("- WEISS");
  delay(2000);

  // Text-Test
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 100);
  tft.println("TFT_eSPI funktioniert!");
  Serial.println("\nText angezeigt - Display funktioniert!");
  delay(3000);

  // Jetzt die eigentlichen Tests
  Serial.println("\n>>> Test 1: Basis-Farben");
  testBasicColors();
  delay(3000);

  Serial.println("\n>>> Test 2: Schwarz-Weiß Kontrast");
  testBlackWhite();
  delay(3000);

  // WICHTIGSTER TEST: Gradients
  Serial.println("\n>>> Test 3: Farbverläufe");
  Serial.println("ACHTUNG: Auf graue Streifen achten!");
  testGradients();

  Serial.println("\n=================================");
  Serial.println("Test abgeschlossen!");
  Serial.println("=================================");
  Serial.println("\nVergleiche mit LovyanGFX Version:");
  Serial.println("- Sind die Streifen gleich?");
  Serial.println("- Besser/schlechter/gleich?");
}

void loop() {
  // Statisches Testbild bleibt stehen
}

// Test 1: Grundfarben
void testBasicColors() {
  int w = tft.width();
  int h = tft.height();
  int blockW = w / 7;

  tft.fillRect(0, 0, blockW, h, TFT_BLACK);
  Serial.println("- Schwarz");

  tft.fillRect(blockW, 0, blockW, h, TFT_RED);
  Serial.println("- Rot");

  tft.fillRect(blockW * 2, 0, blockW, h, TFT_GREEN);
  Serial.println("- Grün");

  tft.fillRect(blockW * 3, 0, blockW, h, TFT_BLUE);
  Serial.println("- Blau");

  tft.fillRect(blockW * 4, 0, blockW, h, TFT_YELLOW);
  Serial.println("- Gelb");

  tft.fillRect(blockW * 5, 0, blockW, h, TFT_CYAN);
  Serial.println("- Cyan");

  tft.fillRect(blockW * 6, 0, blockW, h, TFT_WHITE);
  Serial.println("- Weiß");
}

// Test 2: Schwarz-Weiß Kontrast
void testBlackWhite() {
  int w = tft.width();
  int h = tft.height();

  tft.fillRect(0, 0, w / 2, h, TFT_BLACK);
  Serial.println("- Links: SCHWARZ");

  tft.fillRect(w / 2, 0, w / 2, h, TFT_WHITE);
  Serial.println("- Rechts: WEISS");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  tft.drawString("SCHWARZ", w / 4, h / 2);

  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawString("WEISS", w * 3 / 4, h / 2);
}

// Test 3: Farbverläufe (WICHTIGSTER TEST!)
void testGradients() {
  int w = tft.width();
  int h = tft.height();
  int barH = h / 3;

  Serial.println("- Gradient 1: Rot → Orange");
  drawGradient(0, 0, w, barH, 0xFF4444, 0xFF8844);

  Serial.println("- Gradient 2: Grün → Cyan");
  drawGradient(0, barH, w, barH, 0x44FF44, 0x44FFFF);

  Serial.println("- Gradient 3: Violett → Magenta");
  drawGradient(0, barH * 2, w, barH, 0x8844FF, 0xFF44FF);
}

// Zeichnet einen vertikalen Farbverlauf
void drawGradient(int x, int y, int w, int h, uint32_t color1, uint32_t color2) {
  // Farben extrahieren
  uint8_t r1 = (color1 >> 16) & 0xFF;
  uint8_t g1 = (color1 >> 8) & 0xFF;
  uint8_t b1 = color1 & 0xFF;

  uint8_t r2 = (color2 >> 16) & 0xFF;
  uint8_t g2 = (color2 >> 8) & 0xFF;
  uint8_t b2 = color2 & 0xFF;

  // Gradient Zeile für Zeile zeichnen
  for (int i = 0; i < h; i++) {
    float ratio = (float)i / (float)h;

    uint8_t r = r1 + (r2 - r1) * ratio;
    uint8_t g = g1 + (g2 - g1) * ratio;
    uint8_t b = b1 + (b2 - b1) * ratio;

    // TFT_eSPI's color565 Funktion verwenden
    uint16_t color = tft.color565(r, g, b);

    tft.drawFastHLine(x, y + i, w, color);
  }
}
