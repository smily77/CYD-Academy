/*
  18_Compass_Orientation - 3D Kompass & Lage-Anzeige mit BNO055

  Dieses Beispiel zeigt eine professionelle Kompass- und Orientierungsanzeige
  mit dem BNO055 9-DoF Sensor (Absolute Orientation Sensor).

  Funktionen:
  - 3D Kompass mit Nordpfeil
  - Künstlicher Horizont (Attitude Indicator wie im Flugzeug)
  - Euler-Winkel Anzeige (Heading, Roll, Pitch)
  - Kalibrierungsstatus für alle Sensoren
  - Quaternionen-basierte 3D-Orientierung
  - Smooth Animationen
  - Multiple View Modes

  Lernziele:
  - I2C-Kommunikation (BNO055)
  - Sensor-Fusion verstehen
  - Euler-Winkel & Quaternionen
  - 3D-Orientierung visualisieren
  - Kompass-Mathematik
  - Kalibrierungsprozesse

  Hardware:
  - 1x CYD Display (2.8" oder 3.5")
  - 1x BNO055 Absolute Orientation Sensor
  - 4x Dupont-Kabel (VCC, GND, SDA, SCL)

  BNO055 Verkabelung:
  - VCC → 3.3V (oder 5V, Sensor akzeptiert beides)
  - GND → GND
  - SDA → GPIO 22 (extSDA)
  - SCL → GPIO 27 (extSCL)

  Installation:
  1. Adafruit BNO055 Library installieren:
     Arduino IDE: Library Manager → "Adafruit BNO055" → installieren
     (Installiert auch Adafruit Sensor & BusIO automatisch)

     PlatformIO: lib_deps = adafruit/Adafruit BNO055 @ ^1.6.0

  2. In CYD_Display_Config.h definieren (falls nicht vorhanden):
     #define extSDA 22
     #define extSCL 27

  Steuerung:
  - Touch Links: Vorheriger Ansichtsmodus
  - Touch Rechts: Nächster Ansichtsmodus
  - Touch Mitte: Kalibrierung zurücksetzen

  Ansichtsmodi:
  1. Kompass + Horizont (Standard)
  2. Nur Kompass (groß)
  3. Nur Horizont (groß)
  4. Daten-Ansicht (alle Werte)
*/

#include <CYD_Display_Config.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

// I2C Pins (CYD Standard - external I2C)
#ifndef extSDA
#define extSDA 22
#endif

#ifndef extSCL
#define extSCL 27
#endif

// ===== KONSTANTEN =====

// Farben (Dark Theme)
#define COLOR_BG 0x0000          // Schwarz
#define COLOR_SKY 0x03EF         // Hellblau (Himmel)
#define COLOR_GROUND 0x8BC34A    // Braun/Grün (Boden)
#define COLOR_HORIZON 0xFFFF     // Weiß (Horizont-Linie)
#define COLOR_COMPASS_BG 0x1082  // Dunkelgrau
#define COLOR_COMPASS_RING 0x4208 // Grau
#define COLOR_NORTH 0xF800       // Rot (Nord)
#define COLOR_SOUTH 0xFFFF       // Weiß (Süd)
#define COLOR_EAST_WEST 0x07E0   // Grün (Ost/West)
#define COLOR_NEEDLE 0xF800      // Rot (Kompass-Nadel)
#define COLOR_TEXT 0xFFFF        // Weiß
#define COLOR_TEXT_DIM 0x7BEF    // Grau
#define COLOR_CAL_NONE 0xF800    // Rot (nicht kalibriert)
#define COLOR_CAL_LOW 0xFD20     // Orange (teilweise)
#define COLOR_CAL_MED 0xFFE0     // Gelb (mittel)
#define COLOR_CAL_FULL 0x07E0    // Grün (voll kalibriert)

// Ansichtsmodi
enum ViewMode {
  VIEW_COMBO,      // Kompass + Horizont
  VIEW_COMPASS,    // Nur Kompass
  VIEW_HORIZON,    // Nur Horizont
  VIEW_DATA        // Daten-Ansicht
};

// ===== GLOBALE VARIABLEN =====

LGFX lcd;
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

// Sensor-Status
bool sensorConnected = false;

