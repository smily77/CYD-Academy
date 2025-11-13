/*
  FroggerGameModern.h - Klassisches Arcade-Spiel mit modernen Effekten als Klasse gekapselt

  Teil der CYD_Games Library für CYD Display

  Verwendung:
    FroggerGameModern game;
    game.init(&lcd);
    // In loop():
    game.update();
*/

#ifndef FROGGERGAMEMODERN_H
#define FROGGERGAMEMODERN_H

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <CYD_Input.h>

// Forward declaration
class LGFX;

// ===== PIN DEFINITIONS =====
#define FROG_BTN_UP     tasteA
#define FROG_BTN_LEFT   tasteB
#define FROG_BTN_RIGHT  tasteC
#define FROG_BTN_DOWN   tasteD

// ===== DISPLAY DIMENSIONS =====
#define FROG_SCREEN_WIDTH  320
#define FROG_SCREEN_HEIGHT 240

// ===== GRID CONFIGURATION =====
#define FROG_CELL_SIZE 10
#define FROG_GRID_WIDTH  (FROG_SCREEN_WIDTH / FROG_CELL_SIZE)   // 32
#define FROG_GRID_HEIGHT (FROG_SCREEN_HEIGHT / FROG_CELL_SIZE)  // 24
#define FROG_PLAY_OFFSET_Y 8  // UI oben, Spielfeld startet bei Y=8

// ===== MODERNE FARBPALETTE =====
#define FROG_COLOR_BG           0x0000  // Schwarz
#define FROG_COLOR_GRASS_DARK   0x1A40  // Dunkelgrün
#define FROG_COLOR_GRASS_LIGHT  0x3D60  // Hellgrün
#define FROG_COLOR_ROAD_DARK    0x2945  // Dunkelgrau
#define FROG_COLOR_ROAD_LIGHT   0x5ACB  // Hellgrau
#define FROG_COLOR_WATER_DARK   0x0016  // Dunkelblau
#define FROG_COLOR_WATER_LIGHT  0x021F  // Hellblau
#define FROG_COLOR_FROG         0x07E0  // Grün
#define FROG_COLOR_FROG_SHADOW  0x0300  // Dunkles Grün
#define FROG_COLOR_CAR_RED      0xF800  // Rot
#define FROG_COLOR_CAR_BLUE     0x001F  // Blau
#define FROG_COLOR_CAR_SHADOW   0x1082  // Grau-Shadow
#define FROG_COLOR_HEADLIGHT    0xFFE0  // Gelb
#define FROG_COLOR_HEADLIGHT_G  0x8C20  // Gelb-Glow
#define FROG_COLOR_LOG          0xD69A  // Hellbraun
#define FROG_COLOR_LOG_DARK     0x8C40  // Dunkelbraun
#define FROG_COLOR_LOG_SHADOW   0x1082  // Grau-Shadow
#define FROG_COLOR_HOME         0xFFE0  // Gelb
#define FROG_COLOR_TEXT         0xFFFF  // Weiß
#define FROG_COLOR_SPLASH1      0x07FF  // Cyan
#define FROG_COLOR_SPLASH2      0x03FF  // Türkis
#define FROG_COLOR_SPLASH3      0x01BF  // Dunkel Cyan

// ===== GAME CONSTANTS =====
#define FROG_MAX_CARS 8
#define FROG_MAX_LOGS 8
#define FROG_NUM_HOMES 5
#define FROG_MAX_PARTICLES 20
#define FROG_MOVE_DELAY 150  // ms zwischen Bewegungen

// ===== STRUCTS =====
struct FroggerFrog {
  float x, y;            // Grid-Position
  float animX, animY;    // Animierte Position für smooth movement
  int maxY;              // Weiteste Y-Position erreicht
  bool alive;
  int invulnerable;      // Frames invulnerabel
  bool moving;           // Bewegungsanimation aktiv
  int moveFrames;        // Frames für Bewegung
};

struct FroggerCar {
  int lane;              // Fahrspur
  float x;               // Position
  int w;                 // Breite in Zellen
  int h;                 // Höhe in Zellen
  float speed;           // Geschwindigkeit
  uint16_t color;        // Farbe
  bool active;
};

