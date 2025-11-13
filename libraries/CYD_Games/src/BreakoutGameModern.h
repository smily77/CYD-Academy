/*
  BreakoutGameModern.h - Modernes Breakout mit Fancy Design als Klasse gekapselt

  Teil der CYD_Games Library für CYD Display

  Verwendung:
    BreakoutGameModern game;
    game.init(&lcd);
    // In loop():
    game.update();
*/

#ifndef BREAKOUTGAMEMODERN_H
#define BREAKOUTGAMEMODERN_H

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <CYD_Input.h>

// Forward declaration
class LGFX;

// ===== PIN DEFINITIONS =====
#define BREAKOUT_POT_PADDLE   potiLeft
#define BREAKOUT_BTN_START    tasteA
#define BREAKOUT_BTN_PAUSE    tasteD

// ===== DISPLAY DIMENSIONS =====
#define BREAKOUT_SCREEN_WIDTH  320
#define BREAKOUT_SCREEN_HEIGHT 240

// ===== MODERNE FARBPALETTE =====
#define BREAKOUT_COLOR_BG_TOP     0x0814
#define BREAKOUT_COLOR_BG_BOTTOM  0x0000
#define BREAKOUT_COLOR_NEON_RED     0xF800
#define BREAKOUT_COLOR_NEON_ORANGE  0xFD20
#define BREAKOUT_COLOR_NEON_YELLOW  0xFFE0
#define BREAKOUT_COLOR_NEON_GREEN   0x07E0
#define BREAKOUT_COLOR_NEON_CYAN    0x07FF
#define BREAKOUT_COLOR_NEON_BLUE    0x001F
#define BREAKOUT_COLOR_NEON_PINK    0xF81F
#define BREAKOUT_COLOR_PADDLE       BREAKOUT_COLOR_NEON_CYAN
#define BREAKOUT_COLOR_BALL         BREAKOUT_COLOR_NEON_YELLOW
#define BREAKOUT_COLOR_TEXT         0xFFFF
#define BREAKOUT_COLOR_SHADOW       0x2104
#define BREAKOUT_COLOR_HIGHLIGHT    0xFFFF

// ===== STRUCTS =====
struct BreakoutPaddle {
  int x, y;
  int w, h;
  uint16_t color;
};

struct BreakoutBall {
  float x, y;
  float vx, vy;
  int size;
  uint16_t color;
  bool stuck;
};

struct BreakoutTrailPoint {
  int x, y;
  bool active;
};

struct BreakoutBrick {
  int x, y;
  int w, h;
  uint16_t color;
  int points;
  bool active;
};

struct BreakoutParticle {
  float x, y;
  float oldX, oldY;
  float vx, vy;
  int life;
  uint16_t color;
  bool active;
};

// ===== CLASS DEFINITION =====
class BreakoutGameModern {
public:
  BreakoutGameModern() :
    lcd(nullptr),
    trailIndex(0),
    score(0),
    lives(3),
    level(1),
    gameOver(false),
    levelComplete(false),
    paused(false),
    oldPaddleX(0),
    oldBallX(0),
    oldBallY(0),
    glowPhase(0),
    lastPauseState(HIGH)
  {}

  void init(LGFX* display) {
    lcd = display;

    Serial.println("Initialisiere Modern Breakout...");

    // Input initialisieren
    CYD_Input::init();

    // Zufallsgenerator
    randomSeed(analogRead(35) + micros());

    // Partikel initialisieren
    for (int i = 0; i < MAX_PARTICLES; i++) {
      particles[i].active = false;
    }

    // Trail initialisieren
    for (int i = 0; i < TRAIL_LENGTH; i++) {
      trail[i].active = false;
    }

    initGame();

    Serial.println("Modern Breakout bereit!");
  }

