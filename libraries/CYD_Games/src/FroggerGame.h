/*
  FroggerGame.h - Encapsulated Frogger Game
  Teil der CYD_Games Library

  WICHTIG: CYD_Display_Config.h muss VOR diesem Header inkludiert werden!
*/

#ifndef FROGGERGAME_H
#define FROGGERGAME_H

#include <Arduino.h>
#include <CYD_Input.h>
// Forward declaration - LGFX muss bereits definiert sein
class LGFX;

// Pin-Mapping (muss VOR der Klasse definiert sein)
#define FRG_BTN_LEFT   tasteB
#define FRG_BTN_UP     tasteA
#define FRG_BTN_RIGHT  tasteC
#define FRG_BTN_DOWN   tasteD

// Spielfeld-Dimensionen
#define FRG_SCREEN_WIDTH  320
#define FRG_SCREEN_HEIGHT 240

// Grid-Konfiguration
#define FRG_CELL_SIZE 10
#define FRG_GRID_WIDTH  (FRG_SCREEN_WIDTH / FRG_CELL_SIZE)   // 32
#define FRG_GRID_HEIGHT (FRG_SCREEN_HEIGHT / FRG_CELL_SIZE)  // 24
#define FRG_PLAY_OFFSET_Y 8  // UI oben, Spielfeld startet bei Y=8

// Farben
#define FRG_COLOR_BG        0x0000  // Schwarz
#define FRG_COLOR_ROAD      0x4208  // Dunkelgrau
#define FRG_COLOR_GRASS     0x2444  // Grün
#define FRG_COLOR_WATER     0x001F  // Blau
#define FRG_COLOR_FROG      0x07E0  // Grün
#define FRG_COLOR_CAR_RED   0xF800  // Rot
#define FRG_COLOR_CAR_BLUE  0x001F  // Blau
#define FRG_COLOR_LOG       0xD69A  // Hellbraun
#define FRG_COLOR_HOME      0xFFE0  // Gelb
#define FRG_COLOR_TEXT      0xFFFF  // Weiß

// Strukturen (müssen VOR der Klasse definiert sein)
struct FRG_Frog {
  float x, y;            // Grid-Position
  int maxY;              // Weiteste Y-Position erreicht
  bool alive;
  int invulnerable;      // Frames invulnerabel
};

struct FRG_Car {
  int lane;              // Fahrspur
  float x;               // Position
  int w;                 // Breite in Zellen
  int h;                 // Höhe in Zellen
  float speed;           // Geschwindigkeit
  uint16_t color;        // Farbe
  bool active;
};

struct FRG_Log {
  int lane;              // Reihe
  float x;               // Position
  int w;                 // Breite in Zellen
  int h;                 // Höhe in Zellen
  float speed;           // Geschwindigkeit
  bool active;
};

struct FRG_Home {
  int x;                 // X-Position
  bool filled;           // Schon erreicht?
};

// Array-Größen
#define FRG_MAX_CARS 8
#define FRG_MAX_LOGS 8
#define FRG_NUM_HOMES 5
#define FRG_MOVE_DELAY 150  // ms zwischen Bewegungen

class FroggerGame {
public:
  FroggerGame() :
    lcd(nullptr),
    score(0),
    lives(3),
    level(1),
    gameOver(false),
    oldFrogX(-1),
    oldFrogY(-1),
    lastMoveTime(0)
  {
    // Konstruktor
  }

  void init(LGFX* display) {
    lcd = display;

    score = 0;
    level = 1;
    lives = 3;
    gameOver = false;

    // Frosch initialisieren
    frog.x = FRG_GRID_WIDTH / 2.0;
    frog.y = 0;
    frog.maxY = 0;
    frog.alive = true;
    frog.invulnerable = 0;
    oldFrogX = -1;
    oldFrogY = -1;
    lastMoveTime = 0;

    // Input initialisieren
    CYD_Input::init();

    // Bildschirm aufbauen
    drawBackground();
    drawUI();

    // Level initialisieren
    initLevel();

    Serial.println("Frogger gestartet!");
  }

