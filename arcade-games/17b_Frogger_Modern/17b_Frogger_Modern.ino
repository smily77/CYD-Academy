/*
  Frogger Modern - Klassisches Arcade-Spiel mit modernen Effekten

  Hardware:
  - CYD Display (320x240)
  - 4x Digitale Buttons:
    * tasteA: Oben
    * tasteB:  Links
    * tasteC: Rechts
    * tasteD: Unten

  Hinweis: Alle Pins sind in CYD_Display_Config.h definiert.

  Steuerung:
  - Tasten: Bewege Frosch hoch/runter/links/rechts

  Spielregeln:
  - Bringe den Frosch sicher nach oben zu den Häusern
  - Straße: Vermeide Autos
  - Fluss: Springe auf Baumstämme!
  - Erreiche alle 5 Häuser für nächstes Level
  - 3 Leben
  - Punkte: Vorwärts bewegen = +10, Haus erreichen = +50

  Moderne Features:
  - Gradient-Hintergründe für alle Zonen
  - Animierte Wasser-Ripples
  - Auto-Scheinwerfer mit Glow-Effekten
  - Holz-Textur auf Baumstämmen
  - Partikel-Splash beim Wasserkontakt
  - Schatten unter Autos und Baumstämmen
  - Flüssige Frosch-Hüpf-Animation
  - Moderne UI
*/

#include <CYD_Display_Config.h>
#include <CYD_Input.h>

// Pin-Mapping: Physische Pins -> Logische Namen
#define BTN_UP     tasteA
#define BTN_LEFT   tasteB
#define BTN_RIGHT  tasteC
#define BTN_DOWN   tasteD

// Display Objekt
LGFX lcd;

// Display-Dimensionen
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// Grid-Konfiguration
#define CELL_SIZE 10
#define GRID_WIDTH  (SCREEN_WIDTH / CELL_SIZE)   // 32
#define GRID_HEIGHT (SCREEN_HEIGHT / CELL_SIZE)  // 24
#define PLAY_OFFSET_Y 8  // UI oben, Spielfeld startet bei Y=8

// Farben - Moderne Palette
#define COLOR_BG           0x0000  // Schwarz
#define COLOR_GRASS_DARK   0x1A40  // Dunkelgrün
#define COLOR_GRASS_LIGHT  0x3D60  // Hellgrün
#define COLOR_ROAD_DARK    0x2945  // Dunkelgrau
#define COLOR_ROAD_LIGHT   0x5ACB  // Hellgrau
#define COLOR_WATER_DARK   0x0016  // Dunkelblau
#define COLOR_WATER_LIGHT  0x021F  // Hellblau
#define COLOR_FROG         0x07E0  // Grün
#define COLOR_FROG_SHADOW  0x0300  // Dunkles Grün
#define COLOR_CAR_RED      0xF800  // Rot
#define COLOR_CAR_BLUE     0x001F  // Blau
#define COLOR_CAR_SHADOW   0x1082  // Grau-Shadow
#define COLOR_HEADLIGHT    0xFFE0  // Gelb
#define COLOR_HEADLIGHT_G  0x8C20  // Gelb-Glow
#define COLOR_LOG          0xD69A  // Hellbraun
#define COLOR_LOG_DARK     0x8C40  // Dunkelbraun
#define COLOR_LOG_SHADOW   0x1082  // Grau-Shadow
#define COLOR_HOME         0xFFE0  // Gelb
#define COLOR_TEXT         0xFFFF  // Weiß
#define COLOR_SPLASH1      0x07FF  // Cyan
#define COLOR_SPLASH2      0x03FF  // Türkis
#define COLOR_SPLASH3      0x01BF  // Dunkel Cyan

// Strukturen
struct Frog {
  float x, y;            // Grid-Position
  float animX, animY;    // Animierte Position für smooth movement
  int maxY;              // Weiteste Y-Position erreicht
  bool alive;
  int invulnerable;      // Frames invulnerabel
  bool moving;           // Bewegungsanimation aktiv
  int moveFrames;        // Frames für Bewegung
};

