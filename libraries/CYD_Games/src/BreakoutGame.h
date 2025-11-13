/*
  BreakoutGame.h - Klassisches Breakout als Klasse gekapselt

  Teil der CYD_Games Library für CYD Display

  Verwendung:
    BreakoutGame game;
    game.init(lcd);
    // In loop():
    game.update();
*/

#ifndef BREAKOUT_GAME_H
#define BREAKOUT_GAME_H

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <CYD_Input.h>

// Pin-Mapping (muss VOR der Klasse definiert sein)
#define POT_PADDLE  potiLeft
#define BTN_START   tasteA
#define BTN_PAUSE   tasteD

class BreakoutGame {
public:
  // Konstruktor
  BreakoutGame() :
    lcd(nullptr),
    score(0),
    lives(3),
    level(1),
    gameOver(false),
    levelComplete(false),
    paused(false),
    oldPaddleX(0),
    oldBallX(0),
    oldBallY(0)
  {}

  // Initialisierung
  void init(LGFX* display) {
    lcd = display;

    Serial.println("Initialisiere Breakout...");

    // Input initialisieren
    CYD_Input::init();

    score = 0;
    lives = 3;
    level = 1;
    gameOver = false;
    levelComplete = false;

    // Schläger
    paddle.w = 60;
    paddle.h = 8;
    paddle.x = SCREEN_WIDTH / 2 - paddle.w / 2;
    paddle.y = SCREEN_HEIGHT - 20;
    paddle.color = COLOR_PADDLE;
    oldPaddleX = -1;

    // Ball
    resetBall();

    // Steine
    initLevel();

    // Display
    lcd->fillScreen(COLOR_BG);
    drawBricks();
    drawUI();

    Serial.println("Breakout bereit!");
  }

