/*
  Touch Calibration Test

  Hilft beim Finden der richtigen Touch-Transformation für Portrait-Modus.

  Zeigt 4 Kreise in den Ecken des Displays:
  - Oben Links (rot)
  - Oben Rechts (grün)
  - Unten Links (blau)
  - Unten Rechts (gelb)

  Wenn du einen Kreis berührst, zeigt es die RAW-Touch-Koordinaten an.
*/

#include <CYD_Display_Config.h>

LGFX lcd;

int SCREEN_WIDTH;
int SCREEN_HEIGHT;

void setup() {
  Serial.begin(115200);
  Serial.println("\n\nTouch Calibration Test");

  lcd.init();
  lcd.setRotation(1);  // Portrait-Modus Jetz Landscape fuer Test

  SCREEN_WIDTH = lcd.width();
  SCREEN_HEIGHT = lcd.height();

  Serial.printf("Display: %dx%d (Portrait)\n", SCREEN_WIDTH, SCREEN_HEIGHT);
  Serial.println("\nBerühre die farbigen Kreise:");
  Serial.println("- ROT = Oben Links");
  Serial.println("- GRÜN = Oben Rechts");
  Serial.println("- BLAU = Unten Links");
  Serial.println("- GELB = Unten Rechts");

  drawCalibrationScreen();
}

void loop() {
  uint16_t x, y;

  if (lcd.getTouch(&x, &y)) {
    // RAW-Koordinaten ausgeben
    Serial.printf("RAW Touch: x=%3d, y=%3d", x, y);

    // Versuche zu erkennen, welche Ecke berührt wurde
    // (basierend auf RAW-Koordinaten)
    String corner = detectCorner(x, y);

    Serial.printf(" -> %s\n", corner.c_str());

    // Visuelles Feedback
    lcd.fillCircle(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 30, TFT_WHITE);
    delay(100);
    drawCalibrationScreen();

    delay(300);  // Debounce
  }
}

void drawCalibrationScreen() {
  lcd.fillScreen(TFT_BLACK);

  int radius = 30;
  int offset = 40;

  // Oben Links - ROT
  lcd.fillCircle(offset, offset, radius, TFT_RED);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(offset - 10, offset - 5);
  lcd.print("TL");

  // Oben Rechts - GRÜN
  lcd.fillCircle(SCREEN_WIDTH - offset, offset, radius, TFT_GREEN);
  lcd.setCursor(SCREEN_WIDTH - offset - 15, offset - 5);
  lcd.print("TR");

  // Unten Links - BLAU
  lcd.fillCircle(offset, SCREEN_HEIGHT - offset, radius, TFT_BLUE);
  lcd.setCursor(offset - 10, SCREEN_HEIGHT - offset - 5);
  lcd.print("BL");

  // Unten Rechts - GELB
  lcd.fillCircle(SCREEN_WIDTH - offset, SCREEN_HEIGHT - offset, radius, TFT_YELLOW);
  lcd.setCursor(SCREEN_WIDTH - offset - 15, SCREEN_HEIGHT - offset - 5);
  lcd.print("BR");

  // Info in der Mitte
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_WHITE);
  lcd.setCursor(10, SCREEN_HEIGHT/2 - 20);
  lcd.printf("Display: %dx%d", SCREEN_WIDTH, SCREEN_HEIGHT);
  lcd.setCursor(10, SCREEN_HEIGHT/2);
  lcd.print("Touch Calibration");
  lcd.setCursor(10, SCREEN_HEIGHT/2 + 20);
  lcd.print("Siehe Serial Monitor");
}

String detectCorner(uint16_t x, uint16_t y) {
  // Diese Funktion versucht zu erraten, welche Ecke berührt wurde
  // basierend auf verschiedenen möglichen Transformationen

  String result = "Unknown";

  // Test 1: Keine Transformation (falls Touch schon korrekt ist)
  if (x < SCREEN_WIDTH/2 && y < SCREEN_HEIGHT/2) {
    result += " [NoTrans: TL]";
  } else if (x >= SCREEN_WIDTH/2 && y < SCREEN_HEIGHT/2) {
    result += " [NoTrans: TR]";
  } else if (x < SCREEN_WIDTH/2 && y >= SCREEN_HEIGHT/2) {
    result += " [NoTrans: BL]";
  } else {
    result += " [NoTrans: BR]";
  }

  // Test 2: Swap X/Y
  uint16_t tx = y;
  uint16_t ty = x;
  if (tx < SCREEN_WIDTH/2 && ty < SCREEN_HEIGHT/2) {
    result += " [Swap: TL]";
  } else if (tx >= SCREEN_WIDTH/2 && ty < SCREEN_HEIGHT/2) {
    result += " [Swap: TR]";
  } else if (tx < SCREEN_WIDTH/2 && ty >= SCREEN_HEIGHT/2) {
    result += " [Swap: BL]";
  } else {
    result += " [Swap: BR]";
  }

  // Test 3: Swap + Invert Y
  tx = y;
  ty = (SCREEN_HEIGHT - 1) - x;
  if (tx < SCREEN_WIDTH/2 && ty < SCREEN_HEIGHT/2) {
    result += " [Swap+InvY: TL]";
  } else if (tx >= SCREEN_WIDTH/2 && ty < SCREEN_HEIGHT/2) {
    result += " [Swap+InvY: TR]";
  } else if (tx < SCREEN_WIDTH/2 && ty >= SCREEN_HEIGHT/2) {
    result += " [Swap+InvY: BL]";
  } else {
    result += " [Swap+InvY: BR]";
  }

  // Test 4: Swap + Invert X
  tx = (SCREEN_WIDTH - 1) - y;
  ty = x;
  if (tx < SCREEN_WIDTH/2 && ty < SCREEN_HEIGHT/2) {
    result += " [Swap+InvX: TL]";
  } else if (tx >= SCREEN_WIDTH/2 && ty < SCREEN_HEIGHT/2) {
    result += " [Swap+InvX: TR]";
  } else if (tx < SCREEN_WIDTH/2 && ty >= SCREEN_HEIGHT/2) {
    result += " [Swap+InvX: BL]";
  } else {
    result += " [Swap+InvX: BR]";
  }

  return result;
}
