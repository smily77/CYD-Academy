/*
 * CYD-Academy - Beispiel 20: Luftqualität mit CCS811 Sensor
 *
 * Dieses Beispiel demonstriert die Messung der Luftqualität mit dem CCS811 Sensor.
 * Der Sensor misst eCO2 (equivalent CO2) und TVOC (Total Volatile Organic Compounds).
 *
 * Hardware:
 * - ESP32-2432S028R (CYD - Cheap Yellow Display)
 * - CCS811 Air Quality Sensor (I2C)
 *
 * I2C Verbindung:
 * - SDA: GPIO 22 (extSDA)
 * - SCL: GPIO 27 (extSCL)
 * - I2C-Adresse: 0x5A (Standard) oder 0x5B (ADDR Pin HIGH)
 *
 * Features:
 * - eCO2 Messung (400-8192 ppm)
 * - TVOC Messung (0-1187 ppb)
 * - Luftqualitäts-Index (AQI) mit Farbcodierung
 * - Dual-Channel Verlaufsdiagramme
 * - Aufwärmphase-Timer (20 Minuten für optimale Genauigkeit)
 * - Baseline-Speicherung im EEPROM
 * - Drei Messmodi: Eco (60s), Normal (10s), Fast (1s)
 * - Warnungen bei kritischen Werten
 * - Touch-Bedienung
 *
 * Bibliotheken:
 * - Adafruit CCS811 Library (für komplexe Sensor-Kommunikation)
 *
 * Installation:
 * Arduino IDE -> Tools -> Manage Libraries -> "Adafruit CCS811" installieren
 *
 * Autor: CYD-Academy
 * Lizenz: MIT
 */

#include <Wire.h>
#include <CYD_Display_Config.h>
#include <Adafruit_CCS811.h>
#include <EEPROM.h>

// ========== Display Setup ==========
LGFX lcd;

// ========== CCS811 Sensor ==========
Adafruit_CCS811 ccs;

// I2C Pins vom CYD
#define extSDA 22
#define extSCL 27

// CCS811 I2C-Adresse (Standard: 0x5A, alternativ: 0x5B wenn ADDR HIGH)
#define CCS811_ADDR 0x5A

// ========== Messmodi ==========
enum MeasurementMode {
  MODE_ECO = 0,     // 60 Sekunden (Eco)
  MODE_NORMAL = 1,  // 10 Sekunden (Normal)
  MODE_FAST = 2     // 1 Sekunde (Fast)
};

MeasurementMode measurementMode = MODE_NORMAL;

const char* modeNames[] = {"ECO (60s)", "NORMAL (10s)", "FAST (1s)"};
const uint8_t ccsModeCodes[] = {
  CCS811_DRIVE_MODE_60SEC,   // Mode 1: 60s
  CCS811_DRIVE_MODE_10SEC,   // Mode 2: 10s
  CCS811_DRIVE_MODE_1SEC     // Mode 3: 1s
};

// ========== Messwerte ==========
uint16_t eco2 = 400;      // eCO2 in ppm (400-8192)
uint16_t tvoc = 0;        // TVOC in ppb (0-1187)

// ========== Statistik ==========
uint16_t eco2Min = 8192;
uint16_t eco2Max = 400;
float eco2Avg = 400;

uint16_t tvocMin = 1187;
uint16_t tvocMax = 0;
float tvocAvg = 0;

unsigned long totalMeasurements = 0;
unsigned long errorCount = 0;

// ========== Verlaufsdiagramm ==========
#define MAX_HISTORY 100
uint16_t eco2History[MAX_HISTORY];
uint16_t tvocHistory[MAX_HISTORY];
int historyIndex = 0;

// ========== Timing ==========
unsigned long lastMeasurement = 0;
unsigned long measurementInterval = 10000;  // Standard: 10s
unsigned long sensorStartTime = 0;
bool sensorWarmedUp = false;

// Aufwärmzeit: 20 Minuten für beste Genauigkeit
#define WARMUP_TIME_MS (20UL * 60UL * 1000UL)  // 20 Minuten

// ========== Baseline (für bessere Genauigkeit) ==========
#define EEPROM_SIZE 512
#define EEPROM_BASELINE_ADDR 0
#define BASELINE_SAVE_INTERVAL (24UL * 60UL * 60UL * 1000UL)  // 24 Stunden
unsigned long lastBaselineSave = 0;

