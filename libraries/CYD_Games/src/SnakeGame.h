/*
  SnakeGame.h - Klassisches Snake-Spiel als Klasse gekapselt

  Teil der CYD_Games Library für CYD Display

  Verwendung:
    SnakeGame game;
    game.init(lcd);
    // In loop():
    game.update();
*/

#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <CYD_Input.h>

// Pin-Mapping (muss VOR der Klasse definiert sein)
#define BTN_LEFT   tasteB
#define BTN_UP     tasteA
#define BTN_RIGHT  tasteC
#define BTN_DOWN   tasteD

class SnakeGame {
public:
  // Konstruktor
  SnakeGame() :
    lcd(nullptr),
    snakeLength(3),
    currentDirection(DIR_RIGHT),
    nextDirection(DIR_RIGHT),
    score(0),
    gameOver(false),
    lastMoveTime(0),
    moveDelay(200),
    lastButtonPress(0)
  {}

  // Initialisierung
  void init(LGFX* display) {
    lcd = display;

    Serial.println("Initialisiere Snake...");

    // Input initialisieren
    CYD_Input::init();

    // Snake in der Mitte starten
    snakeLength = 3;
    snake[0].x = GRID_WIDTH / 2;
    snake[0].y = GRID_HEIGHT / 2;
    snake[1].x = snake[0].x - 1;
    snake[1].y = snake[0].y;
    snake[2].x = snake[1].x - 1;
    snake[2].y = snake[1].y;

    currentDirection = DIR_RIGHT;
    nextDirection = DIR_RIGHT;

    score = 0;
    gameOver = false;
    moveDelay = 200;

    // Food platzieren
    spawnFood();

    // Bildschirm aufbauen
    lcd->fillScreen(COLOR_BG);
    drawBorder();
    drawScore();
    drawGame();

    Serial.println("Snake bereit!");
  }

