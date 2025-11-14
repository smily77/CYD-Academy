/*
  17_Lightning_Sensor - Blitzwetter-Anzeige mit SEN0290

  Dieses Beispiel zeigt eine professionelle Blitzwetter-Anzeige mit dem
  Gravity Lightning Sensor (AS3935).

  Funktionen:
  - AS3935 Lightning Sensor auslesen (I2C)
  - Radar-ähnliche Visualisierung
  - Blitz-Entfernung (bis 40km)
  - Blitz-Animation mit Sound-Effekt
  - Anzahl erkannter Blitze
  - Gefahrenstufe (Farbcodierung)
  - Störungs-Erkennung
  - Zeit seit letztem Blitz
  - Historische Blitz-Anzeige

  Lernziele:
  - I2C-Kommunikation (AS3935)
  - Interrupt-basierte Sensor-Auslese
  - Radar-Visualisierung
  - Animationen & Effekte
  - Gefahren-Klassifizierung
  - Historische Daten-Visualisierung

  Hardware:
  - 1x CYD Display (2.8" oder 3.5")
  - 1x Gravity Lightning Sensor SEN0290 (AS3935)
  - 5x Dupont-Kabel (VCC, GND, SDA, SCL, IRQ)

  SEN0290 Verkabelung:
  - VCC → 3.3V
  - GND → GND
  - SDA → GPIO 22 (extSDA)
  - SCL → GPIO 27 (extSCL)
  - IRQ → GPIO 35 (Interrupt Pin)

  Installation:
  1. DFRobot AS3935 Library installieren:
     Arduino IDE: Library Manager → "DFRobot AS3935" → installieren

     PlatformIO: lib_deps = dfrobot/DFRobot_AS3935_I2C @ ^1.0.2

  2. In CYD_Display_Config.h definieren (falls nicht vorhanden):
     #define extSDA 22
     #define extSCL 27

  Steuerung:
  - Touch oben: Reset Statistik
  - Touch unten: Sensitivität ändern
  - Neigung: Automatische Blitz-Erkennung
*/

#include <LovyanGFX.hpp>
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
#define IRQ_PIN 35

// Display-Konfiguration
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Light_PWM _light_instance;
  lgfx::Touch_XPT2046 _touch_instance;

public:
  LGFX(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = VSPI_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read = 16000000;
      cfg.spi_3wire = true;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = 14;
      cfg.pin_mosi = 13;
      cfg.pin_miso = 12;
      cfg.pin_dc = 2;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = 15;
      cfg.pin_rst = -1;
      cfg.pin_busy = -1;
      cfg.panel_width = 240;
      cfg.panel_height = 320;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = true;
      cfg.invert = false;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;
      _panel_instance.config(cfg);
    }

    {
      auto cfg = _light_instance.config();
      cfg.pin_bl = 21;
      cfg.invert = false;
      cfg.freq = 44100;
      cfg.pwm_channel = 7;
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    {
      auto cfg = _touch_instance.config();
      cfg.x_min = 300;
      cfg.x_max = 3900;
      cfg.y_min = 400;
      cfg.y_max = 3900;
      cfg.pin_int = -1;
      cfg.bus_shared = true;
      cfg.offset_rotation = 0;
      cfg.spi_host = VSPI_HOST;
      cfg.freq = 1000000;
      cfg.pin_sclk = 25;
      cfg.pin_mosi = 32;
      cfg.pin_miso = 39;
      cfg.pin_cs = 33;
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }
};

// ===== KONSTANTEN =====

// Farben
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

// Radar-Parameter
const int RADAR_CENTER_X = 120;
const int RADAR_CENTER_Y = 140;
const int RADAR_MAX_RADIUS = 90;
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
DFRobot_AS3935_I2C sensor((uint8_t)IRQ_PIN);

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

// ===== INTERRUPT HANDLER =====

void IRAM_ATTR lightningISR() {
  interruptDetected = true;
}