  void update() {
    // Animationen
    glowPhase += 0.08;
    if (glowPhase > 6.28) glowPhase = 0;

    if (gameOver) {
      if (CYD_Input::readButton(CYD_BTN_A)) {
        initGame();
        gameOver = false;
        delay(300);
      }
      return;
    }

    // Pause-Button
    static bool lastPauseState = false;
    bool pauseState = CYD_Input::readButton(CYD_BTN_D);
    if (pauseState && !lastPauseState) {
      paused = !paused;
      Serial.println(paused ? "PAUSE" : "WEITER");
      delay(200);
    }
    lastPauseState = pauseState;

    if (paused) {
      // Zeige Pause-Text
      int boxX = BREAKOUT_SCREEN_WIDTH/2 - 60;
      int boxY = BREAKOUT_SCREEN_HEIGHT/2 - 30;
      lcd->fillRect(boxX, boxY, 120, 60, 0x2104);
      lcd->drawRect(boxX, boxY, 120, 60, BREAKOUT_COLOR_NEON_CYAN);
      lcd->setTextSize(3);
      lcd->setTextColor(BREAKOUT_COLOR_NEON_YELLOW, 0x2104);
      lcd->setCursor(boxX + 15, boxY + 20);
      lcd->println("PAUSE");
      delay(100);
      return;
    }

    if (levelComplete) {
      delay(2000);
      level++;
      initLevel();
      levelComplete = false;
    }

    // Schläger aktualisieren
    updatePaddle();

    // Ball-Start mit Button
    if (ball.stuck) {
      if (CYD_Input::readButton(CYD_BTN_A)) {
        launchBall();
        delay(200);
      }
    } else {
      // Ball bewegen
      updateBall();
      checkCollisions();
    }

    // Zeichnen
    drawGame();

    delay(16);  // ~60 FPS
  }

  bool isGameOver() { return gameOver; }
  int getScore() { return score; }
  int getLives() { return lives; }
  int getLevel() { return level; }

private:
  LGFX* lcd;

  // Game objects
  BreakoutPaddle paddle;
  BreakoutBall ball;

  // Ball-Trail
  static const int TRAIL_LENGTH = 10;
  BreakoutTrailPoint trail[TRAIL_LENGTH];
  int trailIndex;

  // Steine
  static const int BRICK_ROWS = 6;
  static const int BRICK_COLS = 10;
  static const int BRICK_COUNT = BRICK_ROWS * BRICK_COLS;
  BreakoutBrick bricks[BRICK_COUNT];

  // Partikel
  static const int MAX_PARTICLES = 40;
  BreakoutParticle particles[MAX_PARTICLES];

  // Game State
  int score;
  int lives;
  int level;
  bool gameOver;
  bool levelComplete;
  bool paused;

  // Alte Positionen
  int oldPaddleX;
  float oldBallX;
  float oldBallY;

  // Animationen
  float glowPhase;

  // Button States
  int lastPauseState;

  // === METHODEN ===

  void initGame() {
    score = 0;
    lives = 3;
    level = 1;
    gameOver = false;
    levelComplete = false;

    // Schläger
    paddle.w = 60;
    paddle.h = 10;
    paddle.x = BREAKOUT_SCREEN_WIDTH / 2 - paddle.w / 2;
    paddle.y = BREAKOUT_SCREEN_HEIGHT - 25;
    paddle.color = BREAKOUT_COLOR_PADDLE;
    oldPaddleX = -1;  // Ungültiger Wert damit Schläger beim ersten Frame gezeichnet wird

    // Ball
    resetBall();

    // Level
    initLevel();

    // Bildschirm
    drawGradientBackground();
    drawModernBricks();
    drawModernUI();
  }

  void initLevel() {
    // Stein-Dimensionen
    int brickW = 28;
    int brickH = 10;
    int brickSpacing = 4;
    int startX = 10;
    int startY = 45;

    uint16_t rowColors[BRICK_ROWS] = {
      BREAKOUT_COLOR_NEON_RED, BREAKOUT_COLOR_NEON_ORANGE, BREAKOUT_COLOR_NEON_YELLOW,
      BREAKOUT_COLOR_NEON_GREEN, BREAKOUT_COLOR_NEON_CYAN, BREAKOUT_COLOR_NEON_BLUE
    };

    int rowPoints[BRICK_ROWS] = {7, 6, 5, 4, 3, 2};

    int index = 0;
    for (int row = 0; row < BRICK_ROWS; row++) {
      for (int col = 0; col < BRICK_COLS; col++) {
        bricks[index].x = startX + col * (brickW + brickSpacing);
        bricks[index].y = startY + row * (brickH + brickSpacing);
        bricks[index].w = brickW;
        bricks[index].h = brickH;
        bricks[index].color = rowColors[row];
        bricks[index].points = rowPoints[row];
        bricks[index].active = true;
        index++;
      }
    }

    resetBall();
    drawGradientBackground();
    drawModernBricks();
    drawModernUI();
  }

