/*
  PongGameModern.h - Modernes Pong mit Fancy Design als Klasse gekapselt

  Teil der CYD_Games Library für CYD Display

  Verwendung:
    PongGameModern game;
    game.init(&lcd);
    // In loop():
    game.update();
*/

#ifndef PONGGAMEMODERN_H
#define PONGGAMEMODERN_H

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <CYD_Input.h>

// Forward declaration
class LGFX;

// ===== PIN DEFINITIONS =====
#define PONG_POT_LEFT            potiLeft
#define PONG_POT_RIGHT           potiRight
#define PONG_BUTTON_AUTO_ON      tasteC
#define PONG_BUTTON_AUTO_OFF     tasteB
#define PONG_BUTTON_RESET_SCORE  tasteA

// ===== DISPLAY DIMENSIONS =====
#define PONG_SCREEN_WIDTH  320
#define PONG_SCREEN_HEIGHT 240

// ===== MODERNE FARBPALETTE =====
#define PONG_COLOR_BG_TOP     0x0814
#define PONG_COLOR_BG_BOTTOM  0x0000
#define PONG_COLOR_NEON_CYAN   0x07FF
#define PONG_COLOR_NEON_PINK   0xF81F
#define PONG_COLOR_NEON_YELLOW 0xFFE0
#define PONG_COLOR_NEON_GREEN  0x07E0
#define PONG_COLOR_NEON_ORANGE 0xFD20
#define PONG_COLOR_TEXT       0xFFFF
#define PONG_COLOR_TEXT_DIM   0x8410
#define PONG_COLOR_SHADOW     0x2104
#define PONG_COLOR_HIGHLIGHT  0xFFFF
#define PONG_COLOR_PADDLE_MANUAL  PONG_COLOR_NEON_CYAN
#define PONG_COLOR_PADDLE_AUTO    PONG_COLOR_NEON_PINK
#define PONG_COLOR_PADDLE_RIGHT   PONG_COLOR_NEON_GREEN

// ===== STRUCTS =====
struct PongPaddle {
  int x, y;
  int w, h;
  uint16_t color;
};

struct PongBall {
  float x, y;
  float vx, vy;
  int size;
  uint16_t color;
};

struct PongParticle {
  float x, y;
  float vx, vy;
  int life;
  uint16_t color;
  bool active;
};

struct PongTrailPoint {
  int x, y;
  bool active;
};

// ===== CLASS DEFINITION =====
class PongGameModern {
public:
  PongGameModern() :
    lcd(nullptr),
    scoreLeft(0),
    scoreRight(0),
    trailIndex(0),
    oldPaddleLeftY(0),
    oldPaddleLeftColor(PONG_COLOR_PADDLE_MANUAL),
    oldPaddleRightY(0),
    oldBallX(0),
    oldBallY(0),
    autoModeLeftPaddle(false),
    lastButtonStateAutoOn(false),
    lastButtonStateAutoOff(false),
    lastButtonStateResetScore(false),
    lastParticleSpawn(0),
    glowPhase(0)
  {}

  void init(LGFX* display) {
    lcd = display;

    Serial.println("Initialisiere Modern Pong...");

    // Input initialisieren
    CYD_Input::init();

    // Partikel initialisieren
    for (int i = 0; i < MAX_PARTICLES; i++) {
      particles[i].active = false;
    }

    // Trail initialisieren
    for (int i = 0; i < TRAIL_LENGTH; i++) {
      trail[i].active = false;
    }

    initGame();

    Serial.println("Modern Pong bereit!");
  }

  void update() {
    // Glow-Animation aktualisieren
    glowPhase += 0.05;
    if (glowPhase > 6.28) glowPhase = 0;  // 2*PI

    // Taster-Eingaben
    handleButtons();

    // Schläger aktualisieren
    updatePaddles();

    // Ball bewegen
    updateBall();

    // Kollisionen prüfen
    checkCollisions();

    // Partikel aktualisieren
    updateParticles();

    // Trail aktualisieren
    updateTrail();

    // Alles zeichnen
    drawGame();

    // Frame-Rate ~60 FPS
    delay(16);
  }

  bool isGameOver() { return false; }
  int getScoreLeft() { return scoreLeft; }
  int getScoreRight() { return scoreRight; }

private:
  LGFX* lcd;

