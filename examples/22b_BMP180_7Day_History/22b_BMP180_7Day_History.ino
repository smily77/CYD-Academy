/*
  CYD-Academy - Beispiel 22b: BMP180 7-Tage-Verlauf

  Einfache Wetterstation mit 7-Tage-Verlaufsanzeige für Luftdruck und Temperatur.
  Im Vergleich zu Beispiel 22 liegt der Fokus auf langfristiger Datenerfassung
  und übersichtlicher Visualisierung ohne komplexe Wettervorhersage.

  Hardware:
  - ESP32-2432S028R (CYD - Cheap Yellow Display)
  - BMP180 Barometer/Temperatur-Sensor (I2C)
  - WiFi für NTP-Zeitstempel

  I2C Verbindung (am CYD):
  - SDA: GPIO 22 (extSDA)
  - SCL: GPIO 27 (extSCL)
  - VCC: 3.3V
  - GND: GND

  Features:
  - Messung alle 2 Stunden über 7 Tage (84 Datenpunkte)
  - Persistente Speicherung in ESP32 Preferences (NVS)
  - NTP-Synchronisierung für genaue Zeitstempel
  - Zwei übersichtliche Liniendiagramme
  - Min/Max Markierungen
  - Touch: Detailansicht und manuelle Messung

  Bibliotheken (Arduino Library Manager):
  - Adafruit BMP085 Library (funktioniert mit BMP180)
  - LovyanGFX
  - WiFi (ESP32 Core)
  - Preferences (ESP32 Core)
  - Time (ESP32 Core)

  * WICHTIG: Erstelle eine Datei "Credentials.h" im gleichen Verzeichnis mit:
  *   const char* ssid = "DeinWiFiName";
  *   const char* password = "DeinWiFiPasswort";

  Autor: CYD-Academy
  Datum: 2025
  Lizenz: MIT
*/

#include <CYD_Display_Config.h>
#include "Credentials.h"
#include <Adafruit_BMP085.h>
#include <WiFi.h>
#include <Preferences.h>
#include <time.h>

// Display-Objekt (aus CYD_Display_Config.h)
LGFX lcd;

// Sensor-Objekt
Adafruit_BMP085 bmp;

// Preferences für persistente Speicherung
Preferences preferences;

// NTP Server
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;        // GMT+1 (Deutschland)
const int daylightOffset_sec = 3600;    // Sommerzeit

// Messintervall und Speicher
const int MEASUREMENT_INTERVAL = 2 * 60 * 60;  // 2 Stunden in Sekunden
const int MAX_DATA_POINTS = 84;                // 7 Tage * 12 Messungen/Tag
const unsigned long SCREEN_UPDATE_INTERVAL = 60000;  // Display-Update jede Minute

// Datenstrukturen
struct DataPoint {
  time_t timestamp;
  float temperature;
  float pressure;
};

DataPoint historyData[MAX_DATA_POINTS];
int dataCount = 0;
int currentIndex = 0;

// Aktuelle Messwerte
float currentTemp = 0.0;
float currentPressure = 0.0;
float currentAltitude = 0.0;

// Statistiken
float minTemp = 999.0;
float maxTemp = -999.0;
float minPressure = 9999.0;
float maxPressure = 0.0;

// Zeitvariablen
unsigned long lastMeasurement = 0;
unsigned long lastScreenUpdate = 0;
bool ntpSynced = false;

// Display-Parameter
const int GRAPH_HEIGHT = 80;
const int GRAPH_Y_TEMP = 40;
const int GRAPH_Y_PRESSURE = 140;
const int GRAPH_X = 40;
const int GRAPH_WIDTH = 240;

// Sea-Level Pressure für Höhenberechnung (Standard: 1013.25 hPa)
float seaLevelPressure = 1013.25;

// Touch-Modi
enum TouchMode {
  MODE_NORMAL,
  MODE_DETAIL
};

TouchMode currentMode = MODE_NORMAL;

// Detailansicht
int detailIndex = -1;

