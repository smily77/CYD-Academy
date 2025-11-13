/*
  SnakeGameModern.h - Modernes Snake mit Fancy Design als Klasse gekapselt

  Teil der CYD_Games Library für CYD Display

  Verwendung:
    SnakeGameModern game;
    game.init(&lcd);
    // In loop():
    game.update();
*/

#ifndef SNAKEGAMEMODERN_H
#define SNAKEGAMEMODERN_H

#include <Arduino.h>
#include <LovyanGFX.hpp>

// Forward declaration
class LGFX;

// ===== PIN DEFINITIONS =====
#define SNAKE_BTN_LEFT   tasteB
#define SNAKE_BTN_UP     tasteA
#define SNAKE_BTN_RIGHT  tasteC
#define SNAKE_BTN_DOWN   tasteD

// ===== DISPLAY DIMENSIONS =====
#define SNAKE_SCREEN_WIDTH  320
#define SNAKE_SCREEN_HEIGHT 240

// ===== GRID CONFIGURATION =====
#define SNAKE_GRID_SIZE 10
#define SNAKE_GRID_WIDTH  (SNAKE_SCREEN_WIDTH / SNAKE_GRID_SIZE)
#define SNAKE_GRID_HEIGHT (SNAKE_SCREEN_HEIGHT / SNAKE_GRID_SIZE)

// ===== MODERNE FARBPALETTE =====
#define SNAKE_COLOR_BG_TOP     0x0410
#define SNAKE_COLOR_BG_BOTTOM  0x0000
#define SNAKE_COLOR_NEON_GREEN   0x07E0
#define SNAKE_COLOR_NEON_LIME    0xBFE0
#define SNAKE_COLOR_NEON_CYAN    0x07FF
#define SNAKE_COLOR_NEON_ORANGE  0xFD20
#define SNAKE_COLOR_NEON_RED     0xF800
#define SNAKE_COLOR_NEON_PINK    0xF81F
#define SNAKE_COLOR_NEON_YELLOW  0xFFE0
#define SNAKE_COLOR_SNAKE_BODY   SNAKE_COLOR_NEON_GREEN
#define SNAKE_COLOR_SNAKE_HEAD   SNAKE_COLOR_NEON_CYAN
#define SNAKE_COLOR_FOOD_BASE    SNAKE_COLOR_NEON_ORANGE
#define SNAKE_COLOR_WALL         SNAKE_COLOR_NEON_CYAN
#define SNAKE_COLOR_TEXT         0xFFFF
#define SNAKE_COLOR_SHADOW       0x2104
#define SNAKE_COLOR_HIGHLIGHT    0xFFFF

// ===== ENUMS & STRUCTS =====
enum SnakeDirection {
  SNAKE_DIR_NONE = 0,
  SNAKE_DIR_UP,
  SNAKE_DIR_DOWN,
  SNAKE_DIR_LEFT,
  SNAKE_DIR_RIGHT
};

struct SnakePosition {
  int x, y;
};

struct SnakeParticle {
  float x, y;
  float oldX, oldY;
  float vx, vy;
  int life;
  uint16_t color;
  bool active;
};

// ===== CLASS DEFINITION =====
class SnakeGameModern {
public:
  SnakeGameModern() :
    lcd(nullptr),
    snakeLength(3),
    currentDirection(SNAKE_DIR_RIGHT),
    nextDirection(SNAKE_DIR_RIGHT),
    score(0),
    lastScore(-1),
    lastSnakeLength(-1),
    lastMoveDelay(-1),
    gameOver(false),
    lastMoveTime(0),
    moveDelay(200),
    lastButtonPress(0),
    glowPhase(0),
    foodPulse(0)
  {}

  void init(LGFX* display) {
    lcd = display;

    Serial.println("Initialisiere Modern Snake...");

    // Partikel initialisieren
    for (int i = 0; i < MAX_PARTICLES; i++) {
      particles[i].active = false;
    }

    // Zufallsgenerator
    randomSeed(analogRead(34) + micros());

    initGame();

    Serial.println("Modern Snake bereit!");
  }