  void resetBall() {
    ball.size = 8;
    ball.color = BREAKOUT_COLOR_BALL;
    ball.stuck = true;
    ball.x = paddle.x + paddle.w / 2 - ball.size / 2;
    ball.y = paddle.y - ball.size - 2;
    ball.vx = 0;
    ball.vy = 0;

    // Ungültige Werte damit Ball beim ersten Frame gezeichnet wird
    oldBallX = -1;
    oldBallY = -1;
  }

  void launchBall() {
    ball.stuck = false;

    float angle = random(30, 150);
    float rad = angle * 3.14159 / 180.0;

    // Langsamer Start, wird mit Level schneller
    float baseSpeed = 1.5;
    float speedBoost = (level - 1) * 0.3;
    float speed = baseSpeed + speedBoost;
    speed = constrain(speed, 1.5, 4.0);

    ball.vx = cos(rad) * speed;
    ball.vy = -abs(sin(rad)) * speed;

    Serial.printf("Ball gestartet! Speed: %.1f (Level %d)\n", speed, level);
  }

  void updatePaddle() {
    // Analog-Wert lesen
    int potValue = CYD_Input::readPoti(CYD_POTI_LEFT);
    int newX = map(potValue, 0, 1000, 0, BREAKOUT_SCREEN_WIDTH - paddle.w);
    newX = constrain(newX, 0, BREAKOUT_SCREEN_WIDTH - paddle.w);

    // Deadzone: Nur bewegen wenn Änderung > 5 Pixel
    if (abs(newX - paddle.x) > 5) {
      paddle.x = newX;

      // Ball bewegen wenn festgeklebt
      if (ball.stuck) {
        oldBallX = ball.x;
        oldBallY = ball.y;
        ball.x = paddle.x + paddle.w / 2 - ball.size / 2;
      }
    }
  }

  void updateBall() {
    oldBallX = ball.x;
    oldBallY = ball.y;

    ball.x += ball.vx;
    ball.y += ball.vy;

    // Kollision mit Wänden
    if (ball.x <= 0 || ball.x >= BREAKOUT_SCREEN_WIDTH - ball.size) {
      ball.vx = -ball.vx;
      ball.x = constrain(ball.x, 0, BREAKOUT_SCREEN_WIDTH - ball.size);
    }

    // Kollision mit oberer Wand
    if (ball.y <= 0) {
      ball.vy = -ball.vy;
      ball.y = 0;
    }

    // Ball unten raus
    if (ball.y > BREAKOUT_SCREEN_HEIGHT) {
      lives--;
      Serial.printf("Leben verloren! Verbleibend: %d\n", lives);

      if (lives <= 0) {
        handleGameOver();
      } else {
        resetBall();
        drawModernUI();
      }
    }
  }

  void updateTrail() {
    trail[trailIndex].x = ball.x + ball.size/2;
    trail[trailIndex].y = ball.y + ball.size/2;
    trail[trailIndex].active = true;
    trailIndex = (trailIndex + 1) % TRAIL_LENGTH;
  }