// Orientierungs-Daten
float heading = 0.0;   // 0-360° (Nord = 0°)
float roll = 0.0;      // -180 bis +180°
float pitch = 0.0;     // -90 bis +90°

// Kalibrierungsstatus (0-3 für jeden Sensor)
uint8_t calSystem = 0;
uint8_t calGyro = 0;
uint8_t calAccel = 0;
uint8_t calMag = 0;

// View Mode
ViewMode currentView = VIEW_COMBO;

// Touch-Handling
unsigned long lastTouchTime = 0;

// Animation Smoothing
float smoothHeading = 0.0;
float smoothRoll = 0.0;
float smoothPitch = 0.0;
const float SMOOTH_FACTOR = 0.1;  // 0.0 = kein Smoothing, 1.0 = volle Updates

// ===== SETUP =====

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== BNO055 Compass & Orientation Demo ===");

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

  // BNO055 initialisieren
  lcd.fillScreen(COLOR_BG);
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(10, 140);
  lcd.print("Initializing");
  lcd.setCursor(10, 160);
  lcd.print("BNO055...");

  if (!bno.begin()) {
    Serial.println("BNO055 not found!");
    drawErrorScreen();
    sensorConnected = false;
    while (1) delay(1000);
  }

  sensorConnected = true;
  Serial.println("BNO055 found!");

  // Warte auf Sensor-Initialisierung
  delay(1000);

  // Setze zu externem Crystal (genauer)
  bno.setExtCrystalUse(true);

  Serial.println("BNO055 initialized!");
  Serial.println("Move sensor in figure-8 motion to calibrate magnetometer");
  Serial.println("Tilt in all directions to calibrate gyro & accelerometer");

  // Initiale Anzeige
  lcd.fillScreen(COLOR_BG);
  drawCalibrationHint();
  delay(3000);
}

// ===== LOOP =====

void loop() {
  if (!sensorConnected) {
    delay(1000);
    return;
  }

  // Sensor auslesen
  readSensor();

  // Smooth Animation
  smoothValues();

  // Touch-Handling
  handleTouch();

  // UI zeichnen (basierend auf View Mode)
  drawUI();

  delay(20);  // ~50 FPS
}

// ===== SENSOR FUNKTIONEN =====

void readSensor() {
  // Euler-Winkel auslesen
  sensors_event_t event;
  bno.getEvent(&event);

  heading = event.orientation.x;  // 0-360°
  pitch = event.orientation.y;    // -180 bis +180°
  roll = event.orientation.z;     // -90 bis +90°

  // Kalibrierungsstatus
  bno.getCalibration(&calSystem, &calGyro, &calAccel, &calMag);

  // Debug (alle 2 Sekunden)
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 2000) {
    lastDebug = millis();
    Serial.printf("Heading: %.1f°  Roll: %.1f°  Pitch: %.1f°\n",
                  heading, roll, pitch);
    Serial.printf("Calibration - Sys:%d Gyro:%d Accel:%d Mag:%d\n",
                  calSystem, calGyro, calAccel, calMag);
  }
}

void smoothValues() {
  // Exponential Smoothing für flüssige Animation

  // Heading (mit Wrap-around für 360°/0°)
  float headingDiff = heading - smoothHeading;
  if (headingDiff > 180) headingDiff -= 360;
  if (headingDiff < -180) headingDiff += 360;
  smoothHeading += headingDiff * SMOOTH_FACTOR;
  if (smoothHeading < 0) smoothHeading += 360;
  if (smoothHeading >= 360) smoothHeading -= 360;

  // Roll & Pitch
  smoothRoll += (roll - smoothRoll) * SMOOTH_FACTOR;
  smoothPitch += (pitch - smoothPitch) * SMOOTH_FACTOR;
}

// ===== TOUCH HANDLING =====

