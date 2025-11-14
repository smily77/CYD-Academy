/*
  13b_Racing_Track_Obstacles - Rennstrecke mit Hindernissen und Kollisionserkennung

  Erweitert das 13_Racing_Track Beispiel um:
  - Zufällige Hindernisse auf der Strecke
  - Kollisionserkennung zwischen Auto und Hindernissen
  - Score-System für ausgewichene Hindernisse
  - Visual Feedback bei Crash

  Lernziele:
  - Kollisionserkennung (Distanz-basiert)
  - Bounding Boxes / Collision Circles
  - Objekt-Management (Arrays, Structs)
  - Game State (Score, Crashes)
  - Visual Feedback (Flash-Effekte)

  Hardware:
  - 1x CYD Display (2.8" oder 3.5")
  - Nur Touch-Input!

  Steuerung:
  - Touch oben: Beschleunigen
  - Touch unten: Bremsen
  - Kein Touch: Langsam abbremsen
  - ZIEL: Hindernisse ausweichen für Punkte!
*/

#include <LovyanGFX.hpp>

// Display-Konfiguration (automatische Erkennung)
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
#define COLOR_GRASS 0x8BC34A     // Grünes Gras
#define COLOR_ROAD_OUTER 0x4208  // Dunkler Asphalt
#define COLOR_ROAD_INNER 0x630C  // Hellerer Asphalt
#define COLOR_LINE 0xFFFF        // Weiße Mittellinie
#define COLOR_CAR 0xF800         // Rotes Auto
#define COLOR_TEXT 0xFFFF        // Weißer Text
#define COLOR_BARREL 0xFD20      // Orange (Ölfass)
#define COLOR_CONE 0xFFE0        // Gelb (Verkehrskegel)
#define COLOR_ROCK 0x7BEF        // Grau (Stein)
#define COLOR_CRASH 0xF800       // Rot (Flash bei Crash)

// Pfad-Punkte
const int PATH_POINTS = 64;
struct Point {
  float x, y;
};

Point trackPath[PATH_POINTS];

// Hindernisse
const int MAX_OBSTACLES = 12;
enum ObstacleType {
  OBS_BARREL,   // Ölfass
  OBS_CONE,     // Verkehrskegel
  OBS_ROCK      // Stein
};

struct Obstacle {
  float pathPosition;     // Position auf Pfad (0.0 - 1.0)
  ObstacleType type;
  float radius;           // Kollisions-Radius
  bool avoided;           // Wurde ausgewichen?
  bool hitThisLap;        // Wurde diese Runde getroffen?
};

Obstacle obstacles[MAX_OBSTACLES];

// Auto-Parameter
float carPosition = 0.0;
float carSpeed = 0.0;
const float MAX_SPEED = 0.015;
const float ACCELERATION = 0.0005;
const float BRAKE = 0.001;
const float FRICTION = 0.0002;
const float CRASH_SPEED_REDUCTION = 0.5;  // Bei Crash: 50% Geschwindigkeit

// Game State
int score = 0;
int crashes = 0;
unsigned long crashFlashTime = 0;
const int FLASH_DURATION = 200;  // ms

// Kollisions-Distanzen
const float COLLISION_DISTANCE = 12.0;  // Pixel - direkte Kollision
const float AVOID_DISTANCE = 18.0;      // Pixel - knappe Ausweichen (Score)

// Rundenzeit
unsigned long lapStartTime = 0;
unsigned long bestLapTime = 0;
int lapCount = 0;

// Display
LGFX lcd;

// ===== SETUP =====

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Racing Track with Obstacles Demo ===");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(0);  // Portrait
  lcd.setBrightness(255);

  // Pfad generieren
  generateTrackPath();

  // Hindernisse platzieren
  generateObstacles();

  // Initiales Zeichnen
  drawTrack();

  lapStartTime = millis();

  Serial.println("Ready! Weiche Hindernissen aus für Punkte!");
  Serial.println("Touch: Oben=Gas, Unten=Bremse");
}

// ===== LOOP =====

