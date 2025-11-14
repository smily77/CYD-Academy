/*
  16_Spirit_Level - Digitale Wasserwaage mit MPU6050 & Physik-Simulation

  Dieses Beispiel zeigt eine realistische Wasserwaage mit schöner 3D-Kugel.

  Funktionen:
  - MPU6050 Accelerometer/Gyro auslesen (I2C)
  - Physikalisch realistische Kugel-Bewegung
  - Schöne 3D-Kugel mit Licht/Schatten
  - Wasserwaagen-Kreise (konzentrisch)
  - Grad-Anzeige (Neigung in X/Y)
  - Kalibrierungs-Funktion
  - Smooth Animation (60 FPS)

  Lernziele:
  - I2C-Kommunikation (MPU6050)
  - Sensor-Daten auslesen & verarbeiten
  - Physik-Simulation (Gravitation, Reibung, Kollision)
  - 3D-Grafik-Effekte (Radial-Gradient)
  - Delta-Time für smooth Animation
  - Kalibrierung

  Hardware:
  - 1x CYD Display (2.8" oder 3.5")
  - 1x MPU6050 Sensor (GY-521 Modul)
  - 4x Dupont-Kabel (VCC, GND, SDA, SCL)

  MPU6050 Verkabelung:
  - VCC → 3.3V (NICHT 5V!)
  - GND → GND
  - SDA → GPIO 22 (extSDA)
  - SCL → GPIO 27 (extSCL)

  Installation:
  1. MPU6050 Library installieren:
     Arduino IDE: Library Manager → "Adafruit MPU6050" → installieren
     (Installiert auch Adafruit Sensor & BusIO automatisch)

     PlatformIO: lib_deps = adafruit/Adafruit MPU6050 @ ^2.2.0

  2. In CYD_Display_Config.h definieren (falls nicht vorhanden):
     #define extSDA 22
     #define extSCL 27

  Steuerung:
  - Touch irgendwo: Kalibrieren (aktuellen Winkel als 0° setzen)
  - Neige CYD: Kugel rollt entsprechend
*/

#include <LovyanGFX.hpp>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// I2C Pins (CYD Standard - external I2C)
#ifndef extSDA
#define extSDA 22
#endif

#ifndef extSCL
#define extSCL 27
#endif

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
#define COLOR_BG 0x18E3          // Dunkles Grau
#define COLOR_CIRCLE 0x4208      // Wasserwaagen-Kreise
#define COLOR_BALL 0xFD20        // Orange (Ball Hauptfarbe)
#define COLOR_BALL_LIGHT 0xFFE0  // Gelb (Lichtreflex)
#define COLOR_BALL_DARK 0x8C00   // Dunkelorange (Schatten)
#define COLOR_SHADOW 0x2104      // Schatten unter Ball
#define COLOR_TEXT 0xFFFF        // Weiß
#define COLOR_LEVEL 0x07E0       // Grün (Level OK)
#define COLOR_ERROR 0xF800       // Rot (Nicht Level)

// Physik-Konstanten
const float GRAVITY_SCALE = 15.0;    // Skalierung der Gravitation
const float FRICTION = 0.98;         // Reibung (0.0 - 1.0)
const float BOUNCE = 0.6;            // Bounce-Faktor (0.0 - 1.0)
const float MAX_VELOCITY = 8.0;      // Maximale Geschwindigkeit

// Ball-Parameter
const float BALL_RADIUS = 20.0;
const float AREA_SIZE = 100.0;       // Wasserwaagen-Bereich (Radius)

// Wasserwaagen-Toleranz
const float LEVEL_THRESHOLD = 2.0;   // Grad für "Level"

// ===== GLOBALE VARIABLEN =====

LGFX lcd;
Adafruit_MPU6050 mpu;

// Ball-Position & Physik
float ballX = 120.0;
float ballY = 160.0;
float velocityX = 0.0;
float velocityY = 0.0;

// Kalibrierung
float offsetX = 0.0;
float offsetY = 0.0;

// Sensor-Daten
float accelX = 0.0;
float accelY = 0.0;
float angleX = 0.0;  // Grad
float angleY = 0.0;  // Grad

// Timing
unsigned long lastFrame = 0;

// Status
bool mpuConnected = false;

