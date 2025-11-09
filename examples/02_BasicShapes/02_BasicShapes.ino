/*
 * CYD Academy - Basic Shapes Beispiel
 *
 * Dieses Beispiel demonstriert das Zeichnen verschiedener Grundformen:
 * - Rechtecke (gefüllt und ungefüllt)
 * - Kreise (gefüllt und ungefüllt)
 * - Linien
 * - Dreiecke
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

void setup() {
  Serial.begin(115200);
  Serial.println("CYD Basic Shapes gestartet!");

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
  lcd.fillScreen(TFT_BLACK);
}

void loop() {
  // Berechnete Größen für verschiedene Displays
  int boxW = SCREEN_WIDTH / 5;
  int boxH = SCREEN_HEIGHT / 4;
  int spacing = SCREEN_WIDTH / 20;

  // 1. Rechtecke zeichnen
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.setCursor(10, 10);
  lcd.println("Rechtecke");

  // Gefülltes Rechteck
  lcd.fillRect(spacing, boxH, boxW, boxH, TFT_RED);

  // Ungefülltes Rechteck (Rahmen)
  lcd.drawRect(spacing * 2 + boxW, boxH, boxW, boxH, TFT_GREEN);

  // Rechteck mit abgerundeten Ecken
  lcd.fillRoundRect(spacing * 3 + boxW * 2, boxH, boxW, boxH, 10, TFT_BLUE);

  delay(3000);

  // 2. Kreise zeichnen
  lcd.fillScreen(TFT_BLACK);
  lcd.setCursor(10, 10);
  lcd.println("Kreise");

  int radius = SCREEN_WIDTH / 10;
  int centerY = SCREEN_HEIGHT / 3;

  // Gefüllter Kreis (x, y, radius, farbe)
  lcd.fillCircle(SCREEN_WIDTH / 6, centerY, radius, TFT_YELLOW);

  // Ungefüllter Kreis
  lcd.drawCircle(SCREEN_WIDTH / 2, centerY, radius, TFT_CYAN);

  // Mehrere konzentrische Kreise
  int maxRadius = radius;
  for (int r = radius / 4; r <= maxRadius; r += radius / 4) {
    lcd.drawCircle(5 * SCREEN_WIDTH / 6, centerY, r, TFT_MAGENTA);
  }

  delay(3000);

  // 3. Linien zeichnen
  lcd.fillScreen(TFT_BLACK);
  lcd.setCursor(10, 10);
  lcd.println("Linien");

  int lineY = SCREEN_HEIGHT / 5;

  // Horizontale Linien
  lcd.drawLine(spacing, lineY, SCREEN_WIDTH - spacing, lineY, TFT_RED);
  lcd.drawLine(spacing, lineY + 20, SCREEN_WIDTH - spacing, lineY + 20, TFT_GREEN);
  lcd.drawLine(spacing, lineY + 40, SCREEN_WIDTH - spacing, lineY + 40, TFT_BLUE);

  // Diagonale Linien (Kreuz)
  int crossSize = SCREEN_WIDTH / 4;
  int crossX = spacing;
  int crossY = lineY + 70;
  lcd.drawLine(crossX, crossY, crossX + crossSize, crossY + crossSize, TFT_YELLOW);
  lcd.drawLine(crossX + crossSize, crossY, crossX, crossY + crossSize, TFT_CYAN);

  // Strahlenmuster
  int starX = 3 * SCREEN_WIDTH / 4;
  int starY = SCREEN_HEIGHT / 2;
  int starRadius = SCREEN_WIDTH / 8;
  for (int i = 0; i < 8; i++) {
    float angle = i * 3.14159 / 4;
    int x = starX + cos(angle) * starRadius;
    int y = starY + sin(angle) * starRadius;
    lcd.drawLine(starX, starY, x, y, TFT_MAGENTA);
  }

  delay(3000);

  // 4. Dreiecke zeichnen
  lcd.fillScreen(TFT_BLACK);
  lcd.setCursor(10, 10);
  lcd.println("Dreiecke");

  int triSize = SCREEN_WIDTH / 5;
  int triY = SCREEN_HEIGHT / 4;

  // Gefülltes Dreieck (x1, y1, x2, y2, x3, y3, farbe)
  lcd.fillTriangle(
    SCREEN_WIDTH / 6, triY,
    SCREEN_WIDTH / 6 - triSize / 2, triY + triSize,
    SCREEN_WIDTH / 6 + triSize / 2, triY + triSize,
    TFT_ORANGE
  );

  // Ungefülltes Dreieck
  lcd.drawTriangle(
    SCREEN_WIDTH / 2, triY,
    SCREEN_WIDTH / 2 - triSize / 2, triY + triSize,
    SCREEN_WIDTH / 2 + triSize / 2, triY + triSize,
    TFT_GREENYELLOW
  );

  // Mehrere Dreiecke (Pyramide)
  int pyrX = 5 * SCREEN_WIDTH / 6;
  int pyrY = triY;
  int pyrSize1 = triSize / 3;
  int pyrSize2 = triSize / 2;

  lcd.fillTriangle(pyrX, pyrY, pyrX - pyrSize1, pyrY + pyrSize1, pyrX + pyrSize1, pyrY + pyrSize1, TFT_RED);
  lcd.fillTriangle(pyrX, pyrY + pyrSize1, pyrX - pyrSize2, pyrY + pyrSize1 + pyrSize2, pyrX + pyrSize2, pyrY + pyrSize1 + pyrSize2, TFT_YELLOW);

  delay(3000);

  // 5. Kombinierte Formen - Einfaches Haus
  lcd.fillScreen(TFT_BLACK);
  lcd.setCursor(10, 10);
  lcd.println("Kombiniert");

  int houseW = SCREEN_WIDTH / 3;
  int houseH = SCREEN_HEIGHT / 3;
  int houseX = (SCREEN_WIDTH - houseW) / 2;
  int houseY = SCREEN_HEIGHT / 3;

  // Haus
  lcd.fillRect(houseX, houseY, houseW, houseH, TFT_BROWN);

  // Dach
  lcd.fillTriangle(
    houseX + houseW / 2, houseY - houseH / 3,
    houseX - houseW / 10, houseY,
    houseX + houseW + houseW / 10, houseY,
    TFT_RED
  );

  // Tür
  int doorW = houseW / 4;
  int doorH = houseH / 2;
  lcd.fillRect(houseX + doorW, houseY + houseH - doorH, doorW, doorH, TFT_BLUE);

  // Fenster
  int windowSize = houseW / 5;
  lcd.fillRect(houseX + houseW - windowSize - windowSize / 2, houseY + houseH / 4, windowSize, windowSize, TFT_CYAN);

  delay(3000);

  // 6. Animierter Balken (Ladebalken)
  lcd.fillScreen(TFT_BLACK);
  lcd.setCursor(10, 10);
  lcd.println("Ladebalken");

  int barWidth = SCREEN_WIDTH - 40;
  int barHeight = SCREEN_HEIGHT / 8;
  int barX = 20;
  int barY = SCREEN_HEIGHT / 2 - barHeight / 2;

  // Rahmen zeichnen
  lcd.drawRect(barX - 2, barY - 2, barWidth + 4, barHeight + 4, TFT_WHITE);

  // Balken füllen
  for (int i = 0; i <= barWidth; i += 5) {
    lcd.fillRect(barX, barY, i, barHeight, TFT_GREEN);
    delay(50);
  }

  delay(2000);
}
