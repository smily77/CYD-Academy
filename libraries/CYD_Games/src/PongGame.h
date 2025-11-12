/*
  PongGame.h - Klassisches Pong als Klasse gekapselt

  Teil der CYD_Games Library für CYD Display

  Verwendung:
    PongGame game;
    game.init(lcd);
    // In loop():
    game.update();
*/

#ifndef PONG_GAME_H
#define PONG_GAME_H

#include <Arduino.h>
#include <LovyanGFX.hpp>

class PongGame {
public:
  // Konstruktor
  PongGame() :
    lcd(nullptr),
    scoreLeft(0),
    scoreRight(0),
    autoModeLeftPaddle(false),
    lastButtonStateAutoOn(HIGH),
    lastButtonStateAutoOff(HIGH),
    lastButtonStateResetScore(HIGH)
  {}

  // Initialisierung
  void init(LGFX* display) {
    lcd = display;

    Serial.println("Initialisiere Pong...");

    // Linker Schläger
    paddleLeft.x = 10;
    paddleLeft.y = SCREEN_HEIGHT / 2 - 30;
    paddleLeft.w = 5;
    paddleLeft.h = 60;
    autoModeLeftPaddle = false;
    paddleLeft.color = COLOR_PADDLE;

    // Rechter Schläger
    paddleRight.x = SCREEN_WIDTH - 15;
    paddleRight.y = SCREEN_HEIGHT / 2 - 30;
    paddleRight.w = 5;
    paddleRight.h = 60;
    paddleRight.color = COLOR_PADDLE;

    // Ball
    resetBall();

    // Score
    scoreLeft = 0;
    scoreRight = 0;

    // Alte Positionen
    oldPaddleLeftY = paddleLeft.y;
    oldPaddleLeftColor = paddleLeft.color;
    oldPaddleRightY = paddleRight.y;
    oldBallX = ball.x;
    oldBallY = ball.y;

    // Display
    lcd->fillScreen(COLOR_BG);
    drawField();
    drawScore();

    Serial.println("Pong bereit!");
  }

  // Haupt-Update
  void update() {
    // Taster-Eingaben
    handleButtons();

    // Schläger-Positionen
    updatePaddles();

    // Ball bewegen
    updateBall();

    // Kollisionen
    checkCollisions();

    // Zeichnen
    drawGame();

    delay(16);  // ~60 FPS
  }

  // Status
  int getScoreLeft() { return scoreLeft; }
  int getScoreRight() { return scoreRight; }
  bool isAutoMode() { return autoModeLeftPaddle; }

private:
  // Display-Referenz
  LGFX* lcd;

  // Konstanten
  static const int SCREEN_WIDTH = 320;
  static const int SCREEN_HEIGHT = 240;

  // Farben
  static const uint16_t COLOR_BG = 0x0000;
  static const uint16_t COLOR_FG = 0xFFFF;
  static const uint16_t COLOR_PADDLE = 0x07FF;
  static const uint16_t COLOR_PADDLE_AUTO = 0xF800;
  static const uint16_t COLOR_BALL = 0xFFE0;
  static const uint16_t COLOR_LINE = 0x4208;