// ===== SETUP =====

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Spirit Level Demo ===");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(0);  // Portrait
  lcd.setBrightness(255);
  lcd.fillScreen(COLOR_BG);

  // I2C initialisieren
  Serial.printf("Initializing I2C (SDA=%d, SCL=%d)...\n", extSDA, extSCL);
  Wire.begin(extSDA, extSCL);

  // MPU6050 initialisieren
  if (!mpu.begin(0x68, &Wire)) {  // 0x68 = Standard I2C-Adresse
    Serial.println("MPU6050 not found!");
    drawErrorScreen();
    mpuConnected = false;
    return;
  }

  mpuConnected = true;
  Serial.println("MPU6050 found!");

  // MPU6050 konfigurieren
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("Accelerometer range: ±2G");
  Serial.println("Gyro range: ±250°/s");
  Serial.println("Filter: 21 Hz");

  // Initiale Kalibrierung (3 Sekunden warten)
  lcd.fillScreen(COLOR_BG);
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(20, 140);
  lcd.print("Calibrating...");
  lcd.setTextSize(1);
  lcd.setCursor(30, 170);
  lcd.print("Keep level!");

  delay(1000);

  calibrate();

  Serial.println("Calibration done!");
  Serial.println("Ready! Touch screen to recalibrate.");

  lastFrame = millis();
}

// ===== LOOP =====

void loop() {
  if (!mpuConnected) {
    delay(1000);
    return;
  }

  // Delta Time berechnen
  unsigned long currentTime = millis();
  float deltaTime = (currentTime - lastFrame) / 1000.0;  // in Sekunden
  lastFrame = currentTime;

  // Begrenze delta time (bei Lag)
  if (deltaTime > 0.05) deltaTime = 0.05;  // Max 50ms

  // Sensor auslesen
  readSensor();

  // Physik-Update
  updatePhysics(deltaTime);

  // Touch prüfen (Kalibrierung)
  handleTouch();

  // Zeichnen
  drawUI();

  // Frame-Rate-Limit (~60 FPS)
  delay(16);
}

// ===== MPU6050 FUNKTIONEN =====

void readSensor() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Accelerometer (in g)
  accelX = a.acceleration.x - offsetX;
  accelY = a.acceleration.y - offsetY;

  // Winkel berechnen (in Grad)
  // atan2 gibt Radiant, * 57.2958 = Grad
  angleX = atan2(accelY, accelZ()) * 57.2958;
  angleY = atan2(accelX, accelZ()) * 57.2958;

  // Begrenze Winkel
  angleX = constrain(angleX, -90, 90);
  angleY = constrain(angleY, -90, 90);
}

float accelZ() {
  // Z-Komponente aus X und Y berechnen (für Winkel)
  // Annahme: Gesamtbeschleunigung = 1g (Gravitation)
  float z = sqrt(1.0 - accelX * accelX - accelY * accelY);
  if (isnan(z)) z = 0.01;
  return z;
}

void calibrate() {
  Serial.println("Calibrating...");

  float sumX = 0;
  float sumY = 0;
  const int samples = 100;

  for (int i = 0; i < samples; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    sumX += a.acceleration.x;
    sumY += a.acceleration.y;

    delay(10);
  }

  offsetX = sumX / samples;
  offsetY = sumY / samples;

  Serial.printf("Offsets: X=%.3f, Y=%.3f\n", offsetX, offsetY);
}

// ===== PHYSIK =====

