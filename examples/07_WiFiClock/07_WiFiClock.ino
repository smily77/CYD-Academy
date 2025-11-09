/*
 * CYD Academy - WiFi Uhr mit NTP
 *
 * Zeigt eine digitale Uhr auf dem Display.
 * Holt die aktuelle Zeit vom Internet über NTP (Network Time Protocol).
 *
 * WICHTIG: Erstelle eine Datei "Credentials.h" im gleichen Verzeichnis mit:
 *   const char* ssid = "DeinWiFiName";
 *   const char* password = "DeinWiFiPasswort";
 *
 * Hardware: CYD 2.8" oder 3.5" ESP32 Display
 * Library: LovyanGFX
 */

#include <LovyanGFX.hpp>
#include <CYD_Display_Config.h>
#include <WiFi.h>
#include <time.h>
#include "Credentials.h"  // WiFi-Zugangsdaten

// Display-Objekt erstellen
static LGFX lcd;

// Display-Auflösung (wird dynamisch ermittelt)
int SCREEN_WIDTH;
int SCREEN_HEIGHT;

// NTP Server Einstellungen für Schweiz
const char* ntpServer = "ch.pool.ntp.org";  // Schweizer NTP-Server
const char* ntpServer2 = "europe.pool.ntp.org";  // Fallback

// Zeitzone für Zürich mit automatischer Sommer/Winterzeit-Umstellung
const char* timezone = "CET-1CEST,M3.5.0,M10.5.0/3";

// Zeitvariablen
struct tm timeinfo;
bool timeValid = false;

// WiFi Status
bool wifiConnected = false;

// Update-Timer
unsigned long lastTimeUpdate = 0;
int lastDisplayedMinute = -1;  // Letzte angezeigte Minute (um Flickern zu vermeiden)

// Helligkeitssteuerung
#ifdef photoR_PIN
int lastBrightness = -1;
const int BRIGHTNESS_SAMPLES = 10;  // Anzahl Messungen für Durchschnitt
#endif

// Farben
#define COLOR_BG        TFT_BLACK
#define COLOR_TIME      0x07FF    // Cyan
#define COLOR_TIME_SHADOW 0x0410  // Dunkelcyan
#define COLOR_DATE      0xFD20    // Orange
#define COLOR_HEADER    0x4208    // Dunkelgrau
#define COLOR_TEXT      TFT_WHITE
#define COLOR_WIFI_OK   TFT_GREEN
#define COLOR_WIFI_BAD  TFT_RED

void setup() {
  Serial.begin(115200);
  Serial.println("CYD WiFi Uhr gestartet!");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);  // Landscape

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
    Serial.println("Photoresistor initialisiert - Automatische Helligkeitssteuerung aktiv");
  #endif

  lcd.setBrightness(200);
  lcd.fillScreen(COLOR_BG);

  // Startbildschirm
  showStartScreen();

  // WiFi verbinden
  connectWiFi();

  // Zeit vom NTP-Server holen
  if (wifiConnected) {
    Serial.println("Hole Zeit vom NTP-Server...");
    configTzTime(timezone, ntpServer, ntpServer2);

    // Warten bis Zeit synchronisiert ist (max 5 Sekunden)
    int attempts = 0;
    while (!getLocalTime(&timeinfo) && attempts < 10) {
      delay(500);
      Serial.print(".");
      attempts++;
    }

    if (getLocalTime(&timeinfo)) {
      timeValid = true;
      Serial.println("\nZeit erfolgreich synchronisiert!");
      Serial.println(&timeinfo, "%A, %d. %B %Y %H:%M:%S");
    } else {
      Serial.println("\nFehler: Konnte Zeit nicht abrufen!");
    }
  }

  // Hauptbildschirm zeichnen
  drawClockScreen();
}

