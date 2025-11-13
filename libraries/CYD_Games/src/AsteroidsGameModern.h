/*
  AsteroidsGameModern.h - Klassisches Arcade-Spiel mit modernen Effekten als Klasse

  Teil der CYD_Games Library für CYD Display

  Verwendung:
    AsteroidsGameModern game;
    game.init(&lcd);
    // In loop():
    game.update();
*/

#ifndef ASTEROIDSGAMEMODERN_H
#define ASTEROIDSGAMEMODERN_H

#include <Arduino.h>
#include <LovyanGFX.hpp>

// Forward declaration
class LGFX;

// ===== PIN DEFINITIONS =====
#define AST_BTN_LEFT   tasteB
#define AST_BTN_RIGHT  tasteC
#define AST_BTN_SHOOT  tasteA
#define AST_BTN_THRUST tasteD

// ===== DISPLAY DIMENSIONS =====
#define AST_SCREEN_WIDTH  320
#define AST_SCREEN_HEIGHT 240

// ===== MODERNE FARBPALETTE =====
#define AST_COLOR_BG_TOP      0x0015  // Dunkles Blau (oben)
#define AST_COLOR_BG_BOTTOM   0x0000  // Schwarz (unten)
#define AST_COLOR_SHIP        0x07FF  // Cyan
#define AST_COLOR_SHIP_GLOW   0x0410  // Dunkles Cyan
#define AST_COLOR_THRUST      0xFD20  // Orange (Schub-Flamme)
#define AST_COLOR_THRUST_GLOW 0x8200  // Dunkles Orange
#define AST_COLOR_BULLET      0xFFFF  // Weiß
#define AST_COLOR_BULLET_GLOW 0x8410  // Grau
#define AST_COLOR_ASTEROID    0xA514  // Grau-Weiß
#define AST_COLOR_ASTEROID_GLOW 0x4208 // Dunkles Grau
#define AST_COLOR_TEXT        0xFFFF  // Weiß
#define AST_COLOR_PARTICLE1   0xFFE0  // Gelb
#define AST_COLOR_PARTICLE2   0xFD20  // Orange
#define AST_COLOR_PARTICLE3   0xF800  // Rot

// ===== GAME CONSTANTS =====
#define AST_MAX_ASTEROIDS 20
#define AST_MAX_BULLETS 5
#define AST_MAX_PARTICLES 40

#define AST_SHIP_SIZE 8
#define AST_SHIP_ACCELERATION 0.15
#define AST_SHIP_MAX_SPEED 5.0
#define AST_SHIP_ROTATION_SPEED 5.0
#define AST_SHIP_FRICTION 0.98

#define AST_BULLET_SPEED 6.0
#define AST_BULLET_LIFE 60

#define AST_ASTEROID_LARGE_RADIUS 20
#define AST_ASTEROID_MEDIUM_RADIUS 12
#define AST_ASTEROID_SMALL_RADIUS 6

#define AST_FRAME_DELAY 16  // ~60 FPS

// ===== STRUCTS =====
struct AstShip {
  float x, y;           // Position
  float vx, vy;         // Geschwindigkeit
  float angle;          // Rotation in Grad (0 = nach oben)
  int size;             // Größe
  bool alive;
  int invulnerable;     // Frames invulnerabel nach Tod
  float oldX, oldY;     // Alte Position für Löschen
  float oldAngle;       // Alte Rotation
};

struct AstAsteroid {
  float x, y;           // Position
  float vx, vy;         // Geschwindigkeit
  int size;             // 0=klein, 1=mittel, 2=groß
  int radius;           // Radius für Kollision
  bool active;
  float oldX, oldY;     // Alte Position für Löschen
};

struct AstBullet {
  float x, y;           // Position
  float vx, vy;         // Geschwindigkeit
  int life;             // Verbleibende Lebensdauer (Frames)
  bool active;
  float oldX, oldY;     // Alte Position für Trail
};

struct AstParticle {
  float x, y;
  float vx, vy;
  uint16_t color;
  uint8_t life;  // 0-100
  bool active;
};

// ===== CLASS DEFINITION =====
class AsteroidsGameModern {
public:
  AsteroidsGameModern() :
    lcd(nullptr),
    score(0),
    lives(3),
    level(1),
    gameOver(false),
    lastFrame(0),
    lastShootState(HIGH),
    backgroundDrawn(false)
  {
    // Arrays werden im init() initialisiert
  }

