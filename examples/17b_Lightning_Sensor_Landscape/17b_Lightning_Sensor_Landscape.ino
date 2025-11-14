/*
  17b_Lightning_Sensor_Landscape - Blitzwetter-Anzeige (Landscape Layout)

  Optimierte Landscape-Version des Lightning Sensor Beispiels mit
  verbessertem Layout für horizontale Anzeige (320x240).

  Optimierungen v2:
  - Anti-Flacker: Nur ändern was sich ändert
  - Bessere Schriftdarstellung mit größeren Fonts
  - Selektives Redraw für flüssige Darstellung
  - Statische Elemente nur einmal zeichnen

  Funktionen:
  - AS3935 Lightning Sensor auslesen (I2C)
  - Optimiertes Landscape Layout (320x240)
  - Radar links, Info-Panel rechts
  - Blitz-Entfernung (bis 40km)
  - Blitz-Animation mit Sound-Effekt
  - Anzahl erkannter Blitze
  - Gefahrenstufe (Farbcodierung)
  - Störungs-Erkennung
  - Zeit seit letztem Blitz
  - Historische Blitz-Anzeige

  Hardware:
  - 1x CYD Display (2.8" oder 3.5")
  - 1x Gravity Lightning Sensor SEN0290 (AS3935)
  - 5x Dupont-Kabel (VCC, GND, SDA, SCL, IRQ)

  SEN0290 Verkabelung:
  - VCC → 3.3V
  - GND → GND
  - SDA → GPIO 22 (extSDA)
  - SCL → GPIO 27 (extSCL)
  - IRQ → GPIO (extIrqPin aus CYD_Display_Config.h - OPTIONAL)

  Steuerung:
  - Touch links: Reset Statistik
  - Touch rechts: Sensitivität ändern
*/

#include <CYD_Display_Config.h>
#include <Wire.h>
#include <DFRobot_AS3935_I2C.h>

// I2C Pins (CYD Standard - external I2C)
#ifndef extSDA
#define extSDA 22
#endif

#ifndef extSCL
#define extSCL 27
#endif

// Interrupt Pin für AS3935
#ifdef extIrqPin
  #define IRQ_PIN extIrqPin
  #define USE_INTERRUPT
#else
  #define IRQ_PIN -1
#endif

// AS3935 I2C-Adresse (Standard: 0x03)
#define AS3935_I2C_ADDR AS3935_ADD3

// ===== LAYOUT KONSTANTEN (Landscape 320x240) =====

#define HEADER_HEIGHT 25
#define FOOTER_HEIGHT 25
#define MAIN_HEIGHT (240 - HEADER_HEIGHT - FOOTER_HEIGHT)

// Radar (linke Seite)
#define RADAR_AREA_WIDTH 200
#define RADAR_CENTER_X 100
#define RADAR_CENTER_Y (HEADER_HEIGHT + MAIN_HEIGHT / 2)
#define RADAR_MAX_RADIUS 80

// Info-Panel (rechte Seite)
#define INFO_PANEL_X 200
#define INFO_PANEL_WIDTH (320 - INFO_PANEL_X)

// ===== FARBEN =====

#define COLOR_BG 0x0000
#define COLOR_RADAR_BG 0x0841
#define COLOR_RADAR_GRID 0x1082
#define COLOR_SAFE 0x07E0
#define COLOR_WARNING 0xFFE0
#define COLOR_DANGER 0xFD20
#define COLOR_CRITICAL 0xF800
#define COLOR_LIGHTNING 0xFFFF
#define COLOR_TEXT 0xFFFF
#define COLOR_TEXT_DIM 0x7BEF

// ===== SENSOR KONSTANTEN =====

const int DISTANCE_RINGS[] = {10, 20, 30, 40};

// Blitz-Historie
const int MAX_HISTORY = 20;
struct LightningEvent {
  uint8_t distance;
  unsigned long time;
  uint32_t energy;
};

// ===== GLOBALE VARIABLEN =====

LGFX lcd;
DFRobot_AS3935_I2C sensor((uint8_t)IRQ_PIN, AS3935_I2C_ADDR);