void handleTouch() {
  uint16_t x, y;
  if (!lcd.getTouch(&x, &y)) return;

  // Debouncing
  if (millis() - lastTouchTime < 500) return;
  lastTouchTime = millis();

  // Linker Bereich: Vorheriger Modus
  if (x < 80) {
    currentView = (ViewMode)((currentView - 1 + 4) % 4);
    Serial.printf("View Mode: %d\n", currentView);
    lcd.fillScreen(COLOR_BG);
  }
  // Rechter Bereich: Nächster Modus
  else if (x > 160) {
    currentView = (ViewMode)((currentView + 1) % 4);
    Serial.printf("View Mode: %d\n", currentView);
    lcd.fillScreen(COLOR_BG);
  }
  // Mitte: Kalibrierung zurücksetzen (nur Info, BNO055 macht auto-cal)
  else {
    Serial.println("Calibration reset - move sensor to recalibrate");

    lcd.fillRect(40, 140, 160, 40, COLOR_TEXT);
    lcd.setTextSize(1);
    lcd.setTextColor(COLOR_BG);
    lcd.setCursor(50, 150);
    lcd.print("Move sensor in");
    lcd.setCursor(50, 165);
    lcd.print("figure-8 motion!");
    delay(2000);
    lcd.fillScreen(COLOR_BG);
  }
}

// ===== UI ZEICHNEN =====

void drawUI() {
  // Zeichne basierend auf View Mode
  switch (currentView) {
    case VIEW_COMBO:
      drawComboView();
      break;
    case VIEW_COMPASS:
      drawCompassView();
      break;
    case VIEW_HORIZON:
      drawHorizonView();
      break;
    case VIEW_DATA:
      drawDataView();
      break;
  }

  // Kalibrierungs-Indikator (immer sichtbar, klein)
  drawCalibrationIndicator();
}

// ===== VIEW MODES =====

void drawComboView() {
  // Kompass oben (120px Durchmesser)
  drawCompass(120, 90, 55);

  // Horizont unten (120px breit)
  drawHorizon(120, 200, 110, 80);

  // Werte ganz unten
  drawCompactValues(10, 295);
}

void drawCompassView() {
  // Großer Kompass (160px Durchmesser)
  drawCompass(120, 140, 75);

  // Heading groß anzeigen
  lcd.setTextSize(3);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(80, 250);
  lcd.printf("%.0f", smoothHeading);
  lcd.print((char)247);  // Grad-Symbol

  // Label
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(100, 280);
  lcd.print("HEADING");
}

void drawHorizonView() {
  // Großer Horizont (200px breit, 160px hoch)
  drawHorizon(120, 140, 200, 160);

  // Roll & Pitch Werte
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);

  lcd.setCursor(10, 270);
  lcd.printf("R:%.0f", smoothRoll);
  lcd.print((char)247);

  lcd.setCursor(130, 270);
  lcd.printf("P:%.0f", smoothPitch);
  lcd.print((char)247);
}

void drawDataView() {
  // Vollständige Daten-Ansicht
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT);

  int y = 20;

  // Header
  lcd.setTextSize(2);
  lcd.setCursor(40, y);
  lcd.print("ORIENTATION");
  y += 30;

  // Euler-Winkel
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(10, y);
  lcd.print("Heading (Yaw):");
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(150, y);
  lcd.printf("%.2f", heading);
  lcd.print((char)247);
  y += 20;

  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(10, y);
  lcd.print("Roll:");
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(150, y);
  lcd.printf("%.2f", roll);
  lcd.print((char)247);
  y += 20;

  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(10, y);
  lcd.print("Pitch:");
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(150, y);
  lcd.printf("%.2f", pitch);
  lcd.print((char)247);
  y += 30;

  // Kalibrierung (groß)
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(40, y);
  lcd.print("CALIBRATION");
  y += 25;

  lcd.setTextSize(1);
  drawCalibrationBar(10, y, "System", calSystem);
  y += 25;
  drawCalibrationBar(10, y, "Gyro", calGyro);
  y += 25;
  drawCalibrationBar(10, y, "Accel", calAccel);
  y += 25;
  drawCalibrationBar(10, y, "Mag", calMag);
  y += 35;

  // Richtungs-Info
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(10, y);
  lcd.print("Cardinal:");
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(100, y);
  lcd.print(getCardinalDirection(heading));
}

// ===== KOMPASS ZEICHNEN =====

