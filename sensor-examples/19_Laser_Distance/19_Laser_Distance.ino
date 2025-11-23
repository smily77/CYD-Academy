/*
  19_Laser_Distance - Laser-Distanzmesser mit R50-C ToF Sensor

  Dieses Beispiel zeigt eine professionelle Distanzmess-Anzeige mit dem
  Aero Selfie R50-C Laser Range Finder (ToF Time-of-Flight Sensor).

  Funktionen:
  - R50-C Laser Distance Sensor auslesen (I2C)
  - Live Distanz-Anzeige (bis 50 Meter)
  - Horizontaler Distanz-Balken
  - Scrollendes Verlaufs-Diagramm
  - Min/Max/Average Tracking
  - Mehrere Messmodi (Single, Continuous, Fast)
  - Genauigkeits-Anzeige
  - Historische Statistiken

  Lernziele:
  - I2C-Kommunikation (R50-C)
  - Time-of-Flight Technologie verstehen
  - Scrollendes Diagramm implementieren
  - Statistik-Berechnung (Min, Max, Avg)
  - Multiple Sensor-Modi
  - Daten-Visualisierung

  Hardware:
  - 1x CYD Display (2.8" oder 3.5")
  - 1x Aero Selfie R50-C Laser Range Finder
  - 4x Dupont-Kabel (VCC, GND, SDA, SCL)

  R50-C Verkabelung:
  - VCC → 5V (Sensor benötigt 5V!)
  - GND → GND
  - SDA → GPIO 22 (extSDA)
  - SCL → GPIO 27 (extSCL)

  Installation:
  1. R50-C Library installieren:
     HINWEIS: Keine offizielle Arduino Library verfügbar
     Wir nutzen direkte I2C-Kommunikation (einfach!)

  2. In CYD_Display_Config.h definieren (falls nicht vorhanden):
     #define extSDA 22
     #define extSCL 27

  Steuerung:
  - Touch Links: Vorheriger Modus
  - Touch Rechts: Nächster Modus
  - Touch Mitte: Reset Min/Max/Statistik

  Messmodi:
  1. Single Shot - Eine Messung per Touch
  2. Continuous - Kontinuierliche Messung (2 Hz)
  3. Fast - Schnelle Messung (10 Hz)
*/

#include <CYD_Display_Config.h>
#include <Wire.h>

// I2C Pins (CYD Standard - external I2C)
#ifndef extSDA
#define extSDA 22
#endif

#ifndef extSCL
#define extSCL 27
#endif

// R50-C I2C-Adresse
#define R50C_I2C_ADDR 0x52

// R50-C Register
#define R50C_REG_DISTANCE_H 0x00  // Distanz High Byte
#define R50C_REG_DISTANCE_L 0x01  // Distanz Low Byte
#define R50C_REG_MODE 0x02        // Modus-Register
#define R50C_REG_MEASURE 0x03     // Messung triggern

// R50-C Modi
#define R50C_MODE_SINGLE 0x00     // Single Shot
#define R50C_MODE_CONTINUOUS 0x01 // Kontinuierlich
#define R50C_MODE_FAST 0x02       // Schnell

// ===== KONSTANTEN =====

// Farben
#define COLOR_BG 0x0000          // Schwarz
#define COLOR_PANEL 0x1082       // Dunkelgrau
#define COLOR_TEXT 0xFFFF        // Weiß
#define COLOR_TEXT_DIM 0x7BEF    // Grau
#define COLOR_DISTANCE 0x07FF    // Cyan (Hauptanzeige)
#define COLOR_BAR_BG 0x2104      // Dunkelgrau
#define COLOR_BAR_CLOSE 0x07E0   // Grün (nah)
#define COLOR_BAR_MID 0xFFE0     // Gelb (mittel)
#define COLOR_BAR_FAR 0xF800     // Rot (weit)
#define COLOR_GRAPH_LINE 0x03EF  // Blau
#define COLOR_GRAPH_GRID 0x2104  // Dunkelgrau
#define COLOR_MIN 0x001F         // Blau (Min)
#define COLOR_MAX 0xF800         // Rot (Max)
#define COLOR_AVG 0xFFE0         // Gelb (Avg)