// ========== Display Layout ==========
#define COLOR_BACKGROUND TFT_BLACK
#define COLOR_TEXT TFT_WHITE
#define COLOR_GOOD TFT_GREEN
#define COLOR_MODERATE TFT_YELLOW
#define COLOR_UNHEALTHY TFT_ORANGE
#define COLOR_VERY_UNHEALTHY TFT_RED
#define COLOR_HAZARDOUS TFT_PURPLE

// Touch-Bereiche
struct TouchButton {
  int x, y, w, h;
  String label;
  uint16_t color;
};

TouchButton modeButtons[3] = {
  {10, 380, 90, 40, "ECO", TFT_DARKGREEN},
  {110, 380, 90, 40, "NORMAL", TFT_BLUE},
  {210, 380, 90, 40, "FAST", TFT_MAROON}
};

TouchButton resetButton = {10, 430, 300, 35, "RESET", TFT_DARKGREY};

// ========== Sensor Status ==========
bool sensorAvailable = false;
String statusMessage = "Initialisiere...";

// ========== Setup ==========
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== CYD Air Quality Monitor - CCS811 ===");

  // EEPROM initialisieren
  EEPROM.begin(EEPROM_SIZE);

  // Display initialisieren
  lcd.init();
  lcd.setRotation(0);  // Portrait
  lcd.fillScreen(COLOR_BACKGROUND);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setTextDatum(TC_DATUM);

  // Willkommensbildschirm
  lcd.setTextSize(2);
  lcd.drawString("CYD Air Quality", 160, 100);
  lcd.setTextSize(1);
  lcd.drawString("CCS811 Sensor", 160, 130);
  lcd.drawString("Initialisiere I2C...", 160, 200);

  // I2C initialisieren
  Wire.begin(extSDA, extSCL);
  Serial.printf("I2C initialisiert auf SDA: %d, SCL: %d\n", extSDA, extSCL);

  delay(1000);

  // CCS811 initialisieren
  lcd.drawString("Suche CCS811...", 160, 230);

  if (!ccs.begin(CCS811_ADDR, &Wire)) {
    Serial.println("FEHLER: CCS811 nicht gefunden!");
    lcd.fillScreen(COLOR_BACKGROUND);
    lcd.setTextColor(TFT_RED);
    lcd.drawString("CCS811 FEHLER!", 160, 200);
    lcd.setTextColor(COLOR_TEXT);
    lcd.drawString("Prüfe Verkabelung:", 160, 240);
    lcd.drawString("SDA -> GPIO 22", 160, 260);
    lcd.drawString("SCL -> GPIO 27", 160, 280);
    lcd.drawString("VCC -> 3.3V", 160, 300);
    lcd.drawString("GND -> GND", 160, 320);

    statusMessage = "SENSOR FEHLT!";
    sensorAvailable = false;

    // Trotzdem UI zeichnen (Demo-Modus)
    delay(3000);
  } else {
    Serial.println("CCS811 gefunden!");
    sensorAvailable = true;

    // Warten bis Sensor bereit
    lcd.drawString("Warte auf Sensor...", 160, 260);
    while (!ccs.available()) {
      delay(100);
    }

    Serial.println("CCS811 bereit!");

    // Messmodus setzen (Standard: 10s)
    ccs.setDriveMode(ccsModeCodes[MODE_NORMAL]);
    Serial.printf("Messmodus: %s\n", modeNames[MODE_NORMAL]);

    // Baseline laden (falls vorhanden)
    loadBaseline();

    // Startzeit merken (für Aufwärmphase)
    sensorStartTime = millis();

    statusMessage = "Sensor aktiv";
  }

  // Verlaufsdiagramm initialisieren
  for (int i = 0; i < MAX_HISTORY; i++) {
    eco2History[i] = 400;
    tvocHistory[i] = 0;
  }

  // Hauptbildschirm zeichnen
  drawMainScreen();

  Serial.println("Setup abgeschlossen!");
  Serial.println("Hinweis: Sensor benötigt 20 Minuten Aufwärmzeit für beste Ergebnisse.");
}

