/*
  AsteroidsGame.h - Encapsulated Asteroids Game
  Teil der CYD_Games Library

  WICHTIG: CYD_Display_Config.h muss VOR diesem Header inkludiert werden!
*/

#ifndef ASTEROIDSGAME_H
#define ASTEROIDSGAME_H

#include <Arduino.h>
#include <math.h>
#include <CYD_Input.h>
// Forward declaration - LGFX muss bereits definiert sein
class LGFX;

// Pin-Mapping (muss VOR der Klasse definiert sein)
#define AST_POT_ROTATION  potiLeft
#define AST_BTN_LEFT      tasteB
#define AST_BTN_RIGHT     tasteC
#define AST_BTN_SHOOT     tasteA
#define AST_BTN_THRUST    tasteD

// Display-Dimensionen
#define AST_SCREEN_WIDTH  320
#define AST_SCREEN_HEIGHT 240

// Farben
#define AST_COLOR_BG      0x0000  // Schwarz
#define AST_COLOR_SHIP    0x07FF  // Cyan
#define AST_COLOR_THRUST  0xFD20  // Orange (Schub-Flamme)
#define AST_COLOR_BULLET  0xFFFF  // Weiß
#define AST_COLOR_ASTEROID 0xFFFF // Weiß
#define AST_COLOR_TEXT    0xFFFF  // Weiß

// Spieler-Schiff
struct AST_Ship {
  float x, y;           // Position
  float vx, vy;         // Geschwindigkeit
  float angle;          // Rotation in Grad (0 = nach oben)
  int size;             // Größe
  bool alive;
  int invulnerable;     // Frames invulnerabel nach Tod
};

// Asteroid
struct AST_Asteroid {
  float x, y;           // Position
  float vx, vy;         // Geschwindigkeit
  int size;             // 0=klein, 1=mittel, 2=groß
  int radius;           // Radius für Kollision
  bool active;
};

// Geschoss
struct AST_Bullet {
  float x, y;           // Position
  float vx, vy;         // Geschwindigkeit
  int life;             // Verbleibende Lebensdauer (Frames)
  bool active;
};

// Array-Größen
#define AST_MAX_ASTEROIDS 20
#define AST_MAX_BULLETS 5

// Konstanten
#define AST_SHIP_SIZE 8
#define AST_SHIP_ACCELERATION 0.15
#define AST_SHIP_MAX_SPEED 5.0
#define AST_SHIP_ROTATION_SPEED 5.0
#define AST_SHIP_FRICTION 0.98

#define AST_BULLET_SPEED 6.0
#define AST_BULLET_LIFE 60  // Frames

#define AST_ASTEROID_LARGE_RADIUS 20
#define AST_ASTEROID_MEDIUM_RADIUS 12
#define AST_ASTEROID_SMALL_RADIUS 6

#define AST_FRAME_DELAY 16  // ~60 FPS

class AsteroidsGame {
public:
  AsteroidsGame() :
    lcd(nullptr),
    score(0),
    lives(3),
    level(1),
    gameOver(false),
    lastFrame(0),
    lastShootState(false),
    usePotRotation(false)
  {
    // Konstruktor - Arrays werden in init() initialisiert
  }

  void init(LGFX* display, bool enablePotRotation = false) {
    lcd = display;
    usePotRotation = enablePotRotation;

    score = 0;
    level = 1;
    lives = 3;
    gameOver = false;
    lastFrame = 0;
    lastShootState = HIGH;

    // Schiff initialisieren
    ship.x = AST_SCREEN_WIDTH / 2;
    ship.y = AST_SCREEN_HEIGHT / 2;
    ship.vx = 0;
    ship.vy = 0;
    ship.angle = 0;  // Nach oben
    ship.size = AST_SHIP_SIZE;
    ship.alive = true;
    ship.invulnerable = 0;

    // Bullets initialisieren
    for (int i = 0; i < AST_MAX_BULLETS; i++) {
      bullets[i].active = false;
    }

    // Asteroids initialisieren
    for (int i = 0; i < AST_MAX_ASTEROIDS; i++) {
      asteroids[i].active = false;
    }

    // Input initialisieren
    CYD_Input::init();

    if (usePotRotation) {
      analogSetAttenuation(ADC_0db);  // 0-1V Range
      pinMode(AST_POT_ROTATION, INPUT);
    }

    // Bildschirm aufbauen
    lcd->fillScreen(AST_COLOR_BG);
    drawUI();

    // Level starten
    initLevel();

    Serial.println("Asteroids gestartet!");
  }