void loop() {
  static unsigned long lastFrame = 0;
  unsigned long currentTime = millis();
  float deltaTime = (currentTime - lastFrame) / 1000.0;
  lastFrame = currentTime;

  // Touch lesen
  handleInput();

  // Geschwindigkeit aktualisieren (mit Reibung)
  if (carSpeed > 0) {
    carSpeed -= FRICTION;
    if (carSpeed < 0) carSpeed = 0;
  }

  // Position aktualisieren
  carPosition += carSpeed;

  // Kollisionserkennung
  checkCollisions();

  // Runde erkennen
  if (carPosition >= 1.0) {
    carPosition -= 1.0;

    // Rundenzeit
    unsigned long lapTime = currentTime - lapStartTime;
    lapCount++;

    if (bestLapTime == 0 || lapTime < bestLapTime) {
      bestLapTime = lapTime;
    }

    Serial.printf("Runde %d: %.2f s | Score: %d | Crashes: %d\n",
                  lapCount, lapTime / 1000.0, score, crashes);

    // Reset avoided flags für neue Runde
    resetObstacleFlags();

    lapStartTime = currentTime;
  }

  // Neu zeichnen
  drawTrack();
  drawObstacles();
  drawCar();
  drawUI();

  // Flash-Effekt bei Crash
  if (currentTime - crashFlashTime < FLASH_DURATION) {
    // Roten Overlay für Crash
    lcd.fillRect(0, 0, lcd.width(), 20, COLOR_CRASH);
  }

  delay(16);  // ~60 FPS
}

// ===== FUNKTIONEN =====

void generateTrackPath() {
  // Identisch zu 13_Racing_Track
  for (int i = 0; i < PATH_POINTS; i++) {
    float t = (float)i / PATH_POINTS;
    float angle = t * 2 * PI;

    float x = 120 + sin(angle) * 80;
    float y = 160 + sin(angle * 2) * 120;

    trackPath[i].x = x;
    trackPath[i].y = y;
  }
}

void generateObstacles() {
  // Zufällige Hindernisse auf der Strecke platzieren
  randomSeed(analogRead(0));  // Random Seed

  for (int i = 0; i < MAX_OBSTACLES; i++) {
    obstacles[i].pathPosition = random(100, 900) / 1000.0;  // 0.1 - 0.9
    obstacles[i].type = (ObstacleType)random(0, 3);  // 0-2

    // Radius je nach Typ
    switch (obstacles[i].type) {
      case OBS_BARREL:
        obstacles[i].radius = 6.0;
        break;
      case OBS_CONE:
        obstacles[i].radius = 5.0;
        break;
      case OBS_ROCK:
        obstacles[i].radius = 7.0;
        break;
    }

    obstacles[i].avoided = false;
    obstacles[i].hitThisLap = false;
  }

  // Sortiere nach Position für bessere Übersicht
  // (Optional - Bubble Sort für kleine Arrays)
  for (int i = 0; i < MAX_OBSTACLES - 1; i++) {
    for (int j = 0; j < MAX_OBSTACLES - i - 1; j++) {
      if (obstacles[j].pathPosition > obstacles[j + 1].pathPosition) {
        Obstacle temp = obstacles[j];
        obstacles[j] = obstacles[j + 1];
        obstacles[j + 1] = temp;
      }
    }
  }

  Serial.println("Hindernisse platziert:");
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    Serial.printf("  #%d: Pos %.2f, Type %d\n",
                  i, obstacles[i].pathPosition, obstacles[i].type);
  }
}

void resetObstacleFlags() {
  // Reset für neue Runde
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    obstacles[i].avoided = false;
    obstacles[i].hitThisLap = false;
  }
}

Point getPointOnPath(float t) {
  // Interpoliere zwischen Pfadpunkten
  t = fmod(t, 1.0);
  if (t < 0) t += 1.0;

  float index = t * PATH_POINTS;
  int i0 = (int)index;
  int i1 = (i0 + 1) % PATH_POINTS;
  float frac = index - i0;

  Point p;
  p.x = trackPath[i0].x + (trackPath[i1].x - trackPath[i0].x) * frac;
  p.y = trackPath[i0].y + (trackPath[i1].y - trackPath[i0].y) * frac;

  return p;
}

float getAngle(float t) {
  // Berechne Richtung (Tangente)
  Point p1 = getPointOnPath(t);
  Point p2 = getPointOnPath(t + 0.01);

  return atan2(p2.y - p1.y, p2.x - p1.x);
}

