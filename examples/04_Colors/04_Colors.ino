/*
 * CYD Academy - Colors Beispiel
 *
 * Dieses Beispiel demonstriert verschiedene Farboperationen:
 * - Vordefinierte Farben
 * - RGB Farben erstellen
 * - Farbverläufe (Gradients)
 * - Regenbogeneffekte
 * - HSV zu RGB Konvertierung
 *
 * Hardware: CYD 2.8" oder 3.5" ESP32 Display
 * Library: LovyanGFX
 */

#include <LovyanGFX.hpp>
#include <CYD_Display_Config.h>

// Display-Objekt erstellen
static LGFX lcd;

// Display-Auflösung (wird dynamisch ermittelt)
int SCREEN_WIDTH;
int SCREEN_HEIGHT;

void setup() {
  Serial.begin(115200);
  Serial.println("CYD Colors Demo gestartet!");

  // Display initialisieren
  lcd.init();

  // Display-Auflösung ermitteln
  SCREEN_WIDTH = lcd.width();
  SCREEN_HEIGHT = lcd.height();

  Serial.print("Display-Auflösung: ");
  Serial.print(SCREEN_WIDTH);
  Serial.print("x");
  Serial.println(SCREEN_HEIGHT);

  // Optionale Pins initialisieren (falls definiert)
  #ifdef LED_PIN_R
    pinMode(LED_PIN_R, OUTPUT);
    pinMode(LED_PIN_G, OUTPUT);
    pinMode(LED_PIN_B, OUTPUT);
    digitalWrite(LED_PIN_R, HIGH);  // HIGH = OFF
    digitalWrite(LED_PIN_G, HIGH);  // HIGH = OFF
    digitalWrite(LED_PIN_B, HIGH);  // HIGH = OFF
    Serial.println("RGB LED initialisiert");
  #endif

  #ifdef photoR_PIN
    // pinMode nicht nötig - ADC Pins sind standardmäßig INPUT
    Serial.println("Photoresistor initialisiert");
  #endif

  lcd.setBrightness(200);
  lcd.setRotation(1);
}

void loop() {
  // 1. Vordefinierte Farben anzeigen
  showPredefinedColors();
  delay(3000);

  // 2. RGB Farbmischung
  showRGBColors();
  delay(3000);

  // 3. Horizontaler Farbverlauf
  showHorizontalGradient();
  delay(3000);

  // 4. Vertikaler Farbverlauf
  showVerticalGradient();
  delay(3000);

  // 5. Regenbogen-Balken
  showRainbowBars();
  delay(3000);

  // 6. Radialer Farbverlauf
  showRadialGradient();
  delay(3000);

  // 7. Farb-Animation
  showColorAnimation();
  delay(2000);
}

// Vordefinierte Farben
void showPredefinedColors() {
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(10, 10);
  lcd.println("Vordefinierte Farben");

  int boxSize = SCREEN_WIDTH / 8;
  int spacing = SCREEN_WIDTH / 32;
  int y = SCREEN_HEIGHT / 5;

  // Erste Reihe
  lcd.fillRect(spacing, y, boxSize, boxSize, TFT_RED);
  lcd.fillRect(spacing + boxSize + spacing, y, boxSize, boxSize, TFT_GREEN);
  lcd.fillRect(spacing + (boxSize + spacing) * 2, y, boxSize, boxSize, TFT_BLUE);
  lcd.fillRect(spacing + (boxSize + spacing) * 3, y, boxSize, boxSize, TFT_YELLOW);
  lcd.fillRect(spacing + (boxSize + spacing) * 4, y, boxSize, boxSize, TFT_CYAN);
  lcd.fillRect(spacing + (boxSize + spacing) * 5, y, boxSize, boxSize, TFT_MAGENTA);

  // Zweite Reihe
  y = SCREEN_HEIGHT / 5 + boxSize + spacing;
  lcd.fillRect(spacing, y, boxSize, boxSize, TFT_ORANGE);
  lcd.fillRect(spacing + boxSize + spacing, y, boxSize, boxSize, TFT_PURPLE);
  lcd.fillRect(spacing + (boxSize + spacing) * 2, y, boxSize, boxSize, TFT_PINK);
  lcd.fillRect(spacing + (boxSize + spacing) * 3, y, boxSize, boxSize, TFT_BROWN);
  lcd.fillRect(spacing + (boxSize + spacing) * 4, y, boxSize, boxSize, TFT_LIGHTGREY);
  lcd.fillRect(spacing + (boxSize + spacing) * 5, y, boxSize, boxSize, TFT_DARKGREY);

  // Dritte Reihe
  y = SCREEN_HEIGHT / 5 + (boxSize + spacing) * 2;
  lcd.fillRect(spacing, y, boxSize, boxSize, TFT_WHITE);
  lcd.fillRect(spacing + boxSize + spacing, y, boxSize, boxSize, TFT_BLACK);
  lcd.drawRect(spacing + boxSize + spacing, y, boxSize, boxSize, TFT_WHITE);  // Rahmen für schwarzes Quadrat
  lcd.fillRect(spacing + (boxSize + spacing) * 2, y, boxSize, boxSize, TFT_GREENYELLOW);
  lcd.fillRect(spacing + (boxSize + spacing) * 3, y, boxSize, boxSize, TFT_NAVY);
  lcd.fillRect(spacing + (boxSize + spacing) * 4, y, boxSize, boxSize, TFT_MAROON);
  lcd.fillRect(spacing + (boxSize + spacing) * 5, y, boxSize, boxSize, TFT_OLIVE);
}