//=============================================================================
// Setup
//=============================================================================

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n=== CYD-Academy: Beispiel 22b - BMP180 7-Tage-Verlauf ===\n"));

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);  // Landscape
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextDatum(MC_DATUM);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);

  showStartupScreen();

  // I2C initialisieren (extSDA=22, extSCL=27 aus CYD_Display_Config.h)
  Wire.begin(extSDA, extSCL);
  delay(100);

  // BMP180 initialisieren
  lcd.drawString("Initialisiere BMP180...", 160, 120);
  if (!bmp.begin()) {
    lcd.fillScreen(TFT_RED);
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString("FEHLER:", 160, 80);
    lcd.drawString("BMP180 nicht gefunden!", 160, 120);
    lcd.drawString("Verkabelung pruefen:", 160, 160);
    lcd.drawString("SDA=22, SCL=27", 160, 180);
    while (1) delay(1000);
  }
  Serial.println(F("✓ BMP180 gefunden"));

  // WiFi verbinden
  lcd.fillScreen(TFT_BLACK);
  lcd.drawString("Verbinde WiFi...", 160, 80);
  lcd.drawString(ssid, 160, 100);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    lcd.drawString(String(".").c_str(), 160 + (attempts % 10) * 10, 120);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("\n✓ WiFi verbunden"));
    lcd.fillScreen(TFT_BLACK);
    lcd.drawString("WiFi verbunden!", 160, 100);
    lcd.drawString(WiFi.localIP().toString().c_str(), 160, 120);

    // NTP synchronisieren
    lcd.drawString("Synchronisiere Zeit...", 160, 160);
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    delay(2000);

    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      ntpSynced = true;
      Serial.println(F("✓ NTP synchronisiert"));
      char timeStr[64];
      strftime(timeStr, sizeof(timeStr), "%d.%m.%Y %H:%M:%S", &timeinfo);
      lcd.drawString(timeStr, 160, 180);
      delay(1000);
    } else {
      Serial.println(F("✗ NTP-Sync fehlgeschlagen"));
      lcd.drawString("Zeit-Sync fehlgeschlagen", 160, 180);
      delay(2000);
    }
  } else {
    Serial.println(F("\n✗ WiFi-Verbindung fehlgeschlagen"));
    lcd.fillScreen(TFT_RED);
    lcd.drawString("WiFi Fehler!", 160, 120);
    lcd.drawString("Fortfahren ohne Zeit...", 160, 140);
    delay(2000);
  }

  // Preferences laden
  lcd.fillScreen(TFT_BLACK);
  lcd.drawString("Lade gespeicherte Daten...", 160, 120);
  loadHistoryData();

  Serial.print(F("Geladene Datenpunkte: "));
  Serial.println(dataCount);

  lcd.drawString(String(dataCount) + " Datenpunkte geladen", 160, 140);
  delay(1000);

  // Erste Messung
  performMeasurement();
  updateStatistics();

  Serial.println(F("\n=== Setup abgeschlossen ===\n"));
  delay(500);
}

//=============================================================================
// Main Loop
//=============================================================================

void loop() {
  unsigned long currentMillis = millis();

  // Periodische Messung (alle 2 Stunden)
  time_t now = time(nullptr);
  if (ntpSynced && now - lastMeasurement >= MEASUREMENT_INTERVAL) {
    performMeasurement();
    updateStatistics();
    saveHistoryData();
    lastMeasurement = now;
  }

  // Display-Update (jede Minute oder bei Touch)
  if (currentMillis - lastScreenUpdate >= SCREEN_UPDATE_INTERVAL) {
    // Aktuelle Werte aktualisieren (ohne zu speichern)
    readCurrentValues();
    lastScreenUpdate = currentMillis;
  }

  // Display zeichnen
  drawScreen();

  // Touch-Handling
  handleTouch();

  delay(100);
}

//=============================================================================
// Sensor-Funktionen
//=============================================================================

void readCurrentValues() {
  currentTemp = bmp.readTemperature();
  currentPressure = bmp.readPressure() / 100.0;  // Pa -> hPa
  currentAltitude = bmp.readAltitude(seaLevelPressure * 100);  // hPa -> Pa

  Serial.print(F("Aktuell: "));
  Serial.print(currentTemp, 1);
  Serial.print(F("°C, "));
  Serial.print(currentPressure, 1);
  Serial.print(F(" hPa, "));
  Serial.print(currentAltitude, 0);
  Serial.println(F(" m"));
}

void performMeasurement() {
  readCurrentValues();

  // Neue Messung zum Verlauf hinzufügen
  historyData[currentIndex].timestamp = time(nullptr);
  historyData[currentIndex].temperature = currentTemp;
  historyData[currentIndex].pressure = currentPressure;

  currentIndex = (currentIndex + 1) % MAX_DATA_POINTS;
  if (dataCount < MAX_DATA_POINTS) {
    dataCount++;
  }

  Serial.println(F("Messung gespeichert"));
}