  // Game objects
  PongPaddle paddleLeft;
  PongPaddle paddleRight;
  PongBall ball;

  // Partikel-System
  static const int MAX_PARTICLES = 20;
  PongParticle particles[MAX_PARTICLES];

  // Ball-Trail
  static const int TRAIL_LENGTH = 8;
  PongTrailPoint trail[TRAIL_LENGTH];
  int trailIndex;

  // Score
  int scoreLeft;
  int scoreRight;

  // Alte Positionen
  int oldPaddleLeftY;
  uint16_t oldPaddleLeftColor;
  int oldPaddleRightY;
  float oldBallX;
  float oldBallY;

  // Auto-Modus
  bool autoModeLeftPaddle;

  // Taster-Variablen
  bool lastButtonStateAutoOn;
  bool lastButtonStateAutoOff;
  bool lastButtonStateResetScore;

  // Animationen
  unsigned long lastParticleSpawn;
  float glowPhase;

  // === METHODEN ===

  void initGame() {
    // Linker Schläger
    paddleLeft.x = 15;
    paddleLeft.y = PONG_SCREEN_HEIGHT / 2 - 35;
    paddleLeft.w = 8;
    paddleLeft.h = 70;
    autoModeLeftPaddle = false;
    paddleLeft.color = PONG_COLOR_PADDLE_MANUAL;

    // Rechter Schläger
    paddleRight.x = PONG_SCREEN_WIDTH - 23;
    paddleRight.y = PONG_SCREEN_HEIGHT / 2 - 35;
    paddleRight.w = 8;
    paddleRight.h = 70;
    paddleRight.color = PONG_COLOR_PADDLE_RIGHT;

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

    // Gradient-Hintergrund zeichnen
    drawGradientBackground();
  }

  void resetBall() {
    ball.x = PONG_SCREEN_WIDTH / 2;
    ball.y = PONG_SCREEN_HEIGHT / 2;
    ball.size = 8;
    ball.color = PONG_COLOR_NEON_YELLOW;

    // Zufällige Start-Richtung
    ball.vx = (random(0, 2) == 0) ? -3.5 : 3.5;
    ball.vy = random(-2, 3);

    delay(500);
  }

  void handleButtons() {
    bool currentButtonStateAutoOn = CYD_Input::readButton(CYD_BTN_C);
    if (currentButtonStateAutoOn && !lastButtonStateAutoOn) {
      if (!autoModeLeftPaddle) {
        autoModeLeftPaddle = true;
        paddleLeft.color = PONG_COLOR_PADDLE_AUTO;
        Serial.println("Auto-Modus EIN");
      }
    }
    lastButtonStateAutoOn = currentButtonStateAutoOn;

    bool currentButtonStateAutoOff = CYD_Input::readButton(CYD_BTN_B);
    if (currentButtonStateAutoOff && !lastButtonStateAutoOff) {
      if (autoModeLeftPaddle) {
        autoModeLeftPaddle = false;
        paddleLeft.color = PONG_COLOR_PADDLE_MANUAL;
        Serial.println("Auto-Modus AUS");
      }
    }
    lastButtonStateAutoOff = currentButtonStateAutoOff;

    bool currentButtonStateResetScore = CYD_Input::readButton(CYD_BTN_A);
    if (currentButtonStateResetScore && !lastButtonStateResetScore) {
      Serial.println("Spielstand zurückgesetzt!");
      initGame();
    }
    lastButtonStateResetScore = currentButtonStateResetScore;
  }

  void updatePaddles() {
    oldPaddleLeftY = paddleLeft.y;
    oldPaddleLeftColor = paddleLeft.color;
    oldPaddleRightY = paddleRight.y;

    // Linker Schläger (Spieler oder AI)
    if (autoModeLeftPaddle) {
      int targetY = ball.y - (paddleLeft.h / 2);
      paddleLeft.y += (targetY - paddleLeft.y) * 0.1;
    } else {
      int potLeftValue = CYD_Input::readPoti(CYD_POTI_LEFT);
      paddleLeft.y = map(potLeftValue, 1000, 0, 0, PONG_SCREEN_HEIGHT - paddleLeft.h);
    }

    // Rechter Schläger (immer manuell)
    int potRightValue = CYD_Input::readPoti(CYD_POTI_RIGHT);
    paddleRight.y = map(potRightValue, 1000, 0, 0, PONG_SCREEN_HEIGHT - paddleRight.h);

    // Grenzen
    paddleLeft.y = constrain(paddleLeft.y, 0, PONG_SCREEN_HEIGHT - paddleLeft.h);
    paddleRight.y = constrain(paddleRight.y, 0, PONG_SCREEN_HEIGHT - paddleRight.h);
  }