// ========== Main Loop ==========
void loop() {
  static unsigned long lastUpdate = 0;

  // Messung durchführen
  if (sensorAvailable && millis() - lastMeasurement >= measurementInterval) {
    readSensor();
    lastMeasurement = millis();
  }

  // Display aktualisieren (alle 500ms)
  if (millis() - lastUpdate >= 500) {
    updateDisplay();
    lastUpdate = millis();
  }

  // Touch-Eingabe verarbeiten
  handleTouch();

  // Baseline regelmäßig speichern (alle 24h)
  if (sensorAvailable && millis() - lastBaselineSave >= BASELINE_SAVE_INTERVAL) {
    saveBaseline();
    lastBaselineSave = millis();
  }

  delay(10);
}

// ========== Sensor auslesen ==========
void readSensor() {
  if (!ccs.available()) {
    return;
  }

  // Temperatur/Feuchtigkeits-Kompensation (optional)
  // Wenn ein BME280/DHT22 vorhanden ist, kann man die Genauigkeit verbessern:
  // float temp = 25.0;  // Von anderem Sensor
  // float hum = 50.0;   // Von anderem Sensor
  // ccs.setEnvironmentalData((uint8_t)hum, temp);

  if (!ccs.readData()) {
    // Erfolgreiche Messung
    eco2 = ccs.geteCO2();
    tvoc = ccs.getTVOC();

    // Validierung
    if (eco2 >= 400 && eco2 <= 8192) {
      updateStatistics();
      addToHistory();

      Serial.printf("[%lu] eCO2: %d ppm | TVOC: %d ppb | AQI: %s\n",
                    millis() / 1000, eco2, tvoc, getAQILevel().c_str());
    } else {
      errorCount++;
      Serial.println("Ungültige Messung");
    }
  } else {
    errorCount++;
    Serial.println("Fehler beim Lesen des Sensors");
  }
}

// ========== Statistik aktualisieren ==========
void updateStatistics() {
  totalMeasurements++;

  // eCO2
  if (eco2 < eco2Min) eco2Min = eco2;
  if (eco2 > eco2Max) eco2Max = eco2;
  eco2Avg = ((eco2Avg * (totalMeasurements - 1)) + eco2) / totalMeasurements;

  // TVOC
  if (tvoc < tvocMin) tvocMin = tvoc;
  if (tvoc > tvocMax) tvocMax = tvoc;
  tvocAvg = ((tvocAvg * (totalMeasurements - 1)) + tvoc) / totalMeasurements;
}

// ========== Verlauf aktualisieren ==========
void addToHistory() {
  eco2History[historyIndex] = eco2;
  tvocHistory[historyIndex] = tvoc;
  historyIndex = (historyIndex + 1) % MAX_HISTORY;
}

// ========== Luftqualitäts-Index bestimmen ==========
String getAQILevel() {
  // Basierend auf eCO2 Werten (ppm)
  if (eco2 < 600) return "Ausgezeichnet";
  if (eco2 < 800) return "Gut";
  if (eco2 < 1000) return "Mäßig";
  if (eco2 < 1500) return "Ungesund";
  return "Gefährlich";
}

uint16_t getAQIColor() {
  if (eco2 < 600) return COLOR_GOOD;
  if (eco2 < 800) return TFT_GREENYELLOW;
  if (eco2 < 1000) return COLOR_MODERATE;
  if (eco2 < 1500) return COLOR_UNHEALTHY;
  if (eco2 < 2500) return COLOR_VERY_UNHEALTHY;
  return COLOR_HAZARDOUS;
}

// ========== Hauptbildschirm zeichnen ==========
void drawMainScreen() {
  lcd.fillScreen(COLOR_BACKGROUND);

  // Titel
  lcd.setTextSize(2);
  lcd.setTextColor(TFT_CYAN);
  lcd.drawString("Luftqualität", 160, 5);

  // Statische Labels
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_DARKGREY);

  // eCO2 Bereich (oben)
  lcd.drawRect(5, 35, 310, 75, TFT_DARKGREY);
  lcd.drawString("eCO2 (equivalent CO2)", 160, 40);

  // TVOC Bereich
  lcd.drawRect(5, 115, 310, 75, TFT_DARKGREY);
  lcd.drawString("TVOC (flüchtige Verbindungen)", 160, 120);

  // AQI Bereich
  lcd.drawRect(5, 195, 310, 50, TFT_DARKGREY);
  lcd.drawString("Luftqualitäts-Index", 160, 200);

  // Graph-Bereich
  lcd.drawRect(5, 250, 310, 120, TFT_DARKGREY);
  lcd.setTextDatum(TL_DATUM);
  lcd.drawString("Verlauf", 10, 255);

  // Buttons zeichnen
  drawButtons();

  // Status-Zeile
  lcd.setTextDatum(TC_DATUM);
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_DARKGREY);
  lcd.drawString("Status:", 160, 470);
}