  void update() {
    if (gameOver) {
      // Warte auf Neustart
      if (CYD_Input::readButton(CYD_BTN_A)) {
        init(lcd);
        delay(300);
      }
      return;
    }

    // Input
    handleInput();

    // Update
    updateCars();
    updateLogs();
    checkCollisions();
    checkGoal();

    // Zeichnen
    drawGame();

    // Level komplett?
    if (checkLevelComplete()) {
      level++;
      Serial.printf("Level %d!\n", level);
      initLevel();
      delay(2000);
    }

    delay(16);  // ~60 FPS
  }

  // Status-Abfragen
  bool isGameOver() const { return gameOver; }
  int getScore() const { return score; }
  int getLevel() const { return level; }
  int getLives() const { return lives; }

private:
  LGFX* lcd;

  // Game State
  FRG_Frog frog;
  FRG_Car cars[FRG_MAX_CARS];
  FRG_Log logs[FRG_MAX_LOGS];
  FRG_Home homes[FRG_NUM_HOMES];

  int score;
  int lives;
  int level;
  bool gameOver;

  float oldFrogX;
  float oldFrogY;
  unsigned long lastMoveTime;

  // ===== INITIALISIERUNG =====

  void initLevel() {
    Serial.printf("Level %d gestartet!\n", level);

    // Frosch zurücksetzen
    frog.x = FRG_GRID_WIDTH / 2.0;
    frog.y = 0;
    frog.maxY = 0;
    frog.alive = true;
    frog.invulnerable = 30;
    oldFrogX = -1;
    oldFrogY = -1;

    // Häuser initialisieren
    int homeSpacing = FRG_GRID_WIDTH / (FRG_NUM_HOMES + 1);
    for (int i = 0; i < FRG_NUM_HOMES; i++) {
      homes[i].x = homeSpacing * (i + 1);
      homes[i].filled = false;
    }

    // Autos initialisieren
    int carCount = 0;
    int carLanes[] = {2, 4, 6, 8};
    for (int i = 0; i < 4; i++) {
      int lane = carLanes[i];
      int numCars = 2;
      bool dirRight = (lane % 4 == 0);

      for (int j = 0; j < numCars; j++) {
        if (carCount < FRG_MAX_CARS) {
          cars[carCount].lane = lane;
          cars[carCount].x = (FRG_GRID_WIDTH / numCars) * j;
          cars[carCount].w = 3 + random(0, 2);
          cars[carCount].h = 2;
          cars[carCount].speed = (0.15 + level * 0.03) * (dirRight ? 1 : -1);
          cars[carCount].color = (random(0, 2) == 0) ? FRG_COLOR_CAR_RED : FRG_COLOR_CAR_BLUE;
          cars[carCount].active = true;
          carCount++;
        }
      }
    }

    // Baumstämme initialisieren
    int logCount = 0;
    int logLanes[] = {12, 14, 16, 18};
    for (int i = 0; i < 4; i++) {
      int lane = logLanes[i];
      int numLogs = 2;
      bool dirRight = (lane % 4 == 0);

      for (int j = 0; j < numLogs; j++) {
        if (logCount < FRG_MAX_LOGS) {
          logs[logCount].lane = lane;
          logs[logCount].x = (FRG_GRID_WIDTH / numLogs) * j;
          logs[logCount].w = 5 + random(0, 3);
          logs[logCount].h = 2;
          logs[logCount].speed = (0.1 + level * 0.02) * (dirRight ? 1 : -1);
          logs[logCount].active = true;
          logCount++;
        }
      }
    }

    // Zeichne Spielfeld
    drawCars();
    drawLogs();
    drawHomes();
  }

  // ===== INPUT =====

  void handleInput() {
    if (!frog.alive) return;

    unsigned long now = millis();
    if (now - lastMoveTime < FRG_MOVE_DELAY) return;

    float newX = frog.x;
    float newY = frog.y;
    bool moved = false;

    if (CYD_Input::readButton(CYD_BTN_B)) {
      newX--;
      moved = true;
    } else if (CYD_Input::readButton(CYD_BTN_C)) {
      newX++;
      moved = true;
    } else if (CYD_Input::readButton(CYD_BTN_A)) {
      newY++;
      moved = true;
    } else if (CYD_Input::readButton(CYD_BTN_D)) {
      newY--;
      moved = true;
    }

    if (moved) {
      if (newX >= 0 && newX < FRG_GRID_WIDTH && newY >= 0 && newY < FRG_GRID_HEIGHT) {
        frog.x = newX;
        frog.y = newY;

        // Punkte für Vorwärtsbewegung
        if ((int)newY > frog.maxY) {
          score += 10;
          frog.maxY = (int)newY;
          drawUI();
        }

        lastMoveTime = now;
      }
    }
  }

