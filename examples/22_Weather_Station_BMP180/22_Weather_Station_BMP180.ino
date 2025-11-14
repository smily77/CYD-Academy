/*
 * CYD-Academy - Beispiel 22: Wetterstation mit BMP180
 *
 * Dieses Beispiel demonstriert eine Wettervorhersage mit dem BMP180 Barometer.
 * Der Sensor misst Luftdruck und Temperatur für Wetter-Trends.
 *
 * Hardware:
 * - ESP32-2432S028R (CYD - Cheap Yellow Display)
 * - BMP180 Barometric Pressure Sensor
 *
 * Verbindung (I2C):
 * - SDA: GPIO 22 (extSDA)
 * - SCL: GPIO 27 (extSCL)
 * - I2C-Adresse: 0x77 (Standard) oder 0x76
 *
 * Features:
 * - Luftdruck-Messung (300-1100 hPa)
 * - Temperatur-Messung (-40 bis +85°C)
 * - Wettervorhersage basierend auf Drucktrend (3h Geschichte)
 * - Wetter-Icons (Sonne, Wolken, Regen, Gewitter, Schnee)
 * - 24h Druckverlaufsdiagramm
 * - Höhenberechnung aus Luftdruck
 * - Touch-Bedienung für Units (hPa/mmHg, °C/°F, m/ft)
 * - Min/Max Statistik
 *
 * Bibliotheken:
 * - Adafruit BMP085 Library (funktioniert auch mit BMP180)
 *
 * Installation:
 * Arduino IDE -> Tools -> Manage Libraries -> "Adafruit BMP085 Unified" installieren
 *
 * Autor: CYD-Academy
 * Lizenz: MIT
 */

#include <Wire.h>
#include <CYD_Display_Config.h>
#include <Adafruit_BMP085.h>

// ========== Display Setup ==========
LGFX lcd;

// ========== BMP180 Sensor ==========
Adafruit_BMP085 bmp;

// I2C Pins vom CYD
#define extSDA 22
#define extSCL 27

// ========== Units ==========
enum PressureUnit {
  UNIT_HPA = 0,   // Hektopascal
  UNIT_MMHG = 1   // Millimeter Quecksilbersäule
};

enum TempUnit {
  UNIT_CELSIUS = 0,
  UNIT_FAHRENHEIT = 1
};

enum AltitudeUnit {
  UNIT_METER = 0,
  UNIT_FEET = 1
};

PressureUnit pressureUnit = UNIT_HPA;
TempUnit tempUnit = UNIT_CELSIUS;
AltitudeUnit altitudeUnit = UNIT_METER;

// ========== Messwerte ==========
float currentPressure = 1013.25;  // hPa
float currentTemp = 20.0;         // °C
float currentAltitude = 0.0;      // m

// Referenzdruck auf Meereshöhe (einstellbar)
float seaLevelPressure = 1013.25;  // hPa

// ========== Drucktrend für Wettervorhersage ==========
#define TREND_SAMPLES 36  // 36 x 5 Min = 3 Stunden
float pressureTrend[TREND_SAMPLES];
int trendIndex = 0;
unsigned long lastTrendUpdate = 0;
#define TREND_INTERVAL 300000  // 5 Minuten

// ========== Wettervorhersage ==========
enum WeatherForecast {
  WEATHER_SUNNY = 0,
  WEATHER_PARTLY_CLOUDY,
  WEATHER_CLOUDY,
  WEATHER_RAINY,
  WEATHER_STORMY,
  WEATHER_SNOW
};

WeatherForecast currentWeather = WEATHER_PARTLY_CLOUDY;

// ========== 24h Druckverlauf ==========
#define HISTORY_24H 288  // 288 x 5 Min = 24h
float pressureHistory[HISTORY_24H];
int historyIndex = 0;

// ========== Statistik ==========
float pressureMin = 1100.0;
float pressureMax = 300.0;
float tempMin = 85.0;
float tempMax = -40.0;

// ========== Timing ==========
unsigned long lastMeasurement = 0;
#define MEASUREMENT_INTERVAL 2000  // 2 Sekunden

// ========== Sensor Status ==========
bool sensorAvailable = false;
String statusMessage = "Initialisiere...";