// Sensor-Status
bool sensorConnected = false;
volatile bool interruptDetected = false;

// Blitz-Daten
uint32_t totalLightningCount = 0;
uint8_t lastDistance = 0;
uint32_t lastEnergy = 0;
unsigned long lastLightningTime = 0;
uint32_t disturbanceCount = 0;
uint32_t noiseCount = 0;

// Blitz-Animation
bool lightningAnimation = false;
unsigned long animationStartTime = 0;
const int ANIMATION_DURATION = 1000;

// Historie
LightningEvent history[MAX_HISTORY];
int historyIndex = 0;
int historyCount = 0;

// Touch-Debouncing
unsigned long lastTouchTime = 0;

// Sensor-Einstellungen
uint8_t sensitivity = 2;

// ===== ANTI-FLACKER VARIABLEN =====

// Letzte angezeigte Werte (für selektives Redraw)
uint8_t lastDisplayedDistance = 255;
uint32_t lastDisplayedCount = 0xFFFFFFFF;
String lastDisplayedStatus = "";
String lastDisplayedLast = "";
uint32_t lastDisplayedNoise = 0xFFFFFFFF;
uint32_t lastDisplayedDisturb = 0xFFFFFFFF;
uint8_t lastDisplayedSensitivity = 255;

// Redraw-Flags
bool radarNeedsFullRedraw = true;
bool infoPanelNeedsFullRedraw = true;
bool footerNeedsRedraw = true;

// Letzter Radar-Update
unsigned long lastRadarUpdate = 0;
const int RADAR_UPDATE_INTERVAL = 200;  // Radar alle 200ms aktualisieren

// ===== HILFSFUNKTIONEN =====

void scanI2C() {
  Serial.println("\n=== I2C Scanner ===");
  Serial.printf("Scanning I2C bus (SDA=%d, SCL=%d)...\n", extSDA, extSCL);

  byte count = 0;

  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.printf("✓ Device found at address 0x%02X\n", addr);
      count++;

      if (addr == AS3935_ADD1) {
        Serial.println("  → AS3935_ADD1 (A0=high, A1=low)");
      } else if (addr == AS3935_ADD2) {
        Serial.println("  → AS3935_ADD2 (A0=low, A1=high)");
      } else if (addr == AS3935_ADD3) {
        Serial.println("  → AS3935_ADD3 (A0=high, A1=high)");
      }
    }
  }

  if (count == 0) {
    Serial.println("✗ No I2C devices found!");
    Serial.println("  Check wiring:");
    Serial.printf("  - SDA -> GPIO %d\n", extSDA);
    Serial.printf("  - SCL -> GPIO %d\n", extSCL);
    Serial.println("  - VCC -> 3.3V");
    Serial.println("  - GND -> GND");
  } else {
    Serial.printf("\n✓ Found %d device(s)\n", count);
  }
  Serial.println("==================\n");
}

// ===== INTERRUPT HANDLER =====

#ifdef USE_INTERRUPT
void IRAM_ATTR lightningISR() {
  interruptDetected = true;
}
#endif

