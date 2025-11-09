/*
 * CYD Academy - Hello World Beispiel
 *
 * Dieses Beispiel zeigt einen einfachen "Hello World" Text auf dem Display.
 * Es demonstriert die Grundlagen der Textausgabe mit LovyanGFX.
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
  // Serielle Kommunikation starten
  Serial.begin(115200);
  Serial.println("CYD Hello World gestartet!");

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

  // Display-Helligkeit setzen (0-255)
  lcd.setBrightness(200);

  // Display-Ausrichtung setzen (0-3)
  lcd.setRotation(1);

  // Hintergrund löschen (schwarz)
  lcd.fillScreen(TFT_BLACK);

  // Textgröße setzen (relativ zur Bildschirmgröße)
  lcd.setTextSize(3);

  // Textfarbe setzen (Vordergrund, Hintergrund)
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);

  // Text zentriert ausgeben (angepasst an Bildschirmbreite)
  lcd.setCursor(SCREEN_WIDTH / 6, SCREEN_HEIGHT / 5);
  lcd.println("Hello World!");

  // Zweite Zeile mit anderer Farbe
  lcd.setTextColor(TFT_CYAN, TFT_BLACK);
  lcd.setCursor(SCREEN_WIDTH / 6, SCREEN_HEIGHT / 5 + 40);
  lcd.println("CYD Display");

  // Dritte Zeile
  lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  lcd.setCursor(SCREEN_WIDTH / 6, SCREEN_HEIGHT / 5 + 80);
  lcd.println("LovyanGFX");

  // Info-Text kleiner
  lcd.setTextSize(2);
  lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  lcd.setCursor(SCREEN_WIDTH / 15, SCREEN_HEIGHT - 40);
  lcd.println("ESP32 bereit!");

  // Display-Info anzeigen
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
  lcd.setCursor(5, SCREEN_HEIGHT - 10);
  lcd.print(SCREEN_WIDTH);
  lcd.print("x");
  lcd.print(SCREEN_HEIGHT);
}

void loop() {
  // Nichts zu tun in der Loop
  delay(1000);
}
