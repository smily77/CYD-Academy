/*
  Beispiel 08: Analoge Uhr mit WiFi/NTP

  Zeigt eine analoge Uhr mit Stunden-, Minuten- und Sekundenzeiger.
  - Automatische Zeit-Synchronisation über WiFi/NTP
  - Timezone: Zürich (CET/CEST mit automatischer Sommer-/Winterzeit)
  - Optional: Helligkeitssteuerung über Photoresistor
  - Dynamisch skalierbar für verschiedene Display-Größen

  Erstelle eine Datei "Credentials.h" mit:
    const char* ssid = "DeinWiFiName";
    const char* password = "DeinWiFiPasswort";
*/

#include <CYD_Display_Config.h>
#include <WiFi.h>
#include <time.h>
#include "Credentials.h"  // WiFi-Zugangsdaten

// WiFi und NTP Server
const char* ntpServer = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const char* timezone = "CET-1CEST,M3.5.0,M10.5.0/3";  // Zürich

// Display-Variablen (werden dynamisch gesetzt)
int SCREEN_WIDTH;
int SCREEN_HEIGHT;

// Uhr-Geometrie (wird dynamisch berechnet)
int centerX;
int centerY;
int clockRadius;
int hourHandLength;
int minuteHandLength;
int secondHandLength;

// Status
bool timeInitialized = false;
int lastSecond = -1;
int lastMinute = -1;
int lastHour = -1;

// Helligkeitssteuerung
#ifdef photoR_PIN
int lastBrightness = -1;
const int BRIGHTNESS_SAMPLES = 10;  // Anzahl Messungen für Durchschnitt
#endif

LGFX lcd;

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.setRotation(1);  // Landscape

  // Display-Größe ermitteln
  SCREEN_WIDTH = lcd.width();
  SCREEN_HEIGHT = lcd.height();

  // Uhr-Geometrie berechnen
  centerX = SCREEN_WIDTH / 2;
  centerY = SCREEN_HEIGHT / 2;

  // Radius basierend auf kleinerer Dimension
  int minDimension = (SCREEN_WIDTH < SCREEN_HEIGHT) ? SCREEN_WIDTH : SCREEN_HEIGHT;
  clockRadius = (minDimension / 2) - 20;  // 20px Rand

  // Zeigerlängen
  hourHandLength = clockRadius * 0.5;
  minuteHandLength = clockRadius * 0.7;
  secondHandLength = clockRadius * 0.85;

  // Optional: RGB LED initialisieren
  #ifdef LED_PIN_R
    pinMode(LED_PIN_R, OUTPUT);
    pinMode(LED_PIN_G, OUTPUT);
    pinMode(LED_PIN_B, OUTPUT);
    digitalWrite(LED_PIN_R, HIGH);  // OFF
    digitalWrite(LED_PIN_G, HIGH);  // OFF
    digitalWrite(LED_PIN_B, HIGH);  // OFF
  #endif

  // Optional: Photoresistor initialisieren
  #ifdef photoR_PIN
    // pinMode nicht nötig - ADC Pins sind standardmäßig INPUT
  #endif

  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextDatum(MC_DATUM);

  // WiFi verbinden
  lcd.drawString("Verbinde mit WiFi...", centerX, centerY);
  Serial.println("Verbinde mit WiFi...");

  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi verbunden!");
    Serial.print("IP Adresse: ");
    Serial.println(WiFi.localIP());

    lcd.fillScreen(TFT_BLACK);
    lcd.drawString("WiFi verbunden!", centerX, centerY);
    lcd.drawString("Synchronisiere Zeit...", centerX, centerY + 20);

    // Zeit konfigurieren (mit Timezone-String für automatische Sommer-/Winterzeit)
    configTzTime(timezone, ntpServer, ntpServer2);

    // Warte auf Zeit-Synchronisation
    struct tm timeinfo;
    attempts = 0;
    while (!getLocalTime(&timeinfo) && attempts < 10) {
      Serial.println("Warte auf Zeit-Synchronisation...");
      delay(1000);
      attempts++;
    }

    if (attempts < 10) {
      timeInitialized = true;
      Serial.println("Zeit synchronisiert!");

      #ifdef LED_PIN_G
        digitalWrite(LED_PIN_G, LOW);  // Grün = Erfolg
        delay(1000);
        digitalWrite(LED_PIN_G, HIGH); // OFF
      #endif
    } else {
      Serial.println("Zeit-Synchronisation fehlgeschlagen!");
      lcd.fillScreen(TFT_BLACK);
      lcd.setTextColor(TFT_RED);
      lcd.drawString("Zeit-Sync fehlgeschlagen!", centerX, centerY);

      #ifdef LED_PIN_R
        digitalWrite(LED_PIN_R, LOW);  // Rot = Fehler
        delay(2000);
        digitalWrite(LED_PIN_R, HIGH); // OFF
      #endif
    }
  } else {
    Serial.println("\nWiFi Verbindung fehlgeschlagen!");
    lcd.fillScreen(TFT_BLACK);
    lcd.setTextColor(TFT_RED);
    lcd.drawString("WiFi Verbindung fehlgeschlagen!", centerX, centerY);

    #ifdef LED_PIN_R
      digitalWrite(LED_PIN_R, LOW);  // Rot = Fehler
      delay(2000);
      digitalWrite(LED_PIN_R, HIGH); // OFF
    #endif
  }

  delay(1000);
  lcd.fillScreen(TFT_BLACK);

  // Zifferblatt zeichnen (bleibt statisch)
  drawClockFace();
}