float getDistance(Point p1, Point p2) {
  // Euklidische Distanz
  float dx = p2.x - p1.x;
  float dy = p2.y - p1.y;
  return sqrt(dx * dx + dy * dy);
}

void checkCollisions() {
  Point carPos = getPointOnPath(carPosition);

  for (int i = 0; i < MAX_OBSTACLES; i++) {
    Obstacle &obs = obstacles[i];

    // Position des Hindernisses berechnen
    Point obsPos = getPointOnPath(obs.pathPosition);

    // Distanz berechnen
    float dist = getDistance(carPos, obsPos);

    // Direkte Kollision
    if (dist < COLLISION_DISTANCE && !obs.hitThisLap) {
      handleCrash(i);
    }
    // Knappes Ausweichen (Score)
    else if (dist < AVOID_DISTANCE && !obs.avoided && !obs.hitThisLap) {
      handleAvoid(i);
    }
  }
}

void handleCrash(int obsIndex) {
  obstacles[obsIndex].hitThisLap = true;
  crashes++;

  // Geschwindigkeit reduzieren
  carSpeed *= CRASH_SPEED_REDUCTION;

  // Flash-Effekt
  crashFlashTime = millis();

  Serial.printf("CRASH! Hindernis #%d getroffen! Crashes: %d\n", obsIndex, crashes);
}

void handleAvoid(int obsIndex) {
  obstacles[obsIndex].avoided = true;
  score++;

  Serial.printf("Ausgewichen! Hindernis #%d | Score: %d\n", obsIndex, score);
}

void handleInput() {
  uint16_t x, y;
  bool touched = lcd.getTouch(&x, &y);

  if (touched) {
    if (y < lcd.height() / 2) {
      // Obere Hälfte: Beschleunigen
      carSpeed += ACCELERATION;
      if (carSpeed > MAX_SPEED) carSpeed = MAX_SPEED;
    } else {
      // Untere Hälfte: Bremsen
      carSpeed -= BRAKE;
      if (carSpeed < 0) carSpeed = 0;
    }
  }
}

void drawTrack() {
  // Hintergrund (Gras)
  lcd.fillScreen(COLOR_GRASS);

  // Äußere Straße (dunkler Asphalt)
  for (int i = 0; i < PATH_POINTS; i++) {
    lcd.fillCircle(trackPath[i].x, trackPath[i].y, 40, COLOR_ROAD_OUTER);
  }

  // Innere Straße (heller Asphalt)
  for (int i = 0; i < PATH_POINTS; i++) {
    lcd.fillCircle(trackPath[i].x, trackPath[i].y, 25, COLOR_ROAD_INNER);
  }

  // Mittellinie (gestrichelt)
  for (int i = 0; i < PATH_POINTS; i += 2) {
    int next = (i + 1) % PATH_POINTS;
    lcd.drawLine(trackPath[i].x, trackPath[i].y,
                 trackPath[next].x, trackPath[next].y, COLOR_LINE);
  }

  // Bäume/Dekoration
  lcd.fillCircle(30, 80, 8, 0x2D5A);
  lcd.fillCircle(210, 80, 8, 0x2D5A);
  lcd.fillCircle(30, 240, 8, 0x2D5A);
  lcd.fillCircle(210, 240, 8, 0x2D5A);
  lcd.fillCircle(120, 20, 8, 0x2D5A);
  lcd.fillCircle(120, 300, 8, 0x2D5A);
}