  // Haupt-Update
  void update() {
    if (gameOver) {
      if (CYD_Input::readButton(CYD_BTN_A)) {
        init(lcd);  // Neu starten
        delay(300);
      }
      return;
    }

    // Pause-Button
    static bool lastPauseState = false;
    bool pauseState = CYD_Input::readButton(CYD_BTN_D);
    if (pauseState && !lastPauseState) {
      paused = !paused;
      delay(200);
    }
    lastPauseState = pauseState;

    if (paused) {
      lcd->setTextSize(3);
      lcd->setTextColor(COLOR_TEXT, COLOR_BG);
      lcd->setCursor(SCREEN_WIDTH/2 - 60, SCREEN_HEIGHT/2 - 15);
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

    // Schläger
    updatePaddle();

    // Ball-Start
    if (ball.stuck) {
      if (CYD_Input::readButton(CYD_BTN_A)) {
        launchBall();
        delay(200);
      }
    } else {
      updateBall();
      checkCollisions();
    }

    // Zeichnen
    drawGame();

    delay(16);  // ~60 FPS
  }

  // Status
  int getScore() { return score; }
  int getLives() { return lives; }
  int getLevel() { return level; }
  bool isGameOver() { return gameOver; }

private:
  // Display-Referenz
  LGFX* lcd;

  // Konstanten
  static const int SCREEN_WIDTH = 320;
  static const int SCREEN_HEIGHT = 240;
  static const int BRICK_ROWS = 6;
  static const int BRICK_COLS = 10;
  static const int BRICK_COUNT = BRICK_ROWS * BRICK_COLS;

  // Farben
  static const uint16_t COLOR_BG = 0x0000;
  static const uint16_t COLOR_PADDLE = 0xFFFF;
  static const uint16_t COLOR_BALL = 0xFFFF;
  static const uint16_t COLOR_TEXT = 0xFFFF;
  static const uint16_t COLOR_BRICK_RED = 0xF800;
  static const uint16_t COLOR_BRICK_ORANGE = 0xFD20;
  static const uint16_t COLOR_BRICK_YELLOW = 0xFFE0;
  static const uint16_t COLOR_BRICK_GREEN = 0x07E0;
  static const uint16_t COLOR_BRICK_CYAN = 0x07FF;
  static const uint16_t COLOR_BRICK_BLUE = 0x001F;

  // Strukturen
  struct Paddle {
    int x, y;
    int w, h;
    uint16_t color;
  };

  struct Ball {
    float x, y;
    float vx, vy;
    int size;
    uint16_t color;
    bool stuck;
  };

  struct Brick {
    int x, y;
    int w, h;
    uint16_t color;
    int points;
    bool active;
  };

  // Game State
  Paddle paddle;
  Ball ball;
  Brick bricks[BRICK_COUNT];

  int score;
  int lives;
  int level;
  bool gameOver;
  bool levelComplete;
  bool paused;

  int oldPaddleX;
  float oldBallX;
  float oldBallY;

  // === METHODEN ===

  void initLevel() {
    Serial.printf("Breakout Level %d\n", level);

    int brickW = 28;
    int brickH = 10;
    int brickSpacing = 4;
    int startX = 10;
    int startY = 40;

    uint16_t rowColors[BRICK_ROWS] = {
      COLOR_BRICK_RED, COLOR_BRICK_ORANGE, COLOR_BRICK_YELLOW,
      COLOR_BRICK_GREEN, COLOR_BRICK_CYAN, COLOR_BRICK_BLUE
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
    lcd->fillScreen(COLOR_BG);
    drawBricks();
    drawUI();
  }

  void resetBall() {
    ball.size = 6;
    ball.color = COLOR_BALL;
    ball.stuck = true;
    ball.x = paddle.x + paddle.w / 2 - ball.size / 2;
    ball.y = paddle.y - ball.size - 1;
    ball.vx = 0;
    ball.vy = 0;

    oldBallX = -1;
    oldBallY = -1;
  }

  void launchBall() {
    ball.stuck = false;

    float angle = random(30, 150);
    float rad = angle * 3.14159 / 180.0;

    float baseSpeed = 1.5;
    float speedBoost = (level - 1) * 0.3;
    float speed = baseSpeed + speedBoost;
    speed = constrain(speed, 1.5, 4.0);

    ball.vx = cos(rad) * speed;
    ball.vy = -abs(sin(rad)) * speed;
  }

  void updatePaddle() {
    int potValue = CYD_Input::readPoti(CYD_POTI_LEFT);
    int newX = map(potValue, 0, 1000, 0, SCREEN_WIDTH - paddle.w);
    newX = constrain(newX, 0, SCREEN_WIDTH - paddle.w);

    if (abs(newX - paddle.x) > 5) {
      paddle.x = newX;

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

    // Wände
    if (ball.x <= 0 || ball.x >= SCREEN_WIDTH - ball.size) {
      ball.vx = -ball.vx;
      ball.x = constrain(ball.x, 0, SCREEN_WIDTH - ball.size);
    }

    // Decke
    if (ball.y <= 0) {
      ball.vy = -ball.vy;
      ball.y = 0;
    }

    // Ball unten raus
    if (ball.y > SCREEN_HEIGHT) {
      lives--;

      if (lives <= 0) {
        handleGameOver();
      } else {
        resetBall();
        drawUI();
      }
    }
  }

  void checkCollisions() {
    // Schläger
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

    // Steine
    for (int i = 0; i < BRICK_COUNT; i++) {
      if (!bricks[i].active) continue;

      if (ball.x + ball.size >= bricks[i].x &&
          ball.x <= bricks[i].x + bricks[i].w &&
          ball.y + ball.size >= bricks[i].y &&
          ball.y <= bricks[i].y + bricks[i].h) {

        bricks[i].active = false;
        score += bricks[i].points;

        lcd->fillRect(bricks[i].x, bricks[i].y, bricks[i].w, bricks[i].h, COLOR_BG);

        float ballCenterX = ball.x + ball.size / 2;
        float ballCenterY = ball.y + ball.size / 2;
        float brickCenterX = bricks[i].x + bricks[i].w / 2;
        float brickCenterY = bricks[i].y + bricks[i].h / 2;

        if (abs(ballCenterX - brickCenterX) > abs(ballCenterY - brickCenterY)) {
          ball.vx = -ball.vx;
        } else {
          ball.vy = -ball.vy;
        }

        drawUI();

        if (checkLevelComplete()) {
          levelComplete = true;
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
    Serial.println("Breakout Game Over!");
    Serial.printf("Final Score: %d\n", score);

    lcd->fillRect(SCREEN_WIDTH/4 - 10, SCREEN_HEIGHT/2 - 55,
                 SCREEN_WIDTH/2 + 20, 110, 0x2104);
    lcd->drawRect(SCREEN_WIDTH/4 - 10, SCREEN_HEIGHT/2 - 55,
                 SCREEN_WIDTH/2 + 20, 110, COLOR_TEXT);

    lcd->setTextSize(3);
    lcd->setTextColor(COLOR_TEXT, 0x2104);
    lcd->setCursor(SCREEN_WIDTH/2 - 75, SCREEN_HEIGHT/2 - 30);
    lcd->println("GAME OVER");

    lcd->setTextSize(2);
    lcd->setCursor(SCREEN_WIDTH/2 - 55, SCREEN_HEIGHT/2 + 5);
    lcd->printf("Score: %d", score);

    lcd->setTextSize(1);
    lcd->setCursor(SCREEN_WIDTH/2 - 55, SCREEN_HEIGHT/2 + 30);
    lcd->printf("Level: %d", level);

    lcd->setCursor(SCREEN_WIDTH/2 - 40, SCREEN_HEIGHT/2 + 45);
    lcd->println("Touch to restart");
  }

  void drawBricks() {
    for (int i = 0; i < BRICK_COUNT; i++) {
      if (bricks[i].active) {
        lcd->fillRect(bricks[i].x, bricks[i].y, bricks[i].w, bricks[i].h, bricks[i].color);
      }
    }
  }

  void drawUI() {
    lcd->fillRect(0, 0, SCREEN_WIDTH, 30, COLOR_BG);

    lcd->setTextSize(2);
    lcd->setTextColor(COLOR_TEXT, COLOR_BG);

    lcd->setCursor(5, 5);
    lcd->printf("S:%d", score);

    lcd->setCursor(135, 5);
    lcd->printf("L:%d", level);

    lcd->setCursor(SCREEN_WIDTH - 70, 5);
    lcd->printf("Lvs:%d", lives);
  }

  void drawGame() {
    // Schläger
    if (oldPaddleX != paddle.x) {
      lcd->fillRect(oldPaddleX, paddle.y, paddle.w, paddle.h, COLOR_BG);
      lcd->fillRect(paddle.x, paddle.y, paddle.w, paddle.h, paddle.color);
      oldPaddleX = paddle.x;
    }

    // Ball
    if (oldBallX != ball.x || oldBallY != ball.y) {
      lcd->fillRect(oldBallX, oldBallY, ball.size, ball.size, COLOR_BG);
      lcd->fillRect(ball.x, ball.y, ball.size, ball.size, ball.color);
      oldBallX = ball.x;
      oldBallY = ball.y;
    }
  }
};

#endif // BREAKOUT_GAME_H