  // ===== UPDATE =====

  void updateCars() {
    lcd->startWrite();

    for (int i = 0; i < FRG_MAX_CARS; i++) {
      if (!cars[i].active) continue;

      float oldX = cars[i].x;
      cars[i].x += cars[i].speed;

      // Screen wrapping
      if (cars[i].speed > 0 && cars[i].x >= FRG_GRID_WIDTH) {
        cars[i].x = -cars[i].w;
        oldX = cars[i].x;
      } else if (cars[i].speed < 0 && cars[i].x + cars[i].w <= 0) {
        cars[i].x = FRG_GRID_WIDTH;
        oldX = cars[i].x;
      }

      // Nur Lücke löschen
      int oldScreenX = (int)oldX * FRG_CELL_SIZE;
      int newScreenX = (int)cars[i].x * FRG_CELL_SIZE;
      int screenY = FRG_PLAY_OFFSET_Y + (FRG_GRID_HEIGHT - cars[i].lane - cars[i].h) * FRG_CELL_SIZE;
      int height = cars[i].h * FRG_CELL_SIZE;

      if (cars[i].speed > 0 && newScreenX > oldScreenX) {
        int gapWidth = newScreenX - oldScreenX;
        lcd->fillRect(oldScreenX, screenY, gapWidth, height, FRG_COLOR_ROAD);
      } else if (cars[i].speed < 0 && newScreenX < oldScreenX) {
        int width = cars[i].w * FRG_CELL_SIZE;
        int gapWidth = oldScreenX - newScreenX;
        lcd->fillRect(oldScreenX + width - gapWidth, screenY, gapWidth, height, FRG_COLOR_ROAD);
      }

      drawCar(i);
    }

    lcd->endWrite();
  }

  void updateLogs() {
    lcd->startWrite();

    for (int i = 0; i < FRG_MAX_LOGS; i++) {
      if (!logs[i].active) continue;

      float oldX = logs[i].x;
      logs[i].x += logs[i].speed;

      // Screen wrapping
      if (logs[i].speed > 0 && logs[i].x >= FRG_GRID_WIDTH) {
        logs[i].x = -logs[i].w;
        oldX = logs[i].x;
      } else if (logs[i].speed < 0 && logs[i].x + logs[i].w <= 0) {
        logs[i].x = FRG_GRID_WIDTH;
        oldX = logs[i].x;
      }

      // Nur Lücke löschen
      int oldScreenX = (int)oldX * FRG_CELL_SIZE;
      int newScreenX = (int)logs[i].x * FRG_CELL_SIZE;
      int screenY = FRG_PLAY_OFFSET_Y + (FRG_GRID_HEIGHT - logs[i].lane - logs[i].h) * FRG_CELL_SIZE;
      int height = logs[i].h * FRG_CELL_SIZE;

      if (logs[i].speed > 0 && newScreenX > oldScreenX) {
        int gapWidth = newScreenX - oldScreenX;
        lcd->fillRect(oldScreenX, screenY, gapWidth, height, FRG_COLOR_WATER);
      } else if (logs[i].speed < 0 && newScreenX < oldScreenX) {
        int width = logs[i].w * FRG_CELL_SIZE;
        int gapWidth = oldScreenX - newScreenX;
        lcd->fillRect(oldScreenX + width - gapWidth, screenY, gapWidth, height, FRG_COLOR_WATER);
      }

      drawLog(i);
    }

    lcd->endWrite();

    // Frosch auf Baumstamm mitbewegen
    if (frog.alive && (int)frog.y >= 12 && (int)frog.y <= 19) {
      for (int i = 0; i < FRG_MAX_LOGS; i++) {
        if (!logs[i].active) continue;

        if ((int)frog.y < logs[i].lane || (int)frog.y >= logs[i].lane + logs[i].h) continue;

        if (frog.x >= logs[i].x && frog.x < logs[i].x + logs[i].w) {
          frog.x += logs[i].speed;
          frog.x = constrain(frog.x, 0, FRG_GRID_WIDTH - 1);
          break;
        }
      }
    }
  }

