/*
  16b_Spirit_Level_Maze - Labyrinth-Spiel mit MPU6050 & Kollisionserkennung

  Erweitert 16_Spirit_Level um Labyrinth-Wände und Circle-AABB Kollision.

  Funktionen:
  - MPU6050 Steuerung (wie 16)
  - Labyrinth aus Wänden (Rechtecke)
  - Circle-to-AABB Kollisionserkennung
  - Ziel (grüner Kreis) erreichen
  - Mehrere Levels (steigender Schwierigkeitsgrad)
  - Timer & Moves Counter
  - Visuelle Kollisions-Effekte

  Lernziele:
  - AABB (Axis-Aligned Bounding Box) Kollision
  - Circle-to-Rectangle Kollision
  - Maze-Design & Navigation
  - Level-System
  - Collision Response (verschiedene Kanten)
  - Game State Management

  Hardware:
  - 1x CYD Display (2.8" oder 3.5")
  - 1x MPU6050 Sensor (wie 16)
  - 4x Dupont-Kabel

  Verkabelung: Siehe 16_Spirit_Level

  Steuerung:
  - Neigen: Kugel bewegt sich
  - Touch: Nächstes Level / Restart
  - Ziel: Erreiche grünen Kreis!
*/

#include <CYD_Display_Config.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// I2C Pins
#ifndef extSDA
#define extSDA 22
#endif

#ifndef extSCL
#define extSCL 27
#endif

// ===== KONSTANTEN =====

// Farben
#define COLOR_BG 0x18E3          // Dunkles Grau
#define COLOR_WALL 0x4208        // Dunkelgrau (Wände)
#define COLOR_WALL_LIGHT 0x8410  // Hellgrau (Wand-Top)
#define COLOR_WALL_DARK 0x2104   // Dunkelgrau (Wand-Schatten)
#define COLOR_BALL 0xFD20        // Orange
#define COLOR_BALL_LIGHT 0xFFE0  // Gelb (Highlight)
#define COLOR_BALL_DARK 0x8C00   // Dunkelorange
#define COLOR_SHADOW 0x2104      // Schatten
#define COLOR_GOAL 0x07E0        // Grün (Ziel)
#define COLOR_TEXT 0xFFFF        // Weiß
#define COLOR_COLLISION 0xF800   // Rot (Kollisions-Flash)

// Physik
const float GRAVITY_SCALE = 12.0;
const float FRICTION = 0.96;
const float BOUNCE = 0.4;        // Weniger Bounce gegen Wände
const float MAX_VELOCITY = 6.0;

// Ball
const float BALL_RADIUS = 8.0;   // Kleiner für Labyrinth

// Spielfeld
const int PLAY_AREA_X = 10;
const int PLAY_AREA_Y = 50;
const int PLAY_AREA_W = 220;
const int PLAY_AREA_H = 210;

// ===== STRUKTUREN =====

struct Wall {
  int x, y, w, h;
};

struct Goal {
  int x, y;
  int radius;
};

// ===== LEVEL-DEFINITIONEN =====

// Level 1: Einfach
Wall level1_walls[] = {
  // Rand
  {10, 50, 220, 5},    // Oben
  {10, 255, 220, 5},   // Unten
  {10, 50, 5, 210},    // Links
  {225, 50, 5, 210},   // Rechts

  // Innere Wände
  {60, 50, 5, 100},    // Vertikale Wand links
  {140, 110, 5, 100},  // Vertikale Wand rechts
  {100, 140, 80, 5},   // Horizontale Wand mitte
};
Goal level1_goal = {200, 230, 10};

// Level 2: Mittel
Wall level2_walls[] = {
  // Rand
  {10, 50, 220, 5},
  {10, 255, 220, 5},
  {10, 50, 5, 210},
  {225, 50, 5, 210},

  // Komplexeres Maze
  {60, 50, 5, 80},
  {60, 170, 5, 90},
  {140, 80, 5, 100},
  {140, 210, 5, 50},
  {10, 140, 90, 5},
  {140, 200, 90, 5},
};
Goal level2_goal = {200, 80, 10};