  void init(LGFX* display) {
    lcd = display;

    Serial.println("Initialisiere Asteroids Modern...");

    // Button Inputs konfigurieren
    pinMode(AST_BTN_LEFT, INPUT_PULLUP);
    pinMode(AST_BTN_RIGHT, INPUT_PULLUP);
    pinMode(AST_BTN_SHOOT, INPUT_PULLUP);
    pinMode(AST_BTN_THRUST, INPUT_PULLUP);

    // Zufallsgenerator
    randomSeed(analogRead(34) + analogRead(35) + micros());

    // Spiel initialisieren
    initGame();

    Serial.println("Asteroids Modern bereit!");
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
      if (digitalRead(AST_BTN_SHOOT) == LOW) {
        initGame();
        gameOver = false;
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
    updateParticles();

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

  bool isGameOver() { return gameOver; }
  int getScore() { return score; }

private:
  LGFX* lcd;

  // Game objects
  AstShip ship;
  AstAsteroid asteroids[AST_MAX_ASTEROIDS];
  AstBullet bullets[AST_MAX_BULLETS];
  AstParticle particles[AST_MAX_PARTICLES];

  // Game state
  int score;
  int lives;
  int level;
  bool gameOver;
  unsigned long lastFrame;
  int lastShootState;
  bool backgroundDrawn;

  // === HELPER FUNKTIONEN ===

  // Farb-Interpolation (Lerp)
  uint16_t lerpColor(uint16_t color1, uint16_t color2, float t) {
    t = constrain(t, 0.0, 1.0);

    uint8_t r1 = (color1 >> 11) & 0x1F;
    uint8_t g1 = (color1 >> 5) & 0x3F;
    uint8_t b1 = color1 & 0x1F;

    uint8_t r2 = (color2 >> 11) & 0x1F;
    uint8_t g2 = (color2 >> 5) & 0x3F;
    uint8_t b2 = color2 & 0x1F;

    uint8_t r = r1 + (r2 - r1) * t;
    uint8_t g = g1 + (g2 - g1) * t;
    uint8_t b = b1 + (b2 - b1) * t;

    return (r << 11) | (g << 5) | b;
  }

  // Hintergrundfarbe an Position
  uint16_t getBgColor(int y) {
    return lerpColor(AST_COLOR_BG_TOP, AST_COLOR_BG_BOTTOM, (float)y / AST_SCREEN_HEIGHT);
  }

  // === INITIALISIERUNG ===

  void initGame() {
    Serial.println("Initialisiere Spiel...");

    score = 0;
    level = 1;
    lives = 3;
    gameOver = false;
    backgroundDrawn = false;
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
    ship.oldX = -100;
    ship.oldY = -100;
    ship.oldAngle = 0;

    // Bullets initialisieren
    for (int i = 0; i < AST_MAX_BULLETS; i++) {
      bullets[i].active = false;
    }

    // Asteroids initialisieren
    for (int i = 0; i < AST_MAX_ASTEROIDS; i++) {
      asteroids[i].active = false;
    }

    // Partikel initialisieren
    for (int i = 0; i < AST_MAX_PARTICLES; i++) {
      particles[i].active = false;
    }

    // Bildschirm aufbauen
    drawBackground();
    drawUI();

    // Level starten
    initLevel();
  }

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
    ship.oldX = -100;
    ship.oldY = -100;

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

        asteroids[i].oldX = asteroids[i].x;
        asteroids[i].oldY = asteroids[i].y;

        // Zufällige Geschwindigkeit
        float speed = random(5, 15) / 10.0;
        float angle = random(0, 628) / 100.0;  // 0 bis 2*PI
        asteroids[i].vx = cos(angle) * speed;
        asteroids[i].vy = sin(angle) * speed;

        return;
      }
    }
  }

  // === INPUT ===

  void handleInput() {
    // Rotation via Buttons
    if (digitalRead(AST_BTN_LEFT) == LOW) {
      ship.angle -= AST_SHIP_ROTATION_SPEED;
      if (ship.angle < 0) ship.angle += 360;
    }
    if (digitalRead(AST_BTN_RIGHT) == LOW) {
      ship.angle += AST_SHIP_ROTATION_SPEED;
      if (ship.angle >= 360) ship.angle -= 360;
    }

    // Schub
    if (digitalRead(AST_BTN_THRUST) == LOW) {
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
    int shootState = digitalRead(AST_BTN_SHOOT);
    if (shootState == LOW && lastShootState == HIGH) {
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
        bullets[i].oldX = ship.x;
        bullets[i].oldY = ship.y;
        bullets[i].vx = cos(rad) * AST_BULLET_SPEED + ship.vx;
        bullets[i].vy = sin(rad) * AST_BULLET_SPEED + ship.vy;
        bullets[i].life = AST_BULLET_LIFE;
        bullets[i].active = true;
        return;
      }
    }
  }

  // === UPDATE ===

  void updateShip() {
    if (!ship.alive) return;

    // Invulnerabilität reduzieren
    if (ship.invulnerable > 0) {
      ship.invulnerable--;
    }

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

      // Alte Position speichern für Trail
      bullets[i].oldX = bullets[i].x;
      bullets[i].oldY = bullets[i].y;

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
      }
    }
  }

  void updateAsteroids() {
    for (int i = 0; i < AST_MAX_ASTEROIDS; i++) {
      if (!asteroids[i].active) continue;

      // Alte Position speichern
      asteroids[i].oldX = asteroids[i].x;
      asteroids[i].oldY = asteroids[i].y;

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
    }
  }

  void updateParticles() {
    for (int i = 0; i < AST_MAX_PARTICLES; i++) {
      if (!particles[i].active) continue;

      // Alte Position löschen
      uint16_t bgColor = getBgColor((int)particles[i].y);
      lcd->fillCircle((int)particles[i].x, (int)particles[i].y, 1, bgColor);

      // Bewegen
      particles[i].x += particles[i].vx;
      particles[i].y += particles[i].vy;
      particles[i].vy += 0.2;  // Gravitation
      particles[i].life -= 5;

      // Deaktivieren wenn tot oder außerhalb
      if (particles[i].life <= 0 ||
          particles[i].y > AST_SCREEN_HEIGHT ||
          particles[i].x < 0 || particles[i].x > AST_SCREEN_WIDTH) {
        particles[i].active = false;
        continue;
      }

      // Neue Position zeichnen
      lcd->fillCircle((int)particles[i].x, (int)particles[i].y, 1, particles[i].color);
    }
  }

  void spawnExplosion(int x, int y, uint16_t baseColor) {
    // 10-15 Partikel spawnen
    int numParticles = random(10, 16);

    for (int i = 0; i < numParticles; i++) {
      // Freies Partikel finden
      for (int p = 0; p < AST_MAX_PARTICLES; p++) {
        if (!particles[p].active) {
          particles[p].x = x;
          particles[p].y = y;

          // Zufällige Geschwindigkeit in alle Richtungen
          float angle = random(0, 628) / 100.0;  // 0 bis 2*PI
          float speed = random(10, 30) / 10.0;
          particles[p].vx = cos(angle) * speed;
          particles[p].vy = sin(angle) * speed - 1;  // Leicht nach oben

          // Farbe variieren
          int colorChoice = random(0, 3);
          if (colorChoice == 0) particles[p].color = AST_COLOR_PARTICLE1;
          else if (colorChoice == 1) particles[p].color = AST_COLOR_PARTICLE2;
          else particles[p].color = AST_COLOR_PARTICLE3;

          particles[p].life = 100;
          particles[p].active = true;
          break;
        }
      }
    }
  }

  // === KOLLISIONEN ===

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
    AstAsteroid* ast = &asteroids[index];

    // Explosion spawnen
    spawnExplosion((int)ast->x, (int)ast->y, AST_COLOR_ASTEROID);

    // Punkte
    if (ast->size == 2) score += 20;      // Groß
    else if (ast->size == 1) score += 50;  // Mittel
    else score += 100;                     // Klein

    // In kleinere zerbrechen
    if (ast->size > 0) {
      // 2 kleinere Asteroids spawnen
      spawnAsteroid(ast->x, ast->y, ast->size - 1);
      spawnAsteroid(ast->x, ast->y, ast->size - 1);
    }

    ast->active = false;
    drawUI();

    Serial.printf("Asteroid zerstört! Score: %d\n", score);
  }

  void shipDestroyed() {
    Serial.println("Schiff zerstört!");

    // Explosion spawnen
    spawnExplosion((int)ship.x, (int)ship.y, AST_COLOR_SHIP);

    ship.alive = false;
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
      ship.oldX = -100;
      ship.oldY = -100;
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

    // Game Over Screen mit Gradient-Hintergrund
    uint16_t boxColor = lerpColor(AST_COLOR_BG_TOP, AST_COLOR_BG_BOTTOM, 0.5);
    lcd->fillRect(AST_SCREEN_WIDTH/4 - 10, AST_SCREEN_HEIGHT/2 - 55,
                 AST_SCREEN_WIDTH/2 + 20, 110, boxColor);
    lcd->drawRect(AST_SCREEN_WIDTH/4 - 10, AST_SCREEN_HEIGHT/2 - 55,
                 AST_SCREEN_WIDTH/2 + 20, 110, AST_COLOR_TEXT);

    lcd->setTextSize(3);
    lcd->setTextColor(AST_COLOR_TEXT, boxColor);
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

  // === ZEICHNEN ===

  void drawBackground() {
    // Gradient von oben (dunkelblau) nach unten (schwarz)
    for (int y = 0; y < AST_SCREEN_HEIGHT; y++) {
      uint16_t color = lerpColor(AST_COLOR_BG_TOP, AST_COLOR_BG_BOTTOM, (float)y / AST_SCREEN_HEIGHT);
      lcd->drawFastHLine(0, y, AST_SCREEN_WIDTH, color);
    }
    backgroundDrawn = true;
  }

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

    // Glow-Effekt (dickere Linien, dunklere Farbe)
    lcd->drawLine(x1, y1, x2, y2, AST_COLOR_SHIP_GLOW);
    lcd->drawLine(x2, y2, x3, y3, AST_COLOR_SHIP_GLOW);
    lcd->drawLine(x3, y3, x1, y1, AST_COLOR_SHIP_GLOW);

    // Schiff-Dreieck (heller)
    lcd->drawLine(x1-1, y1, x2-1, y2, AST_COLOR_SHIP);
    lcd->drawLine(x2, y2-1, x3, y3-1, AST_COLOR_SHIP);
    lcd->drawLine(x3+1, y3, x1+1, y1, AST_COLOR_SHIP);

    // Schub-Flamme wenn Thrust gedrückt
    if (digitalRead(AST_BTN_THRUST) == LOW) {
      int fx1 = ship.x + cos(rad + PI) * (AST_SHIP_SIZE - 2);
      int fy1 = ship.y + sin(rad + PI) * (AST_SHIP_SIZE - 2);

      int fx2 = ship.x + cos(rad + PI) * (AST_SHIP_SIZE + 5);
      int fy2 = ship.y + sin(rad + PI) * (AST_SHIP_SIZE + 5);

      // Glow für Flamme
      lcd->drawLine(fx1, fy1, fx2, fy2, AST_COLOR_THRUST_GLOW);
      lcd->drawLine(fx1-1, fy1, fx2-1, fy2, AST_COLOR_THRUST);
      lcd->drawLine(fx1+1, fy1, fx2+1, fy2, AST_COLOR_THRUST);
    }
  }

  void eraseShip() {
    // Bereich um alte Schiff-Position löschen (inkl. Thrust-Flamme)
    int eraseSize = AST_SHIP_SIZE + 7;
    for (int y = ship.oldY - eraseSize; y <= ship.oldY + eraseSize; y++) {
      if (y < 0 || y >= AST_SCREEN_HEIGHT) continue;
      uint16_t bgColor = getBgColor(y);
      lcd->drawFastHLine(ship.oldX - eraseSize, y, eraseSize * 2, bgColor);
    }
  }

  void drawAsteroid(int index) {
    AstAsteroid* ast = &asteroids[index];

    int x = (int)ast->x;
    int y = (int)ast->y;
    int r = ast->radius;

    // Glow-Effekt (größerer Kreis, dunklere Farbe)
    lcd->drawCircle(x, y, r + 1, AST_COLOR_ASTEROID_GLOW);

    // Asteroid (Hauptkreis)
    lcd->drawCircle(x, y, r, AST_COLOR_ASTEROID);

    // Innerer Kreis für mehr Detail
    if (r > 8) {
      lcd->drawCircle(x, y, r - 3, AST_COLOR_ASTEROID);
    }
  }

  void eraseAsteroid(int index) {
    AstAsteroid* ast = &asteroids[index];

    int x = (int)ast->oldX;
    int y = (int)ast->oldY;
    int r = ast->radius;

    // Alte Position mit Glow löschen
    for (int dy = -r - 2; dy <= r + 2; dy++) {
      int py = y + dy;
      if (py < 0 || py >= AST_SCREEN_HEIGHT) continue;

      int dx = sqrt((r + 2) * (r + 2) - dy * dy);
      uint16_t bgColor = getBgColor(py);
      lcd->drawFastHLine(x - dx, py, dx * 2 + 1, bgColor);
    }
  }

  void drawBullet(int index) {
    AstBullet* b = &bullets[index];

    int x = (int)b->x;
    int y = (int)b->y;

    // Trail (Linie von alter zu neuer Position)
    if (b->life < AST_BULLET_LIFE - 1) {
      lcd->drawLine((int)b->oldX, (int)b->oldY, x, y, AST_COLOR_BULLET_GLOW);
    }

    // Glow
    lcd->fillCircle(x, y, 2, AST_COLOR_BULLET_GLOW);

    // Heller Kern
    lcd->fillCircle(x, y, 1, AST_COLOR_BULLET);
  }

  void eraseBullet(int index) {
    AstBullet* b = &bullets[index];

    // Trail und Glow löschen
    int x1 = (int)b->oldX;
    int y1 = (int)b->oldY;
    int x2 = (int)b->x;
    int y2 = (int)b->y;

    // Bereich um Trail löschen
    int minX = min(x1, x2) - 3;
    int maxX = max(x1, x2) + 3;
    int minY = min(y1, y2) - 3;
    int maxY = max(y1, y2) + 3;

    for (int y = minY; y <= maxY; y++) {
      if (y < 0 || y >= AST_SCREEN_HEIGHT) continue;
      uint16_t bgColor = getBgColor(y);
      lcd->drawFastHLine(minX, y, maxX - minX + 1, bgColor);
    }
  }

  void drawUI() {
    // Oberer Bereich mit Gradient löschen
    for (int y = 0; y < 25; y++) {
      uint16_t bgColor = getBgColor(y);
      lcd->drawFastHLine(0, y, AST_SCREEN_WIDTH, bgColor);
    }

    lcd->setTextSize(2);
    lcd->setTextColor(AST_COLOR_TEXT, AST_COLOR_BG_TOP);

    // Score (links)
    lcd->setCursor(5, 5);
    lcd->printf("S:%d", score);

    // Level (Mitte)
    lcd->setCursor(AST_SCREEN_WIDTH/2 - 30, 5);
    lcd->printf("L:%d", level);

    // Lives (rechts)
    lcd->setCursor(AST_SCREEN_WIDTH - 80, 5);
    lcd->printf("Lvs:%d", lives);
  }

  void drawGame() {
    // Altes Schiff löschen (wenn Position sich geändert hat)
    if (ship.oldX != ship.x || ship.oldY != ship.y || ship.oldAngle != ship.angle) {
      eraseShip();
      ship.oldX = ship.x;
      ship.oldY = ship.y;
      ship.oldAngle = ship.angle;
    }

    // Alte Asteroids löschen und neue zeichnen
    for (int i = 0; i < AST_MAX_ASTEROIDS; i++) {
      if (!asteroids[i].active) continue;

      if (asteroids[i].oldX != asteroids[i].x || asteroids[i].oldY != asteroids[i].y) {
        eraseAsteroid(i);
      }
      drawAsteroid(i);
    }

    // Alte Bullets löschen und neue zeichnen
    for (int i = 0; i < AST_MAX_BULLETS; i++) {
      if (!bullets[i].active) continue;

      if (bullets[i].oldX != bullets[i].x || bullets[i].oldY != bullets[i].y) {
        eraseBullet(i);
      }
      drawBullet(i);
    }

    // Schiff zeichnen (wird oben gedrawn)
    drawShip();
  }
};

#endif // ASTEROIDSGAMEMODERN_H