  void update() {
    // Animationen aktualisieren
    glowPhase += 0.08;
    if (glowPhase > 6.28) glowPhase = 0;

    foodPulse += 0.1;
    if (foodPulse > 6.28) foodPulse = 0;

    if (gameOver) {
      // Warte auf Button-Druck zum Neustart
      if (checkAnyButton()) {
        initGame();
        gameOver = false;
        delay(300);
      }
      return;
    }

    // Button-Input verarbeiten
    readButtons();

    // Snake bewegen (zeitgesteuert)
    if (millis() - lastMoveTime > moveDelay) {
      lastMoveTime = millis();

      currentDirection = nextDirection;
      moveSnake();

      if (checkCollision()) {
        handleGameOver();
        return;
      }

      if (checkFood()) {
        eatFood();
      }

      drawGame();
    }

    // Partikel aktualisieren und zeichnen (immer)
    updateParticles();
    drawParticles();

    // Wände neu zeichnen (damit Partikel sie nicht löschen)
    drawModernBorder();

    // Score nur neu zeichnen wenn sich etwas geändert hat
    if (score != lastScore || snakeLength != lastSnakeLength || moveDelay != lastMoveDelay) {
      drawModernScore();
      lastScore = score;
      lastSnakeLength = snakeLength;
      lastMoveDelay = moveDelay;
    }

    delay(16);  // ~60 FPS
  }

  bool isGameOver() { return gameOver; }
  int getScore() { return score; }
  int getLength() { return snakeLength; }

private:
  LGFX* lcd;

  // Game State
  static const int MAX_SNAKE_LENGTH = (SNAKE_GRID_WIDTH * SNAKE_GRID_HEIGHT);
  SnakePosition snake[MAX_SNAKE_LENGTH];
  int snakeLength;
  SnakeDirection currentDirection;
  SnakeDirection nextDirection;

  SnakePosition food;
  int score;
  int lastScore;
  int lastSnakeLength;
  int lastMoveDelay;
  bool gameOver;

  // Partikel-System
  static const int MAX_PARTICLES = 30;
  SnakeParticle particles[MAX_PARTICLES];

  // Timing
  unsigned long lastMoveTime;
  int moveDelay;

  // Button Debouncing
  unsigned long lastButtonPress;
  static const int BUTTON_DEBOUNCE = 50;

  // Animationen
  float glowPhase;
  float foodPulse;

  // === PRIVATE METHODS ===

  void initGame() {
    Serial.println("Initialisiere Spiel...");

    // Snake in der Mitte starten
    snakeLength = 3;
    snake[0].x = SNAKE_GRID_WIDTH / 2;
    snake[0].y = SNAKE_GRID_HEIGHT / 2;
    snake[1].x = snake[0].x - 1;
    snake[1].y = snake[0].y;
    snake[2].x = snake[1].x - 1;
    snake[2].y = snake[1].y;

    currentDirection = SNAKE_DIR_RIGHT;
    nextDirection = SNAKE_DIR_RIGHT;

    score = 0;
    moveDelay = 200;

    // Score-Tracking zurücksetzen
    lastScore = score;
    lastSnakeLength = snakeLength;
    lastMoveDelay = moveDelay;

    // Food platzieren
    spawnFood();

    // Bildschirm aufbauen
    drawGradientBackground();
    drawModernBorder();
    drawModernScore();
    drawGame();

    Serial.printf("Snake Position: (%d, %d)\n", snake[0].x, snake[0].y);
    Serial.printf("Food Position: (%d, %d)\n", food.x, food.y);
  }

  void spawnFood() {
    bool validPosition = false;

    while (!validPosition) {
      food.x = random(1, SNAKE_GRID_WIDTH - 1);
      food.y = random(1, SNAKE_GRID_HEIGHT - 1);

      // Prüfen ob Food nicht auf Snake liegt
      validPosition = true;
      for (int i = 0; i < snakeLength; i++) {
        if (snake[i].x == food.x && snake[i].y == food.y) {
          validPosition = false;
          break;
        }
      }
    }

    Serial.printf("Neues Food: (%d, %d)\n", food.x, food.y);
  }