struct FroggerLog {
  int lane;              // Reihe
  float x;               // Position
  int w;                 // Breite in Zellen
  int h;                 // Höhe in Zellen
  float speed;           // Geschwindigkeit
  bool active;
};

struct FroggerHome {
  int x;                 // X-Position
  bool filled;           // Schon erreicht?
};

struct FroggerParticle {
  float x, y;
  float vx, vy;
  uint16_t color;
  uint8_t life;  // 0-100
  bool active;
};

// ===== CLASS DEFINITION =====
class FroggerGameModern {
public:
  FroggerGameModern() :
    lcd(nullptr),
    score(0),
    lives(3),
    level(1),
    gameOver(false),
    oldFrogX(-1),
    oldFrogY(-1),
    lastMoveTime(0),
    waterFrame(0),
    lastWaterAnim(0)
  {}

  void init(LGFX* display) {
    lcd = display;

    Serial.println("\n=== FROGGER MODERN ===\n");

    // Zufallsgenerator
    randomSeed(analogRead(34) + analogRead(35) + micros());

    // Partikel initialisieren
    for (int i = 0; i < FROG_MAX_PARTICLES; i++) {
      particles[i].active = false;
    }

    // Bildschirm aufbauen
    initGame();

    Serial.println("Frogger Modern gestartet!");
    Serial.println("Steuerung:");
    Serial.println("  Links: GPIO 5");
    Serial.println("  Oben:  GPIO 17");
    Serial.println("  Rechts:GPIO 16");
    Serial.println("  Unten: GPIO 18");
  }