  // ===== KOLLISIONEN =====

  void checkCollisions() {
    if (!frog.alive || frog.invulnerable > 0) {
      if (frog.invulnerable > 0) frog.invulnerable--;
      return;
    }

    // Straße: Kollision mit Autos
    if ((int)frog.y >= 2 && (int)frog.y <= 9) {
      for (int i = 0; i < FRG_MAX_CARS; i++) {
        if (!cars[i].active) continue;

        if ((int)frog.y < cars[i].lane || (int)frog.y >= cars[i].lane + cars[i].h) continue;

        if (frog.x >= cars[i].x && frog.x < cars[i].x + cars[i].w) {
          frogDied();
          return;
        }
      }
    }

    // Fluss: Frosch ertrinkt ohne Baumstamm
    if ((int)frog.y >= 12 && (int)frog.y <= 19) {
      bool onLog = false;

      for (int i = 0; i < FRG_MAX_LOGS; i++) {
        if (!logs[i].active) continue;

        if ((int)frog.y < logs[i].lane || (int)frog.y >= logs[i].lane + logs[i].h) continue;

        if (frog.x >= logs[i].x && frog.x < logs[i].x + logs[i].w) {
          onLog = true;
          break;
        }
      }

      if (!onLog) {
        frogDied();
        return;
      }
    }
  }

  void checkGoal() {
    if (!frog.alive) return;

    if ((int)frog.y == 23) {
      bool inHome = false;
      for (int i = 0; i < FRG_NUM_HOMES; i++) {
        if (abs((int)frog.x - homes[i].x) <= 1 && !homes[i].filled) {
          homes[i].filled = true;
          score += 50;
          drawHomes();
          drawUI();

          // Frosch zurücksetzen
          frog.x = FRG_GRID_WIDTH / 2.0;
          frog.y = 0;
          frog.maxY = 0;
          frog.invulnerable = 30;
          oldFrogX = -1;
          oldFrogY = -1;

          inHome = true;
          break;
        }
      }

      if (!inHome) {
        frogDied();
      }
    }
  }

  void frogDied() {
    Serial.println("Frosch gestorben!");

    frog.alive = false;
    eraseFrog();

    lives--;
    drawUI();

    delay(1000);

    if (lives <= 0) {
      handleGameOver();
    } else {
      // Respawn
      frog.x = FRG_GRID_WIDTH / 2.0;
      frog.y = 0;
      frog.maxY = 0;
      frog.alive = true;
      frog.invulnerable = 60;
      oldFrogX = -1;
      oldFrogY = -1;
    }
  }

  bool checkLevelComplete() {
    for (int i = 0; i < FRG_NUM_HOMES; i++) {
      if (!homes[i].filled) return false;
    }
    return true;
  }

  void handleGameOver() {
    gameOver = true;
    Serial.println("\n=== GAME OVER ===");
    Serial.printf("Final Score: %d\n", score);
    Serial.printf("Level erreicht: %d\n", level);

    lcd->fillRect(FRG_SCREEN_WIDTH/4 - 10, FRG_SCREEN_HEIGHT/2 - 45,
                 FRG_SCREEN_WIDTH/2 + 20, 90, 0x2104);
    lcd->drawRect(FRG_SCREEN_WIDTH/4 - 10, FRG_SCREEN_HEIGHT/2 - 45,
                 FRG_SCREEN_WIDTH/2 + 20, 90, FRG_COLOR_TEXT);

    lcd->setTextSize(3);
    lcd->setTextColor(FRG_COLOR_TEXT, 0x2104);
    lcd->setCursor(FRG_SCREEN_WIDTH/2 - 75, FRG_SCREEN_HEIGHT/2 - 30);
    lcd->println("GAME OVER");

    lcd->setTextSize(2);
    lcd->setCursor(FRG_SCREEN_WIDTH/2 - 55, FRG_SCREEN_HEIGHT/2 + 5);
    lcd->printf("Score: %d", score);

    lcd->setTextSize(1);
    lcd->setCursor(FRG_SCREEN_WIDTH/2 - 55, FRG_SCREEN_HEIGHT/2 + 25);
    lcd->println("Press UP to restart");
  }