  void checkCollisions() {
    // Kollision mit Schläger
    if (ball.y + ball.size >= paddle.y &&
        ball.y + ball.size <= paddle.y + paddle.h &&
        ball.x + ball.size >= paddle.x &&
        ball.x <= paddle.x + paddle.w) {

      ball.vy = -abs(ball.vy);
      ball.y = paddle.y - ball.size;

      float hitPos = (ball.x + ball.size/2) - (paddle.x + paddle.w/2);
      ball.vx += hitPos * 0.1;
      ball.vx = constrain(ball.vx, -5, 5);
    }

    // Kollision mit Steinen
    for (int i = 0; i < BRICK_COUNT; i++) {
      if (!bricks[i].active) continue;

      if (ball.x + ball.size >= bricks[i].x &&
          ball.x <= bricks[i].x + bricks[i].w &&
          ball.y + ball.size >= bricks[i].y &&
          ball.y <= bricks[i].y + bricks[i].h) {

        // Stein sofort visuell löschen (mit Gradient-Hintergrund)
        for (int y = 0; y < bricks[i].h; y++) {
          uint16_t bgColor = lerpColor(BREAKOUT_COLOR_BG_TOP, BREAKOUT_COLOR_BG_BOTTOM,
                                        (float)(bricks[i].y + y) / BREAKOUT_SCREEN_HEIGHT);
          lcd->drawFastHLine(bricks[i].x, bricks[i].y + y, bricks[i].w, bgColor);
        }

        // Stein zerstören
        bricks[i].active = false;
        score += bricks[i].points;

        // Ball abprallen
        float ballCenterX = ball.x + ball.size / 2;
        float ballCenterY = ball.y + ball.size / 2;
        float brickCenterX = bricks[i].x + bricks[i].w / 2;
        float brickCenterY = bricks[i].y + bricks[i].h / 2;

        if (abs(ballCenterX - brickCenterX) > abs(ballCenterY - brickCenterY)) {
          ball.vx = -ball.vx;
        } else {
          ball.vy = -ball.vy;
        }

        drawModernUI();

        if (checkLevelComplete()) {
          levelComplete = true;
          Serial.println("Level komplett!");
        }

        break;
      }
    }
  }

  bool checkLevelComplete() {
    for (int i = 0; i < BRICK_COUNT; i++) {
      if (bricks[i].active) return false;
    }
    return true;
  }

  void handleGameOver() {
    gameOver = true;
    Serial.println("\n=== GAME OVER ===");
    Serial.printf("Final Score: %d\n", score);

    // Moderner Game Over Screen
    int boxX = BREAKOUT_SCREEN_WIDTH/4 - 10;
    int boxY = BREAKOUT_SCREEN_HEIGHT/2 - 55;
    int boxW = BREAKOUT_SCREEN_WIDTH/2 + 20;
    int boxH = 110;

    // Box mit Gradient
    for (int y = 0; y < boxH; y++) {
      float factor = (float)y / boxH;
      uint16_t color = lerpColor(0x2104, 0x0000, factor);
      lcd->drawFastHLine(boxX, boxY + y, boxW, color);
    }

    // Neon-Umrandung
    lcd->drawRect(boxX - 1, boxY - 1, boxW + 2, boxH + 2, BREAKOUT_COLOR_NEON_RED);
    lcd->drawRect(boxX - 2, boxY - 2, boxW + 4, boxH + 4, dimColor(BREAKOUT_COLOR_NEON_RED, 0.5));

    int centerX = boxX + boxW / 2;

    // GAME OVER
    lcd->setTextSize(3);
    lcd->setTextColor(BREAKOUT_COLOR_SHADOW);
    lcd->setCursor(centerX - 79, boxY + 22);
    lcd->println("GAME OVER");

    lcd->setTextColor(BREAKOUT_COLOR_NEON_RED);
    lcd->setCursor(centerX - 81, boxY + 20);
    lcd->println("GAME OVER");

    // Score
    lcd->setTextSize(2);
    lcd->setTextColor(BREAKOUT_COLOR_SHADOW);
    lcd->setCursor(centerX - 53, boxY + 57);
    lcd->printf("Score: %d", score);

    lcd->setTextColor(BREAKOUT_COLOR_NEON_YELLOW);
    lcd->setCursor(centerX - 55, boxY + 55);
    lcd->printf("Score: %d", score);

    // Touch to restart
    lcd->setTextSize(1);
    lcd->setTextColor(BREAKOUT_COLOR_NEON_CYAN);
    lcd->setCursor(centerX - 50, boxY + 85);
    lcd->println("Touch to restart");
  }

  void spawnParticles(int x, int y, uint16_t color, int count) {
    for (int i = 0; i < count; i++) {
      for (int j = 0; j < MAX_PARTICLES; j++) {
        if (!particles[j].active) {
          particles[j].x = x;
          particles[j].y = y;
          particles[j].oldX = x;
          particles[j].oldY = y;
          particles[j].vx = random(-40, 40) / 10.0;
          particles[j].vy = random(-40, 40) / 10.0;
          particles[j].life = random(15, 30);
          particles[j].color = color;
          particles[j].active = true;
          break;
        }
      }
    }
  }