void updatePhysics(float dt) {
  // Beschleunigung aus Sensor (invertiert für natürliche Bewegung)
  float accelForceX = -accelX * GRAVITY_SCALE;
  float accelForceY = accelY * GRAVITY_SCALE;

  // Geschwindigkeit aktualisieren
  velocityX += accelForceX * dt;
  velocityY += accelForceY * dt;

  // Reibung anwenden
  velocityX *= FRICTION;
  velocityY *= FRICTION;

  // Maximale Geschwindigkeit begrenzen
  velocityX = constrain(velocityX, -MAX_VELOCITY, MAX_VELOCITY);
  velocityY = constrain(velocityY, -MAX_VELOCITY, MAX_VELOCITY);

  // Position aktualisieren
  ballX += velocityX * dt * 60.0;  // * 60 für bessere Skalierung
  ballY += velocityY * dt * 60.0;

  // Kollision mit Begrenzung (Kreis)
  float centerX = 120.0;
  float centerY = 160.0;
  float maxDist = AREA_SIZE - BALL_RADIUS;

  float dx = ballX - centerX;
  float dy = ballY - centerY;
  float dist = sqrt(dx * dx + dy * dy);

  if (dist > maxDist) {
    // Ball ist außerhalb → Zurückprallen

    // Normalisiere Richtungsvektor
    float nx = dx / dist;
    float ny = dy / dist;

    // Setze Ball auf Rand
    ballX = centerX + nx * maxDist;
    ballY = centerY + ny * maxDist;

    // Reflektiere Geschwindigkeit (Bounce)
    float dotProduct = velocityX * nx + velocityY * ny;
    velocityX -= 2 * dotProduct * nx;
    velocityY -= 2 * dotProduct * ny;

    // Bounce-Dämpfung
    velocityX *= BOUNCE;
    velocityY *= BOUNCE;
  }
}

// ===== TOUCH HANDLING =====

void handleTouch() {
  static bool wasTouched = false;
  uint16_t x, y;
  bool touched = lcd.getTouch(&x, &y);

  if (touched && !wasTouched) {
    wasTouched = true;

    // Kalibrieren
    Serial.println("Recalibrating...");

    lcd.fillRect(0, 280, 240, 40, COLOR_BG);
    lcd.setTextSize(1);
    lcd.setTextColor(COLOR_LEVEL);
    lcd.setCursor(60, 295);
    lcd.print("Calibrating...");

    delay(500);
    calibrate();

    Serial.println("Calibration complete!");
  }

  if (!touched) {
    wasTouched = false;
  }
}

// ===== UI ZEICHNEN =====

void drawUI() {
  // Hintergrund löschen (nur bei Bedarf - hier immer für smooth Animation)
  lcd.fillScreen(COLOR_BG);

  // Wasserwaagen-Kreise zeichnen
  drawLevelCircles();

  // Schatten zeichnen
  drawShadow();

  // Ball zeichnen
  drawBall();

  // Kreuz in der Mitte (Ziel)
  drawCrosshair();

  // Level-Anzeige
  drawLevelIndicator();

  // Winkel-Anzeige
  drawAngleDisplay();
}

void drawLevelCircles() {
  int centerX = 120;
  int centerY = 160;

  // Konzentrische Kreise (wie echte Wasserwaage)
  for (int r = 20; r <= 100; r += 20) {
    lcd.drawCircle(centerX, centerY, r, COLOR_CIRCLE);
  }

  // Äußerer Rand (dicker)
  lcd.drawCircle(centerX, centerY, AREA_SIZE, COLOR_CIRCLE);
  lcd.drawCircle(centerX, centerY, AREA_SIZE - 1, COLOR_CIRCLE);
}

void drawCrosshair() {
  int centerX = 120;
  int centerY = 160;
  int size = 10;

  // Kreuz
  lcd.drawLine(centerX - size, centerY, centerX + size, centerY, COLOR_LEVEL);
  lcd.drawLine(centerX, centerY - size, centerX, centerY + size, COLOR_LEVEL);

  // Kleiner Kreis in Mitte
  lcd.drawCircle(centerX, centerY, 3, COLOR_LEVEL);
}

void drawShadow() {
  // Schatten ist etwas versetzt (als ob Licht von oben links kommt)
  int shadowX = ballX + 5;
  int shadowY = ballY + 5;

  // Schatten als mehrere Kreise mit abnehmender Deckkraft (Blur-Effekt)
  for (int r = BALL_RADIUS; r >= 0; r -= 2) {
    // Alpha-Blending simulieren durch dunklere Farbe
    uint16_t shadowColor = COLOR_SHADOW;
    lcd.fillCircle(shadowX, shadowY, r, shadowColor);
  }
}