  void updateBall() {
    oldBallX = ball.x;
    oldBallY = ball.y;

    ball.x += ball.vx;
    ball.y += ball.vy;

    // Kollision mit oberer/unterer Wand
    if (ball.y <= 0 || ball.y >= PONG_SCREEN_HEIGHT - ball.size) {
      ball.vy = -ball.vy;
      ball.y = constrain(ball.y, 0, PONG_SCREEN_HEIGHT - ball.size);

      // Partikel spawnen
      spawnParticles(ball.x + ball.size/2, ball.y + ball.size/2, PONG_COLOR_NEON_YELLOW, 5);
    }
  }

  void checkCollisions() {
    // Kollision mit linkem Schläger
    if (ball.x <= paddleLeft.x + paddleLeft.w &&
        ball.x >= paddleLeft.x &&
        ball.y + ball.size >= paddleLeft.y &&
        ball.y <= paddleLeft.y + paddleLeft.h) {

      ball.vx = -ball.vx;
      ball.x = paddleLeft.x + paddleLeft.w;

      float hitPos = (ball.y + ball.size/2) - (paddleLeft.y + paddleLeft.h/2);
      ball.vy += hitPos * 0.05;

      // Partikel spawnen
      spawnParticles(paddleLeft.x + paddleLeft.w, ball.y + ball.size/2, paddleLeft.color, 8);

      Serial.println("Hit: Linker Schläger");
    }

    // Kollision mit rechtem Schläger
    if (ball.x + ball.size >= paddleRight.x &&
        ball.x + ball.size <= paddleRight.x + paddleRight.w &&
        ball.y + ball.size >= paddleRight.y &&
        ball.y <= paddleRight.y + paddleRight.h) {

      ball.vx = -ball.vx;
      ball.x = paddleRight.x - ball.size;

      float hitPos = (ball.y + ball.size/2) - (paddleRight.y + paddleRight.h/2);
      ball.vy += hitPos * 0.05;

      // Partikel spawnen
      spawnParticles(paddleRight.x, ball.y + ball.size/2, paddleRight.color, 8);

      Serial.println("Hit: Rechter Schläger");
    }

    // Ball links raus
    if (ball.x < 0) {
      scoreRight++;
      Serial.printf("PUNKT! Rechts: %d - Links: %d\n", scoreRight, scoreLeft);
      resetBall();
    }

    // Ball rechts raus
    if (ball.x > PONG_SCREEN_WIDTH) {
      scoreLeft++;
      Serial.printf("PUNKT! Links: %d - Rechts: %d\n", scoreLeft, scoreRight);
      resetBall();
    }
  }

  void spawnParticles(int x, int y, uint16_t color, int count) {
    for (int i = 0; i < count; i++) {
      // Freien Partikel-Slot finden
      for (int j = 0; j < MAX_PARTICLES; j++) {
        if (!particles[j].active) {
          particles[j].x = x;
          particles[j].y = y;
          particles[j].vx = random(-30, 30) / 10.0;
          particles[j].vy = random(-30, 30) / 10.0;
          particles[j].life = random(10, 25);
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
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].vy += 0.3;  // Gravitation
        particles[i].life--;

        if (particles[i].life <= 0) {
          particles[i].active = false;
        }
      }
    }
  }

  void updateTrail() {
    // Neuen Trail-Point hinzufügen
    trail[trailIndex].x = ball.x + ball.size/2;
    trail[trailIndex].y = ball.y + ball.size/2;
    trail[trailIndex].active = true;

    trailIndex = (trailIndex + 1) % TRAIL_LENGTH;
  }

  void drawGradientBackground() {
    // Vertikaler Gradient von oben nach unten
    for (int y = 0; y < PONG_SCREEN_HEIGHT; y++) {
      // Interpoliere zwischen TOP und BOTTOM
      uint16_t color = lerpColor(PONG_COLOR_BG_TOP, PONG_COLOR_BG_BOTTOM, (float)y / PONG_SCREEN_HEIGHT);
      lcd->drawFastHLine(0, y, PONG_SCREEN_WIDTH, color);
    }
  }