// ========== Display-Farben ==========
#define COLOR_BACKGROUND TFT_BLACK
#define COLOR_TEXT TFT_WHITE
#define COLOR_ACCENT TFT_CYAN
#define COLOR_SUNNY TFT_YELLOW
#define COLOR_CLOUDY TFT_LIGHTGREY
#define COLOR_RAINY TFT_BLUE
#define COLOR_STORMY TFT_PURPLE
#define COLOR_GOOD TFT_GREEN
#define COLOR_WARNING TFT_ORANGE
#define COLOR_ALERT TFT_RED

// Touch-Buttons
struct TouchButton {
  int x, y, w, h;
  String label;
  uint16_t color;
};

TouchButton unitButtons[3] = {
  {10, 430, 90, 35, "hPa", TFT_DARKGREEN},
  {110, 430, 90, 35, "°C", TFT_DARKGREEN},
  {210, 430, 90, 35, "m", TFT_DARKGREEN}
};

// ========== Setup ==========
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== CYD Wetterstation - BMP180 ===");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(0);  // Portrait
  lcd.fillScreen(COLOR_BACKGROUND);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setTextDatum(TC_DATUM);

  // Willkommensbildschirm
  lcd.setTextSize(2);
  lcd.drawString("Wetterstation", 160, 100);
  lcd.setTextSize(1);
  lcd.drawString("BMP180 Barometer", 160, 130);
  lcd.drawString("Initialisiere I2C...", 160, 200);

  // I2C initialisieren
  Wire.begin(extSDA, extSCL);
  Serial.printf("I2C initialisiert auf SDA: %d, SCL: %d\n", extSDA, extSCL);

  delay(1000);

  // BMP180 initialisieren
  lcd.drawString("Suche BMP180...", 160, 230);

  if (!bmp.begin()) {
    Serial.println("FEHLER: BMP180 nicht gefunden!");
    lcd.fillScreen(COLOR_BACKGROUND);
    lcd.setTextColor(TFT_RED);
    lcd.drawString("BMP180 FEHLER!", 160, 200);
    lcd.setTextColor(COLOR_TEXT);
    lcd.drawString("Prüfe Verkabelung:", 160, 240);
    lcd.drawString("SDA -> GPIO 22", 160, 260);
    lcd.drawString("SCL -> GPIO 27", 160, 280);
    lcd.drawString("VCC -> 3.3V", 160, 300);
    lcd.drawString("GND -> GND", 160, 320);

    statusMessage = "SENSOR FEHLT!";
    sensorAvailable = false;

    delay(3000);
  } else {
    Serial.println("BMP180 gefunden!");
    sensorAvailable = true;

    // Erste Messung
    currentPressure = bmp.readPressure() / 100.0;  // Pa -> hPa
    currentTemp = bmp.readTemperature();
    currentAltitude = bmp.readAltitude(seaLevelPressure * 100);  // hPa -> Pa

    Serial.printf("Druck: %.2f hPa, Temp: %.2f °C\n", currentPressure, currentTemp);

    // Historie initialisieren
    for (int i = 0; i < HISTORY_24H; i++) {
      pressureHistory[i] = currentPressure;
    }
    for (int i = 0; i < TREND_SAMPLES; i++) {
      pressureTrend[i] = currentPressure;
    }

    statusMessage = "Sensor bereit";
  }

  // Hauptbildschirm zeichnen
  drawMainScreen();

  Serial.println("Setup abgeschlossen!");
  Serial.println("Hinweis: Wettervorhersage nach 3h Laufzeit genauer.");
}

// ========== Main Loop ==========
void loop() {
  static unsigned long lastUpdate = 0;

  // Messung durchführen
  if (sensorAvailable && millis() - lastMeasurement >= MEASUREMENT_INTERVAL) {
    readSensor();
    lastMeasurement = millis();
  }

  // Drucktrend aktualisieren (alle 5 Min)
  if (sensorAvailable && millis() - lastTrendUpdate >= TREND_INTERVAL) {
    updatePressureTrend();
    lastTrendUpdate = millis();
  }

  // Display aktualisieren (alle 1s)
  if (millis() - lastUpdate >= 1000) {
    updateDisplay();
    lastUpdate = millis();
  }

  // Touch-Eingabe verarbeiten
  handleTouch();

  delay(10);
}