// ===== SETUP =====

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Lightning Sensor Demo (Landscape v2) ===");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);  // Landscape
  lcd.setBrightness(255);
  lcd.fillScreen(COLOR_BG);

  // Splash Screen
  drawSplashScreen();
  delay(2000);

  // I2C initialisieren
  Serial.printf("Initializing I2C (SDA=%d, SCL=%d)...\n", extSDA, extSCL);
  Wire.begin(extSDA, extSCL);

  scanI2C();

  #ifdef USE_INTERRUPT
    pinMode(IRQ_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(IRQ_PIN), lightningISR, RISING);
    Serial.printf("Using interrupt mode on pin %d\n", IRQ_PIN);
  #else
    Serial.println("No interrupt pin available - using polling mode");
  #endif

  // AS3935 initialisieren
  lcd.fillScreen(COLOR_BG);
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setTextDatum(MC_DATUM);
  lcd.drawString("Initializing", 160, 100);
  lcd.drawString("AS3935...", 160, 120);

  Serial.printf("Attempting to initialize AS3935 at address 0x%02X...\n", AS3935_I2C_ADDR);

  int attempts = 0;
  while (!sensor.begin()) {
    attempts++;
    Serial.printf("AS3935 initialization failed (attempt %d)!\n", attempts);

    if (attempts >= 5) {
      Serial.println("✗ AS3935 not responding after 5 attempts!");
      Serial.println("\nPossible issues:");
      Serial.println("1. Wrong I2C address - try changing AS3935_I2C_ADDR");
      Serial.println("   Current: 0x03 (AS3935_ADD3)");
      Serial.println("   Try: 0x01 (AS3935_ADD1) or 0x02 (AS3935_ADD2)");
      Serial.println("2. Sensor not properly connected");
      Serial.println("3. Sensor needs more power-up time");
      Serial.println("\nEntering demo mode (no real sensor data)...");
      sensorConnected = false;
      break;
    }

    lcd.fillScreen(COLOR_BG);
    drawErrorScreen();
    delay(1000);
  }

  if (sensorConnected) {
    Serial.println("✓ AS3935 initialized successfully!");

    sensor.manualCal(96, 64, 64);
    sensor.setNoiseFloorLvl(2);
    sensor.setWatchdogThreshold(sensitivity);
    sensor.setIndoors();
    sensor.disturberEn();

    Serial.println("✓ AS3935 configured!");
    Serial.printf("  Sensitivity: %d/7\n", sensitivity);
    Serial.printf("  I2C Address: 0x%02X\n", AS3935_I2C_ADDR);
    Serial.printf("  Mode: Indoor\n");
  } else {
    Serial.println("⚠ Running in demo mode (sensor not available)");
  }

  // Initiale Anzeige
  lcd.fillScreen(COLOR_BG);
  drawStaticElements();  // Zeichne statische Elemente einmal
  drawUI();

  Serial.println("Ready! Waiting for lightning...");
}

// ===== LOOP =====

void loop() {
  // Sensor auslesen
  if (sensorConnected) {
    #ifdef USE_INTERRUPT
      if (interruptDetected) {
        interruptDetected = false;
        handleLightningInterrupt();
      }
    #else
      static unsigned long lastPollTime = 0;
      if (millis() - lastPollTime >= 100) {
        lastPollTime = millis();
        uint8_t intSource = sensor.getInterruptSrc();
        if (intSource != 0) {
          handleLightningInterrupt();
        }
      }
    #endif
  }

  // Blitz-Animation Update
  if (lightningAnimation) {
    unsigned long elapsed = millis() - animationStartTime;
    if (elapsed > ANIMATION_DURATION) {
      lightningAnimation = false;
      radarNeedsFullRedraw = true;  // Radar neu zeichnen nach Animation
    }
  }

  // UI aktualisieren (nur was sich geändert hat)
  drawUI();

  // Touch-Handling
  handleTouch();

  delay(50);  // 20 FPS
}

// ===== SENSOR FUNKTIONEN =====

void handleLightningInterrupt() {
  delay(2);

  uint8_t intSource = sensor.getInterruptSrc();
  Serial.printf("Interrupt Source: %d\n", intSource);

  switch (intSource) {
    case 1:  // Störung
      Serial.println("Disturber detected");
      disturbanceCount++;
      infoPanelNeedsFullRedraw = true;
      break;

    case 2:  // Rauschen
      Serial.println("Noise level too high");
      noiseCount++;
      infoPanelNeedsFullRedraw = true;
      break;

    case 3:  // Blitz erkannt!
      Serial.println("⚡ LIGHTNING DETECTED! ⚡");

      lastDistance = sensor.getLightningDistKm();
      lastEnergy = sensor.getStrikeEnergyRaw();
      lastLightningTime = millis();
      totalLightningCount++;

      addToHistory(lastDistance, lastEnergy);

      lightningAnimation = true;
      animationStartTime = millis();
      radarNeedsFullRedraw = true;
      infoPanelNeedsFullRedraw = true;

      Serial.printf("Distance: %d km\n", lastDistance);
      Serial.printf("Energy: %lu\n", lastEnergy);
      Serial.printf("Total Count: %lu\n", totalLightningCount);
      break;

    default:
      Serial.printf("Unknown interrupt: %d\n", intSource);
      break;
  }
}