void drawCompass(int centerX, int centerY, int radius) {
  // Hintergrund-Kreis
  lcd.fillCircle(centerX, centerY, radius, COLOR_COMPASS_BG);

  // Äußerer Ring
  lcd.drawCircle(centerX, centerY, radius, COLOR_COMPASS_RING);
  lcd.drawCircle(centerX, centerY, radius - 1, COLOR_COMPASS_RING);

  // Grad-Markierungen (alle 30°)
  for (int deg = 0; deg < 360; deg += 30) {
    float angleRad = (deg - smoothHeading) * PI / 180.0;

    int x1 = centerX + (radius - 8) * sin(angleRad);
    int y1 = centerY - (radius - 8) * cos(angleRad);
    int x2 = centerX + (radius - 3) * sin(angleRad);
    int y2 = centerY - (radius - 3) * cos(angleRad);

    uint16_t color = COLOR_COMPASS_RING;
    if (deg == 0) color = COLOR_NORTH;        // Nord = Rot
    else if (deg == 180) color = COLOR_SOUTH; // Süd = Weiß
    else if (deg == 90 || deg == 270) color = COLOR_EAST_WEST; // O/W = Grün

    lcd.drawLine(x1, y1, x2, y2, color);
  }

  // Himmelsrichtungen (N, E, S, W)
  drawCardinalMarks(centerX, centerY, radius - 18);

  // Nord-Pfeil (zeigt immer nach oben, Kompass dreht sich)
  int arrowSize = radius / 3;

  // Pfeil-Spitze (nach oben)
  lcd.fillTriangle(
    centerX, centerY - arrowSize,          // Spitze
    centerX - 8, centerY - arrowSize + 15, // Links
    centerX + 8, centerY - arrowSize + 15, // Rechts
    COLOR_NEEDLE
  );

  // Pfeil-Ende (nach unten)
  lcd.fillTriangle(
    centerX, centerY + arrowSize,          // Ende
    centerX - 6, centerY + arrowSize - 12, // Links
    centerX + 6, centerY + arrowSize - 12, // Rechts
    0x7BEF  // Grau
  );

  // Zentrierungs-Punkt
  lcd.fillCircle(centerX, centerY, 5, COLOR_TEXT);
  lcd.fillCircle(centerX, centerY, 3, COLOR_BG);
}

void drawCardinalMarks(int centerX, int centerY, int radius) {
  lcd.setTextSize(2);

  // Nord (oben) - relativ zur Heading-Rotation
  float northAngle = -smoothHeading * PI / 180.0;
  int nx = centerX + radius * sin(northAngle);
  int ny = centerY - radius * cos(northAngle);
  lcd.setTextColor(COLOR_NORTH);
  lcd.setCursor(nx - 6, ny - 8);
  lcd.print("N");

  // Ost (rechts)
  float eastAngle = (90 - smoothHeading) * PI / 180.0;
  int ex = centerX + radius * sin(eastAngle);
  int ey = centerY - radius * cos(eastAngle);
  lcd.setTextColor(COLOR_EAST_WEST);
  lcd.setCursor(ex - 6, ey - 8);
  lcd.print("E");

  // Süd (unten)
  float southAngle = (180 - smoothHeading) * PI / 180.0;
  int sx = centerX + radius * sin(southAngle);
  int sy = centerY - radius * cos(southAngle);
  lcd.setTextColor(COLOR_SOUTH);
  lcd.setCursor(sx - 6, sy - 8);
  lcd.print("S");

  // West (links)
  float westAngle = (270 - smoothHeading) * PI / 180.0;
  int wx = centerX + radius * sin(westAngle);
  int wy = centerY - radius * cos(westAngle);
  lcd.setTextColor(COLOR_EAST_WEST);
  lcd.setCursor(wx - 6, wy - 8);
  lcd.print("W");
}

// ===== HORIZONT ZEICHNEN =====