// ========== Sensor auslesen ==========
void readSensor() {
  float pressure = bmp.readPressure() / 100.0;  // Pa -> hPa
  float temp = bmp.readTemperature();           // °C
  float altitude = bmp.readAltitude(seaLevelPressure * 100);  // m

  // Validierung
  if (pressure >= 300 && pressure <= 1100) {
    currentPressure = pressure;
    currentTemp = temp;
    currentAltitude = altitude;

    // Statistik
    if (pressure < pressureMin) pressureMin = pressure;
    if (pressure > pressureMax) pressureMax = pressure;
    if (temp < tempMin) tempMin = temp;
    if (temp > tempMax) tempMax = temp;

    // Debug (reduziert)
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug >= 10000) {
      Serial.printf("Druck: %.2f hPa, Temp: %.2f °C, Höhe: %.1f m\n",
                    pressure, temp, altitude);
      lastDebug = millis();
    }
  }
}

// ========== Drucktrend aktualisieren ==========
void updatePressureTrend() {
  pressureTrend[trendIndex] = currentPressure;
  trendIndex = (trendIndex + 1) % TREND_SAMPLES;

  // In 24h Historie eintragen
  pressureHistory[historyIndex] = currentPressure;
  historyIndex = (historyIndex + 1) % HISTORY_24H;

  // Wettervorhersage berechnen
  calculateWeatherForecast();

  Serial.printf("Drucktrend aktualisiert: %.2f hPa (Index: %d)\n", currentPressure, trendIndex);
}

// ========== Wettervorhersage berechnen ==========
void calculateWeatherForecast() {
  // Berechne Druckänderung über 3 Stunden
  int oldIndex = (trendIndex - TREND_SAMPLES + TREND_SAMPLES) % TREND_SAMPLES;
  float pressureChange = currentPressure - pressureTrend[oldIndex];

  Serial.printf("Druckänderung (3h): %.2f hPa\n", pressureChange);

  // Wettervorhersage basierend auf Drucktrend und absolutem Druck
  if (currentPressure > 1023) {
    // Hochdruck
    if (pressureChange > 2) {
      currentWeather = WEATHER_SUNNY;  // Stark steigend -> Schönwetter
    } else if (pressureChange > -2) {
      currentWeather = WEATHER_PARTLY_CLOUDY;  // Stabil hoch
    } else {
      currentWeather = WEATHER_CLOUDY;  // Fallend trotz Hochdruck
    }
  } else if (currentPressure > 1013) {
    // Normal
    if (pressureChange > 3) {
      currentWeather = WEATHER_PARTLY_CLOUDY;  // Stark steigend
    } else if (pressureChange > 0) {
      currentWeather = WEATHER_CLOUDY;  // Leicht steigend
    } else if (pressureChange > -3) {
      currentWeather = WEATHER_RAINY;  // Leicht fallend
    } else {
      currentWeather = WEATHER_STORMY;  // Stark fallend
    }
  } else {
    // Tiefdruck
    if (pressureChange > 2) {
      currentWeather = WEATHER_CLOUDY;  // Steigend von Tief
    } else if (pressureChange > -2) {
      currentWeather = WEATHER_RAINY;  // Stabil tief
    } else {
      if (currentTemp < 2) {
        currentWeather = WEATHER_SNOW;  // Fallend + kalt
      } else {
        currentWeather = WEATHER_STORMY;  // Fallend + warm
      }
    }
  }
}

// ========== Hauptbildschirm zeichnen ==========
void drawMainScreen() {
  lcd.fillScreen(COLOR_BACKGROUND);

  // Titel
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_ACCENT);
  lcd.setTextDatum(TC_DATUM);
  lcd.drawString("Wetterstation", 160, 5);

  // Bereiche
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_DARKGREY);

  // Wetter-Icon Bereich
  lcd.drawRect(5, 35, 310, 100, TFT_DARKGREY);

  // Messwerte Bereich
  lcd.drawRect(5, 140, 150, 100, TFT_DARKGREY);
  lcd.drawRect(165, 140, 150, 100, TFT_DARKGREY);

  // Graph Bereich
  lcd.drawRect(5, 245, 310, 120, TFT_DARKGREY);
  lcd.setTextDatum(TL_DATUM);
  lcd.drawString("24h Druckverlauf", 10, 250);

  // Statistik
  lcd.drawRect(5, 370, 310, 50, TFT_DARKGREY);

  // Buttons
  drawButtons();

  // Status
  lcd.setTextDatum(TC_DATUM);
  lcd.setTextColor(TFT_DARKGREY);
  lcd.drawString("Status:", 160, 470);
}