// Level 3: Schwer
Wall level3_walls[] = {
  // Rand
  {10, 50, 220, 5},
  {10, 255, 220, 5},
  {10, 50, 5, 210},
  {225, 50, 5, 210},

  // Enges Maze
  {50, 50, 5, 60},
  {50, 140, 5, 70},
  {100, 80, 5, 60},
  {100, 170, 5, 90},
  {150, 50, 5, 100},
  {150, 180, 5, 80},
  {10, 110, 100, 5},
  {100, 140, 100, 5},
  {50, 200, 120, 5},
};
Goal level3_goal = {200, 230, 10};

// Level-Array
Wall* levels[] = {level1_walls, level2_walls, level3_walls};
int levelWallCounts[] = {7, 10, 13};
Goal levelGoals[] = {level1_goal, level2_goal, level3_goal};
const int LEVEL_COUNT = 3;

// ===== GLOBALE VARIABLEN =====

LGFX lcd;
Adafruit_MPU6050 mpu;

// Ball
float ballX = 30.0;
float ballY = 70.0;
float velocityX = 0.0;
float velocityY = 0.0;

// Sensor
float accelX = 0.0;
float accelY = 0.0;
float offsetX = 0.0;
float offsetY = 0.0;

// Game State
int currentLevel = 0;
bool levelComplete = false;
bool mpuConnected = false;
unsigned long levelStartTime = 0;
unsigned long levelTime = 0;
int moves = 0;
bool lastTouched = false;

// Kollisions-Flash
unsigned long collisionFlashTime = 0;
const int FLASH_DURATION = 100;

// Timing
unsigned long lastFrame = 0;

// ===== SETUP =====

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Spirit Level Maze Demo ===");

  lcd.init();
  lcd.setRotation(0);
  lcd.setBrightness(255);
  lcd.fillScreen(COLOR_BG);

  // I2C & MPU6050 (wie 16)
  Wire.begin(extSDA, extSCL);

  if (!mpu.begin(0x68, &Wire)) {
    Serial.println("MPU6050 not found!");
    drawErrorScreen();
    return;
  }

  mpuConnected = true;
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Kalibrierung
  lcd.fillScreen(COLOR_BG);
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(20, 140);
  lcd.print("Calibrating...");
  delay(1000);

  calibrate();

  // Start Level 1
  startLevel(0);

  lastFrame = millis();
}

// ===== LOOP =====

void loop() {
  if (!mpuConnected) {
    delay(1000);
    return;
  }

  if (levelComplete) {
    handleLevelComplete();
    return;
  }

  // Delta Time
  unsigned long currentTime = millis();
  float deltaTime = (currentTime - lastFrame) / 1000.0;
  lastFrame = currentTime;
  if (deltaTime > 0.05) deltaTime = 0.05;

  // Sensor
  readSensor();

  // Physik
  updatePhysics(deltaTime);

  // Kollision mit Wänden
  checkWallCollisions();

  // Ziel-Check
  checkGoal();

  // Touch
  handleTouch();

  // Zeichnen
  drawUI();

  delay(16);
}

// ===== MPU6050 =====

void readSensor() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  accelX = a.acceleration.x - offsetX;
  accelY = a.acceleration.y - offsetY;
}

void calibrate() {
  float sumX = 0, sumY = 0;
  for (int i = 0; i < 100; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    sumX += a.acceleration.x;
    sumY += a.acceleration.y;
    delay(10);
  }
  offsetX = sumX / 100;
  offsetY = sumY / 100;
}

// ===== PHYSIK =====

void updatePhysics(float dt) {
  // Kraft aus Sensor
  float forceX = -accelX * GRAVITY_SCALE;
  float forceY = accelY * GRAVITY_SCALE;

  // Geschwindigkeit
  velocityX += forceX * dt;
  velocityY += forceY * dt;

  velocityX *= FRICTION;
  velocityY *= FRICTION;

  velocityX = constrain(velocityX, -MAX_VELOCITY, MAX_VELOCITY);
  velocityY = constrain(velocityY, -MAX_VELOCITY, MAX_VELOCITY);

  // Bewegungs-Counter (für Stats)
  if (abs(velocityX) > 0.5 || abs(velocityY) > 0.5) {
    moves++;
  }

  // Position
  ballX += velocityX * dt * 60.0;
  ballY += velocityY * dt * 60.0;
}

// ===== KOLLISION =====