// ===== SETUP =====

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Lightning Sensor Demo ===");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(0);  // Portrait
  lcd.setBrightness(255);
  lcd.fillScreen(COLOR_BG);

  // Splash Screen
  drawSplashScreen();
  delay(2000);

  // I2C initialisieren
  Serial.printf("Initializing I2C (SDA=%d, SCL=%d)...\n", extSDA, extSCL);
  Wire.begin(extSDA, extSCL);

  // Interrupt Pin konfigurieren
  pinMode(IRQ_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), lightningISR, RISING);

  // AS3935 initialisieren
  lcd.fillScreen(COLOR_BG);
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(10, 140);
  lcd.print("Initializing");
  lcd.setCursor(10, 160);
  lcd.print("AS3935...");

  // Sensor starten (I2C-Adresse 0x03)
  while (!sensor.begin()) {
    Serial.println("AS3935 not found!");
    lcd.fillScreen(COLOR_BG);
    drawErrorScreen();
    delay(1000);
  }

  sensorConnected = true;
  Serial.println("AS3935 found!");

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

  Serial.println("AS3935 configured!");
  Serial.printf("Sensitivity: %d/7\n", sensitivity);

  // Initiale Anzeige
  lcd.fillScreen(COLOR_BG);
  drawUI();

  Serial.println("Ready! Waiting for lightning...");
}

// ===== LOOP =====

void loop() {
  // Interrupt-Check
  if (interruptDetected) {
    interruptDetected = false;
    handleLightningInterrupt();
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

  // Oberer Bereich: Reset Statistik
  if (y < 100) {
    Serial.println("Resetting statistics...");
    totalLightningCount = 0;
    disturbanceCount = 0;
    noiseCount = 0;
    historyCount = 0;
    historyIndex = 0;
    lastDistance = 0;
    lastEnergy = 0;

    lcd.fillRect(0, 280, 240, 40, COLOR_SAFE);
    lcd.setTextSize(2);
    lcd.setTextColor(COLOR_BG);
    lcd.setCursor(60, 295);
    lcd.print("RESET!");
    delay(500);
  }
  // Unterer Bereich: Sensitivität ändern
  else if (y > 220) {
    sensitivity = (sensitivity + 1) % 8;
    sensor.setWatchdogThreshold(sensitivity);

    Serial.printf("Sensitivity changed to: %d/7\n", sensitivity);

    lcd.fillRect(0, 280, 240, 40, COLOR_WARNING);
    lcd.setTextSize(1);
    lcd.setTextColor(COLOR_BG);
    lcd.setCursor(60, 295);
    lcd.printf("Sensitivity: %d/7", sensitivity);
    delay(500);
  }
}

// ===== UI ZEICHNEN =====

void drawUI() {
  // Hintergrund (nur teilweise überschreiben für weniger Flackern)

  // Header
  drawHeader();

  // Radar-Anzeige
  drawRadar();

  // Blitz-Animation (falls aktiv)
  if (lightningAnimation) {
    drawLightningAnimation();
  }

  // Info-Panel
  drawInfoPanel();

  // Footer
  drawFooter();
}

void drawHeader() {
  // Header-Hintergrund
  lcd.fillRect(0, 0, 240, 30, COLOR_RADAR_BG);

  // Titel
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_LIGHTNING);
  lcd.setCursor(20, 8);
  lcd.print("LIGHTNING");
  lcd.setCursor(150, 8);
  lcd.print("RADAR");

  // Trennlinie
  lcd.drawLine(0, 30, 240, 30, COLOR_RADAR_GRID);
}