// Diagramm-Parameter
const int GRAPH_WIDTH = 220;     // Breite des Diagramms
const int GRAPH_HEIGHT = 80;     // Höhe des Diagramms
const int GRAPH_SAMPLES = 110;   // Anzahl Datenpunkte (2px pro Sample)

// Messmodi
enum MeasureMode {
  MODE_SINGLE,
  MODE_CONTINUOUS,
  MODE_FAST
};

// ===== GLOBALE VARIABLEN =====

LGFX lcd;

// Sensor-Status
bool sensorConnected = false;

// Distanz-Daten
uint16_t currentDistance = 0;    // cm
uint16_t minDistance = 65535;    // cm (init max)
uint16_t maxDistance = 0;        // cm
uint32_t avgSum = 0;             // Summe für Durchschnitt
uint32_t avgCount = 0;           // Anzahl Messungen

// Graph-Daten (Ring-Buffer)
uint16_t graphData[GRAPH_SAMPLES];
int graphIndex = 0;

// Modus
MeasureMode currentMode = MODE_CONTINUOUS;

// Touch & Timing
unsigned long lastTouchTime = 0;
unsigned long lastMeasureTime = 0;
unsigned long measureInterval = 500;  // ms (abhängig von Modus)

// Statistik
uint32_t totalMeasurements = 0;
uint32_t errorCount = 0;

// ===== R50-C FUNKTIONEN =====

bool initR50C() {
  Wire.begin(extSDA, extSCL);

  // Prüfe ob Sensor antwortet
  Wire.beginTransmission(R50C_I2C_ADDR);
  byte error = Wire.endTransmission();

  if (error != 0) {
    Serial.println("R50-C not found!");
    return false;
  }

  Serial.println("R50-C found!");

  // Setze initialen Modus
  setMode(R50C_MODE_CONTINUOUS);

  return true;
}

void setMode(uint8_t mode) {
  Wire.beginTransmission(R50C_I2C_ADDR);
  Wire.write(R50C_REG_MODE);
  Wire.write(mode);
  Wire.endTransmission();

  Serial.printf("R50-C Mode set to: %d\n", mode);
}

void triggerMeasurement() {
  // Nur für Single-Shot Modus
  Wire.beginTransmission(R50C_I2C_ADDR);
  Wire.write(R50C_REG_MEASURE);
  Wire.write(0x01);  // Trigger
  Wire.endTransmission();

  delay(50);  // Warte auf Messung
}

uint16_t readDistance() {
  Wire.beginTransmission(R50C_I2C_ADDR);
  Wire.write(R50C_REG_DISTANCE_H);
  Wire.endTransmission(false);  // Repeated start

  Wire.requestFrom(R50C_I2C_ADDR, 2);

  if (Wire.available() >= 2) {
    uint8_t highByte = Wire.read();
    uint8_t lowByte = Wire.read();

    uint16_t distance = (highByte << 8) | lowByte;  // cm

    // Plausibilitäts-Check (R50-C: max 5000cm = 50m)
    if (distance > 5000) {
      Serial.println("Invalid distance reading");
      errorCount++;
      return 0;
    }

    return distance;
  }

  errorCount++;
  return 0;
}

// ===== SETUP =====

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== R50-C Laser Distance Sensor Demo ===");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(0);  // Portrait
  lcd.setBrightness(255);
  lcd.fillScreen(COLOR_BG);

  // Splash Screen
  drawSplashScreen();
  delay(2000);

  // R50-C initialisieren
  lcd.fillScreen(COLOR_BG);
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(10, 140);
  lcd.print("Initializing");
  lcd.setCursor(10, 160);
  lcd.print("R50-C...");

  if (!initR50C()) {
    drawErrorScreen();
    sensorConnected = false;
    while (1) delay(1000);
  }

  sensorConnected = true;
  Serial.println("R50-C initialized!");

  // Graph mit Nullen initialisieren
  for (int i = 0; i < GRAPH_SAMPLES; i++) {
    graphData[i] = 0;
  }

  // Initiale Anzeige
  lcd.fillScreen(COLOR_BG);
  setMeasureInterval();

  Serial.println("Ready! Point sensor at target.");
}

// ===== LOOP =====