  // ===== ZEICHNEN =====

  void drawBackground() {
    lcd->fillRect(0, 0, FRG_SCREEN_WIDTH, FRG_PLAY_OFFSET_Y, FRG_COLOR_BG);
    lcd->fillRect(0, FRG_PLAY_OFFSET_Y, FRG_SCREEN_WIDTH, 4 * FRG_CELL_SIZE, FRG_COLOR_GRASS);
    lcd->fillRect(0, FRG_PLAY_OFFSET_Y + 4 * FRG_CELL_SIZE, FRG_SCREEN_WIDTH, 8 * FRG_CELL_SIZE, FRG_COLOR_WATER);
    lcd->fillRect(0, FRG_PLAY_OFFSET_Y + 12 * FRG_CELL_SIZE, FRG_SCREEN_WIDTH, 2 * FRG_CELL_SIZE, FRG_COLOR_GRASS);
    lcd->fillRect(0, FRG_PLAY_OFFSET_Y + 14 * FRG_CELL_SIZE, FRG_SCREEN_WIDTH, 8 * FRG_CELL_SIZE, FRG_COLOR_ROAD);
    lcd->fillRect(0, FRG_PLAY_OFFSET_Y + 22 * FRG_CELL_SIZE, FRG_SCREEN_WIDTH, 2 * FRG_CELL_SIZE, FRG_COLOR_GRASS);
  }

  void drawCars() {
    for (int i = 0; i < FRG_MAX_CARS; i++) {
      if (cars[i].active) {
        drawCar(i);
      }
    }
  }

  void drawCar(int index) {
    FRG_Car* c = &cars[index];
    int screenX = (int)c->x * FRG_CELL_SIZE;
    int screenY = FRG_PLAY_OFFSET_Y + (FRG_GRID_HEIGHT - c->lane - c->h) * FRG_CELL_SIZE;
    int width = c->w * FRG_CELL_SIZE;
    int height = c->h * FRG_CELL_SIZE;
    bool movingRight = (c->speed > 0);

    // Auto-Karosserie
    lcd->fillRect(screenX + 1, screenY + 2, width - 2, height - 4, c->color);

    // Abgerundete Ecken vorne
    if (movingRight) {
      lcd->fillRect(screenX + width - 4, screenY + 1, 2, 1, c->color);
      lcd->fillRect(screenX + width - 4, screenY + height - 2, 2, 1, c->color);
    } else {
      lcd->fillRect(screenX + 2, screenY + 1, 2, 1, c->color);
      lcd->fillRect(screenX + 2, screenY + height - 2, 2, 1, c->color);
    }

    // Räder
    lcd->fillCircle(screenX + 5, screenY + height - 3, 2, FRG_COLOR_BG);
    if (width >= 20) {
      lcd->fillCircle(screenX + width - 6, screenY + height - 3, 2, FRG_COLOR_BG);
    }

    // Scheinwerfer
    if (width >= 15) {
      if (movingRight) {
        lcd->fillRect(screenX + width - 3, screenY + 4, 2, 3, 0xFFE0);
        lcd->fillRect(screenX + width - 3, screenY + height - 7, 2, 3, 0xFFE0);
      } else {
        lcd->fillRect(screenX + 1, screenY + 4, 2, 3, 0xFFE0);
        lcd->fillRect(screenX + 1, screenY + height - 7, 2, 3, 0xFFE0);
      }
    }
  }

  void drawLogs() {
    for (int i = 0; i < FRG_MAX_LOGS; i++) {
      if (logs[i].active) {
        drawLog(i);
      }
    }
  }

  void drawLog(int index) {
    FRG_Log* l = &logs[index];
    int screenX = (int)l->x * FRG_CELL_SIZE;
    int screenY = FRG_PLAY_OFFSET_Y + (FRG_GRID_HEIGHT - l->lane - l->h) * FRG_CELL_SIZE;
    int width = l->w * FRG_CELL_SIZE;
    int height = l->h * FRG_CELL_SIZE;

    // Baumstamm-Grundfarbe
    lcd->fillRect(screenX, screenY, width, height, FRG_COLOR_LOG);

    // Jahresringe/Textur
    for (int i = 0; i < width; i += 8) {
      lcd->drawFastVLine(screenX + i, screenY + 1, height - 2, 0x9B60);
    }

    // Rand oben/unten dunkler
    lcd->drawFastHLine(screenX, screenY, width, 0x5280);
    lcd->drawFastHLine(screenX, screenY + height - 1, width, 0x5280);
  }