void loop() {
  if (!timeInitialized) {
    delay(1000);
    return;
  }

  // Optional: Helligkeit über Photoresistor steuern
  #ifdef photoR_PIN
    adjustBrightness();
  #endif

  // Aktuelle Zeit holen
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Fehler beim Abrufen der Zeit");
    delay(1000);
    return;
  }

  // Nur bei Sekundenwechsel aktualisieren
  if (timeinfo.tm_sec != lastSecond) {
    // Alte Zeiger löschen (mit gespeicherten alten Werten!)
    if (lastSecond != -1) {  // Nur wenn wir schon vorherige Werte haben
      drawClockHands(lastHour, lastMinute, lastSecond, TFT_BLACK);
    }

    // Neue Zeiger zeichnen
    drawClockHands(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, TFT_WHITE);

    // Zentrale Achse neu zeichnen
    lcd.fillCircle(centerX, centerY, 5, TFT_RED);

    // Aktuelle Werte speichern für nächstes Mal
    lastSecond = timeinfo.tm_sec;
    lastMinute = timeinfo.tm_min;
    lastHour = timeinfo.tm_hour;
  }

  delay(100);  // Kurze Pause
}

// Zeichnet das statische Zifferblatt
void drawClockFace() {
  // Äußerer Kreis
  lcd.drawCircle(centerX, centerY, clockRadius, TFT_WHITE);
  lcd.drawCircle(centerX, centerY, clockRadius - 1, TFT_WHITE);

  // Stundenmarkierungen
  for (int i = 0; i < 12; i++) {
    float angle = (i * 30 - 90) * PI / 180.0;  // -90° weil 12 Uhr oben ist

    // Äußerer Punkt
    int x1 = centerX + (clockRadius - 10) * cos(angle);
    int y1 = centerY + (clockRadius - 10) * sin(angle);

    // Innerer Punkt
    int x2 = centerX + (clockRadius - 20) * cos(angle);
    int y2 = centerY + (clockRadius - 20) * sin(angle);

    // Dicke Linie für Stundenmarkierung
    lcd.drawLine(x1, y1, x2, y2, TFT_WHITE);
    lcd.drawLine(x1 + 1, y1, x2 + 1, y2, TFT_WHITE);
    lcd.drawLine(x1, y1 + 1, x2, y2 + 1, TFT_WHITE);
  }

  // 5-Minuten-Markierungen
  for (int i = 0; i < 60; i++) {
    if (i % 5 == 0) continue;  // Überspringen der Stundenmarkierungen

    float angle = (i * 6 - 90) * PI / 180.0;

    int x1 = centerX + (clockRadius - 5) * cos(angle);
    int y1 = centerY + (clockRadius - 5) * sin(angle);

    int x2 = centerX + (clockRadius - 10) * cos(angle);
    int y2 = centerY + (clockRadius - 10) * sin(angle);

    lcd.drawLine(x1, y1, x2, y2, TFT_DARKGREY);
  }

  // Zentrale Achse
  lcd.fillCircle(centerX, centerY, 5, TFT_RED);
}

// Zeichnet die Uhrzeiger
void drawClockHands(int hour, int minute, int second, uint16_t color) {
  // Sekundenzeiger (dünn, rot oder schwarz zum Löschen)
  float secondAngle = (second * 6 - 90) * PI / 180.0;
  int sx = centerX + secondHandLength * cos(secondAngle);
  int sy = centerY + secondHandLength * sin(secondAngle);
  lcd.drawLine(centerX, centerY, sx, sy, (color == TFT_BLACK) ? TFT_BLACK : TFT_RED);

  // Minutenzeiger (mittel)
  float minuteAngle = ((minute + second / 60.0) * 6 - 90) * PI / 180.0;
  int mx = centerX + minuteHandLength * cos(minuteAngle);
  int my = centerY + minuteHandLength * sin(minuteAngle);
  lcd.drawLine(centerX, centerY, mx, my, color);
  lcd.drawLine(centerX + 1, centerY, mx + 1, my, color);
  lcd.drawLine(centerX, centerY + 1, mx, my + 1, color);

  // Stundenzeiger (dick)
  float hourAngle = ((hour % 12 + minute / 60.0) * 30 - 90) * PI / 180.0;
  int hx = centerX + hourHandLength * cos(hourAngle);
  int hy = centerY + hourHandLength * sin(hourAngle);
  lcd.drawLine(centerX, centerY, hx, hy, color);
  lcd.drawLine(centerX + 1, centerY, hx + 1, hy, color);
  lcd.drawLine(centerX, centerY + 1, hx, hy + 1, color);
  lcd.drawLine(centerX + 1, centerY + 1, hx + 1, hy + 1, color);
  lcd.drawLine(centerX + 2, centerY, hx + 2, hy, color);
  lcd.drawLine(centerX, centerY + 2, hx, hy + 2, color);
}

#ifdef photoR_PIN
// Passt die Display-Helligkeit basierend auf dem Photoresistor an
void adjustBrightness() {
  static unsigned long lastCheck = 0;
  unsigned long now = millis();

  // Alle 200ms prüfen
  if (now - lastCheck < 200) return;
  lastCheck = now;

  // RAW Wert direkt lesen
  int rawValue = analogRead(photoR_PIN);

  // Umrechnen: 0-4095 (12-bit ADC) -> 10-255 (Helligkeit)
  // RAW niedrig (viel Licht) → Display hell (255)
  // RAW hoch (wenig Licht) → Display sehr dunkel (10)
  int brightness = map(rawValue, 0, 4095, 255, 10);
  brightness = constrain(brightness, 10, 255);

  // Ändern wenn Differenz > 5
  if (abs(brightness - lastBrightness) > 5 || lastBrightness == -1) {
    lcd.setBrightness(brightness);
    lastBrightness = brightness;

    Serial.print("Helligkeit: ");
    Serial.print(brightness);
    Serial.print(" (ADC: ");
    Serial.print(rawValue);
    Serial.println(")");
  }
}
#endif