void loop() {
  if (!sensorConnected) {
    delay(1000);
    return;
  }

  // Messung abhängig von Modus
  unsigned long now = millis();

  if (currentMode == MODE_SINGLE) {
    // Single-Shot: Nur bei Touch
    // (Wird in handleTouch() getriggert)
  } else {
    // Continuous oder Fast: Regelmäßig messen
    if (now - lastMeasureTime >= measureInterval) {
      lastMeasureTime = now;
      performMeasurement();
    }
  }

  // Touch-Handling
  handleTouch();

  // UI zeichnen
  drawUI();

  delay(20);  // ~50 FPS
}

void performMeasurement() {
  if (currentMode == MODE_SINGLE) {
    triggerMeasurement();
  }

  currentDistance = readDistance();

  if (currentDistance > 0) {
    totalMeasurements++;

    // Aktualisiere Min/Max
    if (currentDistance < minDistance) {
      minDistance = currentDistance;
    }
    if (currentDistance > maxDistance) {
      maxDistance = currentDistance;
    }

    // Aktualisiere Average
    avgSum += currentDistance;
    avgCount++;

    // Füge zu Graph hinzu
    addToGraph(currentDistance);

    Serial.printf("Distance: %d cm (%.2f m)\n", currentDistance, currentDistance / 100.0);
  }
}

void addToGraph(uint16_t distance) {
  graphData[graphIndex] = distance;
  graphIndex = (graphIndex + 1) % GRAPH_SAMPLES;
}

void setMeasureInterval() {
  switch (currentMode) {
    case MODE_SINGLE:
      measureInterval = 0;  // Nicht genutzt
      setMode(R50C_MODE_SINGLE);
      break;
    case MODE_CONTINUOUS:
      measureInterval = 500;  // 2 Hz
      setMode(R50C_MODE_CONTINUOUS);
      break;
    case MODE_FAST:
      measureInterval = 100;  // 10 Hz
      setMode(R50C_MODE_FAST);
      break;
  }
}

// ===== TOUCH HANDLING =====

void handleTouch() {
  uint16_t x, y;
  if (!lcd.getTouch(&x, &y)) return;

  // Debouncing
  if (millis() - lastTouchTime < 300) return;
  lastTouchTime = millis();

  // Linker Bereich: Vorheriger Modus
  if (x < 80) {
    currentMode = (MeasureMode)((currentMode - 1 + 3) % 3);
    setMeasureInterval();
    Serial.printf("Mode changed to: %d\n", currentMode);
    lcd.fillScreen(COLOR_BG);
  }
  // Rechter Bereich: Nächster Modus
  else if (x > 160) {
    currentMode = (MeasureMode)((currentMode + 1) % 3);
    setMeasureInterval();
    Serial.printf("Mode changed to: %d\n", currentMode);
    lcd.fillScreen(COLOR_BG);
  }
  // Mitte: Reset oder Single-Messung
  else {
    if (currentMode == MODE_SINGLE) {
      // Trigger Messung
      performMeasurement();
    } else {
      // Reset Statistik
      minDistance = 65535;
      maxDistance = 0;
      avgSum = 0;
      avgCount = 0;
      totalMeasurements = 0;
      errorCount = 0;

      // Clear Graph
      for (int i = 0; i < GRAPH_SAMPLES; i++) {
        graphData[i] = 0;
      }
      graphIndex = 0;

      Serial.println("Statistics reset");

      lcd.fillRect(40, 140, 160, 40, COLOR_DISTANCE);
      lcd.setTextSize(2);
      lcd.setTextColor(COLOR_BG);
      lcd.setCursor(70, 155);
      lcd.print("RESET!");
      delay(500);
      lcd.fillScreen(COLOR_BG);
    }
  }
}

// ===== UI ZEICHNEN =====

void drawUI() {
  // Header
  drawHeader();

  // Haupt-Distanz-Anzeige
  drawMainDistance(10, 40);

  // Distanz-Balken
  drawDistanceBar(10, 100, 220, 30);

  // Graph
  drawGraph(10, 140);

  // Statistiken
  drawStatistics(10, 230);

  // Footer (Modus & Touch-Hilfe)
  drawFooter();
}

void drawHeader() {
  lcd.fillRect(0, 0, 240, 30, COLOR_PANEL);

  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(10, 8);
  lcd.print("LASER RANGE");

  // Status-LED (grün wenn OK)
  uint16_t statusColor = (errorCount == 0 || totalMeasurements > errorCount * 10) ?
                         COLOR_BAR_CLOSE : COLOR_BAR_FAR;
  lcd.fillCircle(220, 15, 6, statusColor);
}