  void drawHomes() {
    for (int i = 0; i < FRG_NUM_HOMES; i++) {
      int screenX = homes[i].x * FRG_CELL_SIZE;
      int screenY = FRG_PLAY_OFFSET_Y;

      if (homes[i].filled) {
        lcd->fillRect(screenX - FRG_CELL_SIZE, screenY, FRG_CELL_SIZE * 2, FRG_CELL_SIZE * 2, FRG_COLOR_HOME);
      } else {
        lcd->drawRect(screenX - FRG_CELL_SIZE, screenY, FRG_CELL_SIZE * 2, FRG_CELL_SIZE * 2, FRG_COLOR_TEXT);
      }
    }
  }

  void drawFrog() {
    if (!frog.alive) return;

    // Blinken wenn invulnerabel
    if (frog.invulnerable > 0 && (frog.invulnerable / 5) % 2 == 0) {
      return;
    }

    int screenX = (int)frog.x * FRG_CELL_SIZE;
    int screenY = FRG_PLAY_OFFSET_Y + (FRG_GRID_HEIGHT - (int)frog.y - 1) * FRG_CELL_SIZE;

    // Frosch-Körper
    lcd->fillCircle(screenX + FRG_CELL_SIZE/2, screenY + FRG_CELL_SIZE/2, FRG_CELL_SIZE/2 - 1, FRG_COLOR_FROG);

    // Augen
    lcd->fillCircle(screenX + 3, screenY + 3, 2, FRG_COLOR_TEXT);
    lcd->fillCircle(screenX + FRG_CELL_SIZE - 3, screenY + 3, 2, FRG_COLOR_TEXT);
    lcd->drawPixel(screenX + 3, screenY + 3, FRG_COLOR_BG);
    lcd->drawPixel(screenX + FRG_CELL_SIZE - 3, screenY + 3, FRG_COLOR_BG);

    // Beine
    lcd->fillRect(screenX + 1, screenY + FRG_CELL_SIZE - 3, 2, 2, 0x05E0);
    lcd->fillRect(screenX + FRG_CELL_SIZE - 3, screenY + FRG_CELL_SIZE - 3, 2, 2, 0x05E0);
  }

  void eraseFrog() {
    if (oldFrogX < 0 || oldFrogY < 0) return;

    int screenX = (int)oldFrogX * FRG_CELL_SIZE;
    int screenY = FRG_PLAY_OFFSET_Y + (FRG_GRID_HEIGHT - (int)oldFrogY - 1) * FRG_CELL_SIZE;

    // Hintergrundfarbe abhängig von Zone
    uint16_t bgColor = FRG_COLOR_GRASS;
    if ((int)oldFrogY >= 2 && (int)oldFrogY <= 9) bgColor = FRG_COLOR_ROAD;
    else if ((int)oldFrogY >= 12 && (int)oldFrogY <= 19) bgColor = FRG_COLOR_WATER;

    lcd->fillRect(screenX, screenY, FRG_CELL_SIZE, FRG_CELL_SIZE, bgColor);
  }

  void drawUI() {
    lcd->fillRect(0, 0, FRG_SCREEN_WIDTH, 8, FRG_COLOR_BG);

    lcd->setTextSize(1);
    lcd->setTextColor(FRG_COLOR_TEXT, FRG_COLOR_BG);

    lcd->setCursor(2, 0);
    lcd->printf("S:%d", score);

    lcd->setCursor(FRG_SCREEN_WIDTH/2 - 15, 0);
    lcd->printf("L:%d", level);

    lcd->setCursor(FRG_SCREEN_WIDTH - 40, 0);
    lcd->printf("Lvs:%d", lives);
  }

  void drawGame() {
    eraseFrog();
    drawFrog();

    oldFrogX = frog.x;
    oldFrogY = frog.y;
  }
};

#endif // FROGGERGAME_H