  void readButtons() {
    unsigned long now = millis();
    if (now - lastButtonPress < BUTTON_DEBOUNCE) {
      return;
    }

    // Verhindere 180-Grad Wendungen
    if (digitalRead(SNAKE_BTN_UP) == LOW && currentDirection != SNAKE_DIR_DOWN) {
      nextDirection = SNAKE_DIR_UP;
      lastButtonPress = now;
      Serial.println("Button: UP");
    }
    else if (digitalRead(SNAKE_BTN_DOWN) == LOW && currentDirection != SNAKE_DIR_UP) {
      nextDirection = SNAKE_DIR_DOWN;
      lastButtonPress = now;
      Serial.println("Button: DOWN");
    }
    else if (digitalRead(SNAKE_BTN_LEFT) == LOW && currentDirection != SNAKE_DIR_RIGHT) {
      nextDirection = SNAKE_DIR_LEFT;
      lastButtonPress = now;
      Serial.println("Button: LEFT");
    }
    else if (digitalRead(SNAKE_BTN_RIGHT) == LOW && currentDirection != SNAKE_DIR_LEFT) {
      nextDirection = SNAKE_DIR_RIGHT;
      lastButtonPress = now;
      Serial.println("Button: RIGHT");
    }
  }

  bool checkAnyButton() {
    return (digitalRead(SNAKE_BTN_UP) == LOW ||
            digitalRead(SNAKE_BTN_DOWN) == LOW ||
            digitalRead(SNAKE_BTN_LEFT) == LOW ||
            digitalRead(SNAKE_BTN_RIGHT) == LOW);
  }

  void moveSnake() {
    // Alte Tail-Position merken (zum Löschen)
    SnakePosition oldTail = snake[snakeLength - 1];

    // Alle Segmente nach hinten verschieben
    for (int i = snakeLength - 1; i > 0; i--) {
      snake[i] = snake[i - 1];
    }

    // Kopf in neue Richtung bewegen
    switch (currentDirection) {
      case SNAKE_DIR_UP:
        snake[0].y--;
        break;
      case SNAKE_DIR_DOWN:
        snake[0].y++;
        break;
      case SNAKE_DIR_LEFT:
        snake[0].x--;
        break;
      case SNAKE_DIR_RIGHT:
        snake[0].x++;
        break;
      default:
        break;
    }

    // Altes Tail löschen
    clearGridCell(oldTail.x, oldTail.y);
  }

  bool checkCollision() {
    // Kollision mit Wänden
    if (snake[0].x <= 0 || snake[0].x >= SNAKE_GRID_WIDTH - 1 ||
        snake[0].y <= 0 || snake[0].y >= SNAKE_GRID_HEIGHT - 1) {
      Serial.println("Kollision: Wand!");
      return true;
    }

    // Kollision mit eigenem Körper
    for (int i = 1; i < snakeLength; i++) {
      if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
        Serial.println("Kollision: Eigener Körper!");
        return true;
      }
    }