// ========== Display aktualisieren ==========
void updateDisplay() {
  // Wetter-Icon
  drawWeatherIcon();

  // Messwerte
  drawMeasurements();

  // Graph
  drawPressureGraph();

  // Statistik
  drawStatistics();

  // Status
  lcd.setTextSize(1);
  lcd.setTextDatum(TC_DATUM);
  lcd.setTextColor(sensorAvailable ? TFT_GREEN : TFT_RED);
  lcd.fillRect(10, 470, 300, 10, COLOR_BACKGROUND);
  lcd.drawString(statusMessage, 160, 470);
}

// ========== Wetter-Icon zeichnen ==========
void drawWeatherIcon() {
  int centerX = 160;
  int centerY = 85;

  // Hintergrund löschen
  lcd.fillRect(10, 40, 300, 90, COLOR_BACKGROUND);

  switch (currentWeather) {
    case WEATHER_SUNNY:
      drawSunIcon(centerX, centerY);
      break;
    case WEATHER_PARTLY_CLOUDY:
      drawSunIcon(centerX - 30, centerY - 10);
      drawCloudIcon(centerX + 20, centerY + 10, TFT_LIGHTGREY);
      break;
    case WEATHER_CLOUDY:
      drawCloudIcon(centerX - 30, centerY, TFT_DARKGREY);
      drawCloudIcon(centerX + 30, centerY, TFT_LIGHTGREY);
      break;
    case WEATHER_RAINY:
      drawCloudIcon(centerX, centerY - 10, TFT_DARKGREY);
      drawRainIcon(centerX, centerY + 20);
      break;
    case WEATHER_STORMY:
      drawCloudIcon(centerX, centerY - 10, TFT_PURPLE);
      drawLightningIcon(centerX, centerY + 20);
      break;
    case WEATHER_SNOW:
      drawCloudIcon(centerX, centerY - 10, TFT_LIGHTGREY);
      drawSnowIcon(centerX, centerY + 20);
      break;
  }

  // Wettertext
  lcd.setTextDatum(TC_DATUM);
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_ACCENT);

  String weatherText[] = {"Sonnig", "Heiter", "Bewölkt", "Regen", "Gewitter", "Schnee"};
  lcd.drawString(weatherText[currentWeather], centerX, 120);
}

// ========== Sonne zeichnen ==========
void drawSunIcon(int x, int y) {
  lcd.fillCircle(x, y, 20, COLOR_SUNNY);

  // Strahlen
  for (int i = 0; i < 8; i++) {
    float angle = i * PI / 4;
    int x1 = x + cos(angle) * 25;
    int y1 = y + sin(angle) * 25;
    int x2 = x + cos(angle) * 35;
    int y2 = y + sin(angle) * 35;
    lcd.drawLine(x1, y1, x2, y2, COLOR_SUNNY);
  }
}

// ========== Wolke zeichnen ==========
void drawCloudIcon(int x, int y, uint16_t color) {
  lcd.fillCircle(x - 15, y, 10, color);
  lcd.fillCircle(x, y - 5, 12, color);
  lcd.fillCircle(x + 15, y, 10, color);
  lcd.fillRect(x - 20, y, 40, 10, color);
}

// ========== Regen zeichnen ==========
void drawRainIcon(int x, int y) {
  for (int i = 0; i < 5; i++) {
    int rx = x - 20 + i * 10;
    lcd.drawLine(rx, y, rx, y + 15, COLOR_RAINY);
  }
}

// ========== Blitz zeichnen ==========
void drawLightningIcon(int x, int y) {
  lcd.fillTriangle(x - 5, y, x + 5, y + 10, x, y + 10, TFT_YELLOW);
  lcd.fillTriangle(x, y + 10, x - 5, y + 20, x + 5, y + 10, TFT_YELLOW);
}

// ========== Schnee zeichnen ==========
void drawSnowIcon(int x, int y) {
  for (int i = 0; i < 5; i++) {
    int sx = x - 20 + i * 10;
    lcd.drawCircle(sx, y + (i % 2) * 5, 2, TFT_WHITE);
    lcd.drawCircle(sx, y + 10 + (i % 2) * 5, 2, TFT_WHITE);
  }
}