// ========== Display aktualisieren ==========
void updateDisplay() {
  // eCO2 Wert
  lcd.setTextDatum(TC_DATUM);
  lcd.fillRect(10, 55, 300, 50, COLOR_BACKGROUND);
  lcd.setTextSize(3);
  lcd.setTextColor(getAQIColor());
  lcd.drawString(String(eco2) + " ppm", 160, 55);

  // TVOC Wert
  lcd.fillRect(10, 135, 300, 50, COLOR_BACKGROUND);
  uint16_t tvocColor = (tvoc < 220) ? COLOR_GOOD :
                       (tvoc < 660) ? COLOR_MODERATE :
                       COLOR_UNHEALTHY;
  lcd.setTextColor(tvocColor);
  lcd.drawString(String(tvoc) + " ppb", 160, 135);

  // AQI Level
  lcd.fillRect(10, 215, 300, 25, COLOR_BACKGROUND);
  lcd.setTextSize(2);
  lcd.setTextColor(getAQIColor());
  lcd.drawString(getAQILevel(), 160, 217);

  // Verlaufsdiagramm
  drawGraph();

  // Aufwärmphase-Anzeige
  if (!sensorWarmedUp) {
    unsigned long warmupElapsed = millis() - sensorStartTime;
    if (warmupElapsed >= WARMUP_TIME_MS) {
      sensorWarmedUp = true;
      statusMessage = "Sensor bereit";
    } else {
      unsigned long remainingMin = (WARMUP_TIME_MS - warmupElapsed) / 60000;
      statusMessage = "Aufwärmen: " + String(remainingMin) + " min";
    }
  }

  // Status-Text
  lcd.setTextSize(1);
  lcd.setTextColor(sensorAvailable ? TFT_GREEN : TFT_RED);
  lcd.fillRect(10, 470, 300, 10, COLOR_BACKGROUND);
  lcd.drawString(statusMessage, 160, 470);
}

// ========== Verlaufsdiagramm ==========
void drawGraph() {
  int graphX = 10;
  int graphY = 270;
  int graphW = 300;
  int graphH = 90;

  // Hintergrund löschen
  lcd.fillRect(graphX, graphY, graphW, graphH, COLOR_BACKGROUND);

  // Gitternetz (horizontal)
  for (int i = 0; i <= 4; i++) {
    int y = graphY + (graphH * i / 4);
    lcd.drawLine(graphX, y, graphX + graphW, y, TFT_DARKGREY);
  }

  // Datenpunkte zeichnen
  for (int i = 0; i < MAX_HISTORY - 1; i++) {
    int idx1 = (historyIndex + i) % MAX_HISTORY;
    int idx2 = (historyIndex + i + 1) % MAX_HISTORY;

    // eCO2 (Cyan)
    if (eco2History[idx1] >= 400 && eco2History[idx2] >= 400) {
      int x1 = graphX + (i * graphW / MAX_HISTORY);
      int x2 = graphX + ((i + 1) * graphW / MAX_HISTORY);

      int y1 = graphY + graphH - map(constrain(eco2History[idx1], 400, 2000), 400, 2000, 0, graphH);
      int y2 = graphY + graphH - map(constrain(eco2History[idx2], 400, 2000), 400, 2000, 0, graphH);

      lcd.drawLine(x1, y1, x2, y2, TFT_CYAN);
    }

    // TVOC (Orange) - zweite Skala
    if (tvocHistory[idx1] <= 1187 && tvocHistory[idx2] <= 1187) {
      int x1 = graphX + (i * graphW / MAX_HISTORY);
      int x2 = graphX + ((i + 1) * graphW / MAX_HISTORY);

      int y1 = graphY + graphH - map(constrain(tvocHistory[idx1], 0, 1000), 0, 1000, 0, graphH);
      int y2 = graphY + graphH - map(constrain(tvocHistory[idx2], 0, 1000), 0, 1000, 0, graphH);

      lcd.drawLine(x1, y1, x2, y2, TFT_ORANGE);
    }
  }

  // Legende
  lcd.setTextDatum(TL_DATUM);
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_CYAN);
  lcd.drawString("eCO2", graphX + 5, graphY + 5);
  lcd.setTextColor(TFT_ORANGE);
  lcd.drawString("TVOC", graphX + 50, graphY + 5);
}