  void update() {
    // Frame-Timing
    unsigned long now = millis();
    if (now - lastFrame < AST_FRAME_DELAY) {
      return;
    }
    lastFrame = now;

    if (gameOver) {
      // Warte auf Neustart-Button
      if (CYD_Input::readButton(CYD_BTN_A)) {
        init(lcd, usePotRotation);
        delay(300);
      }
      return;
    }

    // Input
    handleInput();

    // Update
    updateShip();
    updateBullets();
    updateAsteroids();

    // Kollisionen
    checkCollisions();

    // Zeichnen
    drawGame();

    // Level komplett?
    if (checkLevelComplete()) {
      level++;
      Serial.printf("Level %d!\n", level);
      initLevel();
      delay(2000);
    }
  }

  // Status-Abfragen
  bool isGameOver() const { return gameOver; }
  int getScore() const { return score; }
  int getLevel() const { return level; }
  int getLives() const { return lives; }
  bool isPotRotationEnabled() const { return usePotRotation; }

private:
  LGFX* lcd;

  // Game State
  AST_Ship ship;
  AST_Asteroid asteroids[AST_MAX_ASTEROIDS];
  AST_Bullet bullets[AST_MAX_BULLETS];

  int score;
  int lives;
  int level;
  bool gameOver;
  bool usePotRotation;

  unsigned long lastFrame;
  bool lastShootState;

  // ===== INITIALISIERUNG =====

  void initLevel() {
    Serial.printf("Level %d gestartet!\n", level);

    // Schiff zurücksetzen
    ship.x = AST_SCREEN_WIDTH / 2;
    ship.y = AST_SCREEN_HEIGHT / 2;
    ship.vx = 0;
    ship.vy = 0;
    ship.angle = 0;
    ship.alive = true;
    ship.invulnerable = 60;  // 1 Sekunde invulnerabel

    // Asteroids spawnen (3 + Level)
    int numAsteroids = 3 + level;
    if (numAsteroids > 10) numAsteroids = 10;

    for (int i = 0; i < numAsteroids; i++) {
      spawnAsteroid(-1, -1, 2);  // Große Asteroids an zufälliger Position
    }
  }

  void spawnAsteroid(float x, float y, int size) {
    // Freien Slot finden
    for (int i = 0; i < AST_MAX_ASTEROIDS; i++) {
      if (!asteroids[i].active) {
        asteroids[i].active = true;
        asteroids[i].size = size;

        // Radius basierend auf Größe
        if (size == 2) asteroids[i].radius = AST_ASTEROID_LARGE_RADIUS;
        else if (size == 1) asteroids[i].radius = AST_ASTEROID_MEDIUM_RADIUS;
        else asteroids[i].radius = AST_ASTEROID_SMALL_RADIUS;

        // Position
        if (x < 0 || y < 0) {
          // Zufällige Position am Rand
          int edge = random(0, 4);
          if (edge == 0) { // Oben
            asteroids[i].x = random(0, AST_SCREEN_WIDTH);
            asteroids[i].y = -asteroids[i].radius;
          } else if (edge == 1) { // Rechts
            asteroids[i].x = AST_SCREEN_WIDTH + asteroids[i].radius;
            asteroids[i].y = random(0, AST_SCREEN_HEIGHT);
          } else if (edge == 2) { // Unten
            asteroids[i].x = random(0, AST_SCREEN_WIDTH);
            asteroids[i].y = AST_SCREEN_HEIGHT + asteroids[i].radius;
          } else { // Links
            asteroids[i].x = -asteroids[i].radius;
            asteroids[i].y = random(0, AST_SCREEN_HEIGHT);
          }
        } else {
          asteroids[i].x = x;
          asteroids[i].y = y;
        }

        // Zufällige Geschwindigkeit
        float speed = random(5, 15) / 10.0;
        float angle = random(0, 628) / 100.0;  // 0 bis 2*PI
        asteroids[i].vx = cos(angle) * speed;
        asteroids[i].vy = sin(angle) * speed;

        return;
      }
    }
  }

  // ===== INPUT =====