// ========== Messwerte zeichnen ==========
void drawMeasurements() {
  lcd.setTextDatum(TC_DATUM);

  // Luftdruck
  lcd.fillRect(10, 145, 145, 90, COLOR_BACKGROUND);
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_DARKGREY);
  lcd.drawString("Luftdruck", 82, 150);

  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);

  if (pressureUnit == UNIT_HPA) {
    lcd.drawString(String(currentPressure, 1), 82, 170);
    lcd.setTextSize(1);
    lcd.drawString("hPa", 82, 195);
  } else {
    float mmHg = currentPressure * 0.750062;
    lcd.drawString(String(mmHg, 1), 82, 170);
    lcd.setTextSize(1);
    lcd.drawString("mmHg", 82, 195);
  }

  // Farbe basierend auf Druck
  uint16_t pressureColor = COLOR_GOOD;
  if (currentPressure < 980 || currentPressure > 1040) {
    pressureColor = COLOR_ALERT;
  } else if (currentPressure < 1000 || currentPressure > 1030) {
    pressureColor = COLOR_WARNING;
  }
  lcd.fillCircle(140, 160, 5, pressureColor);

  // Temperatur
  lcd.fillRect(170, 145, 145, 90, COLOR_BACKGROUND);
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_DARKGREY);
  lcd.drawString("Temperatur", 242, 150);

  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);

  if (tempUnit == UNIT_CELSIUS) {
    lcd.drawString(String(currentTemp, 1), 242, 170);
    lcd.setTextSize(1);
    lcd.drawString("°C", 242, 195);
  } else {
    float fahrenheit = currentTemp * 9.0 / 5.0 + 32.0;
    lcd.drawString(String(fahrenheit, 1), 242, 170);
    lcd.setTextSize(1);
    lcd.drawString("°F", 242, 195);
  }

  // Höhe (klein unten)
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_DARKGREY);

  if (altitudeUnit == UNIT_METER) {
    lcd.drawString("Höhe: " + String(currentAltitude, 0) + " m", 242, 215);
  } else {
    float feet = currentAltitude * 3.28084;
    lcd.drawString("Höhe: " + String(feet, 0) + " ft", 242, 215);
  }
}

// ========== Druckverlaufs-Graph ==========
void drawPressureGraph() {
  int graphX = 10;
  int graphY = 270;
  int graphW = 300;
  int graphH = 85;

  // Hintergrund
  lcd.fillRect(graphX, graphY, graphW, graphH, COLOR_BACKGROUND);

  // Min/Max für Skalierung finden (aus letzten 24h)
  float minP = 1100, maxP = 300;
  for (int i = 0; i < HISTORY_24H; i++) {
    if (pressureHistory[i] > 0) {
      if (pressureHistory[i] < minP) minP = pressureHistory[i];
      if (pressureHistory[i] > maxP) maxP = pressureHistory[i];
    }
  }

  // Mindest-Range von 10 hPa
  if (maxP - minP < 10) {
    float center = (maxP + minP) / 2;
    minP = center - 5;
    maxP = center + 5;
  }

  // Gitternetz (3 horizontale Linien)
  for (int i = 0; i <= 2; i++) {
    int y = graphY + (graphH * i / 2);
    lcd.drawLine(graphX, y, graphX + graphW, y, TFT_DARKGREY);

    // Skala
    float pressure = maxP - (maxP - minP) * i / 2;
    lcd.setTextDatum(TL_DATUM);
    lcd.setTextSize(1);
    lcd.setTextColor(TFT_DARKGREY);
    lcd.drawString(String(pressure, 0), graphX + graphW + 5, y - 4);
  }

  // Daten zeichnen
  lcd.setTextColor(COLOR_ACCENT);
  for (int i = 0; i < HISTORY_24H - 1; i++) {
    int idx1 = (historyIndex + i) % HISTORY_24H;
    int idx2 = (historyIndex + i + 1) % HISTORY_24H;

    if (pressureHistory[idx1] > 0 && pressureHistory[idx2] > 0) {
      int x1 = graphX + (i * graphW / HISTORY_24H);
      int x2 = graphX + ((i + 1) * graphW / HISTORY_24H);

      int y1 = graphY + graphH - map(pressureHistory[idx1] * 10, minP * 10, maxP * 10, 0, graphH);
      int y2 = graphY + graphH - map(pressureHistory[idx2] * 10, minP * 10, maxP * 10, 0, graphH);

      lcd.drawLine(x1, y1, x2, y2, COLOR_ACCENT);
    }
  }

  // Zeitachse
  lcd.setTextDatum(TC_DATUM);
  lcd.setTextColor(TFT_DARKGREY);
  lcd.drawString("24h", graphX, graphY + graphH + 2);
  lcd.drawString("12h", graphX + graphW / 2, graphY + graphH + 2);
  lcd.drawString("Jetzt", graphX + graphW, graphY + graphH + 2);
}