// ========== Buttons zeichnen ==========
void drawButtons() {
  // Mode-Buttons
  for (int i = 0; i < 3; i++) {
    TouchButton btn = modeButtons[i];
    uint16_t color = (i == measurementMode) ? TFT_GREEN : btn.color;

    lcd.fillRoundRect(btn.x, btn.y, btn.w, btn.h, 5, color);
    lcd.drawRoundRect(btn.x, btn.y, btn.w, btn.h, 5, TFT_WHITE);

    lcd.setTextDatum(MC_DATUM);
    lcd.setTextSize(1);
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString(btn.label, btn.x + btn.w / 2, btn.y + btn.h / 2);
  }

  // Reset-Button
  lcd.fillRoundRect(resetButton.x, resetButton.y, resetButton.w, resetButton.h, 5, resetButton.color);
  lcd.drawRoundRect(resetButton.x, resetButton.y, resetButton.w, resetButton.h, 5, TFT_WHITE);
  lcd.setTextDatum(MC_DATUM);
  lcd.setTextColor(TFT_WHITE);
  lcd.drawString(resetButton.label, resetButton.x + resetButton.w / 2, resetButton.y + resetButton.h / 2);
}

// ========== Touch-Handler ==========
void handleTouch() {
  uint16_t x, y;
  if (!lcd.getTouch(&x, &y)) return;

  // Mode-Buttons
  for (int i = 0; i < 3; i++) {
    TouchButton btn = modeButtons[i];
    if (x >= btn.x && x <= btn.x + btn.w && y >= btn.y && y <= btn.y + btn.h) {
      if (measurementMode != (MeasurementMode)i) {
        measurementMode = (MeasurementMode)i;

        // Interval anpassen
        switch (measurementMode) {
          case MODE_ECO:    measurementInterval = 60000; break;
          case MODE_NORMAL: measurementInterval = 10000; break;
          case MODE_FAST:   measurementInterval = 1000; break;
        }

        // CCS811 Modus setzen
        if (sensorAvailable) {
          ccs.setDriveMode(ccsModeCodes[measurementMode]);
          Serial.printf("Messmodus geändert: %s\n", modeNames[measurementMode]);
        }

        drawButtons();
      }
      delay(200);
      return;
    }
  }

  // Reset-Button
  if (x >= resetButton.x && x <= resetButton.x + resetButton.w &&
      y >= resetButton.y && y <= resetButton.y + resetButton.h) {
    resetStatistics();
    drawMainScreen();
    delay(200);
  }
}

// ========== Statistik zurücksetzen ==========
void resetStatistics() {
  eco2Min = 8192;
  eco2Max = 400;
  eco2Avg = 400;

  tvocMin = 1187;
  tvocMax = 0;
  tvocAvg = 0;

  totalMeasurements = 0;
  errorCount = 0;

  // Verlauf löschen
  for (int i = 0; i < MAX_HISTORY; i++) {
    eco2History[i] = 400;
    tvocHistory[i] = 0;
  }
  historyIndex = 0;

  Serial.println("Statistik zurückgesetzt");
}

// ========== Baseline speichern ==========
void saveBaseline() {
  if (!sensorAvailable) return;

  uint16_t baseline = ccs.getBaseline();

  EEPROM.write(EEPROM_BASELINE_ADDR, baseline >> 8);     // High Byte
  EEPROM.write(EEPROM_BASELINE_ADDR + 1, baseline & 0xFF); // Low Byte
  EEPROM.commit();

  Serial.printf("Baseline gespeichert: 0x%04X\n", baseline);
}

// ========== Baseline laden ==========
void loadBaseline() {
  uint8_t high = EEPROM.read(EEPROM_BASELINE_ADDR);
  uint8_t low = EEPROM.read(EEPROM_BASELINE_ADDR + 1);

  // Prüfen ob gültig (nicht 0x00 oder 0xFF)
  if (high == 0x00 && low == 0x00) {
    Serial.println("Keine Baseline gespeichert");
    return;
  }
  if (high == 0xFF && low == 0xFF) {
    Serial.println("EEPROM leer");
    return;
  }

  uint16_t baseline = (high << 8) | low;
  ccs.setBaseline(baseline);

  Serial.printf("Baseline geladen: 0x%04X\n", baseline);
}