void drawBall() {
  // 3D-Kugel mit Radial-Gradient

  // Lichtquelle (oben links vom Ball-Zentrum)
  int lightOffsetX = -6;
  int lightOffsetY = -6;

  for (int r = BALL_RADIUS; r >= 0; r--) {
    // Berechne Farbe basierend auf Abstand vom Zentrum
    float t = 1.0 - (float)r / BALL_RADIUS;  // 0.0 (Rand) - 1.0 (Zentrum)

    // Gradient von dunkel (außen) zu hell (innen)
    uint16_t color;

    if (t < 0.3) {
      // Äußerer Rand - dunkel
      color = COLOR_BALL_DARK;
    } else if (t < 0.7) {
      // Mitte - Hauptfarbe
      color = COLOR_BALL;
    } else {
      // Zentrum - Lichtreflex
      color = COLOR_BALL_LIGHT;
    }

    // Lichtreflex (kleiner heller Fleck)
    int highlightX = ballX + lightOffsetX;
    int highlightY = ballY + lightOffsetY;

    if (r > BALL_RADIUS * 0.7) {
      lcd.fillCircle(ballX, ballY, r, color);
    } else if (r > BALL_RADIUS * 0.3) {
      lcd.fillCircle(ballX, ballY, r, color);
    } else {
      // Highlight
      lcd.fillCircle(highlightX, highlightY, r, COLOR_BALL_LIGHT);
    }
  }

  // Umrandung
  lcd.drawCircle(ballX, ballY, BALL_RADIUS, COLOR_BALL_DARK);
}

void drawLevelIndicator() {
  // Große "LEVEL" Anzeige wenn nahe an 0°

  float totalAngle = sqrt(angleX * angleX + angleY * angleY);

  if (totalAngle < LEVEL_THRESHOLD) {
    // LEVEL!
    lcd.fillRect(60, 20, 120, 40, COLOR_LEVEL);
    lcd.setTextSize(3);
    lcd.setTextColor(COLOR_BG);
    lcd.setCursor(75, 28);
    lcd.print("LEVEL");

  } else {
    // Nicht level - zeige Neigung
    lcd.setTextSize(2);
    lcd.setTextColor(COLOR_ERROR);
    lcd.setCursor(70, 25);
    lcd.printf("%.1f", totalAngle);
    lcd.print((char)247);  // Grad-Symbol
  }
}

void drawAngleDisplay() {
  // Winkel-Anzeige unten

  lcd.fillRect(0, 280, 240, 40, 0x0000);  // Schwarzer Hintergrund

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT);

  // X-Achse (Links/Rechts)
  lcd.setCursor(10, 285);
  lcd.print("X:");
  lcd.setCursor(30, 285);
  if (angleX >= 0) lcd.print(" ");
  lcd.printf("%.1f", angleX);
  lcd.print((char)247);

  // Y-Achse (Vor/Zurück)
  lcd.setCursor(10, 300);
  lcd.print("Y:");
  lcd.setCursor(30, 300);
  if (angleY >= 0) lcd.print(" ");
  lcd.printf("%.1f", angleY);
  lcd.print((char)247);

  // Beschleunigung (rechts)
  lcd.setCursor(130, 285);
  lcd.print("aX:");
  lcd.setCursor(155, 285);
  if (accelX >= 0) lcd.print(" ");
  lcd.printf("%.2f", accelX);
  lcd.print("g");

  lcd.setCursor(130, 300);
  lcd.print("aY:");
  lcd.setCursor(155, 300);
  if (accelY >= 0) lcd.print(" ");
  lcd.printf("%.2f", accelY);
  lcd.print("g");

  // Touch-Hilfe (ganz unten, klein)
  lcd.setTextColor(0x7BEF);  // Grau
  lcd.setCursor(50, 312);
  lcd.print("Touch to calibrate");
}

void drawErrorScreen() {
  lcd.fillScreen(COLOR_BG);

  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_ERROR);
  lcd.setCursor(20, 100);
  lcd.print("MPU6050 Error!");

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT);

  lcd.setCursor(10, 140);
  lcd.print("Check wiring:");

  lcd.setCursor(10, 160);
  lcd.printf("SDA -> GPIO %d", extSDA);

  lcd.setCursor(10, 175);
  lcd.printf("SCL -> GPIO %d", extSCL);

  lcd.setCursor(10, 190);
  lcd.print("VCC -> 3.3V (NOT 5V!)");

  lcd.setCursor(10, 205);
  lcd.print("GND -> GND");

  lcd.setCursor(10, 230);
  lcd.print("I2C Address: 0x68");

  lcd.setCursor(10, 250);
  lcd.print("Check Serial Monitor");
  lcd.setCursor(10, 265);
  lcd.print("for details.");
}