void drawMainDistance(int x, int y) {
  // Hintergrund
  lcd.fillRect(x, y, 220, 50, COLOR_PANEL);

  // Distanz (groß)
  lcd.setTextSize(3);
  lcd.setTextColor(COLOR_DISTANCE);

  if (currentDistance > 0) {
    lcd.setCursor(x + 10, y + 15);

    if (currentDistance >= 1000) {
      // Meter-Anzeige (ab 10m)
      float meters = currentDistance / 100.0;
      lcd.printf("%.2f m", meters);
    } else {
      // cm-Anzeige (unter 10m)
      lcd.printf("%d cm", currentDistance);
    }
  } else {
    lcd.setTextColor(COLOR_TEXT_DIM);
    lcd.setCursor(x + 30, y + 15);
    lcd.print("--- cm");
  }
}

void drawDistanceBar(int x, int y, int width, int height) {
  // Hintergrund
  lcd.fillRect(x, y, width, height, COLOR_BAR_BG);

  if (currentDistance > 0) {
    // Berechne Balken-Länge (0-5000cm = 0-50m)
    int barLength = map(currentDistance, 0, 5000, 0, width - 4);
    barLength = constrain(barLength, 0, width - 4);

    // Farbe basierend auf Distanz
    uint16_t barColor;
    if (currentDistance < 1000) {
      barColor = COLOR_BAR_CLOSE;  // < 10m: Grün
    } else if (currentDistance < 3000) {
      barColor = COLOR_BAR_MID;    // 10-30m: Gelb
    } else {
      barColor = COLOR_BAR_FAR;    // > 30m: Rot
    }

    // Zeichne Balken
    lcd.fillRect(x + 2, y + 2, barLength, height - 4, barColor);
  }

  // Skala-Markierungen (10m, 20m, 30m, 40m, 50m)
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);

  for (int dist = 10; dist <= 50; dist += 10) {
    int markX = x + map(dist * 100, 0, 5000, 0, width);
    lcd.setCursor(markX - 8, y + height + 2);
    lcd.printf("%dm", dist);
  }
}

void drawGraph(int x, int y) {
  // Hintergrund
  lcd.fillRect(x, y, GRAPH_WIDTH, GRAPH_HEIGHT, COLOR_BG);

  // Grid-Linien (horizontal, alle 1000cm = 10m)
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_GRAPH_GRID);

  for (int dist = 0; dist <= 5000; dist += 1000) {
    int lineY = y + GRAPH_HEIGHT - map(dist, 0, 5000, 0, GRAPH_HEIGHT);
    lcd.drawLine(x, lineY, x + GRAPH_WIDTH, lineY, COLOR_GRAPH_GRID);

    // Label
    lcd.setCursor(x + 2, lineY - 8);
    lcd.printf("%dm", dist / 100);
  }

  // Zeichne Graph-Linie
  for (int i = 0; i < GRAPH_SAMPLES - 1; i++) {
    int idx1 = (graphIndex + i) % GRAPH_SAMPLES;
    int idx2 = (graphIndex + i + 1) % GRAPH_SAMPLES;

    uint16_t val1 = graphData[idx1];
    uint16_t val2 = graphData[idx2];

    if (val1 > 0 && val2 > 0) {
      int y1 = y + GRAPH_HEIGHT - map(val1, 0, 5000, 0, GRAPH_HEIGHT);
      int y2 = y + GRAPH_HEIGHT - map(val2, 0, 5000, 0, GRAPH_HEIGHT);

      int x1 = x + i * 2;
      int x2 = x + (i + 1) * 2;

      lcd.drawLine(x1, y1, x2, y2, COLOR_GRAPH_LINE);
    }
  }

  // Rahmen
  lcd.drawRect(x, y, GRAPH_WIDTH, GRAPH_HEIGHT, COLOR_TEXT_DIM);
}