void checkWallCollisions() {
  Wall* walls = levels[currentLevel];
  int wallCount = levelWallCounts[currentLevel];

  for (int i = 0; i < wallCount; i++) {
    Wall& wall = walls[i];

    // Circle-AABB Kollision
    if (checkCircleRectCollision(ballX, ballY, BALL_RADIUS, wall)) {
      resolveCollision(wall);

      // Flash-Effekt
      collisionFlashTime = millis();
    }
  }
}

bool checkCircleRectCollision(float cx, float cy, float radius, Wall& rect) {
  // Finde nächsten Punkt im Rechteck zum Kreis-Zentrum
  float closestX = constrain(cx, rect.x, rect.x + rect.w);
  float closestY = constrain(cy, rect.y, rect.y + rect.h);

  // Distanz von diesem Punkt zum Kreis-Zentrum
  float dx = cx - closestX;
  float dy = cy - closestY;
  float distSq = dx * dx + dy * dy;

  return distSq < (radius * radius);
}

void resolveCollision(Wall& wall) {
  // Finde welche Kante am nächsten ist
  float left = abs(ballX - wall.x);
  float right = abs(ballX - (wall.x + wall.w));
  float top = abs(ballY - wall.y);
  float bottom = abs(ballY - (wall.y + wall.h));

  float minDist = min(min(left, right), min(top, bottom));

  if (minDist == left) {
    // Linke Kante
    ballX = wall.x - BALL_RADIUS;
    velocityX = -abs(velocityX) * BOUNCE;
  } else if (minDist == right) {
    // Rechte Kante
    ballX = wall.x + wall.w + BALL_RADIUS;
    velocityX = abs(velocityX) * BOUNCE;
  } else if (minDist == top) {
    // Obere Kante
    ballY = wall.y - BALL_RADIUS;
    velocityY = -abs(velocityY) * BOUNCE;
  } else {
    // Untere Kante
    ballY = wall.y + wall.h + BALL_RADIUS;
    velocityY = abs(velocityY) * BOUNCE;
  }
}

void checkGoal() {
  Goal& goal = levelGoals[currentLevel];

  float dx = ballX - goal.x;
  float dy = ballY - goal.y;
  float dist = sqrt(dx * dx + dy * dy);

  if (dist < goal.radius + BALL_RADIUS) {
    // ZIEL ERREICHT!
    levelComplete = true;
    levelTime = millis() - levelStartTime;

    Serial.printf("Level %d complete! Time: %.1fs, Moves: %d\n",
                  currentLevel + 1, levelTime / 1000.0, moves / 60);
  }
}

// ===== LEVEL MANAGEMENT =====

void startLevel(int level) {
  currentLevel = level;
  levelComplete = false;

  // Ball-Position (Start ist immer links oben)
  ballX = 30.0;
  ballY = 70.0;
  velocityX = 0.0;
  velocityY = 0.0;

  // Stats
  levelStartTime = millis();
  moves = 0;

  Serial.printf("Starting Level %d\n", level + 1);
}

void handleLevelComplete() {
  // Zeige Gewinn-Screen
  lcd.fillScreen(COLOR_BG);

  lcd.setTextSize(3);
  lcd.setTextColor(COLOR_GOAL);
  lcd.setCursor(60, 100);
  lcd.print("GOAL!");

  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(40, 140);
  lcd.printf("Level %d", currentLevel + 1);

  lcd.setTextSize(1);
  lcd.setCursor(50, 170);
  lcd.printf("Time: %.1f s", levelTime / 1000.0);
  lcd.setCursor(50, 185);
  lcd.printf("Moves: %d", moves / 60);

  // Nächstes Level oder Ende
  if (currentLevel < LEVEL_COUNT - 1) {
    lcd.setTextColor(COLOR_GOAL);
    lcd.setCursor(40, 220);
    lcd.print("Touch for next level");
  } else {
    lcd.setTextColor(COLOR_GOAL);
    lcd.setCursor(30, 220);
    lcd.print("All levels complete!");
    lcd.setCursor(50, 240);
    lcd.print("Touch to restart");
  }

  // Warte auf Touch
  while (true) {
    uint16_t x, y;
    if (lcd.getTouch(&x, &y)) {
      delay(300);  // Debounce

      if (currentLevel < LEVEL_COUNT - 1) {
        // Nächstes Level
        startLevel(currentLevel + 1);
      } else {
        // Restart von Level 1
        startLevel(0);
      }

      break;
    }
    delay(50);
  }
}