void addToHistory(uint8_t distance, uint32_t energy) {
  history[historyIndex].distance = distance;
  history[historyIndex].time = millis();
  history[historyIndex].energy = energy;

  historyIndex = (historyIndex + 1) % MAX_HISTORY;

  if (historyCount < MAX_HISTORY) {
    historyCount++;
  }
}

// ===== TOUCH HANDLING =====

void handleTouch() {
  uint16_t x, y;
  if (!lcd.getTouch(&x, &y)) return;

  if (millis() - lastTouchTime < 500) return;
  lastTouchTime = millis();

  // Links: Reset
  if (x < 160) {
    Serial.println("Resetting statistics...");
    totalLightningCount = 0;
    disturbanceCount = 0;
    noiseCount = 0;
    historyCount = 0;
    historyIndex = 0;
    lastDistance = 0;
    lastEnergy = 0;

    // Feedback
    lcd.fillRect(50, 100, 100, 40, COLOR_SAFE);
    lcd.setTextDatum(MC_DATUM);
    lcd.setTextSize(2);
    lcd.setTextColor(COLOR_BG);
    lcd.drawString("RESET!", 100, 120);
    delay(500);

    radarNeedsFullRedraw = true;
    infoPanelNeedsFullRedraw = true;
  }
  // Rechts: Sensitivität
  else {
    sensitivity = (sensitivity + 1) % 8;
    if (sensorConnected) {
      sensor.setWatchdogThreshold(sensitivity);
    }

    Serial.printf("Sensitivity changed to: %d/7\n", sensitivity);

    // Feedback
    lcd.fillRect(200, 100, 120, 40, COLOR_WARNING);
    lcd.setTextDatum(MC_DATUM);
    lcd.setTextSize(2);
    lcd.setTextColor(COLOR_BG);
    lcd.drawString("Sens: " + String(sensitivity) + "/7", 260, 120);
    delay(500);

    footerNeedsRedraw = true;
    infoPanelNeedsFullRedraw = true;
  }
}

// ===== UI ZEICHNEN =====

// Statische Elemente (nur einmal zeichnen)
void drawStaticElements() {
  drawHeader();
  drawRadarBackground();
  drawFooter();
}

void drawUI() {
  // Radar nur alle 200ms oder bei Bedarf aktualisieren
  if (radarNeedsFullRedraw || (millis() - lastRadarUpdate >= RADAR_UPDATE_INTERVAL)) {
    drawRadar();
    lastRadarUpdate = millis();
  }

  // Blitz-Animation
  if (lightningAnimation) {
    drawLightningAnimation();
  }

  // Info-Panel (selektiv)
  drawInfoPanel();

  // Footer nur bei Bedarf
  if (footerNeedsRedraw) {
    drawFooter();
    footerNeedsRedraw = false;
  }
}

void drawHeader() {
  lcd.fillRect(0, 0, 320, HEADER_HEIGHT, COLOR_RADAR_BG);

  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_LIGHTNING);
  lcd.setTextDatum(TL_DATUM);
  lcd.drawString("LIGHTNING RADAR", 30, 5);

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setTextDatum(TR_DATUM);
  lcd.drawString("17b", 315, 9);

  lcd.drawLine(0, HEADER_HEIGHT, 320, HEADER_HEIGHT, COLOR_RADAR_GRID);
}