// RGB Farbmischung
void showRGBColors() {
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(10, 10);
  lcd.println("RGB Farbmischung");

  lcd.setTextSize(1);

  int barHeight = SCREEN_HEIGHT / 12;
  int startX = 10;
  int barWidth = SCREEN_WIDTH - 20;
  int segmentWidth = barWidth / 32;

  // Rot-Verlauf
  lcd.setCursor(10, 40);
  lcd.println("Rot:");
  for (int i = 0; i < 32; i++) {
    uint16_t color = lcd.color565(i * 8, 0, 0);
    lcd.fillRect(startX + i * segmentWidth, 55, segmentWidth, barHeight, color);
  }

  // Grün-Verlauf
  lcd.setCursor(10, 55 + barHeight + 10);
  lcd.println("Gruen:");
  for (int i = 0; i < 32; i++) {
    uint16_t color = lcd.color565(0, i * 8, 0);
    lcd.fillRect(startX + i * segmentWidth, 55 + barHeight + 25, segmentWidth, barHeight, color);
  }

  // Blau-Verlauf
  lcd.setCursor(10, 55 + barHeight * 2 + 35);
  lcd.println("Blau:");
  for (int i = 0; i < 32; i++) {
    uint16_t color = lcd.color565(0, 0, i * 8);
    lcd.fillRect(startX + i * segmentWidth, 55 + barHeight * 2 + 50, segmentWidth, barHeight, color);
  }

  // Graustufen
  lcd.setCursor(10, 55 + barHeight * 3 + 60);
  lcd.println("Graustufen:");
  for (int i = 0; i < 32; i++) {
    uint16_t color = lcd.color565(i * 8, i * 8, i * 8);
    lcd.fillRect(startX + i * segmentWidth, 55 + barHeight * 3 + 75, segmentWidth, barHeight, color);
  }
}

// Horizontaler Farbverlauf
void showHorizontalGradient() {
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(10, 10);
  lcd.println("Horiz. Verlauf");

  // Rot zu Blau
  for (int x = 0; x < SCREEN_WIDTH; x++) {
    int red = map(x, 0, SCREEN_WIDTH - 1, 255, 0);
    int blue = map(x, 0, SCREEN_WIDTH - 1, 0, 255);
    uint16_t color = lcd.color565(red, 0, blue);
    lcd.drawLine(x, 50, x, 100, color);
  }

  // Grün zu Gelb
  for (int x = 0; x < SCREEN_WIDTH; x++) {
    int red = map(x, 0, SCREEN_WIDTH - 1, 0, 255);
    int green = 255;
    uint16_t color = lcd.color565(red, green, 0);
    lcd.drawLine(x, 120, x, 170, color);
  }

  // Regenbogen
  for (int x = 0; x < SCREEN_WIDTH; x++) {
    float hue = (float)x / (float)SCREEN_WIDTH;
    uint16_t color = hsvToRgb565(hue, 1.0, 1.0);
    lcd.drawLine(x, SCREEN_HEIGHT * 4 / 5, x, SCREEN_HEIGHT, color);
  }
}