void loop() {
  // Optional: Helligkeit über Photoresistor steuern
  #ifdef photoR_PIN
    adjustBrightness();
  #endif

  // Zeit aktualisieren (alle paar Sekunden prüfen)
  if (getLocalTime(&timeinfo)) {
    timeValid = true;

    // Display nur aktualisieren wenn sich die Minute geändert hat
    if (timeinfo.tm_min != lastDisplayedMinute) {
      lastDisplayedMinute = timeinfo.tm_min;
      updateClock();
      Serial.print("Uhr aktualisiert: ");
      Serial.print(timeinfo.tm_hour);
      Serial.print(":");
      Serial.println(timeinfo.tm_min);
    }
  } else {
    timeValid = false;
    if (lastDisplayedMinute != -1) {
      showError();
      lastDisplayedMinute = -1;
    }
  }

  // Zeit alle 60 Minuten neu synchronisieren
  if (millis() - lastTimeUpdate >= 3600000) {
    lastTimeUpdate = millis();
    configTzTime(timezone, ntpServer, ntpServer2);
    Serial.println("Zeit re-synchronisiert");
  }

  // Kurze Pause um CPU zu schonen
  delay(500);
}

// WiFi verbinden
void connectWiFi() {
  lcd.fillScreen(COLOR_BG);

  int centerX = SCREEN_WIDTH / 2;
  int startY = SCREEN_HEIGHT / 5;

  lcd.setFont(&fonts::Font4);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setTextDatum(MC_DATUM);
  lcd.drawString("Verbinde WiFi...", centerX, startY);

  lcd.setFont(&fonts::Font2);
  lcd.setTextColor(COLOR_DATE);
  lcd.setTextDatum(MC_DATUM);
  lcd.drawString(String("SSID: ") + ssid, centerX, startY + 40);

  Serial.print("Verbinde mit WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int attempts = 0;
  int barWidth = SCREEN_WIDTH / 2;
  int barHeight = SCREEN_HEIGHT / 12;
  int barX = (SCREEN_WIDTH - barWidth) / 2;
  int barY = startY + 70;

  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");

    // Fortschrittsbalken
    int segmentWidth = barWidth / 20;
    lcd.fillRect(barX + attempts * segmentWidth, barY, segmentWidth - 2, barHeight, COLOR_WIFI_OK);

    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("\nWiFi verbunden!");
    Serial.print("IP Adresse: ");
    Serial.println(WiFi.localIP());

    lcd.fillScreen(COLOR_BG);
    lcd.setFont(&fonts::Font4);
    lcd.setTextColor(COLOR_WIFI_OK);
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString("Verbunden!", centerX, startY + 50);

    lcd.setFont(&fonts::Font2);
    lcd.setTextColor(COLOR_TIME);
    lcd.drawString(String("IP: ") + WiFi.localIP().toString(), centerX, startY + 85);

    delay(2000);
  } else {
    wifiConnected = false;
    Serial.println("\nWiFi Verbindung fehlgeschlagen!");

    lcd.setFont(&fonts::Font4);
    lcd.setTextColor(COLOR_WIFI_BAD);
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString("Fehler!", centerX, startY + 60);

    lcd.setFont(&fonts::Font2);
    lcd.drawString("Pruefe Credentials.h", centerX, startY + 95);

    delay(3000);
  }
}

// Startbildschirm
void showStartScreen() {
  lcd.fillScreen(0x000A);  // Sehr dunkles Blau

  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;

  // Großer Titel mit Schatten-Effekt
  lcd.setFont(&fonts::Font6);
  lcd.setTextDatum(MC_DATUM);

  lcd.setTextColor(0x2104);  // Schatten
  lcd.drawString("CYD Uhr", centerX + 2, centerY - 8);

  lcd.setTextColor(COLOR_DATE);  // Orange
  lcd.drawString("CYD Uhr", centerX, centerY - 10);

  lcd.setFont(&fonts::Font2);
  lcd.setTextColor(COLOR_TIME);
  lcd.drawString("WiFi + NTP", centerX, centerY + 30);

  lcd.setTextColor(COLOR_HEADER);
  lcd.drawString("Zuerich", centerX, centerY + 50);

  delay(1500);
}