// ========== Statistik zeichnen ==========
void drawStatistics() {
  lcd.fillRect(10, 375, 300, 40, COLOR_BACKGROUND);

  lcd.setTextDatum(TL_DATUM);
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_DARKGREY);

  lcd.drawString("Min:", 15, 378);
  lcd.drawString("Max:", 15, 395);

  lcd.setTextColor(COLOR_TEXT);

  // Druck
  if (pressureUnit == UNIT_HPA) {
    lcd.drawString(String(pressureMin, 1) + " hPa", 50, 378);
    lcd.drawString(String(pressureMax, 1) + " hPa", 50, 395);
  } else {
    lcd.drawString(String(pressureMin * 0.750062, 1) + " mmHg", 50, 378);
    lcd.drawString(String(pressureMax * 0.750062, 1) + " mmHg", 50, 395);
  }

  // Temperatur
  if (tempUnit == UNIT_CELSIUS) {
    lcd.drawString(String(tempMin, 1) + " °C", 170, 378);
    lcd.drawString(String(tempMax, 1) + " °C", 170, 395);
  } else {
    lcd.drawString(String(tempMin * 9.0 / 5.0 + 32.0, 1) + " °F", 170, 378);
    lcd.drawString(String(tempMax * 9.0 / 5.0 + 32.0, 1) + " °F", 170, 395);
  }
}

// ========== Buttons zeichnen ==========
void drawButtons() {
  for (int i = 0; i < 3; i++) {
    TouchButton btn = unitButtons[i];

    lcd.fillRoundRect(btn.x, btn.y, btn.w, btn.h, 5, btn.color);
    lcd.drawRoundRect(btn.x, btn.y, btn.w, btn.h, 5, TFT_WHITE);

    lcd.setTextDatum(MC_DATUM);
    lcd.setTextSize(1);
    lcd.setTextColor(TFT_WHITE);

    // Aktueller Wert
    String label = "";
    switch (i) {
      case 0:
        label = (pressureUnit == UNIT_HPA) ? "hPa" : "mmHg";
        break;
      case 1:
        label = (tempUnit == UNIT_CELSIUS) ? "°C" : "°F";
        break;
      case 2:
        label = (altitudeUnit == UNIT_METER) ? "m" : "ft";
        break;
    }

    lcd.drawString(label, btn.x + btn.w / 2, btn.y + btn.h / 2);
  }
}

// ========== Touch-Handler ==========
void handleTouch() {
  uint16_t x, y;
  if (!lcd.getTouch(&x, &y)) return;

  // Unit-Buttons
  for (int i = 0; i < 3; i++) {
    TouchButton btn = unitButtons[i];
    if (x >= btn.x && x <= btn.x + btn.w && y >= btn.y && y <= btn.y + btn.h) {
      switch (i) {
        case 0:
          pressureUnit = (pressureUnit == UNIT_HPA) ? UNIT_MMHG : UNIT_HPA;
          Serial.printf("Druckeinheit: %s\n", (pressureUnit == UNIT_HPA) ? "hPa" : "mmHg");
          break;
        case 1:
          tempUnit = (tempUnit == UNIT_CELSIUS) ? UNIT_FAHRENHEIT : UNIT_CELSIUS;
          Serial.printf("Temperatureinheit: %s\n", (tempUnit == UNIT_CELSIUS) ? "°C" : "°F");
          break;
        case 2:
          altitudeUnit = (altitudeUnit == UNIT_METER) ? UNIT_FEET : UNIT_METER;
          Serial.printf("Höheneinheit: %s\n", (altitudeUnit == UNIT_METER) ? "m" : "ft");
          break;
      }

      drawButtons();
      delay(200);
      return;
    }
  }
}