    return false;
  }

  bool checkFood() {
    if (snake[0].x == food.x && snake[0].y == food.y) {
      Serial.println("Food gegessen!");
      return true;
    }
    return false;
  }

  void eatFood() {
    // Partikel spawnen am Food
    int foodCenterX = food.x * SNAKE_GRID_SIZE + SNAKE_GRID_SIZE/2;
    int foodCenterY = food.y * SNAKE_GRID_SIZE + SNAKE_GRID_SIZE/2;
    spawnParticles(foodCenterX, foodCenterY, SNAKE_COLOR_FOOD_BASE, 15);

    // Snake verlängern
    snakeLength++;
    score += 10;

    // Geschwindigkeit erhöhen
    if (moveDelay > 80) {
      moveDelay -= 5;
    }

    Serial.printf("Score: %d, Länge: %d, Speed: %dms\n", score, snakeLength, moveDelay);

    // Neues Food spawnen
    spawnFood();
    drawModernScore();
  }

  void handleGameOver() {
    gameOver = true;
    Serial.println("\n=== GAME OVER ===");
    Serial.printf("Final Score: %d\n", score);
    Serial.printf("Snake Length: %d\n", snakeLength);

    // Moderner Game Over Screen
    int boxX = SNAKE_SCREEN_WIDTH/4 - 10;
    int boxY = SNAKE_SCREEN_HEIGHT/2 - 55;
    int boxW = SNAKE_SCREEN_WIDTH/2 + 20;
    int boxH = 110;

    // Box mit Gradient
    for (int y = 0; y < boxH; y++) {
      float factor = (float)y / boxH;
      uint16_t color = lerpColor(0x2104, 0x0000, factor);
      lcd->drawFastHLine(boxX, boxY + y, boxW, color);
    }

    // Neon-Umrandung
    lcd->drawRect(boxX - 1, boxY - 1, boxW + 2, boxH + 2, SNAKE_COLOR_NEON_RED);
    lcd->drawRect(boxX - 2, boxY - 2, boxW + 4, boxH + 4, dimColor(SNAKE_COLOR_NEON_RED, 0.5));

    // Text mit Schatten - zentriert in der Box
    int centerX = boxX + boxW / 2;

    lcd->setTextSize(3);
    lcd->setTextColor(SNAKE_COLOR_SHADOW);
    lcd->setCursor(centerX - 79, boxY + 22);
    lcd->println("GAME OVER");

    lcd->setTextColor(SNAKE_COLOR_NEON_RED);
    lcd->setCursor(centerX - 81, boxY + 20);
    lcd->println("GAME OVER");

    // Score
    lcd->setTextSize(2);
    lcd->setTextColor(SNAKE_COLOR_SHADOW);
    lcd->setCursor(centerX - 53, boxY + 57);
    lcd->printf("Score: %d", score);

    lcd->setTextColor(SNAKE_COLOR_NEON_YELLOW);
    lcd->setCursor(centerX - 55, boxY + 55);
    lcd->printf("Score: %d", score);

    // Anweisung
    lcd->setTextSize(1);
    lcd->setTextColor(SNAKE_COLOR_NEON_CYAN);
    lcd->setCursor(centerX - 51, boxY + 85);
    lcd->println("Press any button");
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
        // Alte Position speichern
        particles[i].oldX = particles[i].x;
        particles[i].oldY = particles[i].y;

        // Neue Position berechnen
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].vy += 0.2;  // Gravitation
        particles[i].vx *= 0.98;  // Reibung
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
        // Alte Position löschen (mit Gradient-Hintergrund)
        int oldSize = 1 + (particles[i].life / 10) + 2;
        for (int dy = -oldSize; dy <= oldSize; dy++) {
          for (int dx = -oldSize; dx <= oldSize; dx++) {
            int px = particles[i].oldX + dx;
            int py = particles[i].oldY + dy;
            if (px >= 0 && px < SNAKE_SCREEN_WIDTH && py >= SNAKE_GRID_SIZE && py < SNAKE_SCREEN_HEIGHT) {
              uint16_t bgColor = lerpColor(SNAKE_COLOR_BG_TOP, SNAKE_COLOR_BG_BOTTOM, (float)py / SNAKE_SCREEN_HEIGHT);
              lcd->drawPixel(px, py, bgColor);
            }
          }
        }

        // Neue Position zeichnen (nur wenn nicht im Score-Bereich)
        if (particles[i].y >= SNAKE_GRID_SIZE) {
          float alpha = (float)particles[i].life / 30.0;
          uint16_t color = dimColor(particles[i].color, alpha);

          int size = 1 + (particles[i].life / 10);
          lcd->fillCircle(particles[i].x, particles[i].y, size, color);

          // Glow
          if (alpha > 0.5) {
            lcd->drawCircle(particles[i].x, particles[i].y, size + 1, dimColor(color, 0.3));
          }
        }
      }
    }
  }

  void drawGradientBackground() {
    for (int y = 0; y < SNAKE_SCREEN_HEIGHT; y++) {
      uint16_t color = lerpColor(SNAKE_COLOR_BG_TOP, SNAKE_COLOR_BG_BOTTOM, (float)y / SNAKE_SCREEN_HEIGHT);
      lcd->drawFastHLine(0, y, SNAKE_SCREEN_WIDTH, color);
    }
  }

  void drawModernBorder() {
    for (int x = 0; x < SNAKE_GRID_WIDTH; x++) {
      if (x == 0 || (x >= 12 && x <= 25)) {
        drawModernWall(x, 0);
      }
      drawModernWall(x, SNAKE_GRID_HEIGHT - 1);
    }
    for (int y = 1; y < SNAKE_GRID_HEIGHT - 1; y++) {
      drawModernWall(0, y);
      drawModernWall(SNAKE_GRID_WIDTH - 1, y);
    }
  }

  void drawModernWall(int gridX, int gridY) {
    int pixelX = gridX * SNAKE_GRID_SIZE;
    int pixelY = gridY * SNAKE_GRID_SIZE;

    // Haupt-Block
    lcd->fillRect(pixelX + 2, pixelY + 2, SNAKE_GRID_SIZE - 4, SNAKE_GRID_SIZE - 4, SNAKE_COLOR_WALL);

    // Glow
    lcd->drawRect(pixelX + 1, pixelY + 1, SNAKE_GRID_SIZE - 2, SNAKE_GRID_SIZE - 2, dimColor(SNAKE_COLOR_WALL, 0.5));
    lcd->drawRect(pixelX, pixelY, SNAKE_GRID_SIZE, SNAKE_GRID_SIZE, dimColor(SNAKE_COLOR_WALL, 0.2));
  }

  void drawModernScore() {
    // Kompakte Löschung nur für Text-Bereiche
    lcd->fillRect(SNAKE_GRID_SIZE, 1, 100, 8, SNAKE_COLOR_BG_TOP);
    lcd->fillRect(SNAKE_SCREEN_WIDTH - 60, 1, 58, 8, SNAKE_COLOR_BG_TOP);

    redrawScoreText();
  }

  void redrawScoreText() {
    lcd->setTextSize(1);

    lcd->setTextColor(SNAKE_COLOR_SHADOW);
    lcd->setCursor(SNAKE_GRID_SIZE + 3, 3);
    lcd->printf("Score:%d L:%d", score, snakeLength);

    lcd->setTextColor(SNAKE_COLOR_NEON_YELLOW);
    lcd->setCursor(SNAKE_GRID_SIZE + 2, 2);
    lcd->printf("Score:%d L:%d", score, snakeLength);

    // Speed-Indikator
    lcd->setTextColor(SNAKE_COLOR_NEON_CYAN);
    lcd->setCursor(SNAKE_SCREEN_WIDTH - 60, 2);
    lcd->printf("Spd:%d", 200 - moveDelay);
  }

  void drawGame() {
    // Snake zeichnen
    for (int i = 0; i < snakeLength; i++) {
      if (i == 0) {
        drawModernSnakeHead(snake[i].x, snake[i].y);
      } else {
        drawModernSnakeBody(snake[i].x, snake[i].y, i);
      }
    }

    // Food zeichnen (mit Animation)
    drawModernFood();
  }

  void drawModernSnakeHead(int gridX, int gridY) {
    int pixelX = gridX * SNAKE_GRID_SIZE;
    int pixelY = gridY * SNAKE_GRID_SIZE;

    // Schatten
    lcd->fillRect(pixelX + 2, pixelY + 2, SNAKE_GRID_SIZE - 3, SNAKE_GRID_SIZE - 3, SNAKE_COLOR_SHADOW);

    // Gradient-Kopf
    for (int i = 0; i < SNAKE_GRID_SIZE - 2; i++) {
      float factor = (float)i / (SNAKE_GRID_SIZE - 2);
      uint16_t color = lerpColor(SNAKE_COLOR_SNAKE_HEAD, SNAKE_COLOR_NEON_LIME, factor);
      lcd->drawFastHLine(pixelX + 1, pixelY + 1 + i, SNAKE_GRID_SIZE - 2, color);
    }

    // Pulsierender Glow
    float glow = sin(glowPhase) * 0.5 + 0.5;
    if (glow > 0.3) {
      uint16_t glowColor = dimColor(SNAKE_COLOR_SNAKE_HEAD, glow * 0.6);
      lcd->drawRect(pixelX, pixelY, SNAKE_GRID_SIZE, SNAKE_GRID_SIZE, glowColor);
      if (glow > 0.6) {
        lcd->drawRect(pixelX - 1, pixelY - 1, SNAKE_GRID_SIZE + 2, SNAKE_GRID_SIZE + 2, dimColor(glowColor, 0.4));
      }
    }

    // Augen (je nach Richtung)
    int eye1X = pixelX + SNAKE_GRID_SIZE/2 - 2;
    int eye2X = pixelX + SNAKE_GRID_SIZE/2 + 2;
    int eyeY = pixelY + SNAKE_GRID_SIZE/2;

    if (currentDirection == SNAKE_DIR_LEFT || currentDirection == SNAKE_DIR_RIGHT) {
      eye1X = (currentDirection == SNAKE_DIR_RIGHT) ? pixelX + SNAKE_GRID_SIZE - 3 : pixelX + 2;
      eye2X = eye1X;
      eyeY = pixelY + SNAKE_GRID_SIZE/2 - 1;
      int eyeY2 = pixelY + SNAKE_GRID_SIZE/2 + 1;
      lcd->fillCircle(eye1X, eyeY, 1, SNAKE_COLOR_NEON_RED);
      lcd->fillCircle(eye2X, eyeY2, 1, SNAKE_COLOR_NEON_RED);
    } else {
      eyeY = (currentDirection == SNAKE_DIR_DOWN) ? pixelY + SNAKE_GRID_SIZE - 3 : pixelY + 2;
      lcd->fillCircle(eye1X, eyeY, 1, SNAKE_COLOR_NEON_RED);
      lcd->fillCircle(eye2X, eyeY, 1, SNAKE_COLOR_NEON_RED);
    }
  }

  void drawModernSnakeBody(int gridX, int gridY, int segmentIndex) {
    int pixelX = gridX * SNAKE_GRID_SIZE;
    int pixelY = gridY * SNAKE_GRID_SIZE;

    // Schatten
    lcd->fillRect(pixelX + 2, pixelY + 2, SNAKE_GRID_SIZE - 3, SNAKE_GRID_SIZE - 3, SNAKE_COLOR_SHADOW);

    // Gradient basierend auf Segment-Position
    float segmentFactor = 1.0 - ((float)segmentIndex / snakeLength) * 0.5;
    uint16_t bodyColor = dimColor(SNAKE_COLOR_SNAKE_BODY, segmentFactor);

    // Segment mit leichtem Gradient
    for (int i = 0; i < SNAKE_GRID_SIZE - 2; i++) {
      float factor = (float)i / (SNAKE_GRID_SIZE - 2) * 0.3;
      uint16_t color = dimColor(bodyColor, 1.0 - factor);
      lcd->drawFastHLine(pixelX + 1, pixelY + 1 + i, SNAKE_GRID_SIZE - 2, color);
    }

    // Highlight oben
    lcd->drawFastHLine(pixelX + 2, pixelY + 1, SNAKE_GRID_SIZE - 4, dimColor(SNAKE_COLOR_HIGHLIGHT, 0.3));
  }

  void drawModernFood() {
    int pixelX = food.x * SNAKE_GRID_SIZE;
    int pixelY = food.y * SNAKE_GRID_SIZE;

    // Pulsierende Größe
    float pulse = sin(foodPulse) * 0.3 + 0.7;  // 0.7 bis 1.0
    int size = (SNAKE_GRID_SIZE - 2) * pulse;
    int offset = (SNAKE_GRID_SIZE - size) / 2;

    // Multi-Layer Glow
    for (int r = 3; r >= 0; r--) {
      float intensity = 1.0 - (r * 0.25);
      uint16_t glowColor = dimColor(SNAKE_COLOR_FOOD_BASE, intensity * 0.4);
      lcd->drawRect(pixelX + offset - r, pixelY + offset - r,
                   size + r*2, size + r*2, glowColor);
    }

    // Haupt-Food mit Gradient
    for (int i = 0; i < size; i++) {
      float factor = (float)i / size;
      uint16_t color = lerpColor(SNAKE_COLOR_NEON_YELLOW, SNAKE_COLOR_FOOD_BASE, factor);
      lcd->drawFastHLine(pixelX + offset, pixelY + offset + i, size, color);
    }

    // Highlight
    int hlSize = size / 3;
    lcd->fillRect(pixelX + offset + 1, pixelY + offset + 1, hlSize, hlSize, SNAKE_COLOR_HIGHLIGHT);
  }

  void clearGridCell(int gridX, int gridY) {
    int pixelX = gridX * SNAKE_GRID_SIZE;
    int pixelY = gridY * SNAKE_GRID_SIZE;

    // Mit Gradient-Hintergrund füllen
    for (int y = 0; y < SNAKE_GRID_SIZE; y++) {
      uint16_t bgColor = lerpColor(SNAKE_COLOR_BG_TOP, SNAKE_COLOR_BG_BOTTOM,
                                     (float)(pixelY + y) / SNAKE_SCREEN_HEIGHT);
      lcd->drawFastHLine(pixelX, pixelY + y, SNAKE_GRID_SIZE, bgColor);
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

#endif // SNAKEGAMEMODERN_H