struct Car {
  int lane;              // Fahrspur
  float x;               // Position
  int w;                 // Breite in Zellen
  int h;                 // Höhe in Zellen
  float speed;           // Geschwindigkeit
  uint16_t color;        // Farbe
  bool active;
};

struct Log {
  int lane;              // Reihe
  float x;               // Position
  int w;                 // Breite in Zellen
  int h;                 // Höhe in Zellen
  float speed;           // Geschwindigkeit
  bool active;
};

struct Home {
  int x;                 // X-Position
  bool filled;           // Schon erreicht?
};

// Partikel für Wasser-Splash
struct Particle {
  float x, y;
  float vx, vy;
  uint16_t color;
  uint8_t life;  // 0-100
  bool active;
};

// Array-Größen
#define MAX_CARS 8
#define MAX_LOGS 8
#define NUM_HOMES 5
#define MAX_PARTICLES 20
#define MOVE_DELAY 150  // ms zwischen Bewegungen

// Game Objekte
Frog frog;
Car cars[MAX_CARS];
Log logs[MAX_LOGS];
Home homes[NUM_HOMES];
Particle particles[MAX_PARTICLES];

// Game State
int score = 0;
int lives = 3;
int level = 1;
bool gameOver = false;

float oldFrogX = -1;
float oldFrogY = -1;
unsigned long lastMoveTime = 0;

// Wasser-Animation
int waterFrame = 0;
unsigned long lastWaterAnim = 0;

// ===== HILFSFUNKTIONEN =====

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
    if (!gradient) return COLOR_ROAD_DARK;
    float t = (gridY - 2) / 8.0;
    return lerpColor(COLOR_ROAD_LIGHT, COLOR_ROAD_DARK, t);
  }
  // Fluss (Y=12-19)
  else if (gridY >= 12 && gridY <= 19) {
    if (!gradient) return COLOR_WATER_DARK;
    float t = (gridY - 12) / 8.0;
    return lerpColor(COLOR_WATER_LIGHT, COLOR_WATER_DARK, t);
  }
  // Gras (Rest)
  else {
    if (!gradient) return COLOR_GRASS_DARK;
    float t = (gridY % 4) / 4.0;
    return lerpColor(COLOR_GRASS_LIGHT, COLOR_GRASS_DARK, t);
  }
}

// ===== ARDUINO SETUP/LOOP =====

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== FROGGER MODERN ===\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);
  lcd.fillScreen(COLOR_BG);
  lcd.setBrightness(255);

  // Input initialisieren
  CYD_Input::init();

  // Zufallsgenerator
  randomSeed(CYD_Input::readPoti(CYD_POTI_LEFT) + CYD_Input::readPoti(CYD_POTI_RIGHT) + micros());

  // Spiel initialisieren
  initGame();

  Serial.println("Frogger Modern gestartet!");
  Serial.println("Steuerung:");
  Serial.println("  Links: GPIO 5");
  Serial.println("  Oben:  GPIO 17");
  Serial.println("  Rechts:GPIO 16");
  Serial.println("  Unten: GPIO 18");
}