void drawRadarBackground() {
  // Radar-Hintergrund
  lcd.fillRect(0, HEADER_HEIGHT, RADAR_AREA_WIDTH, MAIN_HEIGHT, COLOR_RADAR_BG);

  // Konzentrische Kreise
  for (int i = 0; i < 4; i++) {
    int distKm = DISTANCE_RINGS[i];
    int radius = map(distKm, 0, 40, 0, RADAR_MAX_RADIUS);

    lcd.drawCircle(RADAR_CENTER_X, RADAR_CENTER_Y, radius, COLOR_RADAR_GRID);

    // Labels
    lcd.setTextSize(1);
    lcd.setTextColor(COLOR_TEXT_DIM);
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString(String(distKm) + "km", RADAR_CENTER_X + radius - 12, RADAR_CENTER_Y);
  }

  // Standort-Marker
  lcd.drawLine(RADAR_CENTER_X - 5, RADAR_CENTER_Y, RADAR_CENTER_X + 5, RADAR_CENTER_Y, COLOR_SAFE);
  lcd.drawLine(RADAR_CENTER_X, RADAR_CENTER_Y - 5, RADAR_CENTER_X, RADAR_CENTER_Y + 5, COLOR_SAFE);
  lcd.fillCircle(RADAR_CENTER_X, RADAR_CENTER_Y, 3, COLOR_SAFE);

  // Trennlinie
  lcd.drawLine(RADAR_AREA_WIDTH, HEADER_HEIGHT, RADAR_AREA_WIDTH, 240 - FOOTER_HEIGHT, COLOR_RADAR_GRID);
}

void drawRadar() {
  // Nur dynamischen Teil neu zeichnen
  if (radarNeedsFullRedraw) {
    drawRadarBackground();
    radarNeedsFullRedraw = false;
  }

  // Historische Blitze
  drawHistoricalLightning();

  // Letzter Blitz
  if (lastDistance > 0 && totalLightningCount > 0) {
    drawLightningOnRadar(lastDistance, millis() - lastLightningTime < 5000);
  }
}

void drawHistoricalLightning() {
  unsigned long now = millis();

  for (int i = 0; i < historyCount; i++) {
    int idx = (historyIndex - 1 - i + MAX_HISTORY) % MAX_HISTORY;
    if (idx < 0 || idx >= MAX_HISTORY) continue;

    LightningEvent& event = history[idx];
    unsigned long age = now - event.time;

    if (age > 300000) continue;  // 5 Minuten

    uint8_t alpha = 255 - (age * 255 / 300000);
    uint16_t color = getDistanceColor(event.distance);

    if (alpha < 128) {
      color = COLOR_TEXT_DIM;
    }

    int radius = map(event.distance, 0, 40, 0, RADAR_MAX_RADIUS);
    float angle = (event.time % 360) * 0.01745;

    int x = RADAR_CENTER_X + radius * cos(angle);
    int y = RADAR_CENTER_Y + radius * sin(angle);

    lcd.fillCircle(x, y, 2, color);
  }
}

void drawLightningOnRadar(uint8_t distance, bool highlight) {
  if (distance == 0 || distance > 40) return;

  int radius = map(distance, 0, 40, 0, RADAR_MAX_RADIUS);
  uint16_t color = getDistanceColor(distance);
  float angle = (lastLightningTime % 360) * 0.01745;

  int x = RADAR_CENTER_X + radius * cos(angle);
  int y = RADAR_CENTER_Y + radius * sin(angle);

  if (highlight) {
    int pulseSize = 5 + (millis() / 100) % 3;
    lcd.fillCircle(x, y, pulseSize, color);
    lcd.drawCircle(x, y, pulseSize + 2, COLOR_LIGHTNING);
    lcd.drawLine(RADAR_CENTER_X, RADAR_CENTER_Y, x, y, color);
  } else {
    lcd.fillCircle(x, y, 4, color);
  }
}

void drawLightningAnimation() {
  unsigned long elapsed = millis() - animationStartTime;

  if (elapsed < 200 && (elapsed / 50) % 2 == 0) {
    lcd.drawRect(0, HEADER_HEIGHT, RADAR_AREA_WIDTH, MAIN_HEIGHT, COLOR_LIGHTNING);
    lcd.drawRect(1, HEADER_HEIGHT + 1, RADAR_AREA_WIDTH - 2, MAIN_HEIGHT - 2, COLOR_LIGHTNING);
  }

  if (elapsed < 500) {
    drawLightningBolt(RADAR_CENTER_X, RADAR_CENTER_Y - 30);
  }
}

void drawLightningBolt(int x, int y) {
  // Blitz-Symbol
  lcd.fillTriangle(x, y, x - 5, y + 10, x + 2, y + 12, COLOR_LIGHTNING);
  lcd.fillTriangle(x + 2, y + 12, x - 3, y + 22, x + 5, y + 15, COLOR_LIGHTNING);
  lcd.fillTriangle(x + 5, y + 15, x + 3, y + 28, x + 12, y + 20, COLOR_LIGHTNING);
}

