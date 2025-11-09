/*
 * CYD Academy - Touch Demo Beispiel
 *
 * Dieses Beispiel demonstriert die Verwendung des Touch-Screens:
 * - Touch-Erkennung
 * - Koordinaten anzeigen
 * - Touch-Visualisierung
 * - Einfaches Zeichenprogramm
 * - Touch-Button
 *
 * Hardware: CYD 2.8" oder 3.5" ESP32 Display
 * Library: LovyanGFX
 */

#include <CYD_Display_Config.h>

// Display-Objekt erstellen
static LGFX lcd;

// Display-Auflösung (wird dynamisch ermittelt)
int SCREEN_WIDTH;
int SCREEN_HEIGHT;

// Variablen für Touch
int lastX = -1;
int lastY = -1;
bool isTouched = false;

// Farben für Zeichnen
uint16_t currentColor = TFT_WHITE;
int colorIndex = 0;
uint16_t colors[] = {TFT_WHITE, TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_CYAN, TFT_MAGENTA, TFT_ORANGE};
int numColors = 8;

// Button-Bereiche (werden dynamisch berechnet)
int clearButtonX;
int clearButtonY;
int clearButtonW;
int clearButtonH;

int colorButtonX;
int colorButtonY;
int colorButtonW;
int colorButtonH;

int drawAreaY;  // Ab welcher Y-Koordinate beginnt der Zeichenbereich

void setup() {
  Serial.begin(115200);
  Serial.println("CYD Touch Demo gestartet!");

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

  // Button-Größen basierend auf Bildschirmgröße berechnen
  clearButtonW = SCREEN_WIDTH / 4;
  clearButtonH = SCREEN_HEIGHT / 6;
  clearButtonX = 10;
  clearButtonY = 10;

  colorButtonW = SCREEN_WIDTH / 4;
  colorButtonH = SCREEN_HEIGHT / 6;
  colorButtonX = clearButtonX + clearButtonW + 10;
  colorButtonY = 10;

  drawAreaY = clearButtonH + 20;

  // Startbildschirm
  drawUI();
}

void drawUI() {
  lcd.fillScreen(TFT_BLACK);

  // Clear Button
  lcd.fillRoundRect(clearButtonX, clearButtonY, clearButtonW, clearButtonH, 5, TFT_RED);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  int textX = clearButtonX + (clearButtonW - lcd.textWidth("CLEAR")) / 2;
  lcd.setCursor(textX, clearButtonY + clearButtonH / 2 - 8);
  lcd.println("CLEAR");

  // Color Button
  lcd.fillRoundRect(colorButtonX, colorButtonY, colorButtonW, colorButtonH, 5, currentColor);
  lcd.setTextColor(TFT_BLACK);
  textX = colorButtonX + (colorButtonW - lcd.textWidth("COLOR")) / 2;
  lcd.setCursor(textX, colorButtonY + colorButtonH / 2 - 8);
  lcd.println("COLOR");

  // Anleitung
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(1);
  lcd.setCursor(SCREEN_WIDTH - 100, 15);
  lcd.println("Zeichne auf");
  lcd.setCursor(SCREEN_WIDTH - 100, 28);
  lcd.println("dem Display!");

  // Zeichenbereich-Linie
  lcd.drawLine(0, drawAreaY, SCREEN_WIDTH, drawAreaY, TFT_DARKGREY);
}

void loop() {
  // Touch-Status abfragen
  uint16_t x, y;
  bool touched = lcd.getTouch(&x, &y);

  if (touched) {
    // Touch erkannt
    Serial.print("Touch detected at X: ");
    Serial.print(x);
    Serial.print(" Y: ");
    Serial.println(y);

    // Prüfen ob Clear-Button gedrückt wurde
    if (x >= clearButtonX && x <= clearButtonX + clearButtonW &&
        y >= clearButtonY && y <= clearButtonY + clearButtonH) {
      Serial.println("Clear button pressed!");
      drawUI();
      lastX = -1;
      lastY = -1;
      delay(200);  // Debounce
      return;
    }

    // Prüfen ob Color-Button gedrückt wurde
    if (x >= colorButtonX && x <= colorButtonX + colorButtonW &&
        y >= colorButtonY && y <= colorButtonY + colorButtonH) {
      Serial.println("Color button pressed!");
      colorIndex = (colorIndex + 1) % numColors;
      currentColor = colors[colorIndex];

      // Button neu zeichnen
      lcd.fillRoundRect(colorButtonX, colorButtonY, colorButtonW, colorButtonH, 5, currentColor);
      lcd.setTextColor(TFT_BLACK);
      lcd.setTextSize(2);
      int textX = colorButtonX + (colorButtonW - lcd.textWidth("COLOR")) / 2;
      lcd.setCursor(textX, colorButtonY + colorButtonH / 2 - 8);
      lcd.println("COLOR");

      lastX = -1;
      lastY = -1;
      delay(200);  // Debounce
      return;
    }

    // Im Zeichenbereich (unterhalb der Buttons)
    if (y > drawAreaY) {
      // Wenn vorherige Position existiert, Linie zeichnen
      if (lastX >= 0 && lastY >= 0) {
        lcd.drawLine(lastX, lastY, x, y, currentColor);
      }

      // Punkt an aktueller Position zeichnen
      lcd.fillCircle(x, y, 3, currentColor);

      // Aktuelle Position speichern
      lastX = x;
      lastY = y;
      isTouched = true;
    }
  } else {
    // Kein Touch mehr
    if (isTouched) {
      // Touch wurde losgelassen
      lastX = -1;
      lastY = -1;
      isTouched = false;
      Serial.println("Touch released");
    }
  }

  delay(10);  // Kurze Verzögerung für Stabilität
}