void loop() {
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

// ===== INITIALISIERUNG =====

void initGame() {
  Serial.println("Initialisiere Spiel...");

  score = 0;
  level = 1;
  lives = 3;
  gameOver = false;
  waterFrame = 0;

  // Frosch initialisieren
  frog.x = GRID_WIDTH / 2.0;
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
  for (int i = 0; i < MAX_PARTICLES; i++) {
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
  frog.x = GRID_WIDTH / 2.0;
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
  int homeSpacing = GRID_WIDTH / (NUM_HOMES + 1);
  for (int i = 0; i < NUM_HOMES; i++) {
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
      if (carCount < MAX_CARS) {
        cars[carCount].lane = lane;
        cars[carCount].x = (GRID_WIDTH / numCars) * j;
        cars[carCount].w = 3 + random(0, 2);
        cars[carCount].h = 2;
        cars[carCount].speed = (0.15 + level * 0.03) * (dirRight ? 1 : -1);
        cars[carCount].color = (random(0, 2) == 0) ? COLOR_CAR_RED : COLOR_CAR_BLUE;
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
      if (logCount < MAX_LOGS) {
        logs[logCount].lane = lane;
        logs[logCount].x = (GRID_WIDTH / numLogs) * j;
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
  if (!frog.alive || frog.moving) return;

  unsigned long now = millis();
  if (now - lastMoveTime < MOVE_DELAY) return;

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
    if (newX >= 0 && newX < GRID_WIDTH && newY >= 0 && newY < GRID_HEIGHT) {
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

// ===== UPDATE =====

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
  lcd.startWrite();

  for (int i = 0; i < MAX_CARS; i++) {
    if (!cars[i].active) continue;

    float oldX = cars[i].x;
    cars[i].x += cars[i].speed;

    // Screen wrapping
    if (cars[i].speed > 0 && cars[i].x >= GRID_WIDTH) {
      cars[i].x = -cars[i].w;
      oldX = cars[i].x;
    } else if (cars[i].speed < 0 && cars[i].x + cars[i].w <= 0) {
      cars[i].x = GRID_WIDTH;
      oldX = cars[i].x;
    }

    // Nur Lücke löschen (mit Gradient)
    int oldScreenX = (int)oldX * CELL_SIZE;
    int newScreenX = (int)cars[i].x * CELL_SIZE;
    int screenY = PLAY_OFFSET_Y + (GRID_HEIGHT - cars[i].lane - cars[i].h) * CELL_SIZE;
    int height = cars[i].h * CELL_SIZE;

    if (cars[i].speed > 0 && newScreenX > oldScreenX) {
      int gapWidth = newScreenX - oldScreenX;
      for (int dy = 0; dy < height; dy++) {
        int gridY = GRID_HEIGHT - cars[i].lane - cars[i].h + (dy / CELL_SIZE);
        uint16_t bgColor = getZoneColor(gridY, true);
        lcd.drawFastHLine(oldScreenX, screenY + dy, gapWidth, bgColor);
      }
    } else if (cars[i].speed < 0 && newScreenX < oldScreenX) {
      int width = cars[i].w * CELL_SIZE;
      int gapWidth = oldScreenX - newScreenX;
      for (int dy = 0; dy < height; dy++) {
        int gridY = GRID_HEIGHT - cars[i].lane - cars[i].h + (dy / CELL_SIZE);
        uint16_t bgColor = getZoneColor(gridY, true);
        lcd.drawFastHLine(oldScreenX + width - gapWidth, screenY + dy, gapWidth, bgColor);
      }
    }

    drawCar(i);
  }

  lcd.endWrite();
}

void updateLogs() {
  lcd.startWrite();

  for (int i = 0; i < MAX_LOGS; i++) {
    if (!logs[i].active) continue;

    float oldX = logs[i].x;
    logs[i].x += logs[i].speed;

    // Screen wrapping
    if (logs[i].speed > 0 && logs[i].x >= GRID_WIDTH) {
      logs[i].x = -logs[i].w;
      oldX = logs[i].x;
    } else if (logs[i].speed < 0 && logs[i].x + logs[i].w <= 0) {
      logs[i].x = GRID_WIDTH;
      oldX = logs[i].x;
    }

    // Nur Lücke löschen (mit Gradient + Ripples)
    int oldScreenX = (int)oldX * CELL_SIZE;
    int newScreenX = (int)logs[i].x * CELL_SIZE;
    int screenY = PLAY_OFFSET_Y + (GRID_HEIGHT - logs[i].lane - logs[i].h) * CELL_SIZE;
    int height = logs[i].h * CELL_SIZE;

    if (logs[i].speed > 0 && newScreenX > oldScreenX) {
      int gapWidth = newScreenX - oldScreenX;
      for (int dy = 0; dy < height; dy++) {
        int gridY = GRID_HEIGHT - logs[i].lane - logs[i].h + (dy / CELL_SIZE);
        uint16_t bgColor = getZoneColor(gridY, true);
        lcd.drawFastHLine(oldScreenX, screenY + dy, gapWidth, bgColor);
      }
    } else if (logs[i].speed < 0 && newScreenX < oldScreenX) {
      int width = logs[i].w * CELL_SIZE;
      int gapWidth = oldScreenX - newScreenX;
      for (int dy = 0; dy < height; dy++) {
        int gridY = GRID_HEIGHT - logs[i].lane - logs[i].h + (dy / CELL_SIZE);
        uint16_t bgColor = getZoneColor(gridY, true);
        lcd.drawFastHLine(oldScreenX + width - gapWidth, screenY + dy, gapWidth, bgColor);
      }
    }

    drawLog(i);
  }

  lcd.endWrite();

  // Frosch auf Baumstamm mitbewegen
  if (frog.alive && !frog.moving && (int)frog.y >= 12 && (int)frog.y <= 19) {
    for (int i = 0; i < MAX_LOGS; i++) {
      if (!logs[i].active) continue;

      if ((int)frog.y < logs[i].lane || (int)frog.y >= logs[i].lane + logs[i].h) continue;

      if (frog.x >= logs[i].x && frog.x < logs[i].x + logs[i].w) {
        frog.x += logs[i].speed;
        frog.animX += logs[i].speed;
        frog.x = constrain(frog.x, 0, GRID_WIDTH - 1);
        frog.animX = constrain(frog.animX, 0, GRID_WIDTH - 1);
        break;
      }
    }
  }
}

void updateParticles() {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (!particles[i].active) continue;

    // Alte Position löschen
    int gridY = ((int)particles[i].y - PLAY_OFFSET_Y) / CELL_SIZE;
    gridY = GRID_HEIGHT - gridY - 1;
    uint16_t bgColor = getZoneColor(gridY, false);
    lcd.fillCircle((int)particles[i].x, (int)particles[i].y, 1, bgColor);

    // Bewegen
    particles[i].x += particles[i].vx;
    particles[i].y += particles[i].vy;
    particles[i].vy += 0.3;  // Gravitation
    particles[i].life -= 8;

    // Deaktivieren wenn tot oder außerhalb
    if (particles[i].life <= 0 ||
        particles[i].y > SCREEN_HEIGHT ||
        particles[i].x < 0 || particles[i].x > SCREEN_WIDTH) {
      particles[i].active = false;
      continue;
    }

    // Neue Position zeichnen
    lcd.fillCircle((int)particles[i].x, (int)particles[i].y, 1, particles[i].color);
  }
}

void spawnSplash(int screenX, int screenY) {
  // 5-8 Partikel spawnen
  int numParticles = random(5, 9);

  for (int i = 0; i < numParticles; i++) {
    // Freies Partikel finden
    for (int p = 0; p < MAX_PARTICLES; p++) {
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
        if (colorChoice == 0) particles[p].color = COLOR_SPLASH1;
        else if (colorChoice == 1) particles[p].color = COLOR_SPLASH2;
        else particles[p].color = COLOR_SPLASH3;

        particles[p].life = 100;
        particles[p].active = true;
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
    for (int i = 0; i < MAX_CARS; i++) {
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

    for (int i = 0; i < MAX_LOGS; i++) {
      if (!logs[i].active) continue;

      if ((int)frog.y < logs[i].lane || (int)frog.y >= logs[i].lane + logs[i].h) continue;

      if (frog.x >= logs[i].x && frog.x < logs[i].x + logs[i].w) {
        onLog = true;
        break;
      }
    }

    if (!onLog) {
      // Splash Effekt beim Ertrinken
      int screenX = (int)frog.animX * CELL_SIZE + CELL_SIZE / 2;
      int screenY = PLAY_OFFSET_Y + (GRID_HEIGHT - (int)frog.animY - 1) * CELL_SIZE + CELL_SIZE / 2;
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
    for (int i = 0; i < NUM_HOMES; i++) {
      if (abs((int)frog.x - homes[i].x) <= 1 && !homes[i].filled) {
        homes[i].filled = true;
        score += 50;
        drawHomes();
        drawUI();

        // Frosch zurücksetzen
        frog.x = GRID_WIDTH / 2.0;
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
    frog.x = GRID_WIDTH / 2.0;
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
  for (int i = 0; i < NUM_HOMES; i++) {
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
  lcd.fillRect(SCREEN_WIDTH/4 - 10, SCREEN_HEIGHT/2 - 45,
               SCREEN_WIDTH/2 + 20, 90, boxColor);
  lcd.drawRect(SCREEN_WIDTH/4 - 10, SCREEN_HEIGHT/2 - 45,
               SCREEN_WIDTH/2 + 20, 90, COLOR_TEXT);

  lcd.setTextSize(3);
  lcd.setTextColor(COLOR_TEXT, boxColor);
  lcd.setCursor(SCREEN_WIDTH/2 - 75, SCREEN_HEIGHT/2 - 30);
  lcd.println("GAME OVER");

  lcd.setTextSize(2);
  lcd.setCursor(SCREEN_WIDTH/2 - 55, SCREEN_HEIGHT/2 + 5);
  lcd.printf("Score: %d", score);

  lcd.setTextSize(1);
  lcd.setCursor(SCREEN_WIDTH/2 - 55, SCREEN_HEIGHT/2 + 25);
  lcd.println("Press UP to restart");
}

// ===== ZEICHNEN =====

void drawBackground() {
  // Zone für Zone mit Gradienten zeichnen
  // Grasbereiche: Y=0-1, Y=10-11, Y=22-23
  // Straße: Y=2-9
  // Fluss: Y=12-19

  for (int gridY = 0; gridY < GRID_HEIGHT - PLAY_OFFSET_Y/CELL_SIZE; gridY++) {
    int screenY = PLAY_OFFSET_Y + gridY * CELL_SIZE;
    uint16_t color = getZoneColor(gridY, true);

    for (int dy = 0; dy < CELL_SIZE; dy++) {
      lcd.drawFastHLine(0, screenY + dy, SCREEN_WIDTH, color);
    }
  }
}

void drawWaterRipples() {
  // Zeichne animierte Ripples im Wasser (Y=12-19)
  lcd.startWrite();

  for (int gridY = 12; gridY <= 19; gridY++) {
    int screenY = PLAY_OFFSET_Y + (GRID_HEIGHT - gridY - 1) * CELL_SIZE;

    for (int x = 0; x < SCREEN_WIDTH; x += 8) {
      int offsetX = (x + waterFrame * 2) % 8;
      if (offsetX < 2) {
        uint16_t rippleColor = lerpColor(getZoneColor(gridY, true), COLOR_WATER_LIGHT, 0.3);
        lcd.drawPixel(x + offsetX, screenY + 2 + (gridY % 2), rippleColor);
      }
    }
  }

  lcd.endWrite();
}

void drawCars() {
  for (int i = 0; i < MAX_CARS; i++) {
    if (cars[i].active) {
      drawCar(i);
    }
  }
}

void drawCar(int index) {
  Car* c = &cars[index];
  int screenX = (int)c->x * CELL_SIZE;
  int screenY = PLAY_OFFSET_Y + (GRID_HEIGHT - c->lane - c->h) * CELL_SIZE;
  int width = c->w * CELL_SIZE;
  int height = c->h * CELL_SIZE;
  bool movingRight = (c->speed > 0);

  // Schatten (links/rechts versetzt)
  int shadowOffset = movingRight ? 1 : -1;
  lcd.fillRect(screenX + shadowOffset, screenY + height - 2, width, 2, COLOR_CAR_SHADOW);

  // Auto-Karosserie
  lcd.fillRect(screenX + 1, screenY + 2, width - 2, height - 4, c->color);

  // Abgerundete Ecken vorne
  if (movingRight) {
    lcd.fillRect(screenX + width - 4, screenY + 1, 2, 1, c->color);
    lcd.fillRect(screenX + width - 4, screenY + height - 2, 2, 1, c->color);
  } else {
    lcd.fillRect(screenX + 2, screenY + 1, 2, 1, c->color);
    lcd.fillRect(screenX + 2, screenY + height - 2, 2, 1, c->color);
  }

  // Räder
  lcd.fillCircle(screenX + 5, screenY + height - 3, 2, COLOR_BG);
  if (width >= 20) {
    lcd.fillCircle(screenX + width - 6, screenY + height - 3, 2, COLOR_BG);
  }

  // Scheinwerfer mit Glow-Effekt
  if (width >= 15) {
    if (movingRight) {
      // Glow
      lcd.fillRect(screenX + width - 4, screenY + 3, 3, 4, COLOR_HEADLIGHT_G);
      lcd.fillRect(screenX + width - 4, screenY + height - 7, 3, 4, COLOR_HEADLIGHT_G);
      // Core
      lcd.fillRect(screenX + width - 3, screenY + 4, 2, 3, COLOR_HEADLIGHT);
      lcd.fillRect(screenX + width - 3, screenY + height - 7, 2, 3, COLOR_HEADLIGHT);
    } else {
      // Glow
      lcd.fillRect(screenX + 1, screenY + 3, 3, 4, COLOR_HEADLIGHT_G);
      lcd.fillRect(screenX + 1, screenY + height - 7, 3, 4, COLOR_HEADLIGHT_G);
      // Core
      lcd.fillRect(screenX + 1, screenY + 4, 2, 3, COLOR_HEADLIGHT);
      lcd.fillRect(screenX + 1, screenY + height - 7, 2, 3, COLOR_HEADLIGHT);
    }
  }
}

void drawLogs() {
  for (int i = 0; i < MAX_LOGS; i++) {
    if (logs[i].active) {
      drawLog(i);
    }
  }
}

void drawLog(int index) {
  Log* l = &logs[index];
  int screenX = (int)l->x * CELL_SIZE;
  int screenY = PLAY_OFFSET_Y + (GRID_HEIGHT - l->lane - l->h) * CELL_SIZE;
  int width = l->w * CELL_SIZE;
  int height = l->h * CELL_SIZE;

  // Schatten
  lcd.fillRect(screenX + 1, screenY + height - 1, width - 1, 2, COLOR_LOG_SHADOW);

  // Baumstamm-Grundfarbe
  lcd.fillRect(screenX, screenY, width, height, COLOR_LOG);

  // Holztextur - Jahresringe (vertikal)
  for (int i = 0; i < width; i += 8) {
    lcd.drawFastVLine(screenX + i, screenY + 1, height - 2, COLOR_LOG_DARK);
    if (i + 1 < width) {
      lcd.drawFastVLine(screenX + i + 1, screenY + 2, height - 4, 0x9B60);
    }
  }

  // Rand oben/unten dunkler
  lcd.drawFastHLine(screenX, screenY, width, 0x5280);
  lcd.drawFastHLine(screenX, screenY + height - 1, width, 0x5280);

  // Holzmaserung (horizontal)
  for (int i = 0; i < height; i += 4) {
    if (i > 0 && i < height - 1) {
      for (int x = 0; x < width; x += 3) {
        lcd.drawPixel(screenX + x, screenY + i, COLOR_LOG_DARK);
      }
    }
  }
}

void drawHomes() {
  for (int i = 0; i < NUM_HOMES; i++) {
    int screenX = homes[i].x * CELL_SIZE;
    int screenY = PLAY_OFFSET_Y;

    if (homes[i].filled) {
      // Gefüllt - Gelb mit Glow
      lcd.fillRect(screenX - CELL_SIZE - 1, screenY - 1,
                   CELL_SIZE * 2 + 2, CELL_SIZE * 2 + 2, 0x8C20);
      lcd.fillRect(screenX - CELL_SIZE, screenY,
                   CELL_SIZE * 2, CELL_SIZE * 2, COLOR_HOME);
    } else {
      // Leer - Nur Rahmen
      lcd.drawRect(screenX - CELL_SIZE, screenY,
                   CELL_SIZE * 2, CELL_SIZE * 2, COLOR_TEXT);
    }
  }
}

void drawFrog() {
  if (!frog.alive) return;

  // Blinken wenn invulnerabel
  if (frog.invulnerable > 0 && (frog.invulnerable / 5) % 2 == 0) {
    return;
  }

  int screenX = (int)frog.animX * CELL_SIZE;
  int screenY = PLAY_OFFSET_Y + (GRID_HEIGHT - (int)frog.animY - 1) * CELL_SIZE;

  // Schatten
  lcd.fillCircle(screenX + CELL_SIZE/2 + 1, screenY + CELL_SIZE - 2,
                 CELL_SIZE/2 - 2, COLOR_FROG_SHADOW);

  // Frosch-Körper
  lcd.fillCircle(screenX + CELL_SIZE/2, screenY + CELL_SIZE/2,
                 CELL_SIZE/2 - 1, COLOR_FROG);

  // Augen (größer und moderner)
  lcd.fillCircle(screenX + 3, screenY + 3, 2, COLOR_TEXT);
  lcd.fillCircle(screenX + CELL_SIZE - 3, screenY + 3, 2, COLOR_TEXT);
  lcd.fillCircle(screenX + 3, screenY + 3, 1, COLOR_BG);
  lcd.fillCircle(screenX + CELL_SIZE - 3, screenY + 3, 1, COLOR_BG);

  // Beine (mit Highlight)
  lcd.fillRect(screenX + 1, screenY + CELL_SIZE - 3, 2, 2, 0x05E0);
  lcd.fillRect(screenX + CELL_SIZE - 3, screenY + CELL_SIZE - 3, 2, 2, 0x05E0);
  lcd.drawPixel(screenX + 1, screenY + CELL_SIZE - 3, 0x07F0);
  lcd.drawPixel(screenX + CELL_SIZE - 3, screenY + CELL_SIZE - 3, 0x07F0);
}

void eraseFrog() {
  if (oldFrogX < 0 || oldFrogY < 0) return;

  int screenX = (int)oldFrogX * CELL_SIZE;
  int screenY = PLAY_OFFSET_Y + (GRID_HEIGHT - (int)oldFrogY - 1) * CELL_SIZE;

  // Hintergrundfarbe abhängig von Zone (mit Gradient)
  for (int dy = 0; dy < CELL_SIZE; dy++) {
    int gridY = GRID_HEIGHT - (int)oldFrogY - 1;
    uint16_t bgColor = getZoneColor(gridY, true);
    lcd.drawFastHLine(screenX, screenY + dy, CELL_SIZE, bgColor);
  }
}

void drawUI() {
  // UI-Bereich löschen
  lcd.fillRect(0, 0, SCREEN_WIDTH, 8, COLOR_BG);

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT, COLOR_BG);

  // Score (links)
  lcd.setCursor(2, 0);
  lcd.printf("SCORE:%d", score);

  // Level (Mitte)
  lcd.setCursor(SCREEN_WIDTH/2 - 20, 0);
  lcd.printf("LV:%d", level);

  // Lives (rechts)
  lcd.setCursor(SCREEN_WIDTH - 50, 0);
  lcd.printf("LIVES:%d", lives);
}

void drawGame() {
  eraseFrog();
  drawFrog();

  oldFrogX = frog.animX;
  oldFrogY = frog.animY;
}