  void update() {
    if (gameOver) {
      // Warte auf Neustart
      if (CYD_Input::readButton(CYD_BTN_A)) {
        initGame();
        gameOver = false;
        delay(300);
      }
      return;
    }

    // Input
    handleInput();

    // Update
    updateCars();
    updateLogs();
    updateParticles();
    updateFrogAnimation();
    updateWaterAnimation();
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

  bool isGameOver() { return gameOver; }
  int getScore() { return score; }
  int getLives() { return lives; }
  int getLevel() { return level; }

private:
  LGFX* lcd;

  // Game objects
  FroggerFrog frog;
  FroggerCar cars[FROG_MAX_CARS];
  FroggerLog logs[FROG_MAX_LOGS];
  FroggerHome homes[FROG_NUM_HOMES];
  FroggerParticle particles[FROG_MAX_PARTICLES];

  // Game state
  int score;
  int lives;
  int level;
  bool gameOver;

  // Frosch-Tracking
  float oldFrogX;
  float oldFrogY;
  unsigned long lastMoveTime;

  // Wasser-Animation
  int waterFrame;
  unsigned long lastWaterAnim;

  // === METHODEN ===

  void initGame() {
    Serial.println("Initialisiere Spiel...");

    score = 0;
    level = 1;
    lives = 3;
    gameOver = false;
    waterFrame = 0;

    // Frosch initialisieren
    frog.x = FROG_GRID_WIDTH / 2.0;
    frog.y = 0;
    frog.animX = frog.x;
    frog.animY = frog.y;
    frog.maxY = 0;
    frog.alive = true;
    frog.invulnerable = 0;
    frog.moving = false;
    frog.moveFrames = 0;
    oldFrogX = -1;
    oldFrogY = -1;
    lastMoveTime = 0;

    // Partikel initialisieren
    for (int i = 0; i < FROG_MAX_PARTICLES; i++) {
      particles[i].active = false;
    }

    // Bildschirm aufbauen
    drawBackground();
    drawUI();

    // Level initialisieren
    initLevel();
  }

  void initLevel() {
    Serial.printf("Level %d gestartet!\n", level);

    // Frosch zurücksetzen
    frog.x = FROG_GRID_WIDTH / 2.0;
    frog.y = 0;
    frog.animX = frog.x;
    frog.animY = frog.y;
    frog.maxY = 0;
    frog.alive = true;
    frog.invulnerable = 30;
    frog.moving = false;
    frog.moveFrames = 0;
    oldFrogX = -1;
    oldFrogY = -1;

    // Häuser initialisieren
    int homeSpacing = FROG_GRID_WIDTH / (FROG_NUM_HOMES + 1);
    for (int i = 0; i < FROG_NUM_HOMES; i++) {
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
        if (carCount < FROG_MAX_CARS) {
          cars[carCount].lane = lane;
          cars[carCount].x = (FROG_GRID_WIDTH / numCars) * j;
          cars[carCount].w = 3 + random(0, 2);
          cars[carCount].h = 2;
          cars[carCount].speed = (0.15 + level * 0.03) * (dirRight ? 1 : -1);
          cars[carCount].color = (random(0, 2) == 0) ? FROG_COLOR_CAR_RED : FROG_COLOR_CAR_BLUE;
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
        if (logCount < FROG_MAX_LOGS) {
          logs[logCount].lane = lane;
          logs[logCount].x = (FROG_GRID_WIDTH / numLogs) * j;
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

  void handleInput() {
    if (!frog.alive || frog.moving) return;

    unsigned long now = millis();
    if (now - lastMoveTime < FROG_MOVE_DELAY) return;

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
      if (newX >= 0 && newX < FROG_GRID_WIDTH && newY >= 0 && newY < FROG_GRID_HEIGHT) {
        frog.x = newX;
        frog.y = newY;
        frog.moving = true;
        frog.moveFrames = 0;

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

  void updateFrogAnimation() {
    if (!frog.moving) return;

    // Smooth interpolation über 8 Frames
    frog.moveFrames++;
    float t = frog.moveFrames / 8.0;
    t = constrain(t, 0.0, 1.0);

    // Easing für natürlichere Bewegung
    t = t * t * (3.0 - 2.0 * t);  // Smoothstep

    frog.animX = oldFrogX + (frog.x - oldFrogX) * t;
    frog.animY = oldFrogY + (frog.y - oldFrogY) * t;

    if (frog.moveFrames >= 8) {
      frog.moving = false;
      frog.animX = frog.x;
      frog.animY = frog.y;
    }
  }

  void updateWaterAnimation() {
    unsigned long now = millis();
    if (now - lastWaterAnim > 100) {
      waterFrame = (waterFrame + 1) % 4;
      lastWaterAnim = now;
      drawWaterRipples();
    }
  }

  void updateCars() {
    lcd->startWrite();

    for (int i = 0; i < FROG_MAX_CARS; i++) {
      if (!cars[i].active) continue;

      float oldX = cars[i].x;
      cars[i].x += cars[i].speed;

      // Screen wrapping
      if (cars[i].speed > 0 && cars[i].x >= FROG_GRID_WIDTH) {
        cars[i].x = -cars[i].w;
        oldX = cars[i].x;
      } else if (cars[i].speed < 0 && cars[i].x + cars[i].w <= 0) {
        cars[i].x = FROG_GRID_WIDTH;
        oldX = cars[i].x;
      }

      // Nur Lücke löschen (mit Gradient)
      int oldScreenX = (int)oldX * FROG_CELL_SIZE;
      int newScreenX = (int)cars[i].x * FROG_CELL_SIZE;
      int screenY = FROG_PLAY_OFFSET_Y + (FROG_GRID_HEIGHT - cars[i].lane - cars[i].h) * FROG_CELL_SIZE;
      int height = cars[i].h * FROG_CELL_SIZE;

      if (cars[i].speed > 0 && newScreenX > oldScreenX) {
        int gapWidth = newScreenX - oldScreenX;
        for (int dy = 0; dy < height; dy++) {
          int gridY = FROG_GRID_HEIGHT - cars[i].lane - cars[i].h + (dy / FROG_CELL_SIZE);
          uint16_t bgColor = getZoneColor(gridY, true);
          lcd->drawFastHLine(oldScreenX, screenY + dy, gapWidth, bgColor);
        }
      } else if (cars[i].speed < 0 && newScreenX < oldScreenX) {
        int width = cars[i].w * FROG_CELL_SIZE;
        int gapWidth = oldScreenX - newScreenX;
        for (int dy = 0; dy < height; dy++) {
          int gridY = FROG_GRID_HEIGHT - cars[i].lane - cars[i].h + (dy / FROG_CELL_SIZE);
          uint16_t bgColor = getZoneColor(gridY, true);
          lcd->drawFastHLine(oldScreenX + width - gapWidth, screenY + dy, gapWidth, bgColor);
        }
      }

      drawCar(i);
    }

    lcd->endWrite();
  }

  void updateLogs() {
    lcd->startWrite();

    for (int i = 0; i < FROG_MAX_LOGS; i++) {
      if (!logs[i].active) continue;

      float oldX = logs[i].x;
      logs[i].x += logs[i].speed;

      // Screen wrapping
      if (logs[i].speed > 0 && logs[i].x >= FROG_GRID_WIDTH) {
        logs[i].x = -logs[i].w;
        oldX = logs[i].x;
      } else if (logs[i].speed < 0 && logs[i].x + logs[i].w <= 0) {
        logs[i].x = FROG_GRID_WIDTH;
        oldX = logs[i].x;
      }

      // Nur Lücke löschen (mit Gradient + Ripples)
      int oldScreenX = (int)oldX * FROG_CELL_SIZE;
      int newScreenX = (int)logs[i].x * FROG_CELL_SIZE;
      int screenY = FROG_PLAY_OFFSET_Y + (FROG_GRID_HEIGHT - logs[i].lane - logs[i].h) * FROG_CELL_SIZE;
      int height = logs[i].h * FROG_CELL_SIZE;

      if (logs[i].speed > 0 && newScreenX > oldScreenX) {
        int gapWidth = newScreenX - oldScreenX;
        for (int dy = 0; dy < height; dy++) {
          int gridY = FROG_GRID_HEIGHT - logs[i].lane - logs[i].h + (dy / FROG_CELL_SIZE);
          uint16_t bgColor = getZoneColor(gridY, true);
          lcd->drawFastHLine(oldScreenX, screenY + dy, gapWidth, bgColor);
        }
      } else if (logs[i].speed < 0 && newScreenX < oldScreenX) {
        int width = logs[i].w * FROG_CELL_SIZE;
        int gapWidth = oldScreenX - newScreenX;
        for (int dy = 0; dy < height; dy++) {
          int gridY = FROG_GRID_HEIGHT - logs[i].lane - logs[i].h + (dy / FROG_CELL_SIZE);
          uint16_t bgColor = getZoneColor(gridY, true);
          lcd->drawFastHLine(oldScreenX + width - gapWidth, screenY + dy, gapWidth, bgColor);
        }
      }

      drawLog(i);
    }

    lcd->endWrite();

    // Frosch auf Baumstamm mitbewegen
    if (frog.alive && !frog.moving && (int)frog.y >= 12 && (int)frog.y <= 19) {
      for (int i = 0; i < FROG_MAX_LOGS; i++) {
        if (!logs[i].active) continue;

        if ((int)frog.y < logs[i].lane || (int)frog.y >= logs[i].lane + logs[i].h) continue;

        if (frog.x >= logs[i].x && frog.x < logs[i].x + logs[i].w) {
          frog.x += logs[i].speed;
          frog.animX += logs[i].speed;
          frog.x = constrain(frog.x, 0, FROG_GRID_WIDTH - 1);
          frog.animX = constrain(frog.animX, 0, FROG_GRID_WIDTH - 1);
          break;
        }
      }
    }
  }

  void updateParticles() {
    for (int i = 0; i < FROG_MAX_PARTICLES; i++) {
      if (!particles[i].active) continue;

      // Alte Position löschen
      int gridY = ((int)particles[i].y - FROG_PLAY_OFFSET_Y) / FROG_CELL_SIZE;
      gridY = FROG_GRID_HEIGHT - gridY - 1;
      uint16_t bgColor = getZoneColor(gridY, false);
      lcd->fillCircle((int)particles[i].x, (int)particles[i].y, 1, bgColor);

      // Bewegen
      particles[i].x += particles[i].vx;
      particles[i].y += particles[i].vy;
      particles[i].vy += 0.3;  // Gravitation
      particles[i].life -= 8;

      // Deaktivieren wenn tot oder außerhalb
      if (particles[i].life <= 0 ||
          particles[i].y > FROG_SCREEN_HEIGHT ||
          particles[i].x < 0 || particles[i].x > FROG_SCREEN_WIDTH) {
        particles[i].active = false;
        continue;
      }

      // Neue Position zeichnen
      lcd->fillCircle((int)particles[i].x, (int)particles[i].y, 1, particles[i].color);
    }
  }

  void spawnSplash(int screenX, int screenY) {
    // 5-8 Partikel spawnen
    int numParticles = random(5, 9);

    for (int i = 0; i < numParticles; i++) {
      // Freies Partikel finden
      for (int p = 0; p < FROG_MAX_PARTICLES; p++) {
        if (!particles[p].active) {
          particles[p].x = screenX;
          particles[p].y = screenY;

          // Zufällige Geschwindigkeit
          float angle = random(0, 628) / 100.0;  // 0 bis 2*PI
          float speed = random(5, 15) / 10.0;
          particles[p].vx = cos(angle) * speed;
          particles[p].vy = sin(angle) * speed - 2;  // Nach oben

          // Farbe variieren
          int colorChoice = random(0, 3);
          if (colorChoice == 0) particles[p].color = FROG_COLOR_SPLASH1;
          else if (colorChoice == 1) particles[p].color = FROG_COLOR_SPLASH2;
          else particles[p].color = FROG_COLOR_SPLASH3;

          particles[p].life = 100;
          particles[p].active = true;
          break;
        }
      }
    }
  }

  void checkCollisions() {
    if (!frog.alive || frog.invulnerable > 0) {
      if (frog.invulnerable > 0) frog.invulnerable--;
      return;
    }

    // Straße: Kollision mit Autos
    if ((int)frog.y >= 2 && (int)frog.y <= 9) {
      for (int i = 0; i < FROG_MAX_CARS; i++) {
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

      for (int i = 0; i < FROG_MAX_LOGS; i++) {
        if (!logs[i].active) continue;

        if ((int)frog.y < logs[i].lane || (int)frog.y >= logs[i].lane + logs[i].h) continue;

        if (frog.x >= logs[i].x && frog.x < logs[i].x + logs[i].w) {
          onLog = true;
          break;
        }
      }

      if (!onLog) {
        // Splash Effekt beim Ertrinken
        int screenX = (int)frog.animX * FROG_CELL_SIZE + FROG_CELL_SIZE / 2;
        int screenY = FROG_PLAY_OFFSET_Y + (FROG_GRID_HEIGHT - (int)frog.animY - 1) * FROG_CELL_SIZE + FROG_CELL_SIZE / 2;
        spawnSplash(screenX, screenY);
        frogDied();
        return;
      }
    }
  }

  void checkGoal() {
    if (!frog.alive || frog.moving) return;

    if ((int)frog.y == 23) {
      bool inHome = false;
      for (int i = 0; i < FROG_NUM_HOMES; i++) {
        if (abs((int)frog.x - homes[i].x) <= 1 && !homes[i].filled) {
          homes[i].filled = true;
          score += 50;
          drawHomes();
          drawUI();

          // Frosch zurücksetzen
          frog.x = FROG_GRID_WIDTH / 2.0;
          frog.y = 0;
          frog.animX = frog.x;
          frog.animY = frog.y;
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
      frog.x = FROG_GRID_WIDTH / 2.0;
      frog.y = 0;
      frog.animX = frog.x;
      frog.animY = frog.y;
      frog.maxY = 0;
      frog.alive = true;
      frog.invulnerable = 60;
      frog.moving = false;
      frog.moveFrames = 0;
      oldFrogX = -1;
      oldFrogY = -1;
    }
  }

  bool checkLevelComplete() {
    for (int i = 0; i < FROG_NUM_HOMES; i++) {
      if (!homes[i].filled) return false;
    }
    return true;
  }

  void handleGameOver() {
    gameOver = true;
    Serial.println("\n=== GAME OVER ===");
    Serial.printf("Final Score: %d\n", score);
    Serial.printf("Level erreicht: %d\n", level);

    // Game Over Screen
    uint16_t boxColor = 0x2104;
    lcd->fillRect(FROG_SCREEN_WIDTH/4 - 10, FROG_SCREEN_HEIGHT/2 - 45,
                 FROG_SCREEN_WIDTH/2 + 20, 90, boxColor);
    lcd->drawRect(FROG_SCREEN_WIDTH/4 - 10, FROG_SCREEN_HEIGHT/2 - 45,
                 FROG_SCREEN_WIDTH/2 + 20, 90, FROG_COLOR_TEXT);

    lcd->setTextSize(3);
    lcd->setTextColor(FROG_COLOR_TEXT, boxColor);
    lcd->setCursor(FROG_SCREEN_WIDTH/2 - 75, FROG_SCREEN_HEIGHT/2 - 30);
    lcd->println("GAME OVER");

    lcd->setTextSize(2);
    lcd->setCursor(FROG_SCREEN_WIDTH/2 - 55, FROG_SCREEN_HEIGHT/2 + 5);
    lcd->printf("Score: %d", score);

    lcd->setTextSize(1);
    lcd->setCursor(FROG_SCREEN_WIDTH/2 - 55, FROG_SCREEN_HEIGHT/2 + 25);
    lcd->println("Press UP to restart");
  }

  // === ZEICHNEN ===

  void drawBackground() {
    // Zone für Zone mit Gradienten zeichnen
    // Grasbereiche: Y=0-1, Y=10-11, Y=22-23
    // Straße: Y=2-9
    // Fluss: Y=12-19

    for (int gridY = 0; gridY < FROG_GRID_HEIGHT - FROG_PLAY_OFFSET_Y/FROG_CELL_SIZE; gridY++) {
      int screenY = FROG_PLAY_OFFSET_Y + gridY * FROG_CELL_SIZE;
      uint16_t color = getZoneColor(gridY, true);

      for (int dy = 0; dy < FROG_CELL_SIZE; dy++) {
        lcd->drawFastHLine(0, screenY + dy, FROG_SCREEN_WIDTH, color);
      }
    }
  }

  void drawWaterRipples() {
    // Zeichne animierte Ripples im Wasser (Y=12-19)
    lcd->startWrite();

    for (int gridY = 12; gridY <= 19; gridY++) {
      int screenY = FROG_PLAY_OFFSET_Y + (FROG_GRID_HEIGHT - gridY - 1) * FROG_CELL_SIZE;

      for (int x = 0; x < FROG_SCREEN_WIDTH; x += 8) {
        int offsetX = (x + waterFrame * 2) % 8;
        if (offsetX < 2) {
          uint16_t rippleColor = lerpColor(getZoneColor(gridY, true), FROG_COLOR_WATER_LIGHT, 0.3);
          lcd->drawPixel(x + offsetX, screenY + 2 + (gridY % 2), rippleColor);
        }
      }
    }

    lcd->endWrite();
  }

  void drawCars() {
    for (int i = 0; i < FROG_MAX_CARS; i++) {
      if (cars[i].active) {
        drawCar(i);
      }
    }
  }

  void drawCar(int index) {
    FroggerCar* c = &cars[index];
    int screenX = (int)c->x * FROG_CELL_SIZE;
    int screenY = FROG_PLAY_OFFSET_Y + (FROG_GRID_HEIGHT - c->lane - c->h) * FROG_CELL_SIZE;
    int width = c->w * FROG_CELL_SIZE;
    int height = c->h * FROG_CELL_SIZE;
    bool movingRight = (c->speed > 0);

    // Schatten (links/rechts versetzt)
    int shadowOffset = movingRight ? 1 : -1;
    lcd->fillRect(screenX + shadowOffset, screenY + height - 2, width, 2, FROG_COLOR_CAR_SHADOW);

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
    lcd->fillCircle(screenX + 5, screenY + height - 3, 2, FROG_COLOR_BG);
    if (width >= 20) {
      lcd->fillCircle(screenX + width - 6, screenY + height - 3, 2, FROG_COLOR_BG);
    }

    // Scheinwerfer mit Glow-Effekt
    if (width >= 15) {
      if (movingRight) {
        // Glow
        lcd->fillRect(screenX + width - 4, screenY + 3, 3, 4, FROG_COLOR_HEADLIGHT_G);
        lcd->fillRect(screenX + width - 4, screenY + height - 7, 3, 4, FROG_COLOR_HEADLIGHT_G);
        // Core
        lcd->fillRect(screenX + width - 3, screenY + 4, 2, 3, FROG_COLOR_HEADLIGHT);
        lcd->fillRect(screenX + width - 3, screenY + height - 7, 2, 3, FROG_COLOR_HEADLIGHT);
      } else {
        // Glow
        lcd->fillRect(screenX + 1, screenY + 3, 3, 4, FROG_COLOR_HEADLIGHT_G);
        lcd->fillRect(screenX + 1, screenY + height - 7, 3, 4, FROG_COLOR_HEADLIGHT_G);
        // Core
        lcd->fillRect(screenX + 1, screenY + 4, 2, 3, FROG_COLOR_HEADLIGHT);
        lcd->fillRect(screenX + 1, screenY + height - 7, 2, 3, FROG_COLOR_HEADLIGHT);
      }
    }
  }

  void drawLogs() {
    for (int i = 0; i < FROG_MAX_LOGS; i++) {
      if (logs[i].active) {
        drawLog(i);
      }
    }
  }

  void drawLog(int index) {
    FroggerLog* l = &logs[index];
    int screenX = (int)l->x * FROG_CELL_SIZE;
    int screenY = FROG_PLAY_OFFSET_Y + (FROG_GRID_HEIGHT - l->lane - l->h) * FROG_CELL_SIZE;
    int width = l->w * FROG_CELL_SIZE;
    int height = l->h * FROG_CELL_SIZE;

    // Schatten
    lcd->fillRect(screenX + 1, screenY + height - 1, width - 1, 2, FROG_COLOR_LOG_SHADOW);

    // Baumstamm-Grundfarbe
    lcd->fillRect(screenX, screenY, width, height, FROG_COLOR_LOG);

    // Holztextur - Jahresringe (vertikal)
    for (int i = 0; i < width; i += 8) {
      lcd->drawFastVLine(screenX + i, screenY + 1, height - 2, FROG_COLOR_LOG_DARK);
      if (i + 1 < width) {
        lcd->drawFastVLine(screenX + i + 1, screenY + 2, height - 4, 0x9B60);
      }
    }

    // Rand oben/unten dunkler
    lcd->drawFastHLine(screenX, screenY, width, 0x5280);
    lcd->drawFastHLine(screenX, screenY + height - 1, width, 0x5280);

    // Holzmaserung (horizontal)
    for (int i = 0; i < height; i += 4) {
      if (i > 0 && i < height - 1) {
        for (int x = 0; x < width; x += 3) {
          lcd->drawPixel(screenX + x, screenY + i, FROG_COLOR_LOG_DARK);
        }
      }
    }
  }

  void drawHomes() {
    for (int i = 0; i < FROG_NUM_HOMES; i++) {
      int screenX = homes[i].x * FROG_CELL_SIZE;
      int screenY = FROG_PLAY_OFFSET_Y;

      if (homes[i].filled) {
        // Gefüllt - Gelb mit Glow
        lcd->fillRect(screenX - FROG_CELL_SIZE - 1, screenY - 1,
                     FROG_CELL_SIZE * 2 + 2, FROG_CELL_SIZE * 2 + 2, 0x8C20);
        lcd->fillRect(screenX - FROG_CELL_SIZE, screenY,
                     FROG_CELL_SIZE * 2, FROG_CELL_SIZE * 2, FROG_COLOR_HOME);
      } else {
        // Leer - Nur Rahmen
        lcd->drawRect(screenX - FROG_CELL_SIZE, screenY,
                     FROG_CELL_SIZE * 2, FROG_CELL_SIZE * 2, FROG_COLOR_TEXT);
      }
    }
  }

  void drawFrog() {
    if (!frog.alive) return;

    // Blinken wenn invulnerabel
    if (frog.invulnerable > 0 && (frog.invulnerable / 5) % 2 == 0) {
      return;
    }

    int screenX = (int)frog.animX * FROG_CELL_SIZE;
    int screenY = FROG_PLAY_OFFSET_Y + (FROG_GRID_HEIGHT - (int)frog.animY - 1) * FROG_CELL_SIZE;

    // Schatten
    lcd->fillCircle(screenX + FROG_CELL_SIZE/2 + 1, screenY + FROG_CELL_SIZE - 2,
                   FROG_CELL_SIZE/2 - 2, FROG_COLOR_FROG_SHADOW);

    // Frosch-Körper
    lcd->fillCircle(screenX + FROG_CELL_SIZE/2, screenY + FROG_CELL_SIZE/2,
                   FROG_CELL_SIZE/2 - 1, FROG_COLOR_FROG);

    // Augen (größer und moderner)
    lcd->fillCircle(screenX + 3, screenY + 3, 2, FROG_COLOR_TEXT);
    lcd->fillCircle(screenX + FROG_CELL_SIZE - 3, screenY + 3, 2, FROG_COLOR_TEXT);
    lcd->fillCircle(screenX + 3, screenY + 3, 1, FROG_COLOR_BG);
    lcd->fillCircle(screenX + FROG_CELL_SIZE - 3, screenY + 3, 1, FROG_COLOR_BG);

    // Beine (mit Highlight)
    lcd->fillRect(screenX + 1, screenY + FROG_CELL_SIZE - 3, 2, 2, 0x05E0);
    lcd->fillRect(screenX + FROG_CELL_SIZE - 3, screenY + FROG_CELL_SIZE - 3, 2, 2, 0x05E0);
    lcd->drawPixel(screenX + 1, screenY + FROG_CELL_SIZE - 3, 0x07F0);
    lcd->drawPixel(screenX + FROG_CELL_SIZE - 3, screenY + FROG_CELL_SIZE - 3, 0x07F0);
  }

  void eraseFrog() {
    if (oldFrogX < 0 || oldFrogY < 0) return;

    int screenX = (int)oldFrogX * FROG_CELL_SIZE;
    int screenY = FROG_PLAY_OFFSET_Y + (FROG_GRID_HEIGHT - (int)oldFrogY - 1) * FROG_CELL_SIZE;

    // Hintergrundfarbe abhängig von Zone (mit Gradient)
    for (int dy = 0; dy < FROG_CELL_SIZE; dy++) {
      int gridY = FROG_GRID_HEIGHT - (int)oldFrogY - 1;
      uint16_t bgColor = getZoneColor(gridY, true);
      lcd->drawFastHLine(screenX, screenY + dy, FROG_CELL_SIZE, bgColor);
    }
  }

  void drawUI() {
    // UI-Bereich löschen
    lcd->fillRect(0, 0, FROG_SCREEN_WIDTH, 8, FROG_COLOR_BG);

    lcd->setTextSize(1);
    lcd->setTextColor(FROG_COLOR_TEXT, FROG_COLOR_BG);

    // Score (links)
    lcd->setCursor(2, 0);
    lcd->printf("SCORE:%d", score);

    // Level (Mitte)
    lcd->setCursor(FROG_SCREEN_WIDTH/2 - 20, 0);
    lcd->printf("LV:%d", level);

    // Lives (rechts)
    lcd->setCursor(FROG_SCREEN_WIDTH - 50, 0);
    lcd->printf("LIVES:%d", lives);
  }

  void drawGame() {
    eraseFrog();
    drawFrog();

    oldFrogX = frog.animX;
    oldFrogY = frog.animY;
  }

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

  // Hintergrundfarbe für Zone
  uint16_t getZoneColor(int gridY, bool gradient) {
    // Straße (Y=2-9)
    if (gridY >= 2 && gridY <= 9) {
      if (!gradient) return FROG_COLOR_ROAD_DARK;
      float t = (gridY - 2) / 8.0;
      return lerpColor(FROG_COLOR_ROAD_LIGHT, FROG_COLOR_ROAD_DARK, t);
    }
    // Fluss (Y=12-19)
    else if (gridY >= 12 && gridY <= 19) {
      if (!gradient) return FROG_COLOR_WATER_DARK;
      float t = (gridY - 12) / 8.0;
      return lerpColor(FROG_COLOR_WATER_LIGHT, FROG_COLOR_WATER_DARK, t);
    }
    // Gras (Rest)
    else {
      if (!gradient) return FROG_COLOR_GRASS_DARK;
      float t = (gridY % 4) / 4.0;
      return lerpColor(FROG_COLOR_GRASS_LIGHT, FROG_COLOR_GRASS_DARK, t);
    }
  }
};

#endif // FROGGERGAMEMODERN_H
