/*
 * CYD Academy - Scroll Text Beispiel
 *
 * Einfaches, funktionierendes Beispiel für scrollenden Text.
 * Wechselt zwischen verschiedenen Demo-Modi alle 10 Sekunden.
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

// Welcher Demo-Modus läuft gerade?
int currentDemo = 0;
unsigned long lastDemoChange = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("CYD Scroll Text Demo gestartet!");

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
  // Alle 10 Sekunden Demo wechseln
  if (millis() - lastDemoChange > 10000) {
    currentDemo = (currentDemo + 1) % 3;
    lastDemoChange = millis();
    lcd.fillScreen(TFT_BLACK);
    Serial.print("Wechsel zu Demo: ");
    Serial.println(currentDemo);
  }

  // Entsprechende Demo ausführen
  switch(currentDemo) {
    case 0:
      demo1_SimpleScroll();
      break;
    case 1:
      demo2_VerticalScroll();
      break;
    case 2:
      demo3_TickerScroll();
      break;
  }
}

// ===== DEMO 1: Einfacher horizontaler Scroll =====
void demo1_SimpleScroll() {
  static int x = -1;  // -1 bedeutet: noch nicht initialisiert
  static unsigned long lastUpdate = 0;

  // Initialisierung beim ersten Aufruf
  if (x == -1) {
    x = SCREEN_WIDTH;
  }

  if (millis() - lastUpdate < 40) {
    return;  // Noch nicht Zeit für Update
  }
  lastUpdate = millis();

  // Bildschirm löschen
  lcd.fillScreen(TFT_BLACK);

  // Titel
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_WHITE);
  lcd.setCursor(5, 5);
  lcd.print("Demo 1: Horizontal Scroll");

  // Scrollender Text
  lcd.setTextSize(3);
  lcd.setTextColor(TFT_YELLOW);
  lcd.setCursor(x, SCREEN_HEIGHT / 3);
  lcd.print("CYD Display!");

  // Position aktualisieren
  x -= 4;
  if (x < -200) {  // Text ist komplett durch
    x = SCREEN_WIDTH;
  }
}

// ===== DEMO 2: Vertikaler Scroll (Nachrichten) =====
void demo2_VerticalScroll() {
  static int messageIndex = 0;
  static unsigned long lastChange = 0;
  static bool needsRedraw = true;

  if (millis() - lastChange > 2000) {
    messageIndex = (messageIndex + 1) % 5;
    lastChange = millis();
    needsRedraw = true;
  }

  if (!needsRedraw) {
    return;
  }
  needsRedraw = false;

  // Bildschirm löschen
  lcd.fillScreen(TFT_NAVY);

  // Titel
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_WHITE);
  lcd.setCursor(5, 5);
  lcd.print("Demo 2: Nachrichten");

  // Nachrichten-Array
  String messages[] = {
    "Willkommen!",
    "CYD Display",
    "LovyanGFX",
    "Arduino IDE",
    "Viel Spass!"
  };

  // Aktuelle Nachricht anzeigen
  lcd.setTextSize(3);
  lcd.setTextColor(TFT_CYAN);
  int textWidth = lcd.textWidth(messages[messageIndex]);
  int x = (SCREEN_WIDTH - textWidth) / 2;
  lcd.setCursor(x, SCREEN_HEIGHT / 3);
  lcd.print(messages[messageIndex]);

  // Zähler
  lcd.setTextSize(2);
  lcd.setTextColor(TFT_GREEN);
  lcd.setCursor(SCREEN_WIDTH / 2 - 20, SCREEN_HEIGHT * 3 / 4);
  lcd.print(messageIndex + 1);
  lcd.print("/5");
}

// ===== DEMO 3: Ticker-Style =====
void demo3_TickerScroll() {
  static int x = -1;  // -1 bedeutet: noch nicht initialisiert
  static unsigned long lastUpdate = 0;

  // Initialisierung beim ersten Aufruf
  if (x == -1) {
    x = SCREEN_WIDTH;
  }

  if (millis() - lastUpdate < 30) {
    return;
  }
  lastUpdate = millis();

  // Hintergrund (nur beim ersten Mal)
  static bool initialized = false;
  if (!initialized || x >= SCREEN_WIDTH - 1) {
    lcd.fillScreen(TFT_DARKGREEN);

    // LIVE Badge
    int badgeW = SCREEN_WIDTH / 6;
    int badgeH = SCREEN_HEIGHT / 6;
    int badgeX = 5;
    int badgeY = SCREEN_HEIGHT / 3;

    lcd.fillRect(badgeX, badgeY, badgeW, badgeH, TFT_RED);
    lcd.setTextSize(2);
    lcd.setTextColor(TFT_WHITE);
    lcd.setCursor(badgeX + 10, badgeY + badgeH / 2 - 8);
    lcd.print("LIVE");

    initialized = true;
  }

  // Ticker-Bereich löschen
  int tickerX = SCREEN_WIDTH / 6 + 10;
  int tickerY = SCREEN_HEIGHT / 3;
  int tickerW = SCREEN_WIDTH - tickerX;
  int tickerH = SCREEN_HEIGHT / 6;

  lcd.fillRect(tickerX, tickerY, tickerW, tickerH, TFT_DARKGREEN);

  // Ticker-Text
  lcd.setTextSize(2);
  lcd.setTextColor(TFT_WHITE);

  if (x > tickerX) {
    lcd.setCursor(x, tickerY + tickerH / 2 - 8);
    lcd.print("CYD funktioniert!");
  } else if (x > -200) {
    lcd.setCursor(tickerX, tickerY + tickerH / 2 - 8);
    String text = "CYD funktioniert!";
    int startChar = (tickerX - x) / 12;  // Welches Zeichen ist das erste sichtbare?
    if (startChar < text.length()) {
      lcd.print(text.substring(startChar));
    }
  }

  // Position aktualisieren
  x -= 3;
  if (x < -200) {
    x = SCREEN_WIDTH;
  }

  // Titel unten
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_YELLOW);
  lcd.setCursor(5, SCREEN_HEIGHT - 10);
  lcd.print("Demo 3: Ticker-Style");
}