  void drawGame() {
    // Hintergrund (alle paar Frames neu zeichnen für Partikel-Clearing)
    static int frameCount = 0;
    if (frameCount % 30 == 0) {
      drawGradientBackground();
    }
    frameCount++;

    // Mittellinie (gepunktet, Neon-Stil)
    drawModernCenterLine();

    // Score mit modernem Design
    drawModernScore();

    // Schläger mit 3D-Effekt zeichnen
    drawModernPaddle(paddleLeft, oldPaddleLeftY, oldPaddleLeftColor);
    drawModernPaddle(paddleRight, oldPaddleRightY, paddleRight.color);

    // Ball-Trail zeichnen
    drawTrail();

    // Ball mit Glow-Effekt zeichnen
    drawModernBall();

    // Partikel zeichnen
    drawParticles();
  }

  void drawModernCenterLine() {
    int centerX = PONG_SCREEN_WIDTH / 2;
    for (int y = 5; y < PONG_SCREEN_HEIGHT - 5; y += 15) {
      // Neon-Linie mit Glow
      lcd->fillRect(centerX - 2, y, 4, 8, PONG_COLOR_NEON_CYAN);
      // Glow-Effekt
      lcd->drawPixel(centerX - 3, y + 3, dimColor(PONG_COLOR_NEON_CYAN, 0.3));
      lcd->drawPixel(centerX + 3, y + 3, dimColor(PONG_COLOR_NEON_CYAN, 0.3));
    }
  }

  void drawModernScore() {
    lcd->setTextSize(4);

    // Linker Score mit Schatten
    lcd->setTextColor(PONG_COLOR_SHADOW);
    lcd->setCursor(PONG_SCREEN_WIDTH / 4 - 22, 12);
    lcd->printf("%d", scoreLeft);

    lcd->setTextColor(autoModeLeftPaddle ? PONG_COLOR_PADDLE_AUTO : PONG_COLOR_PADDLE_MANUAL);
    lcd->setCursor(PONG_SCREEN_WIDTH / 4 - 24, 10);
    lcd->printf("%d", scoreLeft);

    // Rechter Score mit Schatten
    lcd->setTextColor(PONG_COLOR_SHADOW);
    lcd->setCursor(PONG_SCREEN_WIDTH * 3 / 4 - 22, 12);
    lcd->printf("%d", scoreRight);

    lcd->setTextColor(PONG_COLOR_PADDLE_RIGHT);
    lcd->setCursor(PONG_SCREEN_WIDTH * 3 / 4 - 24, 10);
    lcd->printf("%d", scoreRight);

    // Mode-Anzeige (klein, unten)
    if (autoModeLeftPaddle) {
      lcd->setTextSize(1);
      lcd->setTextColor(PONG_COLOR_PADDLE_AUTO);
      lcd->setCursor(5, PONG_SCREEN_HEIGHT - 12);
      lcd->print("AI MODE");
    }
  }

  void drawModernPaddle(PongPaddle &paddle, int oldY, uint16_t oldColor) {
    if (oldY != paddle.y || oldColor != paddle.color) {
      // Alten Schläger löschen (mit Gradient-Hintergrund)
      for (int y = oldY; y < oldY + paddle.h; y++) {
        uint16_t bgColor = lerpColor(PONG_COLOR_BG_TOP, PONG_COLOR_BG_BOTTOM, (float)y / PONG_SCREEN_HEIGHT);
        lcd->drawFastHLine(paddle.x - 2, y, paddle.w + 4, bgColor);
      }
    }

    // 3D-Effekt: Schatten
    lcd->fillRect(paddle.x + 2, paddle.y + 2, paddle.w, paddle.h, PONG_COLOR_SHADOW);

    // Haupt-Schläger mit Gradient
    for (int i = 0; i < paddle.h; i++) {
      float factor = (float)i / paddle.h;
      uint16_t color = lerpColor(paddle.color, dimColor(paddle.color, 0.5), factor);
      lcd->drawFastHLine(paddle.x, paddle.y + i, paddle.w, color);
    }

    // Highlight (oben links)
    lcd->drawFastHLine(paddle.x, paddle.y, paddle.w - 2, PONG_COLOR_HIGHLIGHT);
    lcd->drawFastVLine(paddle.x, paddle.y, 8, PONG_COLOR_HIGHLIGHT);

    // Glow-Effekt (pulsierend)
    float glow = sin(glowPhase) * 0.5 + 0.5;  // 0 bis 1
    if (glow > 0.3) {
      uint16_t glowColor = dimColor(paddle.color, glow * 0.5);
      lcd->drawRect(paddle.x - 1, paddle.y - 1, paddle.w + 2, paddle.h + 2, glowColor);
    }
  }