void drawHorizon(int centerX, int centerY, int width, int height) {
  // Künstlicher Horizont (Attitude Indicator)

  int halfWidth = width / 2;
  int halfHeight = height / 2;

  // Clipping-Bereich (Kreis-Maske)
  int clipRadius = min(halfWidth, halfHeight) - 5;

  // Berechne Horizont-Linie Position basierend auf Pitch
  // pitch = +90° → Horizont ganz unten (schaue nach oben)
  // pitch = -90° → Horizont ganz oben (schaue nach unten)
  int horizonOffset = -(smoothPitch / 90.0) * halfHeight;

  // Roll-Rotation (in Radiant)
  float rollRad = smoothRoll * PI / 180.0;

  // Zeichne innerhalb Kreis
  for (int py = -clipRadius; py <= clipRadius; py++) {
    for (int px = -clipRadius; px <= clipRadius; px++) {
      // Check ob innerhalb Kreis
      if (px * px + py * py > clipRadius * clipRadius) continue;

      // Rotiere um Roll-Achse
      float rotX = px * cos(rollRad) - py * sin(rollRad);
      float rotY = px * sin(rollRad) + py * cos(rollRad);

      // Addiere Pitch-Offset
      int finalY = rotY + horizonOffset;

      // Farbe basierend auf Position
      uint16_t color;
      if (finalY < 0) {
        color = COLOR_SKY;  // Himmel (über Horizont)
      } else {
        color = COLOR_GROUND;  // Boden (unter Horizont)
      }

      lcd.drawPixel(centerX + px, centerY + py, color);
    }
  }

  // Horizont-Linie (weiß, dick)
  // Diese wird als gedrehte Linie durch den Mittelpunkt gezeichnet
  int lineLength = clipRadius;
  int x1 = centerX - lineLength * cos(rollRad);
  int y1 = centerY - lineLength * sin(rollRad) + horizonOffset;
  int x2 = centerX + lineLength * cos(rollRad);
  int y2 = centerY + lineLength * sin(rollRad) + horizonOffset;

  lcd.drawLine(x1, y1, x2, y2, COLOR_HORIZON);
  lcd.drawLine(x1, y1 + 1, x2, y2 + 1, COLOR_HORIZON);  // Dicker

  // Flugzeug-Symbol (fest in Bildmitte)
  drawAircraftSymbol(centerX, centerY);

  // Äußerer Ring
  lcd.drawCircle(centerX, centerY, clipRadius + 1, COLOR_TEXT);
  lcd.drawCircle(centerX, centerY, clipRadius + 2, COLOR_TEXT);
}

void drawAircraftSymbol(int centerX, int centerY) {
  // Stilisiertes Flugzeug (immer in Bildmitte, fest)

  // Rumpf (horizontal)
  lcd.drawLine(centerX - 30, centerY, centerX - 10, centerY, COLOR_NEEDLE);
  lcd.drawLine(centerX + 10, centerY, centerX + 30, centerY, COLOR_NEEDLE);
  lcd.drawLine(centerX - 30, centerY + 1, centerX - 10, centerY + 1, COLOR_NEEDLE);
  lcd.drawLine(centerX + 10, centerY + 1, centerX + 30, centerY + 1, COLOR_NEEDLE);

  // Zentrum (Punkt)
  lcd.fillCircle(centerX, centerY, 3, COLOR_NEEDLE);
  lcd.fillCircle(centerX, centerY, 2, COLOR_TEXT);
}

// ===== KALIBRIERUNG =====

void drawCalibrationIndicator() {
  // Kleiner Indikator oben rechts (immer sichtbar)
  int x = 180;
  int y = 5;

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(x, y);
  lcd.print("CAL:");

  // System-Kalibrierung (größer, wichtigster)
  uint16_t sysColor = getCalibrationColor(calSystem);
  lcd.fillCircle(x + 35, y + 4, 6, sysColor);
  lcd.setTextColor(COLOR_BG);
  lcd.setCursor(x + 32, y + 1);
  lcd.print(calSystem);
}

void drawCalibrationBar(int x, int y, const char* label, uint8_t calValue) {
  // Label
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(x, y);
  lcd.print(label);
  lcd.print(":");

  // Balken (4 Segmente für 0-3)
  int barX = x + 80;
  int barWidth = 30;
  int segmentWidth = barWidth / 4;

  for (int i = 0; i < 4; i++) {
    uint16_t color = (i < calValue) ? getCalibrationColor(i + 1) : 0x2104;
    lcd.fillRect(barX + i * segmentWidth, y, segmentWidth - 2, 12, color);
  }

  // Wert
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(barX + barWidth + 10, y);
  lcd.printf("%d/3", calValue);
}