  void handleInput() {
    if (usePotRotation) {
      // Rotation via Poti (Hauptsteuerung)
      int potValue = analogRead(AST_POT_ROTATION);
      float targetAngle = map(potValue, 0, 1000, 0, 360);
      ship.angle = targetAngle;
      if (ship.angle < 0) ship.angle += 360;
      if (ship.angle >= 360) ship.angle -= 360;

      // Rotation via Buttons (Backup/Feinsteuerung)
      if (CYD_Input::readButton(CYD_BTN_B)) {
        ship.angle -= AST_SHIP_ROTATION_SPEED;
        if (ship.angle < 0) ship.angle += 360;
      }
      if (CYD_Input::readButton(CYD_BTN_C)) {
        ship.angle += AST_SHIP_ROTATION_SPEED;
        if (ship.angle >= 360) ship.angle -= 360;
      }
    } else {
      // Rotation via Buttons (Hauptsteuerung)
      if (CYD_Input::readButton(CYD_BTN_B)) {
        ship.angle -= AST_SHIP_ROTATION_SPEED;
        if (ship.angle < 0) ship.angle += 360;
      }
      if (CYD_Input::readButton(CYD_BTN_C)) {
        ship.angle += AST_SHIP_ROTATION_SPEED;
        if (ship.angle >= 360) ship.angle -= 360;
      }
    }

    // Schub
    if (CYD_Input::readButton(CYD_BTN_D)) {
      float rad = (ship.angle - 90) * PI / 180.0;  // -90 weil 0° = nach oben
      ship.vx += cos(rad) * AST_SHIP_ACCELERATION;
      ship.vy += sin(rad) * AST_SHIP_ACCELERATION;

      // Max speed begrenzen
      float speed = sqrt(ship.vx * ship.vx + ship.vy * ship.vy);
      if (speed > AST_SHIP_MAX_SPEED) {
        ship.vx = (ship.vx / speed) * AST_SHIP_MAX_SPEED;
        ship.vy = (ship.vy / speed) * AST_SHIP_MAX_SPEED;
      }
    }

    // Schießen (mit Entprellen)
    bool shootState = CYD_Input::readButton(CYD_BTN_A);
    if (shootState && !lastShootState) {
      shootBullet();
    }
    lastShootState = shootState;
  }

  void shootBullet() {
    // Freies Geschoss finden
    for (int i = 0; i < AST_MAX_BULLETS; i++) {
      if (!bullets[i].active) {
        float rad = (ship.angle - 90) * PI / 180.0;

        bullets[i].x = ship.x;
        bullets[i].y = ship.y;
        bullets[i].vx = cos(rad) * AST_BULLET_SPEED + ship.vx;
        bullets[i].vy = sin(rad) * AST_BULLET_SPEED + ship.vy;
        bullets[i].life = AST_BULLET_LIFE;
        bullets[i].active = true;
        return;
      }
    }
  }

  // ===== UPDATE =====

  void updateShip() {
    if (!ship.alive) return;

    // Invulnerabilität reduzieren
    if (ship.invulnerable > 0) {
      ship.invulnerable--;
    }

    // Alte Position löschen (Schiff + mögliche Thrust-Flamme)
    eraseShip();

    // Reibung
    ship.vx *= AST_SHIP_FRICTION;
    ship.vy *= AST_SHIP_FRICTION;

    // Bewegung
    ship.x += ship.vx;
    ship.y += ship.vy;

    // Screen wrapping
    if (ship.x < -AST_SHIP_SIZE) ship.x = AST_SCREEN_WIDTH + AST_SHIP_SIZE;
    if (ship.x > AST_SCREEN_WIDTH + AST_SHIP_SIZE) ship.x = -AST_SHIP_SIZE;
    if (ship.y < -AST_SHIP_SIZE) ship.y = AST_SCREEN_HEIGHT + AST_SHIP_SIZE;
    if (ship.y > AST_SCREEN_HEIGHT + AST_SHIP_SIZE) ship.y = -AST_SHIP_SIZE;
  }

  void updateBullets() {
    for (int i = 0; i < AST_MAX_BULLETS; i++) {
      if (!bullets[i].active) continue;

      // Alte Position löschen
      lcd->fillCircle((int)bullets[i].x, (int)bullets[i].y, 1, AST_COLOR_BG);

      // Bewegung
      bullets[i].x += bullets[i].vx;
      bullets[i].y += bullets[i].vy;
      bullets[i].life--;

      // Screen wrapping
      if (bullets[i].x < 0) bullets[i].x = AST_SCREEN_WIDTH;
      if (bullets[i].x > AST_SCREEN_WIDTH) bullets[i].x = 0;
      if (bullets[i].y < 0) bullets[i].y = AST_SCREEN_HEIGHT;
      if (bullets[i].y > AST_SCREEN_HEIGHT) bullets[i].y = 0;

      // Deaktivieren wenn Leben abgelaufen
      if (bullets[i].life <= 0) {
        bullets[i].active = false;
        continue;
      }

      // Neue Position zeichnen
      lcd->fillCircle((int)bullets[i].x, (int)bullets[i].y, 1, AST_COLOR_BULLET);
    }
  }