  void drawModernBall() {
    // Alte Position löschen
    int r = ball.size + 3;
    for (int dy = -r; dy <= r; dy++) {
      for (int dx = -r; dx <= r; dx++) {
        int px = oldBallX + ball.size/2 + dx;
        int py = oldBallY + ball.size/2 + dy;
        if (px >= 0 && px < PONG_SCREEN_WIDTH && py >= 0 && py < PONG_SCREEN_HEIGHT) {
          uint16_t bgColor = lerpColor(PONG_COLOR_BG_TOP, PONG_COLOR_BG_BOTTOM, (float)py / PONG_SCREEN_HEIGHT);
          lcd->drawPixel(px, py, bgColor);
        }
      }
    }

    // Glow-Effekt (äußerer Ring)
    int centerX = ball.x + ball.size/2;
    int centerY = ball.y + ball.size/2;

    // Mehrere Glow-Ringe
    for (int r = ball.size + 2; r >= ball.size/2; r--) {
      float intensity = 1.0 - (float)(r - ball.size/2) / (ball.size + 2);
      uint16_t glowColor = dimColor(ball.color, intensity * 0.6);
      lcd->drawCircle(centerX, centerY, r, glowColor);
    }

    // Haupt-Ball (gefüllt mit Gradient)
    lcd->fillCircle(centerX, centerY, ball.size/2, ball.color);

    // Highlight
    lcd->fillCircle(centerX - ball.size/4, centerY - ball.size/4, ball.size/4, PONG_COLOR_HIGHLIGHT);
  }

  void drawTrail() {
    // Zeichne Trail-Punkte (älter = transparenter)
    for (int i = 0; i < TRAIL_LENGTH; i++) {
      if (trail[i].active) {
        int age = (trailIndex - i + TRAIL_LENGTH) % TRAIL_LENGTH;
        float alpha = 1.0 - (float)age / TRAIL_LENGTH;
        uint16_t color = dimColor(ball.color, alpha * 0.4);

        lcd->fillCircle(trail[i].x, trail[i].y, 2, color);
      }
    }
  }

  void drawParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
      if (particles[i].active) {
        float alpha = (float)particles[i].life / 25.0;
        uint16_t color = dimColor(particles[i].color, alpha);

        int size = 1 + (particles[i].life / 10);
        lcd->fillCircle(particles[i].x, particles[i].y, size, color);
      }
    }
  }

  // === HELPER FUNKTIONEN ===

  // Interpoliere zwischen zwei Farben
  uint16_t lerpColor(uint16_t color1, uint16_t color2, float t) {
    t = constrain(t, 0.0, 1.0);

    // RGB565 dekodieren
    uint8_t r1 = (color1 >> 11) & 0x1F;
    uint8_t g1 = (color1 >> 5) & 0x3F;
    uint8_t b1 = color1 & 0x1F;

    uint8_t r2 = (color2 >> 11) & 0x1F;
    uint8_t g2 = (color2 >> 5) & 0x3F;
    uint8_t b2 = color2 & 0x1F;

    // Interpolieren
    uint8_t r = r1 + (r2 - r1) * t;
    uint8_t g = g1 + (g2 - g1) * t;
    uint8_t b = b1 + (b2 - b1) * t;

    // RGB565 kodieren
    return (r << 11) | (g << 5) | b;
  }

  // Dimme eine Farbe (factor: 0.0 = schwarz, 1.0 = original)
  uint16_t dimColor(uint16_t color, float factor) {
    factor = constrain(factor, 0.0, 1.0);

    uint8_t r = ((color >> 11) & 0x1F) * factor;
    uint8_t g = ((color >> 5) & 0x3F) * factor;
    uint8_t b = (color & 0x1F) * factor;

    return (r << 11) | (g << 5) | b;
  }
};

#endif // PONGGAMEMODERN_H