void drawObstacles() {
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    Point pos = getPointOnPath(obstacles[i].pathPosition);

    // Farbe und Form je nach Typ
    switch (obstacles[i].type) {
      case OBS_BARREL:
        // Ölfass (Orange Kreis mit Streifen)
        lcd.fillCircle(pos.x, pos.y, obstacles[i].radius, COLOR_BARREL);
        lcd.drawCircle(pos.x, pos.y, obstacles[i].radius - 2, 0x0000);
        break;

      case OBS_CONE:
        // Verkehrskegel (Gelbes Dreieck)
        lcd.fillCircle(pos.x, pos.y, obstacles[i].radius, COLOR_CONE);
        lcd.fillTriangle(pos.x, pos.y - 5,
                        pos.x - 4, pos.y + 4,
                        pos.x + 4, pos.y + 4,
                        COLOR_CONE);
        lcd.drawCircle(pos.x, pos.y, obstacles[i].radius - 1, 0xF800);  // Roter Ring
        break;

      case OBS_ROCK:
        // Stein (Grauer unregelmäßiger Kreis)
        lcd.fillCircle(pos.x, pos.y, obstacles[i].radius, COLOR_ROCK);
        lcd.fillCircle(pos.x - 2, pos.y - 2, 3, 0x5AEB);  // Dunkler Fleck
        lcd.fillCircle(pos.x + 2, pos.y + 1, 2, 0x9CF3);  // Heller Fleck
        break;
    }

    // Debug: Zeige Kollisions-Radius (optional)
    // lcd.drawCircle(pos.x, pos.y, COLLISION_DISTANCE, 0xF800);  // Rot
    // lcd.drawCircle(pos.x, pos.y, AVOID_DISTANCE, 0x07E0);      // Grün
  }
}

void drawCar() {
  Point pos = getPointOnPath(carPosition);
  float angle = getAngle(carPosition);

  // Auto (Rechteck mit Rotation)
  float carWidth = 8;
  float carLength = 14;

  // Lokale Koordinaten
  float corners[4][2] = {
    {-carLength/2, -carWidth/2},
    {carLength/2, -carWidth/2},
    {carLength/2, carWidth/2},
    {-carLength/2, carWidth/2}
  };

  // Rotieren und transformieren
  int16_t rotated[4][2];
  for (int i = 0; i < 4; i++) {
    float rx = corners[i][0] * cos(angle) - corners[i][1] * sin(angle);
    float ry = corners[i][0] * sin(angle) + corners[i][1] * cos(angle);
    rotated[i][0] = pos.x + rx;
    rotated[i][1] = pos.y + ry;
  }

  // Auto zeichnen
  lcd.fillTriangle(rotated[0][0], rotated[0][1],
                   rotated[1][0], rotated[1][1],
                   rotated[2][0], rotated[2][1], COLOR_CAR);
  lcd.fillTriangle(rotated[0][0], rotated[0][1],
                   rotated[2][0], rotated[2][1],
                   rotated[3][0], rotated[3][1], COLOR_CAR);

  // Richtungs-Indikator
  float frontX = pos.x + cos(angle) * (carLength/2 + 2);
  float frontY = pos.y + sin(angle) * (carLength/2 + 2);
  lcd.fillCircle(frontX, frontY, 2, 0xFFE0);  // Gelb
}

void drawUI() {
  // Schwarzer Hintergrund für bessere Lesbarkeit
  lcd.fillRect(0, 0, 240, 70, 0x0000);

  // Score (groß und prominent)
  lcd.setTextSize(2);
  lcd.setTextColor(0x07E0, 0x0000);  // Grün
  lcd.setCursor(5, 5);
  lcd.printf("Score: %d", score);

  // Crashes
  lcd.setTextSize(1);
  lcd.setTextColor(0xF800, 0x0000);  // Rot
  lcd.setCursor(5, 28);
  lcd.printf("Crashes: %d", crashes);

  // Geschwindigkeit
  lcd.setTextColor(COLOR_TEXT, 0x0000);
  lcd.setCursor(5, 40);
  lcd.printf("Speed: %.0f%%", (carSpeed / MAX_SPEED) * 100);

  // Rundenzeit
  unsigned long currentLapTime = millis() - lapStartTime;
  lcd.setCursor(5, 52);
  lcd.printf("Lap: %.1f s", currentLapTime / 1000.0);

  // Rechte Seite: Runden & Best
  lcd.setCursor(140, 28);
  lcd.printf("Lap: %d", lapCount);

  if (bestLapTime > 0) {
    lcd.setCursor(140, 40);
    lcd.printf("Best:");
    lcd.setCursor(140, 52);
    lcd.printf("%.1fs", bestLapTime / 1000.0);
  }

  // Touch-Hilfe (klein, unten)
  lcd.setTextColor(0x7BEF, COLOR_GRASS);  // Grau auf Gras
  lcd.setCursor(5, 305);
  lcd.print("Touch: Top=Gas Bot=Brake");
}