  void updateAsteroids() {
    for (int i = 0; i < AST_MAX_ASTEROIDS; i++) {
      if (!asteroids[i].active) continue;

      // Alte Position löschen
      lcd->drawCircle((int)asteroids[i].x, (int)asteroids[i].y,
                     asteroids[i].radius, AST_COLOR_BG);

      // Bewegung
      asteroids[i].x += asteroids[i].vx;
      asteroids[i].y += asteroids[i].vy;

      // Screen wrapping
      if (asteroids[i].x < -asteroids[i].radius * 2)
        asteroids[i].x = AST_SCREEN_WIDTH + asteroids[i].radius * 2;
      if (asteroids[i].x > AST_SCREEN_WIDTH + asteroids[i].radius * 2)
        asteroids[i].x = -asteroids[i].radius * 2;
      if (asteroids[i].y < -asteroids[i].radius * 2)
        asteroids[i].y = AST_SCREEN_HEIGHT + asteroids[i].radius * 2;
      if (asteroids[i].y > AST_SCREEN_HEIGHT + asteroids[i].radius * 2)
        asteroids[i].y = -asteroids[i].radius * 2;

      // Neue Position zeichnen
      lcd->drawCircle((int)asteroids[i].x, (int)asteroids[i].y,
                     asteroids[i].radius, AST_COLOR_ASTEROID);
    }
  }

  // ===== KOLLISIONEN =====

  void checkCollisions() {
    // Bullets vs Asteroids
    for (int b = 0; b < AST_MAX_BULLETS; b++) {
      if (!bullets[b].active) continue;

      for (int a = 0; a < AST_MAX_ASTEROIDS; a++) {
        if (!asteroids[a].active) continue;

        float dx = bullets[b].x - asteroids[a].x;
        float dy = bullets[b].y - asteroids[a].y;
        float dist = sqrt(dx * dx + dy * dy);

        if (dist < asteroids[a].radius) {
          // Treffer!
          bullets[b].active = false;
          lcd->fillCircle((int)bullets[b].x, (int)bullets[b].y, 1, AST_COLOR_BG);

          // Asteroid zerstören
          destroyAsteroid(a);
          break;
        }
      }
    }

    // Ship vs Asteroids
    if (ship.alive && ship.invulnerable == 0) {
      for (int a = 0; a < AST_MAX_ASTEROIDS; a++) {
        if (!asteroids[a].active) continue;

        float dx = ship.x - asteroids[a].x;
        float dy = ship.y - asteroids[a].y;
        float dist = sqrt(dx * dx + dy * dy);

        if (dist < asteroids[a].radius + AST_SHIP_SIZE) {
          // Kollision!
          shipDestroyed();
          break;
        }
      }
    }
  }

  void destroyAsteroid(int index) {
    AST_Asteroid* ast = &asteroids[index];

    // Punkte
    if (ast->size == 2) score += 20;      // Groß
    else if (ast->size == 1) score += 50;  // Mittel
    else score += 100;                     // Klein

    // Asteroid löschen
    lcd->drawCircle((int)ast->x, (int)ast->y, ast->radius, AST_COLOR_BG);

    // In kleinere zerbrechen
    if (ast->size > 0) {
      // 2 kleinere Asteroids spawnen
      spawnAsteroid(ast->x, ast->y, ast->size - 1);
      spawnAsteroid(ast->x, ast->y, ast->size - 1);
    }

    ast->active = false;
    drawUI();
  }

  void shipDestroyed() {
    Serial.println("Schiff zerstört!");

    ship.alive = false;
    eraseShip();

    lives--;
    drawUI();

    delay(1000);

    if (lives <= 0) {
      handleGameOver();
    } else {
      // Respawn
      ship.x = AST_SCREEN_WIDTH / 2;
      ship.y = AST_SCREEN_HEIGHT / 2;
      ship.vx = 0;
      ship.vy = 0;
      ship.angle = 0;
      ship.alive = true;
      ship.invulnerable = 120;  // 2 Sekunden
    }
  }

  bool checkLevelComplete() {
    for (int i = 0; i < AST_MAX_ASTEROIDS; i++) {
      if (asteroids[i].active) return false;
    }
    return true;
  }