  // Haupt-Update (in loop() aufrufen)
  void update() {
    if (gameOver) {
      // Warte auf Button-Druck zum Neustart
      if (checkAnyButton()) {
        init(lcd);  // Neu starten
        delay(300);  // Debounce
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
  }

  // Prüfe ob Spiel aktiv ist (für Menu zum Zurückkehren)
  bool isGameOver() { return gameOver; }
  int getScore() { return score; }

private:
  // Display-Referenz
  LGFX* lcd;

  // Konstanten
  static const int SCREEN_WIDTH = 320;
  static const int SCREEN_HEIGHT = 240;
  static const int GRID_SIZE = 10;
  static const int GRID_WIDTH = SCREEN_WIDTH / GRID_SIZE;   // 32
  static const int GRID_HEIGHT = SCREEN_HEIGHT / GRID_SIZE;  // 24
  static const int MAX_SNAKE_LENGTH = GRID_WIDTH * GRID_HEIGHT;
  static const int BUTTON_DEBOUNCE = 50;

  // Farben
  static const uint16_t COLOR_BG = 0x0000;      // Schwarz
  static const uint16_t COLOR_SNAKE = 0x07E0;   // Grün
  static const uint16_t COLOR_HEAD = 0x07FF;    // Cyan
  static const uint16_t COLOR_FOOD = 0xF800;    // Rot
  static const uint16_t COLOR_WALL = 0x4208;    // Dunkelgrau
  static const uint16_t COLOR_TEXT = 0xFFFF;    // Weiß

  // Richtungen
  enum Direction {
    DIR_NONE = 0,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
  };

  // Position
  struct Position {
    int x, y;
  };

  // Game State
  Position snake[GRID_WIDTH * GRID_HEIGHT];
  int snakeLength;
  Direction currentDirection;
  Direction nextDirection;

  Position food;
  int score;
  bool gameOver;

  // Timing
  unsigned long lastMoveTime;
  int moveDelay;
  unsigned long lastButtonPress;

  // === METHODEN ===

  void spawnFood() {
    bool validPosition = false;

    while (!validPosition) {
      food.x = random(1, GRID_WIDTH - 1);
      food.y = random(1, GRID_HEIGHT - 1);

      // Prüfen ob Food nicht auf Snake liegt
      validPosition = true;
      for (int i = 0; i < snakeLength; i++) {
        if (snake[i].x == food.x && snake[i].y == food.y) {
          validPosition = false;
          break;
        }
      }
    }
  }

  void readButtons() {
    unsigned long now = millis();
    if (now - lastButtonPress < BUTTON_DEBOUNCE) {
      return;
    }

    // Verhindere 180-Grad Wendungen
    // WICHTIG: Alle Buttons einzeln prüfen (NICHT else if!)
    // So werden alle Tasten geprüft, auch wenn eine floatet
    if (CYD_Input::readButton(CYD_BTN_A) && currentDirection != DIR_DOWN) {
      nextDirection = DIR_UP;
      lastButtonPress = now;
    }
    if (CYD_Input::readButton(CYD_BTN_D) && currentDirection != DIR_UP) {
      nextDirection = DIR_DOWN;
      lastButtonPress = now;
    }
    if (CYD_Input::readButton(CYD_BTN_B) && currentDirection != DIR_RIGHT) {
      nextDirection = DIR_LEFT;
      lastButtonPress = now;
    }
    if (CYD_Input::readButton(CYD_BTN_C) && currentDirection != DIR_LEFT) {
      nextDirection = DIR_RIGHT;
      lastButtonPress = now;
    }
  }

  bool checkAnyButton() {
    return (CYD_Input::readButton(CYD_BTN_A) ||
            CYD_Input::readButton(CYD_BTN_D) ||
            CYD_Input::readButton(CYD_BTN_B) ||
            CYD_Input::readButton(CYD_BTN_C));
  }

  void moveSnake() {
    // Alte Tail-Position merken (zum Löschen)
    Position oldTail = snake[snakeLength - 1];

    // Alle Segmente nach hinten verschieben
    for (int i = snakeLength - 1; i > 0; i--) {
      snake[i] = snake[i - 1];
    }

    // Kopf in neue Richtung bewegen
    switch (currentDirection) {
      case DIR_UP:
        snake[0].y--;
        break;
      case DIR_DOWN:
        snake[0].y++;
        break;
      case DIR_LEFT:
        snake[0].x--;
        break;
      case DIR_RIGHT:
        snake[0].x++;
        break;
      default:
        break;
    }

    // Altes Tail löschen
    drawGridCell(oldTail.x, oldTail.y, COLOR_BG);
  }

  bool checkCollision() {
    // Kollision mit Wänden
    if (snake[0].x <= 0 || snake[0].x >= GRID_WIDTH - 1 ||
        snake[0].y <= 0 || snake[0].y >= GRID_HEIGHT - 1) {
      return true;
    }

    // Kollision mit eigenem Körper
    for (int i = 1; i < snakeLength; i++) {
      if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
        return true;
      }
    }

    return false;
  }

  bool checkFood() {
    return (snake[0].x == food.x && snake[0].y == food.y);
  }

  void eatFood() {
    // Snake verlängern
    snakeLength++;
    score += 10;

    // Geschwindigkeit erhöhen (aber nicht zu schnell)
    if (moveDelay > 80) {
      moveDelay -= 5;
    }

    // Neues Food spawnen
    spawnFood();
    drawScore();
  }

  void handleGameOver() {
    gameOver = true;
    Serial.println("Snake Game Over!");
    Serial.printf("Final Score: %d\n", score);

    // Game Over Bildschirm
    lcd->fillRect(SCREEN_WIDTH/4, SCREEN_HEIGHT/2 - 40,
                 SCREEN_WIDTH/2, 80, 0x4208);
    lcd->drawRect(SCREEN_WIDTH/4, SCREEN_HEIGHT/2 - 40,
                 SCREEN_WIDTH/2, 80, COLOR_TEXT);

    lcd->setTextSize(3);
    lcd->setTextColor(COLOR_TEXT, 0x4208);
    lcd->setCursor(SCREEN_WIDTH/2 - 75, SCREEN_HEIGHT/2 - 25);
    lcd->println("GAME OVER");

    lcd->setTextSize(2);
    lcd->setCursor(SCREEN_WIDTH/2 - 50, SCREEN_HEIGHT/2 + 5);
    lcd->printf("Score: %d", score);

    lcd->setTextSize(1);
    lcd->setCursor(SCREEN_WIDTH/2 - 60, SCREEN_HEIGHT/2 + 30);
    lcd->println("Press any button");
  }

  void drawBorder() {
    // Rahmen um das Spielfeld
    for (int x = 0; x < GRID_WIDTH; x++) {
      drawGridCell(x, 0, COLOR_WALL);
      drawGridCell(x, GRID_HEIGHT - 1, COLOR_WALL);
    }
    for (int y = 0; y < GRID_HEIGHT; y++) {
      drawGridCell(0, y, COLOR_WALL);
      drawGridCell(GRID_WIDTH - 1, y, COLOR_WALL);
    }
  }

  void drawScore() {
    // Score-Bereich löschen
    lcd->fillRect(0, 0, GRID_SIZE * 8, GRID_SIZE, COLOR_BG);

    lcd->setTextSize(1);
    lcd->setTextColor(COLOR_TEXT, COLOR_BG);
    lcd->setCursor(GRID_SIZE + 2, 2);
    lcd->printf("Score:%d L:%d", score, snakeLength);
  }

  void drawGame() {
    // Snake zeichnen
    for (int i = 0; i < snakeLength; i++) {
      uint16_t color = (i == 0) ? COLOR_HEAD : COLOR_SNAKE;
      drawGridCell(snake[i].x, snake[i].y, color);
    }

    // Food zeichnen
    drawGridCell(food.x, food.y, COLOR_FOOD);
  }

  void drawGridCell(int gridX, int gridY, uint16_t color) {
    int pixelX = gridX * GRID_SIZE;
    int pixelY = gridY * GRID_SIZE;

    // Zelle mit kleinem Rand zeichnen (für Grid-Effekt)
    lcd->fillRect(pixelX + 1, pixelY + 1, GRID_SIZE - 2, GRID_SIZE - 2, color);
  }
};

#endif // SNAKE_GAME_H