// ===== TOUCH =====

void handleTouch() {
  uint16_t x, y;
  bool touched = lcd.getTouch(&x, &y);

  if (touched && !lastTouched) {
    // Touch-Event (optional: Pause/Menu)
  }

  lastTouched = touched;
}

// ===== UI ZEICHNEN =====

void drawUI() {
  lcd.fillScreen(COLOR_BG);

  // Header
  drawHeader();

  // Wände
  drawWalls();

  // Ziel
  drawGoal();

  // Schatten
  drawShadow();

  // Ball
  drawBall();

  // Kollisions-Flash
  if (millis() - collisionFlashTime < FLASH_DURATION) {
    lcd.drawCircle(ballX, ballY, BALL_RADIUS + 3, COLOR_COLLISION);
    lcd.drawCircle(ballX, ballY, BALL_RADIUS + 4, COLOR_COLLISION);
  }
}

void drawHeader() {
  // Level-Info
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(10, 10);
  lcd.printf("Level %d/%d", currentLevel + 1, LEVEL_COUNT);

  // Timer
  unsigned long elapsed = millis() - levelStartTime;
  lcd.setCursor(120, 10);
  lcd.printf("Time: %.1fs", elapsed / 1000.0);

  // Moves
  lcd.setCursor(10, 25);
  lcd.printf("Moves: %d", moves / 60);

  // Separator
  lcd.drawFastHLine(0, 45, 240, COLOR_WALL);
}

void drawWalls() {
  Wall* walls = levels[currentLevel];
  int wallCount = levelWallCounts[currentLevel];

  for (int i = 0; i < wallCount; i++) {
    Wall& wall = walls[i];

    // 3D-Effekt: Schatten unten-rechts
    lcd.fillRect(wall.x + 2, wall.y + 2, wall.w, wall.h, COLOR_WALL_DARK);

    // Hauptwand
    lcd.fillRect(wall.x, wall.y, wall.w, wall.h, COLOR_WALL);

    // Highlight oben-links
    lcd.drawLine(wall.x, wall.y, wall.x + wall.w, wall.y, COLOR_WALL_LIGHT);
    lcd.drawLine(wall.x, wall.y, wall.x, wall.y + wall.h, COLOR_WALL_LIGHT);
  }
}

void drawGoal() {
  Goal& goal = levelGoals[currentLevel];

  // Pulsierender Effekt
  int pulse = (millis() / 200) % 2;
  int radius = goal.radius + pulse;

  // Äußerer Kreis (leuchtend)
  lcd.fillCircle(goal.x, goal.y, radius + 2, COLOR_GOAL);

  // Innerer Kreis
  lcd.fillCircle(goal.x, goal.y, radius, 0x07E0);

  // Highlight
  lcd.fillCircle(goal.x - 3, goal.y - 3, 3, 0xFFFF);

  // "GOAL" Text in Nähe
  if (sqrt(pow(ballX - goal.x, 2) + pow(ballY - goal.y, 2)) < 40) {
    lcd.setTextSize(1);
    lcd.setTextColor(COLOR_GOAL);
    lcd.setCursor(goal.x - 15, goal.y + 20);
    lcd.print("GOAL");
  }
}

void drawShadow() {
  int shadowX = ballX + 3;
  int shadowY = ballY + 3;

  for (int r = BALL_RADIUS; r >= 0; r -= 2) {
    lcd.fillCircle(shadowX, shadowY, r, COLOR_SHADOW);
  }
}

void drawBall() {
  // 3D-Kugel (wie 16)
  for (int r = BALL_RADIUS; r >= 0; r--) {
    float t = 1.0 - (float)r / BALL_RADIUS;

    uint16_t color;
    if (t < 0.3) {
      color = COLOR_BALL_DARK;
    } else if (t < 0.7) {
      color = COLOR_BALL;
    } else {
      color = COLOR_BALL_LIGHT;
    }

    lcd.fillCircle(ballX, ballY, r, color);
  }

  // Highlight
  lcd.fillCircle(ballX - 3, ballY - 3, 3, COLOR_BALL_LIGHT);

  // Umrandung
  lcd.drawCircle(ballX, ballY, BALL_RADIUS, COLOR_BALL_DARK);
}

void drawErrorScreen() {
  lcd.fillScreen(COLOR_BG);
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_COLLISION);
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
}