// Uhr-Bildschirm zeichnen (nur einmal)
void drawClockScreen() {
  lcd.fillScreen(COLOR_BG);

  int headerHeight = SCREEN_HEIGHT / 6;

  // Header-Bereich mit Gradient-Effekt
  for (int y = 0; y < headerHeight; y++) {
    int color = lcd.color565(0, y * 2, y * 2);
    lcd.drawFastHLine(0, y, SCREEN_WIDTH, color);
  }

  lcd.setFont(&fonts::Font2);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setTextDatum(TL_DATUM);
  lcd.drawString("WiFi Uhr", 8, 8);

  lcd.setTextColor(COLOR_DATE);

  // Zeige ob Sommer- oder Winterzeit
  if (timeinfo.tm_isdst > 0) {
    lcd.drawString("Zuerich (MESZ)", 8, 22);  // Sommerzeit
  } else {
    lcd.drawString("Zuerich (MEZ)", 8, 22);   // Winterzeit
  }

  // WiFi Status rechts oben
  lcd.setTextDatum(TR_DATUM);
  if (wifiConnected) {
    lcd.setTextColor(COLOR_WIFI_OK);
    lcd.drawString("WiFi", SCREEN_WIDTH - 8, 8);

    int rssi = WiFi.RSSI();
    String signal = "";
    if (rssi > -60) signal = "****";
    else if (rssi > -70) signal = "*** ";
    else if (rssi > -80) signal = "**  ";
    else signal = "*   ";

    lcd.drawString(signal, SCREEN_WIDTH - 8, 22);
  } else {
    lcd.setTextColor(COLOR_WIFI_BAD);
    lcd.drawString("----", SCREEN_WIDTH - 8, 8);
  }

  // Trennlinie
  lcd.drawFastHLine(0, headerHeight, SCREEN_WIDTH, COLOR_HEADER);
}

// Uhrzeit aktualisieren mit schönem Font
void updateClock() {
  int headerHeight = SCREEN_HEIGHT / 6;
  int timeY = headerHeight + (SCREEN_HEIGHT - headerHeight) * 2 / 7;

  // Zeit-Bereich löschen
  lcd.fillRect(0, headerHeight + 1, SCREEN_WIDTH, (SCREEN_HEIGHT - headerHeight) * 4 / 7, COLOR_BG);

  // Uhrzeit ohne Sekunden (nur HH:MM) mit großem Font
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);

  // Text zentrieren
  lcd.setFont(&fonts::Font8);
  lcd.setTextDatum(MC_DATUM);
  int centerX = SCREEN_WIDTH / 2;

  // Schatten-Effekt
  lcd.setTextColor(COLOR_TIME_SHADOW);
  lcd.drawString(timeStr, centerX + 4, timeY + 4);

  // Haupt-Text
  lcd.setTextColor(COLOR_TIME);
  lcd.drawString(timeStr, centerX, timeY);

  // Datum-Bereich
  int dateY = headerHeight + (SCREEN_HEIGHT - headerHeight) * 5 / 7;
  lcd.fillRect(0, dateY, SCREEN_WIDTH, SCREEN_HEIGHT - dateY - 15, COLOR_BG);

  lcd.setFont(&fonts::Font4);
  lcd.setTextColor(COLOR_DATE);
  lcd.setTextDatum(MC_DATUM);

  const char* weekDays[] = {"Sonntag", "Montag", "Dienstag", "Mittwoch",
                            "Donnerstag", "Freitag", "Samstag"};
  const char* months[] = {"Januar", "Februar", "Maerz", "April", "Mai", "Juni",
                          "Juli", "August", "September", "Oktober", "November", "Dezember"};

  // Wochentag
  lcd.drawString(weekDays[timeinfo.tm_wday], centerX, dateY + 10);

  // Tag und Monat
  char dateStr[32];
  sprintf(dateStr, "%d. %s %d",
          timeinfo.tm_mday,
          months[timeinfo.tm_mon],
          timeinfo.tm_year + 1900);

  lcd.setFont(&fonts::Font2);
  lcd.setTextColor(COLOR_TEXT);
  lcd.drawString(dateStr, centerX, dateY + 40);

  // Status-Zeile unten
  int statusY = SCREEN_HEIGHT - 12;
  lcd.fillRect(0, statusY, SCREEN_WIDTH, 12, COLOR_BG);
  lcd.setFont(&fonts::Font0);
  lcd.setTextColor(COLOR_HEADER);
  lcd.setTextDatum(TL_DATUM);

  char syncInfo[20];
  sprintf(syncInfo, "Sync: %02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
  lcd.drawString(syncInfo, 5, statusY);

  // Signal-Stärke rechts
  if (wifiConnected) {
    int rssi = WiFi.RSSI();
    char rssiStr[10];
    sprintf(rssiStr, "%ddBm", rssi);
    lcd.setTextDatum(TR_DATUM);
    lcd.drawString(rssiStr, SCREEN_WIDTH - 5, statusY);
  }
}