void drawInfoPanel() {
  int x = INFO_PANEL_X + 10;
  int y = HEADER_HEIGHT + 10;

  // Nur neu zeichnen wenn sich Werte geändert haben

  // ===== ENTFERNUNG =====
  if (lastDisplayedDistance != lastDistance || infoPanelNeedsFullRedraw) {
    // Bereich löschen
    lcd.fillRect(INFO_PANEL_X, y - 5, INFO_PANEL_WIDTH, 50, COLOR_BG);

    lcd.setTextSize(1);
    lcd.setTextColor(COLOR_TEXT_DIM);
    lcd.setTextDatum(TL_DATUM);
    lcd.drawString("Distance:", x, y);

    y += 12;
    if (lastDistance > 0 && totalLightningCount > 0) {
      lcd.setTextSize(4);
      lcd.setTextColor(COLOR_TEXT);
      lcd.setTextDatum(TL_DATUM);
      lcd.drawString(String(lastDistance), x, y);

      lcd.setTextSize(2);
      lcd.setTextDatum(TL_DATUM);
      lcd.drawString("km", x + 45, y + 12);
    } else {
      lcd.setTextSize(1);
      lcd.setTextColor(COLOR_TEXT);
      lcd.setTextDatum(TL_DATUM);
      lcd.drawString("No data", x, y);
    }

    lastDisplayedDistance = lastDistance;
  }

  // ===== STATUS =====
  y = HEADER_HEIGHT + 65;
  String currentStatus;
  if (lastDistance == 0 || totalLightningCount == 0) {
    currentStatus = "Safe";
  } else if (lastDistance <= 10) {
    currentStatus = "CRITICAL!";
  } else if (lastDistance <= 20) {
    currentStatus = "Danger";
  } else if (lastDistance <= 30) {
    currentStatus = "Warning";
  } else {
    currentStatus = "Caution";
  }

  if (lastDisplayedStatus != currentStatus || infoPanelNeedsFullRedraw) {
    lcd.fillRect(INFO_PANEL_X, y - 5, INFO_PANEL_WIDTH, 30, COLOR_BG);

    lcd.setTextSize(1);
    lcd.setTextColor(COLOR_TEXT_DIM);
    lcd.setTextDatum(TL_DATUM);
    lcd.drawString("Status:", x, y);

    y += 12;
    lcd.setTextSize(2);
    lcd.setTextColor(getDistanceColor(lastDistance));
    lcd.setTextDatum(TL_DATUM);
    lcd.drawString(currentStatus, x, y);

    lastDisplayedStatus = currentStatus;
  }

  // ===== COUNT =====
  y = HEADER_HEIGHT + 100;
  if (lastDisplayedCount != totalLightningCount || infoPanelNeedsFullRedraw) {
    lcd.fillRect(INFO_PANEL_X, y - 5, INFO_PANEL_WIDTH, 35, COLOR_BG);

    lcd.setTextSize(1);
    lcd.setTextColor(COLOR_TEXT_DIM);
    lcd.setTextDatum(TL_DATUM);
    lcd.drawString("Count:", x, y);

    y += 12;
    lcd.setTextSize(3);
    lcd.setTextColor(COLOR_TEXT);
    lcd.setTextDatum(TL_DATUM);
    lcd.drawString(String(totalLightningCount), x, y);

    lastDisplayedCount = totalLightningCount;
  }

  // ===== LAST =====
  y = HEADER_HEIGHT + 140;
  String currentLast;
  if (totalLightningCount > 0) {
    unsigned long elapsed = (millis() - lastLightningTime) / 1000;
    if (elapsed < 60) {
      currentLast = String(elapsed) + "s ago";
    } else if (elapsed < 3600) {
      currentLast = String(elapsed / 60) + "m ago";
    } else {
      currentLast = String(elapsed / 3600) + "h ago";
    }
  } else {
    currentLast = "Never";
  }

  // Update alle 5 Sekunden oder bei Änderung
  static unsigned long lastTimeUpdate = 0;
  if (lastDisplayedLast != currentLast || infoPanelNeedsFullRedraw || millis() - lastTimeUpdate > 5000) {
    lcd.fillRect(INFO_PANEL_X, y - 5, INFO_PANEL_WIDTH, 20, COLOR_BG);

    lcd.setTextSize(1);
    lcd.setTextColor(COLOR_TEXT_DIM);
    lcd.setTextDatum(TL_DATUM);
    lcd.drawString("Last:", x, y);

    y += 12;
    lcd.setTextColor(COLOR_TEXT);
    lcd.setTextDatum(TL_DATUM);
    lcd.drawString(currentLast, x, y);

    lastDisplayedLast = currentLast;
    lastTimeUpdate = millis();
  }

  // ===== STATISTIK =====
  y = HEADER_HEIGHT + 165;
  if (lastDisplayedNoise != noiseCount || lastDisplayedDisturb != disturbanceCount || infoPanelNeedsFullRedraw) {
    lcd.fillRect(INFO_PANEL_X, y - 5, INFO_PANEL_WIDTH, 25, COLOR_BG);

    lcd.setTextSize(1);
    lcd.setTextColor(COLOR_TEXT_DIM);
    lcd.setTextDatum(TL_DATUM);
    lcd.drawString("Noise: " + String(noiseCount), x, y);
    lcd.drawString("Disturb: " + String(disturbanceCount), x, y + 12);

    lastDisplayedNoise = noiseCount;
    lastDisplayedDisturb = disturbanceCount;
  }

  infoPanelNeedsFullRedraw = false;
}