void updateStatistics() {
  if (dataCount == 0) return;

  minTemp = 999.0;
  maxTemp = -999.0;
  minPressure = 9999.0;
  maxPressure = 0.0;

  for (int i = 0; i < dataCount; i++) {
    if (historyData[i].temperature < minTemp) minTemp = historyData[i].temperature;
    if (historyData[i].temperature > maxTemp) maxTemp = historyData[i].temperature;
    if (historyData[i].pressure < minPressure) minPressure = historyData[i].pressure;
    if (historyData[i].pressure > maxPressure) maxPressure = historyData[i].pressure;
  }

  Serial.print(F("Stats: Temp "));
  Serial.print(minTemp, 1);
  Serial.print(F("-"));
  Serial.print(maxTemp, 1);
  Serial.print(F("°C, Druck "));
  Serial.print(minPressure, 1);
  Serial.print(F("-"));
  Serial.print(maxPressure, 1);
  Serial.println(F(" hPa"));
}

//=============================================================================
// Datenspeicherung (Preferences / NVS)
//=============================================================================

void saveHistoryData() {
  preferences.begin("weather", false);

  // Metadaten speichern
  preferences.putInt("dataCount", dataCount);
  preferences.putInt("currentIdx", currentIndex);

  // Daten als Blob speichern
  preferences.putBytes("historyData", historyData, sizeof(historyData));

  preferences.end();

  Serial.println(F("Daten gespeichert"));
}

void loadHistoryData() {
  preferences.begin("weather", true);  // read-only

  dataCount = preferences.getInt("dataCount", 0);
  currentIndex = preferences.getInt("currentIdx", 0);

  if (dataCount > 0) {
    preferences.getBytes("historyData", historyData, sizeof(historyData));
  }

  preferences.end();
}

//=============================================================================
// Display-Funktionen
//=============================================================================

void showStartupScreen() {
  lcd.fillScreen(TFT_NAVY);

  // Titel
  lcd.setTextColor(TFT_YELLOW, TFT_NAVY);
  lcd.setTextSize(2);
  lcd.drawString("BMP180", 160, 50);

  lcd.setTextSize(1);
  lcd.setTextColor(TFT_WHITE, TFT_NAVY);
  lcd.drawString("7-Tage Wetterstation", 160, 80);

  // Info
  lcd.setTextColor(TFT_CYAN, TFT_NAVY);
  lcd.drawString("CYD-Academy Beispiel 22b", 160, 120);

  lcd.setTextColor(TFT_LIGHTGREY, TFT_NAVY);
  lcd.setTextSize(1);
  lcd.drawString("Temperatur & Luftdruck", 160, 180);
  lcd.drawString("Langzeit-Verlauf", 160, 200);

  delay(2000);
}

void drawScreen() {
  if (currentMode == MODE_NORMAL) {
    drawNormalView();
  } else {
    drawDetailView();
  }
}

void drawNormalView() {
  lcd.fillScreen(TFT_BLACK);

  // Titel und aktuelle Zeit
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  lcd.setTextDatum(TL_DATUM);
  lcd.drawString("7-Tage Verlauf", 5, 5);

  if (ntpSynced) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char timeStr[32];
      strftime(timeStr, sizeof(timeStr), "%d.%m. %H:%M", &timeinfo);
      lcd.setTextDatum(TR_DATUM);
      lcd.setTextColor(TFT_CYAN, TFT_BLACK);
      lcd.drawString(timeStr, 315, 5);
    }
  }

  // Aktuelle Werte prominent anzeigen
  lcd.setTextDatum(MC_DATUM);
  lcd.setTextSize(2);
  lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
  lcd.drawString(String(currentTemp, 1) + " C", 80, 25);

  lcd.setTextColor(TFT_CYAN, TFT_BLACK);
  lcd.drawString(String(currentPressure, 1) + " hPa", 240, 25);

  // Temperatur-Graph
  lcd.setTextSize(1);
  lcd.setTextDatum(TL_DATUM);
  lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
  lcd.drawString("Temperatur", GRAPH_X, GRAPH_Y_TEMP - 15);
  drawGraph(GRAPH_Y_TEMP, true);

  // Druck-Graph
  lcd.setTextColor(TFT_CYAN, TFT_BLACK);
  lcd.drawString("Luftdruck", GRAPH_X, GRAPH_Y_PRESSURE - 15);
  drawGraph(GRAPH_Y_PRESSURE, false);

  // Statistiken
  lcd.setTextSize(1);
  lcd.setTextDatum(TL_DATUM);
  lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  lcd.drawString("Min: " + String(minTemp, 1) + " C", 5, GRAPH_Y_TEMP + GRAPH_HEIGHT + 5);
  lcd.drawString("Max: " + String(maxTemp, 1) + " C", 5, GRAPH_Y_TEMP + GRAPH_HEIGHT + 18);

  lcd.drawString("Min: " + String(minPressure, 1) + " hPa", 5, GRAPH_Y_PRESSURE + GRAPH_HEIGHT + 5);
  lcd.drawString("Max: " + String(maxPressure, 1) + " hPa", 5, GRAPH_Y_PRESSURE + GRAPH_HEIGHT + 18);

  // Datenpunkte-Info
  lcd.setTextDatum(TR_DATUM);
  lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
  lcd.drawString(String(dataCount) + "/" + String(MAX_DATA_POINTS) + " Punkte", 315, 230);

  // Touch-Hinweis
  lcd.setTextDatum(MC_DATUM);
  lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
  lcd.drawString("Touch: Neues Update | Long: Details", 160, 232);
}