// Fehler anzeigen
void showError() {
  int headerHeight = SCREEN_HEIGHT / 6;
  int timeY = headerHeight + (SCREEN_HEIGHT - headerHeight) * 2 / 7;

  lcd.fillRect(0, headerHeight + 1, SCREEN_WIDTH, (SCREEN_HEIGHT - headerHeight) * 4 / 7, COLOR_BG);

  lcd.setFont(&fonts::Font8);
  lcd.setTextColor(COLOR_WIFI_BAD);
  lcd.setTextDatum(MC_DATUM);
  lcd.drawString("--:--", SCREEN_WIDTH / 2, timeY);

  lcd.setFont(&fonts::Font2);
  lcd.setTextColor(COLOR_DATE);
  lcd.drawString("Keine Zeitdaten!", SCREEN_WIDTH / 2, timeY + 60);
}

#ifdef photoR_PIN
// Passt die Display-Helligkeit basierend auf dem Photoresistor an
void adjustBrightness() {
  static unsigned long lastCheck = 0;
  unsigned long now = millis();

  // Alle 200ms prüfen
  if (now - lastCheck < 200) return;
  lastCheck = now;

  // RAW Wert direkt lesen und sofort ausgeben
  int rawValue = analogRead(photoR_PIN);

  // Bei JEDER Messung ausgeben zum Debuggen
  Serial.print("PhotoR RAW: ");
  Serial.print(rawValue);
  Serial.print(" (0-4095)");

  // Teste beide Mapping-Richtungen
  int brightness1 = map(rawValue, 0, 4095, 255, 10);  // RAW 0=viel Licht -> Display hell (255), RAW hoch -> sehr dunkel (10)
  int brightness2 = map(rawValue, 0, 4095, 10, 255);  // RAW 0=viel Licht -> Display dunkel (10)

  Serial.print(" -> Helligkeit1: ");
  Serial.print(brightness1);
  Serial.print(" / Helligkeit2: ");
  Serial.print(brightness2);

  // Verwende KORREKTES Mapping: RAW 0 = viel Licht = Display hell
  // RAW niedrig (viel Licht) → Display hell (255)
  // RAW hoch (wenig Licht) → Display sehr dunkel (10)
  int brightness = brightness1;
  brightness = constrain(brightness, 10, 255);

  Serial.print(" -> Verwendet: ");
  Serial.println(brightness);

  // Ändern wenn Differenz > 5
  if (abs(brightness - lastBrightness) > 5 || lastBrightness == -1) {
    lcd.setBrightness(brightness);
    lastBrightness = brightness;

    Serial.println(">>> HELLIGKEIT GEAENDERT!");
  }
}
#endif
