/*
  17b_Lightning_Sensor_Landscape - Blitzwetter-Anzeige (Landscape Layout)

  Optimierte Landscape-Version des Lightning Sensor Beispiels mit
  verbessertem Layout für horizontale Anzeige (320x240).

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

  Unterschiede zu 17:
  - Display-Rotation: 1 (Landscape statt Portrait)
  - Optimiertes 2-Spalten Layout
  - Radar auf der linken Seite
  - Kompaktes Info-Panel rechts
  - Bessere Platznutzung

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

  HINWEIS: Der Interrupt-Pin ist optional!
  - Mit extIrqPin: Schneller Interrupt-Modus
  - Ohne extIrqPin: Automatischer Polling-Modus (100ms Intervall)

  Installation:
  1. DFRobot AS3935 Library installieren:
     Arduino IDE: Library Manager → "DFRobot AS3935" → installieren
     PlatformIO: lib_deps = dfrobot/DFRobot_AS3935_I2C @ ^1.0.2

  2. In CYD_Display_Config.h definieren (falls nicht vorhanden):
     #define extSDA 22
     #define extSCL 27

  3. Optional in CYD_Display_Config.h für Interrupt-Modus:
     #define extIrqPin 35  // Oder ein anderer freier Pin

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

// Interrupt Pin für AS3935 (aus CYD_Display_Config.h)
// Falls nicht definiert → Polling-Modus
#ifdef extIrqPin
  #define IRQ_PIN extIrqPin
  #define USE_INTERRUPT
#else
  #define IRQ_PIN -1  // Kein Interrupt verfügbar
#endif

// AS3935 I2C-Adresse (Standard: 0x03)
// Mögliche Adressen: 0x01, 0x02, 0x03 (je nach A0/A1 Jumper)
#define AS3935_I2C_ADDR AS3935_ADD3  // 0x03 (Standard)

// ===== LAYOUT KONSTANTEN (Landscape 320x240) =====

// Bereiche
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

#define COLOR_BG 0x0000          // Schwarz
#define COLOR_RADAR_BG 0x0841    // Dunkelblau
#define COLOR_RADAR_GRID 0x1082  // Blaugrau
#define COLOR_SAFE 0x07E0        // Grün (Sicher)
#define COLOR_WARNING 0xFFE0     // Gelb (Warnung)
#define COLOR_DANGER 0xFD20      // Orange (Gefahr)
#define COLOR_CRITICAL 0xF800    // Rot (Kritisch)
#define COLOR_LIGHTNING 0xFFFF   // Weiß (Blitz)
#define COLOR_TEXT 0xFFFF        // Weiß
#define COLOR_TEXT_DIM 0x7BEF    // Grau

// ===== SENSOR KONSTANTEN =====

const int DISTANCE_RINGS[] = {10, 20, 30, 40};  // km

// Blitz-Historie (letzte 20 Blitze)
const int MAX_HISTORY = 20;
struct LightningEvent {
  uint8_t distance;      // km
  unsigned long time;    // millis()
  uint32_t energy;       // Energie
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
const int ANIMATION_DURATION = 1000;  // ms

// Historie
LightningEvent history[MAX_HISTORY];
int historyIndex = 0;
int historyCount = 0;

// Touch-Debouncing
unsigned long lastTouchTime = 0;

// Sensor-Einstellungen
uint8_t sensitivity = 2;  // 0-7 (höher = empfindlicher)

// ===== HILFSFUNKTIONEN =====

// I2C Scanner - Scannt alle Geräte auf dem Bus
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

      // Zeige AS3935-spezifische Adressen
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
  Serial.println("\n=== Lightning Sensor Demo (Landscape) ===");

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

  // I2C Scanner ausführen (Debug)
  scanI2C();

  // Interrupt Pin konfigurieren (falls verfügbar)
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
  lcd.setCursor(80, 100);
  lcd.print("Initializing");
  lcd.setCursor(80, 120);
  lcd.print("AS3935...");

  // Sensor starten mit I2C-Adresse
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

    // Sensor konfigurieren
    sensor.manualCal(96, 64, 64);  // Kalibrierung (Indoor)

    // Störungsunterdrückung (0-10, höher = mehr Unterdrückung)
    sensor.setNoiseFloorLvl(2);

    // Empfindlichkeit (0-7, höher = empfindlicher)
    sensor.setWatchdogThreshold(sensitivity);

    // Indoor/Outdoor (0 = Outdoor, 1 = Indoor)
    sensor.setIndoors();

    // Störungs-Maskierung (0-10 Minuten)
    sensor.disturberEn();  // Störungen melden

    Serial.println("✓ AS3935 configured!");
    Serial.printf("  Sensitivity: %d/7\n", sensitivity);
    Serial.printf("  I2C Address: 0x%02X\n", AS3935_I2C_ADDR);
    Serial.printf("  Mode: Indoor\n");
  } else {
    Serial.println("⚠ Running in demo mode (sensor not available)");
  }

  // Initiale Anzeige
  lcd.fillScreen(COLOR_BG);
  drawUI();

  Serial.println("Ready! Waiting for lightning...");
}

// ===== LOOP =====

void loop() {
  // Nur Sensor auslesen wenn verbunden
  if (sensorConnected) {
    #ifdef USE_INTERRUPT
      // Interrupt-basierter Modus
      if (interruptDetected) {
        interruptDetected = false;
        handleLightningInterrupt();
      }
    #else
      // Polling-Modus (alle 100ms prüfen)
      static unsigned long lastPollTime = 0;
      if (millis() - lastPollTime >= 100) {
        lastPollTime = millis();

        // Prüfe ob Interrupt-Quelle verfügbar ist
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
    }
  }

  // UI aktualisieren
  drawUI();

  // Touch-Handling
  handleTouch();

  delay(50);  // 20 FPS
}

// ===== SENSOR FUNKTIONEN =====

void handleLightningInterrupt() {
  delay(2);  // Kurz warten (Datasheet Empfehlung)

  uint8_t intSource = sensor.getInterruptSrc();

  Serial.printf("Interrupt Source: %d\n", intSource);

  switch (intSource) {
    case 1:  // Störung erkannt
      Serial.println("Disturber detected");
      disturbanceCount++;
      break;

    case 2:  // Rauschen erkannt
      Serial.println("Noise level too high");
      noiseCount++;
      break;

    case 3:  // Blitz erkannt!
      Serial.println("⚡ LIGHTNING DETECTED! ⚡");

      // Entfernung lesen (km)
      lastDistance = sensor.getLightningDistKm();

      // Energie lesen
      lastEnergy = sensor.getStrikeEnergyRaw();

      // Zeit merken
      lastLightningTime = millis();

      // Zähler erhöhen
      totalLightningCount++;

      // Historie hinzufügen
      addToHistory(lastDistance, lastEnergy);

      // Animation starten
      lightningAnimation = true;
      animationStartTime = millis();

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

  // Debouncing
  if (millis() - lastTouchTime < 500) return;
  lastTouchTime = millis();

  // Linke Seite: Reset Statistik
  if (x < 160) {
    Serial.println("Resetting statistics...");
    totalLightningCount = 0;
    disturbanceCount = 0;
    noiseCount = 0;
    historyCount = 0;
    historyIndex = 0;
    lastDistance = 0;
    lastEnergy = 0;

    lcd.fillRect(50, 100, 100, 40, COLOR_SAFE);
    lcd.setTextSize(2);
    lcd.setTextColor(COLOR_BG);
    lcd.setCursor(65, 115);
    lcd.print("RESET!");
    delay(500);
  }
  // Rechte Seite: Sensitivität ändern
  else {
    sensitivity = (sensitivity + 1) % 8;
    if (sensorConnected) {
      sensor.setWatchdogThreshold(sensitivity);
    }

    Serial.printf("Sensitivity changed to: %d/7\n", sensitivity);

    lcd.fillRect(200, 100, 120, 40, COLOR_WARNING);
    lcd.setTextSize(1);
    lcd.setTextColor(COLOR_BG);
    lcd.setCursor(210, 115);
    lcd.printf("Sens: %d/7", sensitivity);
    delay(500);
  }
}

// ===== UI ZEICHNEN =====

void drawUI() {
  // Header
  drawHeader();

  // Radar-Anzeige (links)
  drawRadar();

  // Blitz-Animation (falls aktiv)
  if (lightningAnimation) {
    drawLightningAnimation();
  }

  // Info-Panel (rechts)
  drawInfoPanel();

  // Footer
  drawFooter();
}

void drawHeader() {
  // Header-Hintergrund
  lcd.fillRect(0, 0, 320, HEADER_HEIGHT, COLOR_RADAR_BG);

  // Titel
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_LIGHTNING);
  lcd.setCursor(30, 5);
  lcd.print("LIGHTNING RADAR");

  // Version (klein rechts)
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(265, 9);
  lcd.print("17b");

  // Trennlinie
  lcd.drawLine(0, HEADER_HEIGHT, 320, HEADER_HEIGHT, COLOR_RADAR_GRID);
}

void drawRadar() {
  // Radar-Hintergrund (linker Bereich)
  lcd.fillRect(0, HEADER_HEIGHT, RADAR_AREA_WIDTH, MAIN_HEIGHT, COLOR_RADAR_BG);

  // Konzentrische Kreise (Entfernungsringe)
  for (int i = 0; i < 4; i++) {
    int distKm = DISTANCE_RINGS[i];
    int radius = map(distKm, 0, 40, 0, RADAR_MAX_RADIUS);

    lcd.drawCircle(RADAR_CENTER_X, RADAR_CENTER_Y, radius, COLOR_RADAR_GRID);

    // Entfernungs-Label
    lcd.setTextSize(1);
    lcd.setTextColor(COLOR_TEXT_DIM);
    lcd.setCursor(RADAR_CENTER_X + radius - 12, RADAR_CENTER_Y - 5);
    lcd.printf("%dkm", distKm);
  }

  // Kreuz in der Mitte (Standort)
  lcd.drawLine(RADAR_CENTER_X - 5, RADAR_CENTER_Y, RADAR_CENTER_X + 5, RADAR_CENTER_Y, COLOR_SAFE);
  lcd.drawLine(RADAR_CENTER_X, RADAR_CENTER_Y - 5, RADAR_CENTER_X, RADAR_CENTER_Y + 5, COLOR_SAFE);
  lcd.fillCircle(RADAR_CENTER_X, RADAR_CENTER_Y, 3, COLOR_SAFE);

  // Historische Blitze anzeigen (mit Fade-out)
  drawHistoricalLightning();

  // Letzter Blitz (prominenter)
  if (lastDistance > 0 && totalLightningCount > 0) {
    drawLightningOnRadar(lastDistance, millis() - lastLightningTime < 5000);
  }

  // Trennlinie rechts
  lcd.drawLine(RADAR_AREA_WIDTH, HEADER_HEIGHT, RADAR_AREA_WIDTH, 240 - FOOTER_HEIGHT, COLOR_RADAR_GRID);
}

void drawHistoricalLightning() {
  unsigned long now = millis();

  for (int i = 0; i < historyCount; i++) {
    int idx = (historyIndex - 1 - i + MAX_HISTORY) % MAX_HISTORY;

    if (idx < 0 || idx >= MAX_HISTORY) continue;

    LightningEvent& event = history[idx];

    // Alter berechnen
    unsigned long age = now - event.time;

    // Nur letzte 5 Minuten anzeigen
    if (age > 300000) continue;

    // Fade-out basierend auf Alter (0-255)
    uint8_t alpha = 255 - (age * 255 / 300000);

    // Farbe basierend auf Entfernung
    uint16_t color = getDistanceColor(event.distance);

    // Dimmen für ältere Blitze
    if (alpha < 128) {
      color = COLOR_TEXT_DIM;
    }

    // Position auf Radar
    int radius = map(event.distance, 0, 40, 0, RADAR_MAX_RADIUS);

    // Zufälliger Winkel (für Visualisierung)
    float angle = (event.time % 360) * 0.01745;  // zu Radiant

    int x = RADAR_CENTER_X + radius * cos(angle);
    int y = RADAR_CENTER_Y + radius * sin(angle);

    // Kleiner Punkt
    lcd.fillCircle(x, y, 2, color);
  }
}

void drawLightningOnRadar(uint8_t distance, bool highlight) {
  if (distance == 0 || distance > 40) return;

  // Position auf Radar
  int radius = map(distance, 0, 40, 0, RADAR_MAX_RADIUS);

  // Farbe basierend auf Entfernung
  uint16_t color = getDistanceColor(distance);

  // Zufälliger Winkel (basierend auf Zeit)
  float angle = (lastLightningTime % 360) * 0.01745;  // zu Radiant

  int x = RADAR_CENTER_X + radius * cos(angle);
  int y = RADAR_CENTER_Y + radius * sin(angle);

  // Blitz-Symbol
  if (highlight) {
    // Pulsierender Effekt
    int pulseSize = 5 + (millis() / 100) % 3;
    lcd.fillCircle(x, y, pulseSize, color);
    lcd.drawCircle(x, y, pulseSize + 2, COLOR_LIGHTNING);
  } else {
    lcd.fillCircle(x, y, 4, color);
  }

  // Linie zum Zentrum (optional)
  if (highlight) {
    lcd.drawLine(RADAR_CENTER_X, RADAR_CENTER_Y, x, y, color);
  }
}

void drawLightningAnimation() {
  unsigned long elapsed = millis() - animationStartTime;

  // Blitz-Effekt (Radar-Rand blinken)
  if (elapsed < 200 && (elapsed / 50) % 2 == 0) {
    // Rahmen blinken (nur Radar-Bereich)
    lcd.drawRect(0, HEADER_HEIGHT, RADAR_AREA_WIDTH, MAIN_HEIGHT, COLOR_LIGHTNING);
    lcd.drawRect(1, HEADER_HEIGHT + 1, RADAR_AREA_WIDTH - 2, MAIN_HEIGHT - 2, COLOR_LIGHTNING);
  }

  // Blitz-Symbol in der Mitte
  if (elapsed < 500) {
    drawLightningBolt(RADAR_CENTER_X, RADAR_CENTER_Y - 30);
  }
}

void drawLightningBolt(int x, int y) {
  // Einfaches Blitz-Symbol (Zickzack)
  int points[][2] = {
    {x, y},
    {x - 5, y + 10},
    {x + 2, y + 12},
    {x - 3, y + 22},
    {x + 8, y + 18},
    {x + 3, y + 28},
    {x + 5, y + 15},
    {x + 12, y + 20},
    {x, y}
  };

  // Blitz zeichnen
  for (int i = 0; i < 7; i++) {
    lcd.drawLine(points[i][0], points[i][1],
                 points[i + 1][0], points[i + 1][1],
                 COLOR_LIGHTNING);
  }

  // Gefüllt
  lcd.fillTriangle(x, y, x - 5, y + 10, x + 2, y + 12, COLOR_LIGHTNING);
  lcd.fillTriangle(x + 2, y + 12, x - 3, y + 22, x + 5, y + 15, COLOR_LIGHTNING);
  lcd.fillTriangle(x + 5, y + 15, x + 3, y + 28, x + 12, y + 20, COLOR_LIGHTNING);
}

void drawInfoPanel() {
  // Panel-Hintergrund (rechter Bereich)
  lcd.fillRect(INFO_PANEL_X, HEADER_HEIGHT, INFO_PANEL_WIDTH, MAIN_HEIGHT, COLOR_BG);

  int x = INFO_PANEL_X + 10;
  int y = HEADER_HEIGHT + 10;

  // ===== ENTFERNUNG =====
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(x, y);
  lcd.print("Distance:");

  y += 12;
  lcd.setTextColor(COLOR_TEXT);
  if (lastDistance > 0 && totalLightningCount > 0) {
    lcd.setTextSize(3);
    lcd.setCursor(x, y);
    lcd.printf("%d", lastDistance);
    lcd.setTextSize(1);
    lcd.setCursor(x + 25, y + 12);
    lcd.print("km");
  } else {
    lcd.setTextSize(1);
    lcd.setCursor(x, y);
    lcd.print("No data");
  }

  // ===== STATUS =====
  y += 35;
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(x, y);
  lcd.print("Status:");

  y += 12;
  if (lastDistance == 0 || totalLightningCount == 0) {
    lcd.setTextColor(COLOR_SAFE);
    lcd.setCursor(x, y);
    lcd.print("Safe");
  } else {
    uint16_t statusColor = getDistanceColor(lastDistance);
    lcd.setTextColor(statusColor);
    lcd.setCursor(x, y);

    if (lastDistance <= 10) {
      lcd.print("CRITICAL!");
    } else if (lastDistance <= 20) {
      lcd.print("Danger");
    } else if (lastDistance <= 30) {
      lcd.print("Warning");
    } else {
      lcd.print("Caution");
    }
  }

  // ===== COUNT =====
  y += 25;
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(x, y);
  lcd.print("Count:");

  y += 12;
  lcd.setTextColor(COLOR_TEXT);
  lcd.setTextSize(2);
  lcd.setCursor(x, y);
  lcd.printf("%lu", totalLightningCount);

  // ===== LAST =====
  y += 30;
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(x, y);
  lcd.print("Last:");

  y += 12;
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(x, y);

  if (totalLightningCount > 0) {
    unsigned long elapsed = (millis() - lastLightningTime) / 1000;  // Sekunden

    if (elapsed < 60) {
      lcd.printf("%lus", elapsed);
    } else if (elapsed < 3600) {
      lcd.printf("%lum", elapsed / 60);
    } else {
      lcd.printf("%luh", elapsed / 3600);
    }
    lcd.print(" ago");
  } else {
    lcd.print("Never");
  }

  // ===== STATISTIK =====
  y += 25;
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(x, y);
  lcd.print("Noise:");

  y += 10;
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(x, y);
  lcd.printf("%lu", noiseCount);

  y += 15;
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(x, y);
  lcd.print("Disturb:");

  y += 10;
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(x, y);
  lcd.printf("%lu", disturbanceCount);
}

void drawFooter() {
  // Footer-Hintergrund
  lcd.fillRect(0, 240 - FOOTER_HEIGHT, 320, FOOTER_HEIGHT, 0x2104);

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);

  // Statistik
  lcd.setCursor(5, 217);
  lcd.printf("Sensitivity: %d/7", sensitivity);

  // Touch-Hilfe
  lcd.setTextColor(0x5ACB);  // Noch dunkler
  lcd.setCursor(180, 217);
  lcd.print("Left:Reset  Right:Sens");
}

uint16_t getDistanceColor(uint8_t distance) {
  if (distance == 0) return COLOR_TEXT_DIM;
  if (distance <= 10) return COLOR_CRITICAL;  // 0-10km: Rot
  if (distance <= 20) return COLOR_DANGER;    // 10-20km: Orange
  if (distance <= 30) return COLOR_WARNING;   // 20-30km: Gelb
  return COLOR_SAFE;                          // 30-40km: Grün
}

// ===== HELPER FUNKTIONEN =====

void drawSplashScreen() {
  lcd.fillScreen(COLOR_RADAR_BG);

  // Großes Blitz-Symbol
  drawLightningBolt(160, 80);

  // Titel
  lcd.setTextSize(3);
  lcd.setTextColor(COLOR_LIGHTNING);
  lcd.setCursor(40, 130);
  lcd.print("LIGHTNING");

  lcd.setTextSize(2);
  lcd.setCursor(100, 160);
  lcd.print("SENSOR");

  // Version
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(120, 185);
  lcd.print("Example 17b");

  lcd.setTextSize(1);
  lcd.setCursor(95, 200);
  lcd.print("Landscape Layout");

  lcd.setCursor(95, 215);
  lcd.print("SEN0290 / AS3935");
}

void drawErrorScreen() {
  lcd.fillScreen(COLOR_BG);

  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_CRITICAL);
  lcd.setCursor(70, 60);
  lcd.print("AS3935 Error!");

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT);

  lcd.setCursor(10, 100);
  lcd.print("Check wiring:");

  lcd.setCursor(10, 120);
  lcd.printf("SDA -> GPIO %d", extSDA);

  lcd.setCursor(10, 135);
  lcd.printf("SCL -> GPIO %d", extSCL);

  lcd.setCursor(10, 150);
  lcd.printf("IRQ -> GPIO %d (optional)", IRQ_PIN);

  lcd.setCursor(10, 165);
  lcd.print("VCC -> 3.3V");

  lcd.setCursor(10, 180);
  lcd.print("GND -> GND");

  lcd.setCursor(10, 200);
  lcd.printf("I2C Address: 0x%02X", AS3935_I2C_ADDR);

  lcd.setCursor(10, 220);
  lcd.print("Check Serial Monitor for details.");
}