void drawGraph(int yPos, bool isTemperature) {
  if (dataCount < 2) {
    lcd.setTextDatum(MC_DATUM);
    lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
    lcd.drawString("Sammle Daten...", GRAPH_X + GRAPH_WIDTH/2, yPos + GRAPH_HEIGHT/2);
    return;
  }

  // Graph-Rahmen
  lcd.drawRect(GRAPH_X, yPos, GRAPH_WIDTH, GRAPH_HEIGHT, TFT_DARKGREY);

  // Bestimme Min/Max für Skalierung
  float minVal = 9999.0;
  float maxVal = -9999.0;

  for (int i = 0; i < dataCount; i++) {
    float val = isTemperature ? historyData[i].temperature : historyData[i].pressure;
    if (val < minVal) minVal = val;
    if (val > maxVal) maxVal = val;
  }

  // Etwas Padding für bessere Darstellung
  float range = maxVal - minVal;
  minVal -= range * 0.1;
  maxVal += range * 0.1;
  if (range < 0.1) {  // Vermeidie Division durch 0
    minVal -= 1.0;
    maxVal += 1.0;
  }

  // Y-Achsen-Beschriftung
  lcd.setTextSize(1);
  lcd.setTextDatum(TR_DATUM);
  lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
  lcd.drawString(String(maxVal, 1), GRAPH_X - 2, yPos);
  lcd.drawString(String(minVal, 1), GRAPH_X - 2, yPos + GRAPH_HEIGHT - 8);

  // Mittelwert-Linie
  float midVal = (minVal + maxVal) / 2.0;
  int midY = yPos + GRAPH_HEIGHT/2;
  lcd.drawLine(GRAPH_X, midY, GRAPH_X + GRAPH_WIDTH, midY, TFT_DARKGREY);
  lcd.drawString(String(midVal, 1), GRAPH_X - 2, midY - 4);

  // Datenpunkte zeichnen
  uint16_t color = isTemperature ? TFT_ORANGE : TFT_CYAN;

  for (int i = 1; i < dataCount; i++) {
    float val1 = isTemperature ? historyData[i-1].temperature : historyData[i-1].pressure;
    float val2 = isTemperature ? historyData[i].temperature : historyData[i].pressure;

    int x1 = GRAPH_X + ((i - 1) * GRAPH_WIDTH) / (dataCount - 1);
    int x2 = GRAPH_X + (i * GRAPH_WIDTH) / (dataCount - 1);

    int y1 = yPos + GRAPH_HEIGHT - ((val1 - minVal) / (maxVal - minVal) * GRAPH_HEIGHT);
    int y2 = yPos + GRAPH_HEIGHT - ((val2 - minVal) / (maxVal - minVal) * GRAPH_HEIGHT);

    // Linie zeichnen
    lcd.drawLine(x1, y1, x2, y2, color);

    // Punkte zeichnen
    lcd.fillCircle(x2, y2, 2, color);
  }

  // Zeitachse (7 Tage = 168 Stunden)
  if (ntpSynced && dataCount > 0) {
    lcd.setTextDatum(BC_DATUM);
    lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);

    // Ältester Datenpunkt
    time_t oldestTime = historyData[0].timestamp;
    struct tm oldest_tm;
    localtime_r(&oldestTime, &oldest_tm);
    char oldestStr[16];
    strftime(oldestStr, sizeof(oldestStr), "%d.%m", &oldest_tm);
    lcd.drawString(oldestStr, GRAPH_X, yPos + GRAPH_HEIGHT + 10);

    // Neuester Datenpunkt
    time_t newestTime = historyData[dataCount-1].timestamp;
    struct tm newest_tm;
    localtime_r(&newestTime, &newest_tm);
    char newestStr[16];
    strftime(newestStr, sizeof(newestStr), "%d.%m", &newest_tm);
    lcd.drawString(newestStr, GRAPH_X + GRAPH_WIDTH, yPos + GRAPH_HEIGHT + 10);
  }
}