  void updateParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
      if (particles[i].active) {
        particles[i].oldX = particles[i].x;
        particles[i].oldY = particles[i].y;

        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].vy += 0.2;
        particles[i].vx *= 0.98;
        particles[i].life--;

        if (particles[i].life <= 0) {
          particles[i].active = false;
        }
      }
    }
  }

  void drawParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
      if (particles[i].active) {
        // Alte Position löschen
        int oldSize = 1 + (particles[i].life / 10) + 2;
        for (int dy = -oldSize; dy <= oldSize; dy++) {
          for (int dx = -oldSize; dx <= oldSize; dx++) {
            int px = particles[i].oldX + dx;
            int py = particles[i].oldY + dy;
            if (px >= 0 && px < BREAKOUT_SCREEN_WIDTH && py >= 0 && py < BREAKOUT_SCREEN_HEIGHT) {
              uint16_t bgColor = lerpColor(BREAKOUT_COLOR_BG_TOP, BREAKOUT_COLOR_BG_BOTTOM, (float)py / BREAKOUT_SCREEN_HEIGHT);
              lcd->drawPixel(px, py, bgColor);
            }
          }
        }

        // Neue Position zeichnen
        float alpha = (float)particles[i].life / 30.0;
        uint16_t color = dimColor(particles[i].color, alpha);

        int size = 1 + (particles[i].life / 10);
        lcd->fillCircle(particles[i].x, particles[i].y, size, color);

        if (alpha > 0.5) {
          lcd->drawCircle(particles[i].x, particles[i].y, size + 1, dimColor(color, 0.3));
        }
      }
    }
  }

  void drawGradientBackground() {
    for (int y = 0; y < BREAKOUT_SCREEN_HEIGHT; y++) {
      uint16_t color = lerpColor(BREAKOUT_COLOR_BG_TOP, BREAKOUT_COLOR_BG_BOTTOM, (float)y / BREAKOUT_SCREEN_HEIGHT);
      lcd->drawFastHLine(0, y, BREAKOUT_SCREEN_WIDTH, color);
    }
  }

  void drawModernBricks() {
    for (int i = 0; i < BRICK_COUNT; i++) {
      if (bricks[i].active) {
        drawModernBrick(i);
      }
    }
  }

  void drawModernBrick(int index) {
    BreakoutBrick* b = &bricks[index];

    // Schatten
    lcd->fillRect(b->x + 2, b->y + 2, b->w - 2, b->h - 2, BREAKOUT_COLOR_SHADOW);

    // Haupt-Stein mit Gradient
    for (int i = 0; i < b->h; i++) {
      float factor = (float)i / b->h;
      uint16_t color = lerpColor(b->color, dimColor(b->color, 0.6), factor);
      lcd->drawFastHLine(b->x, b->y + i, b->w, color);
    }

    // Highlight
    lcd->drawFastHLine(b->x + 1, b->y + 1, b->w - 2, BREAKOUT_COLOR_HIGHLIGHT);

    // Glow
    lcd->drawRect(b->x - 1, b->y - 1, b->w + 2, b->h + 2, dimColor(b->color, 0.3));
  }

  void drawModernUI() {
    // UI-Bereich löschen
    for (int y = 0; y < 35; y++) {
      uint16_t bgColor = lerpColor(BREAKOUT_COLOR_BG_TOP, BREAKOUT_COLOR_BG_BOTTOM, (float)y / BREAKOUT_SCREEN_HEIGHT);
      lcd->drawFastHLine(0, y, BREAKOUT_SCREEN_WIDTH, bgColor);
    }

    lcd->setTextSize(2);

    // Score (kompakter für hohe Werte)
    lcd->setTextColor(BREAKOUT_COLOR_SHADOW);
    lcd->setCursor(7, 7);
    lcd->printf("S:%d", score);

    lcd->setTextColor(BREAKOUT_COLOR_NEON_YELLOW);
    lcd->setCursor(5, 5);
    lcd->printf("S:%d", score);

    // Level (Mitte - mehr Abstand)
    lcd->setTextColor(BREAKOUT_COLOR_SHADOW);
    lcd->setCursor(137, 7);
    lcd->printf("L:%d", level);

    lcd->setTextColor(BREAKOUT_COLOR_NEON_CYAN);
    lcd->setCursor(135, 5);
    lcd->printf("L:%d", level);

    // Lives
    lcd->setTextColor(BREAKOUT_COLOR_SHADOW);
    lcd->setCursor(BREAKOUT_SCREEN_WIDTH - 68, 7);
    lcd->printf("Lvs:%d", lives);

    lcd->setTextColor(BREAKOUT_COLOR_NEON_RED);
    lcd->setCursor(BREAKOUT_SCREEN_WIDTH - 70, 5);
    lcd->printf("Lvs:%d", lives);

    // Leben-Symbole (Herzen)
    for (int i = 0; i < lives; i++) {
      lcd->fillCircle(BREAKOUT_SCREEN_WIDTH - 100 + i * 15, 25, 3, BREAKOUT_COLOR_NEON_RED);
    }
  }

  void drawGame() {
    // Schläger zeichnen - NUR wenn er sich bewegt hat
    if (oldPaddleX != paddle.x) {
      // Alten Schläger löschen
      for (int y = 0; y < paddle.h + 4; y++) {
        uint16_t bgColor = lerpColor(BREAKOUT_COLOR_BG_TOP, BREAKOUT_COLOR_BG_BOTTOM, (float)(paddle.y + y) / BREAKOUT_SCREEN_HEIGHT);
        lcd->drawFastHLine(oldPaddleX - 2, paddle.y + y - 2, paddle.w + 4, bgColor);
      }

      // Neuen Schläger mit 3D-Effekt
      // Schatten
      lcd->fillRect(paddle.x + 2, paddle.y + 2, paddle.w, paddle.h, BREAKOUT_COLOR_SHADOW);

      // Gradient
      for (int i = 0; i < paddle.h; i++) {
        float factor = (float)i / paddle.h;
        uint16_t color = lerpColor(BREAKOUT_COLOR_PADDLE, dimColor(BREAKOUT_COLOR_PADDLE, 0.5), factor);
        lcd->drawFastHLine(paddle.x, paddle.y + i, paddle.w, color);
      }

      // Highlight
      lcd->drawFastHLine(paddle.x, paddle.y, paddle.w - 2, BREAKOUT_COLOR_HIGHLIGHT);

      // Pulsierender Glow
      float glow = sin(glowPhase) * 0.5 + 0.5;
      if (glow > 0.3) {
        uint16_t glowColor = dimColor(BREAKOUT_COLOR_PADDLE, glow * 0.6);
        lcd->drawRect(paddle.x - 1, paddle.y - 1, paddle.w + 2, paddle.h + 2, glowColor);
      }

      // Position aktualisieren NACH dem Zeichnen
      oldPaddleX = paddle.x;
    }

    // Ball zeichnen - NUR wenn er sich bewegt hat
    if (oldBallX != ball.x || oldBallY != ball.y) {
      // Ball löschen (alte Position)
      int oldCenterX = oldBallX + ball.size/2;
      int oldCenterY = oldBallY + ball.size/2;
      uint16_t bgColor = lerpColor(BREAKOUT_COLOR_BG_TOP, BREAKOUT_COLOR_BG_BOTTOM, (float)oldCenterY / BREAKOUT_SCREEN_HEIGHT);
      lcd->fillCircle(oldCenterX, oldCenterY, ball.size/2, bgColor);

      // Ball zeichnen (neu)
      int centerX = ball.x + ball.size/2;
      int centerY = ball.y + ball.size/2;
      lcd->fillCircle(centerX, centerY, ball.size/2, BREAKOUT_COLOR_BALL);

      // Position aktualisieren NACH dem Zeichnen
      oldBallX = ball.x;
      oldBallY = ball.y;
    }
  }

  // === HELPER FUNKTIONEN ===

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

  uint16_t dimColor(uint16_t color, float factor) {
    factor = constrain(factor, 0.0, 1.0);

    uint8_t r = ((color >> 11) & 0x1F) * factor;
    uint8_t g = ((color >> 5) & 0x3F) * factor;
    uint8_t b = (color & 0x1F) * factor;

    return (r << 11) | (g << 5) | b;
  }
};

#endif // BREAKOUTGAMEMODERN_H