void drawStatistics(int x, int y) {
  lcd.setTextSize(1);

  // Min
  lcd.setTextColor(COLOR_MIN);
  lcd.setCursor(x, y);
  lcd.print("Min:");
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(x + 30, y);
  if (minDistance < 65535) {
    lcd.printf("%d cm", minDistance);
  } else {
    lcd.print("---");
  }

  // Max
  lcd.setTextColor(COLOR_MAX);
  lcd.setCursor(x, y + 15);
  lcd.print("Max:");
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(x + 30, y + 15);
  if (maxDistance > 0) {
    lcd.printf("%d cm", maxDistance);
  } else {
    lcd.print("---");
  }

  // Average
  lcd.setTextColor(COLOR_AVG);
  lcd.setCursor(x, y + 30);
  lcd.print("Avg:");
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(x + 30, y + 30);
  if (avgCount > 0) {
    uint16_t avgDistance = avgSum / avgCount;
    lcd.printf("%d cm", avgDistance);
  } else {
    lcd.print("---");
  }

  // Messungen
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(x + 120, y);
  lcd.print("Samples:");
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(x + 175, y);
  lcd.printf("%lu", totalMeasurements);

  // Fehler
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(x + 120, y + 15);
  lcd.print("Errors:");
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(x + 175, y + 15);
  lcd.printf("%lu", errorCount);

  // Genauigkeit (%)
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(x + 120, y + 30);
  lcd.print("Accuracy:");
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(x + 185, y + 30);
  if (totalMeasurements > 0) {
    float accuracy = 100.0 * (totalMeasurements - errorCount) / totalMeasurements;
    lcd.printf("%.0f%%", accuracy);
  } else {
    lcd.print("--");
  }
}

void drawFooter() {
  lcd.fillRect(0, 280, 240, 40, COLOR_PANEL);

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT);

  // Modus-Anzeige
  lcd.setCursor(10, 285);
  lcd.print("Mode:");
  lcd.setCursor(45, 285);

  switch (currentMode) {
    case MODE_SINGLE:
      lcd.print("SINGLE");
      break;
    case MODE_CONTINUOUS:
      lcd.print("CONT (2Hz)");
      break;
    case MODE_FAST:
      lcd.print("FAST (10Hz)");
      break;
  }

  // Touch-Hilfe
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(10, 300);

  if (currentMode == MODE_SINGLE) {
    lcd.print("L/R:Mode  M:Measure");
  } else {
    lcd.print("L/R:Mode  M:Reset");
  }
}

// ===== HELPER FUNKTIONEN =====

void drawSplashScreen() {
  lcd.fillScreen(COLOR_BG);

  // Laser-Symbol (stilisiert)
  lcd.fillRect(100, 80, 40, 15, COLOR_BAR_FAR);  // Sensor
  lcd.fillTriangle(140, 85, 140, 90, 180, 87, COLOR_DISTANCE);  // Strahl

  // Ziel
  lcd.drawCircle(180, 87, 10, COLOR_TEXT);
  lcd.drawCircle(180, 87, 5, COLOR_TEXT);
  lcd.fillCircle(180, 87, 2, COLOR_BAR_FAR);

  // Titel
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(40, 150);
  lcd.print("LASER RANGE");
  lcd.setCursor(60, 170);
  lcd.print("FINDER");

  // Version
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(70, 210);
  lcd.print("Example 19");

  lcd.setCursor(50, 230);
  lcd.print("R50-C ToF Sensor");

  lcd.setCursor(60, 250);
  lcd.print("Max 50 Meter");
}

void drawErrorScreen() {
  lcd.fillScreen(COLOR_BG);

  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_BAR_FAR);
  lcd.setCursor(20, 100);
  lcd.print("R50-C Error!");

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT);

  lcd.setCursor(10, 140);
  lcd.print("Check wiring:");

  lcd.setCursor(10, 160);
  lcd.printf("SDA -> GPIO %d", extSDA);

  lcd.setCursor(10, 175);
  lcd.printf("SCL -> GPIO %d", extSCL);

  lcd.setCursor(10, 190);
  lcd.print("VCC -> 5V (NOT 3.3V!)");

  lcd.setCursor(10, 205);
  lcd.print("GND -> GND");

  lcd.setCursor(10, 225);
  lcd.print("I2C Address: 0x52");

  lcd.setCursor(10, 245);
  lcd.print("Check Serial Monitor");
  lcd.setCursor(10, 260);
  lcd.print("for details.");
}