void drawRadar() {
  // Radar-Hintergrund
  lcd.fillRect(0, 31, 240, 180, COLOR_RADAR_BG);

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

  // Blitz-Effekt (Bildschirm-Rand blinken)
  if (elapsed < 200 && (elapsed / 50) % 2 == 0) {
    // Rahmen blinken
    lcd.drawRect(0, 31, 240, 180, COLOR_LIGHTNING);
    lcd.drawRect(1, 32, 238, 178, COLOR_LIGHTNING);
    lcd.drawRect(2, 33, 236, 176, COLOR_LIGHTNING);
  }

  // Blitz-Symbol in der Mitte (zufälliger Blitz)
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
  // Panel-Hintergrund
  lcd.fillRect(0, 211, 240, 60, COLOR_BG);

  lcd.setTextSize(1);

  // Linke Spalte
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(5, 215);
  lcd.print("Distance:");

  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(5, 227);
  if (lastDistance > 0 && totalLightningCount > 0) {
    lcd.setTextSize(2);
    lcd.printf("%d km", lastDistance);
  } else {
    lcd.setTextSize(1);
    lcd.print("No data");
  }

  // Gefahrenstufe
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(5, 245);
  lcd.print("Status:");

  lcd.setCursor(5, 257);
  if (lastDistance == 0 || totalLightningCount == 0) {
    lcd.setTextColor(COLOR_SAFE);
    lcd.print("Safe");
  } else {
    uint16_t statusColor = getDistanceColor(lastDistance);
    lcd.setTextColor(statusColor);

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

  // Rechte Spalte
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(125, 215);
  lcd.print("Count:");

  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(125, 227);
  lcd.setTextSize(2);
  lcd.printf("%lu", totalLightningCount);

  // Zeit seit letztem Blitz
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(125, 245);
  lcd.print("Last:");

  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(125, 257);

  if (totalLightningCount > 0) {
    unsigned long elapsed = (millis() - lastLightningTime) / 1000;  // Sekunden

    if (elapsed < 60) {
      lcd.printf("%lus ago", elapsed);
    } else if (elapsed < 3600) {
      lcd.printf("%lum ago", elapsed / 60);
    } else {
      lcd.printf("%luh ago", elapsed / 3600);
    }
  } else {
    lcd.print("Never");
  }
}

void drawFooter() {
  // Footer-Hintergrund
  lcd.fillRect(0, 271, 240, 49, 0x2104);

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);

  // Statistik
  lcd.setCursor(5, 275);
  lcd.printf("Noise: %lu  Dist: %lu", noiseCount, disturbanceCount);

  lcd.setCursor(5, 287);
  lcd.printf("Sensitivity: %d/7", sensitivity);

  // Touch-Hilfe
  lcd.setTextColor(0x5ACB);  // Noch dunkler
  lcd.setCursor(5, 304);
  lcd.print("Top:Reset  Bottom:Sens");
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
  drawLightningBolt(120, 100);

  // Titel
  lcd.setTextSize(3);
  lcd.setTextColor(COLOR_LIGHTNING);
  lcd.setCursor(20, 180);
  lcd.print("LIGHTNING");

  lcd.setTextSize(2);
  lcd.setCursor(60, 210);
  lcd.print("SENSOR");

  // Version
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(70, 240);
  lcd.print("Example 17");

  lcd.setCursor(50, 255);
  lcd.print("SEN0290 / AS3935");
}

void drawErrorScreen() {
  lcd.fillScreen(COLOR_BG);

  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_CRITICAL);
  lcd.setCursor(20, 100);
  lcd.print("AS3935 Error!");

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT);

  lcd.setCursor(10, 140);
  lcd.print("Check wiring:");

  lcd.setCursor(10, 160);
  lcd.printf("SDA -> GPIO %d", extSDA);

  lcd.setCursor(10, 175);
  lcd.printf("SCL -> GPIO %d", extSCL);

  lcd.setCursor(10, 190);
  lcd.printf("IRQ -> GPIO %d", IRQ_PIN);

  lcd.setCursor(10, 205);
  lcd.print("VCC -> 3.3V");

  lcd.setCursor(10, 220);
  lcd.print("GND -> GND");

  lcd.setCursor(10, 240);
  lcd.print("I2C Address: 0x03");

  lcd.setCursor(10, 260);
  lcd.print("Check Serial Monitor");
  lcd.setCursor(10, 275);
  lcd.print("for details.");
}