  // Pin-Mapping
  #define POT_LEFT            potiLeft
  #define POT_RIGHT           potiRight
  #define BUTTON_AUTO_ON      tasteC
  #define BUTTON_AUTO_OFF     tasteB
  #define BUTTON_RESET_SCORE  tasteA

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
  };

  // Game State
  Paddle paddleLeft;
  Paddle paddleRight;
  Ball ball;

  int scoreLeft;
  int scoreRight;

  int oldPaddleLeftY;
  uint16_t oldPaddleLeftColor;
  int oldPaddleRightY;
  float oldBallX;
  float oldBallY;

  bool autoModeLeftPaddle;

  int lastButtonStateAutoOn;
  int lastButtonStateAutoOff;
  int lastButtonStateResetScore;

  // === METHODEN ===

  void resetBall() {
    ball.x = SCREEN_WIDTH / 2;
    ball.y = SCREEN_HEIGHT / 2;
    ball.size = 6;
    ball.color = COLOR_BALL;

    // Zufällige Start-Richtung
    ball.vx = (random(0, 2) == 0) ? -3.5 : 3.5;
    ball.vy = random(-2, 3);

    delay(500);
  }

  void drawField() {
    // Mittellinie (gestrichelt)
    int centerX = SCREEN_WIDTH / 2;
    for (int y = 0; y < SCREEN_HEIGHT; y += 10) {
      lcd->fillRect(centerX - 1, y, 2, 5, COLOR_LINE);
    }
  }

  void drawScore() {
    lcd->setTextSize(3);
    lcd->setTextColor(COLOR_FG, COLOR_BG);

    // Linker Score
    lcd->fillRect(SCREEN_WIDTH / 4 - 20, 10, 40, 24, COLOR_BG);
    lcd->setCursor(SCREEN_WIDTH / 4 - 20, 10);
    lcd->printf("%d", scoreLeft);

    // Rechter Score
    lcd->fillRect(SCREEN_WIDTH * 3 / 4 - 20, 10, 40, 24, COLOR_BG);
    lcd->setCursor(SCREEN_WIDTH * 3 / 4 - 20, 10);
    lcd->printf("%d", scoreRight);
  }

  void handleButtons() {
    // Auto-Modus EIN
    int currentButtonStateAutoOn = digitalRead(BUTTON_AUTO_ON);
    if (currentButtonStateAutoOn == LOW && lastButtonStateAutoOn == HIGH) {
      if (!autoModeLeftPaddle) {
        autoModeLeftPaddle = true;
        paddleLeft.color = COLOR_PADDLE_AUTO;
        Serial.println("Auto-Modus EIN");
      }
    }
    lastButtonStateAutoOn = currentButtonStateAutoOn;

    // Auto-Modus AUS
    int currentButtonStateAutoOff = digitalRead(BUTTON_AUTO_OFF);
    if (currentButtonStateAutoOff == LOW && lastButtonStateAutoOff == HIGH) {
      if (autoModeLeftPaddle) {
        autoModeLeftPaddle = false;
        paddleLeft.color = COLOR_PADDLE;
        Serial.println("Auto-Modus AUS");
      }
    }
    lastButtonStateAutoOff = currentButtonStateAutoOff;

    // Spielstand RESET
    int currentButtonStateResetScore = digitalRead(BUTTON_RESET_SCORE);
    if (currentButtonStateResetScore == LOW && lastButtonStateResetScore == HIGH) {
      Serial.println("Spielstand wird zurückgesetzt!");
      init(lcd);  // Neu starten
    }
    lastButtonStateResetScore = currentButtonStateResetScore;
  }

  void updatePaddles() {
    oldPaddleLeftY = paddleLeft.y;
    oldPaddleLeftColor = paddleLeft.color;
    oldPaddleRightY = paddleRight.y;

    // Linker Schläger
    if (autoModeLeftPaddle) {
      // AI: Folgt dem Ball
      int targetY = ball.y - (paddleLeft.h / 2);
      paddleLeft.y += (targetY - paddleLeft.y) * 0.1;
    } else {
      // Manuell
      int potLeftValue = analogRead(POT_LEFT);
      paddleLeft.y = map(potLeftValue, 1000, 0, 0, SCREEN_HEIGHT - paddleLeft.h);
    }

    // Rechter Schläger (immer manuell)
    int potRightValue = analogRead(POT_RIGHT);
    paddleRight.y = map(potRightValue, 1000, 0, 0, SCREEN_HEIGHT - paddleRight.h);

    // Grenzen
    paddleLeft.y = constrain(paddleLeft.y, 0, SCREEN_HEIGHT - paddleLeft.h);
    paddleRight.y = constrain(paddleRight.y, 0, SCREEN_HEIGHT - paddleRight.h);
  }

  void updateBall() {
    oldBallX = ball.x;
    oldBallY = ball.y;

    ball.x += ball.vx;
    ball.y += ball.vy;

    // Wand-Kollision
    if (ball.y <= 0 || ball.y >= SCREEN_HEIGHT - ball.size) {
      ball.vy = -ball.vy;
      ball.y = constrain(ball.y, 0, SCREEN_HEIGHT - ball.size);
    }
  }

  void checkCollisions() {
    // Linker Schläger
    if (ball.x <= paddleLeft.x + paddleLeft.w &&
        ball.x >= paddleLeft.x &&
        ball.y + ball.size >= paddleLeft.y &&
        ball.y <= paddleLeft.y + paddleLeft.h) {

      ball.vx = -ball.vx;
      ball.x = paddleLeft.x + paddleLeft.w;

      float hitPos = (ball.y + ball.size/2) - (paddleLeft.y + paddleLeft.h/2);
      ball.vy += hitPos * 0.05;
    }

    // Rechter Schläger
    if (ball.x + ball.size >= paddleRight.x &&
        ball.x + ball.size <= paddleRight.x + paddleRight.w &&
        ball.y + ball.size >= paddleRight.y &&
        ball.y <= paddleRight.y + paddleRight.h) {

      ball.vx = -ball.vx;
      ball.x = paddleRight.x - ball.size;

      float hitPos = (ball.y + ball.size/2) - (paddleRight.y + paddleRight.h/2);
      ball.vy += hitPos * 0.05;
    }

    // Ball links raus
    if (ball.x < 0) {
      scoreRight++;
      Serial.printf("PUNKT! Rechts: %d - Links: %d\n", scoreRight, scoreLeft);
      drawScore();
      resetBall();
    }

    // Ball rechts raus
    if (ball.x > SCREEN_WIDTH) {
      scoreLeft++;
      Serial.printf("PUNKT! Links: %d - Rechts: %d\n", scoreLeft, scoreRight);
      drawScore();
      resetBall();
    }
  }

  void drawGame() {
    // Hintergrund-Elemente neu zeichnen
    drawField();
    drawScore();

    // Linken Schläger
    if (oldPaddleLeftY != paddleLeft.y || oldPaddleLeftColor != paddleLeft.color) {
      lcd->fillRect(paddleLeft.x, oldPaddleLeftY, paddleLeft.w, paddleLeft.h, COLOR_BG);
      lcd->fillRect(paddleLeft.x, paddleLeft.y, paddleLeft.w, paddleLeft.h, paddleLeft.color);
      oldPaddleLeftColor = paddleLeft.color;
    }

    // Rechten Schläger
    if (oldPaddleRightY != paddleRight.y) {
      lcd->fillRect(paddleRight.x, oldPaddleRightY, paddleRight.w, paddleRight.h, COLOR_BG);
      lcd->fillRect(paddleRight.x, paddleRight.y, paddleRight.w, paddleRight.h, paddleRight.color);
    }

    // Ball
    lcd->fillRect(oldBallX, oldBallY, ball.size, ball.size, COLOR_BG);
    lcd->fillRect(ball.x, ball.y, ball.size, ball.size, ball.color);
  }
};

#endif // PONG_GAME_H