void drawFooter() {
  lcd.fillRect(0, 240 - FOOTER_HEIGHT, 320, FOOTER_HEIGHT, 0x2104);

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setTextDatum(TL_DATUM);
  lcd.drawString("Sensitivity: " + String(sensitivity) + "/7", 5, 217);

  lcd.setTextColor(0x5ACB);
  lcd.setTextDatum(TR_DATUM);
  lcd.drawString("Left:Reset  Right:Sens", 315, 217);
}

uint16_t getDistanceColor(uint8_t distance) {
  if (distance == 0) return COLOR_SAFE;
  if (distance <= 10) return COLOR_CRITICAL;
  if (distance <= 20) return COLOR_DANGER;
  if (distance <= 30) return COLOR_WARNING;
  return COLOR_SAFE;
}

// ===== HELPER FUNKTIONEN =====

void drawSplashScreen() {
  lcd.fillScreen(COLOR_RADAR_BG);

  drawLightningBolt(160, 80);

  lcd.setTextSize(3);
  lcd.setTextColor(COLOR_LIGHTNING);
  lcd.setTextDatum(MC_DATUM);
  lcd.drawString("LIGHTNING", 160, 130);

  lcd.setTextSize(2);
  lcd.drawString("SENSOR", 160, 160);

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.drawString("Example 17b", 160, 185);
  lcd.drawString("Landscape Layout v2", 160, 200);
  lcd.drawString("SEN0290 / AS3935", 160, 215);
}

void drawErrorScreen() {
  lcd.fillScreen(COLOR_BG);

  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_CRITICAL);
  lcd.setTextDatum(MC_DATUM);
  lcd.drawString("AS3935 Error!", 160, 60);

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setTextDatum(TL_DATUM);

  int y = 100;
  lcd.drawString("Check wiring:", 10, y);
  y += 20;
  lcd.drawString("SDA -> GPIO " + String(extSDA), 10, y);
  y += 15;
  lcd.drawString("SCL -> GPIO " + String(extSCL), 10, y);
  y += 15;
  lcd.drawString("IRQ -> GPIO " + String(IRQ_PIN) + " (optional)", 10, y);
  y += 15;
  lcd.drawString("VCC -> 3.3V", 10, y);
  y += 15;
  lcd.drawString("GND -> GND", 10, y);
  y += 20;
  lcd.drawString("I2C Address: 0x" + String(AS3935_I2C_ADDR, HEX), 10, y);
  y += 20;
  lcd.drawString("Check Serial Monitor for details.", 10, y);
}