// Vertikaler Farbverlauf
void showVerticalGradient() {
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(10, 10);
  lcd.println("Vert. Verlauf");

  // Weiß zu Schwarz (links)
  for (int y = 40; y < SCREEN_HEIGHT; y++) {
    int brightness = map(y, 40, SCREEN_HEIGHT - 1, 255, 0);
    uint16_t color = lcd.color565(brightness, brightness, brightness);
    lcd.drawLine(10, y, SCREEN_WIDTH / 4, y, color);
  }

  // Gelb zu Blau (mitte)
  for (int y = 40; y < SCREEN_HEIGHT; y++) {
    int red = map(y, 40, SCREEN_HEIGHT - 1, 255, 0);
    int green = map(y, 40, SCREEN_HEIGHT - 1, 255, 0);
    int blue = map(y, 40, SCREEN_HEIGHT - 1, 0, 255);
    uint16_t color = lcd.color565(red, green, blue);
    lcd.drawLine(SCREEN_WIDTH / 3, y, SCREEN_WIDTH * 2 / 3, y, color);
  }

  // Rot zu Grün (rechts)
  for (int y = 40; y < SCREEN_HEIGHT; y++) {
    int red = map(y, 40, SCREEN_HEIGHT - 1, 255, 0);
    int green = map(y, 40, SCREEN_HEIGHT - 1, 0, 255);
    uint16_t color = lcd.color565(red, green, 0);
    lcd.drawLine(SCREEN_WIDTH * 3 / 4, y, SCREEN_WIDTH - 10, y, color);
  }
}

// Regenbogen-Balken
void showRainbowBars() {
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(10, 10);
  lcd.println("Regenbogen");

  int barHeight = 30;
  int numBars = 7;

  uint16_t rainbowColors[] = {
    TFT_RED,
    TFT_ORANGE,
    TFT_YELLOW,
    TFT_GREEN,
    TFT_CYAN,
    TFT_BLUE,
    TFT_PURPLE
  };

  for (int i = 0; i < numBars; i++) {
    int y = 40 + i * barHeight;
    lcd.fillRect(0, y, SCREEN_WIDTH, barHeight, rainbowColors[i]);
  }
}

// Radialer Farbverlauf
void showRadialGradient() {
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(10, 10);
  lcd.println("Radialer Verlauf");

  int centerX = SCREEN_WIDTH / 2;
  int centerY = (SCREEN_HEIGHT + 40) / 2;  // Mitte zwischen Header und unten
  int maxRadius = (SCREEN_HEIGHT - 80) / 2;

  for (int r = maxRadius; r > 0; r -= 2) {
    int brightness = map(r, 0, maxRadius, 255, 0);
    uint16_t color = lcd.color565(brightness, 0, 255 - brightness);
    lcd.fillCircle(centerX, centerY, r, color);
  }
}

// Farb-Animation
void showColorAnimation() {
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(10, 10);
  lcd.println("Animation");

  for (int frame = 0; frame < 100; frame++) {
    for (int y = 40; y < SCREEN_HEIGHT; y += 5) {
      float hue = ((float)y / (float)(SCREEN_HEIGHT - 40) + (float)frame / 100.0);
      if (hue > 1.0) hue -= 1.0;
      uint16_t color = hsvToRgb565(hue, 1.0, 1.0);
      lcd.fillRect(0, y, SCREEN_WIDTH, 5, color);
    }
    delay(20);
  }
}

// HSV zu RGB565 Konvertierung
uint16_t hsvToRgb565(float h, float s, float v) {
  float r, g, b;

  int i = int(h * 6);
  float f = h * 6 - i;
  float p = v * (1 - s);
  float q = v * (1 - f * s);
  float t = v * (1 - (1 - f) * s);

  switch(i % 6) {
    case 0: r = v, g = t, b = p; break;
    case 1: r = q, g = v, b = p; break;
    case 2: r = p, g = v, b = t; break;
    case 3: r = p, g = q, b = v; break;
    case 4: r = t, g = p, b = v; break;
    case 5: r = v, g = p, b = q; break;
  }

  return lcd.color565(r * 255, g * 255, b * 255);
}