uint16_t getCalibrationColor(uint8_t calValue) {
  switch (calValue) {
    case 0: return COLOR_CAL_NONE;  // Rot
    case 1: return COLOR_CAL_LOW;   // Orange
    case 2: return COLOR_CAL_MED;   // Gelb
    case 3: return COLOR_CAL_FULL;  // Grün
    default: return COLOR_CAL_NONE;
  }
}

// ===== HELPER FUNKTIONEN =====

void drawCompactValues(int x, int y) {
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);

  lcd.setCursor(x, y);
  lcd.printf("H:%.0f", smoothHeading);
  lcd.print((char)247);

  lcd.setCursor(x + 70, y);
  lcd.printf("R:%.0f", smoothRoll);
  lcd.print((char)247);

  lcd.setCursor(x + 140, y);
  lcd.printf("P:%.0f", smoothPitch);
  lcd.print((char)247);
}

const char* getCardinalDirection(float heading) {
  if (heading >= 337.5 || heading < 22.5) return "N";
  if (heading >= 22.5 && heading < 67.5) return "NE";
  if (heading >= 67.5 && heading < 112.5) return "E";
  if (heading >= 112.5 && heading < 157.5) return "SE";
  if (heading >= 157.5 && heading < 202.5) return "S";
  if (heading >= 202.5 && heading < 247.5) return "SW";
  if (heading >= 247.5 && heading < 292.5) return "W";
  if (heading >= 292.5 && heading < 337.5) return "NW";
  return "?";
}

void drawCalibrationHint() {
  lcd.fillScreen(COLOR_BG);

  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(30, 50);
  lcd.print("CALIBRATION");

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);

  lcd.setCursor(10, 90);
  lcd.print("Move sensor slowly:");

  lcd.setCursor(10, 110);
  lcd.print("1. Figure-8 motion");
  lcd.setCursor(10, 125);
  lcd.print("   (Magnetometer)");

  lcd.setCursor(10, 150);
  lcd.print("2. Tilt all directions");
  lcd.setCursor(10, 165);
  lcd.print("   (Accel & Gyro)");

  lcd.setCursor(10, 190);
  lcd.print("3. Wait for green");
  lcd.setCursor(10, 205);
  lcd.print("   indicator");

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(20, 240);
  lcd.print("Touch to change views");
}

void drawSplashScreen() {
  lcd.fillScreen(COLOR_BG);

  // Kompass-Symbol (groß)
  lcd.drawCircle(120, 120, 50, COLOR_COMPASS_RING);
  lcd.drawCircle(120, 120, 48, COLOR_COMPASS_RING);

  // N
  lcd.setTextSize(3);
  lcd.setTextColor(COLOR_NORTH);
  lcd.setCursor(108, 60);
  lcd.print("N");

  // Pfeil
  lcd.fillTriangle(120, 80, 110, 100, 130, 100, COLOR_NEEDLE);

  // Titel
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(30, 200);
  lcd.print("COMPASS &");
  lcd.setCursor(20, 220);
  lcd.print("ORIENTATION");

  // Version
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setCursor(70, 250);
  lcd.print("Example 18");

  lcd.setCursor(50, 270);
  lcd.print("BNO055 9-DoF");
}

void drawErrorScreen() {
  lcd.fillScreen(COLOR_BG);

  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_CAL_NONE);
  lcd.setCursor(20, 100);
  lcd.print("BNO055 Error!");

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT);

  lcd.setCursor(10, 140);
  lcd.print("Check wiring:");

  lcd.setCursor(10, 160);
  lcd.printf("SDA -> GPIO %d", extSDA);

  lcd.setCursor(10, 175);
  lcd.printf("SCL -> GPIO %d", extSCL);

  lcd.setCursor(10, 190);
  lcd.print("VCC -> 3.3V or 5V");

  lcd.setCursor(10, 205);
  lcd.print("GND -> GND");

  lcd.setCursor(10, 225);
  lcd.print("I2C Address: 0x28");

  lcd.setCursor(10, 245);
  lcd.print("Check Serial Monitor");
  lcd.setCursor(10, 260);
  lcd.print("for details.");
}