void drawDetailView() {
  lcd.fillScreen(TFT_BLACK);

  // Titel
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  lcd.setTextDatum(MC_DATUM);
  lcd.drawString("Detailansicht", 160, 10);

  if (detailIndex < 0 || detailIndex >= dataCount) {
    lcd.setTextColor(TFT_RED, TFT_BLACK);
    lcd.drawString("Keine Daten verfuegbar", 160, 120);
    lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
    lcd.drawString("Touch: Zurueck", 160, 220);
    return;
  }

  DataPoint& dp = historyData[detailIndex];

  // Zeitstempel
  struct tm dp_tm;
  localtime_r(&dp.timestamp, &dp_tm);
  char timeStr[64];
  strftime(timeStr, sizeof(timeStr), "%d.%m.%Y %H:%M Uhr", &dp_tm);

  lcd.setTextColor(TFT_CYAN, TFT_BLACK);
  lcd.drawString(timeStr, 160, 35);

  // Temperatur
  lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
  lcd.setTextSize(2);
  lcd.drawString("Temperatur:", 160, 70);
  lcd.setTextSize(3);
  lcd.drawString(String(dp.temperature, 1) + " C", 160, 95);

  // Luftdruck
  lcd.setTextSize(2);
  lcd.setTextColor(TFT_CYAN, TFT_BLACK);
  lcd.drawString("Luftdruck:", 160, 135);
  lcd.setTextSize(3);
  lcd.drawString(String(dp.pressure, 1) + " hPa", 160, 160);

  // Navigation
  lcd.setTextSize(1);
  lcd.setTextDatum(BL_DATUM);
  lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  if (detailIndex > 0) {
    lcd.drawString("< Aelter", 10, 230);
  }

  lcd.setTextDatum(BC_DATUM);
  lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
  lcd.drawString(String(detailIndex + 1) + " / " + String(dataCount), 160, 230);

  lcd.setTextDatum(BR_DATUM);
  lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  if (detailIndex < dataCount - 1) {
    lcd.drawString("Neuer >", 310, 230);
  }

  lcd.setTextDatum(MC_DATUM);
  lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  lcd.drawString("Touch Mitte: Zurueck", 160, 215);
}

//=============================================================================
// Touch-Handling
//=============================================================================

void handleTouch() {
  uint16_t touchX, touchY;
  static unsigned long touchStartTime = 0;
  static bool wasTouched = false;

  if (lcd.getTouch(&touchX, &touchY)) {
    if (!wasTouched) {
      touchStartTime = millis();
      wasTouched = true;
    }

    unsigned long touchDuration = millis() - touchStartTime;

    // Long press (> 1 Sekunde) -> Detail-Modus
    if (touchDuration > 1000 && currentMode == MODE_NORMAL) {
      currentMode = MODE_DETAIL;
      detailIndex = dataCount - 1;  // Neuesten Datenpunkt anzeigen
      delay(300);
      wasTouched = false;
      return;
    }
  } else {
    if (wasTouched) {
      // Touch wurde losgelassen
      unsigned long touchDuration = millis() - touchStartTime;

      if (currentMode == MODE_NORMAL) {
        // Kurzer Touch -> Neue Messung
        if (touchDuration < 1000) {
          performMeasurement();
          updateStatistics();
          saveHistoryData();
          lastScreenUpdate = millis();  // Sofortiges Update
        }
      } else {
        // Detail-Modus: Navigation
        if (touchX < 106) {
          // Linke Seite: Älterer Datenpunkt
          if (detailIndex > 0) {
            detailIndex--;
          }
        } else if (touchX > 214) {
          // Rechte Seite: Neuerer Datenpunkt
          if (detailIndex < dataCount - 1) {
            detailIndex++;
          }
        } else {
          // Mitte: Zurück zur Normalansicht
          currentMode = MODE_NORMAL;
        }
        delay(200);
      }

      wasTouched = false;
    }
  }
}