  void handleGameOver() {
    gameOver = true;
    Serial.println("\n=== GAME OVER ===");
    Serial.printf("Final Score: %d\n", score);
    Serial.printf("Level erreicht: %d\n", level);

    // Game Over Screen
    lcd->fillRect(AST_SCREEN_WIDTH/4 - 10, AST_SCREEN_HEIGHT/2 - 55,
                 AST_SCREEN_WIDTH/2 + 20, 110, 0x2104);
    lcd->drawRect(AST_SCREEN_WIDTH/4 - 10, AST_SCREEN_HEIGHT/2 - 55,
                 AST_SCREEN_WIDTH/2 + 20, 110, AST_COLOR_TEXT);

    lcd->setTextSize(3);
    lcd->setTextColor(AST_COLOR_TEXT, 0x2104);
    lcd->setCursor(AST_SCREEN_WIDTH/2 - 75, AST_SCREEN_HEIGHT/2 - 30);
    lcd->println("GAME OVER");

    lcd->setTextSize(2);
    lcd->setCursor(AST_SCREEN_WIDTH/2 - 55, AST_SCREEN_HEIGHT/2 + 5);
    lcd->printf("Score: %d", score);

    lcd->setTextSize(1);
    lcd->setCursor(AST_SCREEN_WIDTH/2 - 55, AST_SCREEN_HEIGHT/2 + 30);
    lcd->printf("Level: %d", level);

    lcd->setCursor(AST_SCREEN_WIDTH/2 - 55, AST_SCREEN_HEIGHT/2 + 45);
    lcd->println("Press SHOOT to restart");
  }

  // ===== ZEICHNEN =====

  void drawShip() {
    if (!ship.alive) return;

    // Blinken wenn invulnerabel
    if (ship.invulnerable > 0 && (ship.invulnerable / 5) % 2 == 0) {
      return;  // Nicht zeichnen (Blink-Effekt)
    }

    float rad = (ship.angle - 90) * PI / 180.0;

    // Dreieck-Punkte berechnen
    int x1 = ship.x + cos(rad) * AST_SHIP_SIZE;
    int y1 = ship.y + sin(rad) * AST_SHIP_SIZE;

    int x2 = ship.x + cos(rad + 2.5) * AST_SHIP_SIZE;
    int y2 = ship.y + sin(rad + 2.5) * AST_SHIP_SIZE;

    int x3 = ship.x + cos(rad - 2.5) * AST_SHIP_SIZE;
    int y3 = ship.y + sin(rad - 2.5) * AST_SHIP_SIZE;

    // Dreieck zeichnen
    lcd->drawLine(x1, y1, x2, y2, AST_COLOR_SHIP);
    lcd->drawLine(x2, y2, x3, y3, AST_COLOR_SHIP);
    lcd->drawLine(x3, y3, x1, y1, AST_COLOR_SHIP);

    // Schub-Flamme wenn Thrust gedrückt
    if (CYD_Input::readButton(CYD_BTN_D)) {
      int fx1 = ship.x + cos(rad + PI) * (AST_SHIP_SIZE - 2);
      int fy1 = ship.y + sin(rad + PI) * (AST_SHIP_SIZE - 2);

      int fx2 = ship.x + cos(rad + PI) * (AST_SHIP_SIZE + 4);
      int fy2 = ship.y + sin(rad + PI) * (AST_SHIP_SIZE + 4);

      lcd->drawLine(fx1, fy1, fx2, fy2, AST_COLOR_THRUST);
    }
  }

  void eraseShip() {
    if (!ship.alive && ship.invulnerable == 0) return;

    // Bereich um Schiff löschen (inkl. Thrust-Flamme)
    int eraseSize = AST_SHIP_SIZE + 6;
    lcd->fillRect(ship.x - eraseSize, ship.y - eraseSize,
                 eraseSize * 2, eraseSize * 2, AST_COLOR_BG);
  }

  void drawUI() {
    // Oberer Bereich löschen
    lcd->fillRect(0, 0, AST_SCREEN_WIDTH, 20, AST_COLOR_BG);

    lcd->setTextSize(2);
    lcd->setTextColor(AST_COLOR_TEXT, AST_COLOR_BG);

    // Score (links)
    lcd->setCursor(5, 2);
    lcd->printf("S:%d", score);

    // Level (Mitte)
    lcd->setCursor(AST_SCREEN_WIDTH/2 - 30, 2);
    lcd->printf("L:%d", level);

    // Lives (rechts)
    lcd->setCursor(AST_SCREEN_WIDTH - 80, 2);
    lcd->printf("Lvs:%d", lives);
  }

  void drawGame() {
    drawShip();
  }
};

#endif // ASTEROIDSGAME_H
