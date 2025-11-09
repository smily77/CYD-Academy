/*
  Frogger - Klassisches Arcade-Spiel

  Hardware:
  - CYD Display (320x240)
  - 4x Digitale Buttons:
    * GPIO 5:  Links
    * GPIO 16: Rechts
    * GPIO 17: Oben
    * GPIO 18: Unten

  Steuerung:
  - Tasten: Bewege Frosch hoch/runter/links/rechts

  Spielregeln:
  - Bringe den Frosch sicher nach oben zu den Häusern
  - Straße (Y=2-9): Vermeide Autos (4 BREITE Spuren, je 2 Reihen hoch!)
    * Y=2-3, 4-5, 6-7, 8-9: Je 2 Autos pro Spur
    * Autos haben schöne Details: Fenster, Räder, Scheinwerfer
  - Erholungsspuren (Y=10-11): Sichere Zone zum Ausruhen
  - Fluss (Y=12-19): Springe auf Baumstämme! (4 BREITE Spuren, je 2 Reihen hoch!)
    * Y=12-13, 14-15, 16-17, 18-19: Je 2 Baumstämme pro Spur (nicht zu viele!)
    * Baumstämme sind 5-7 Zellen lang (gut springbar)
  - WICHTIG: Frosch ERTRINKT im Wasser, Baumstämme retten und tragen dich!
  - Objekte sind BREIT (2 Reihen hoch) - viel einfacher zu treffen!
  - Erreiche alle 5 Häuser für nächstes Level
  - 3 Leben
  - Punkte: Vorwärts bewegen = +10, Haus erreichen = +50
*/

#include <CYD_Display_Config.h>

// Pin-Mapping: Physische Pins -> Logische Namen
#define BTN_LEFT   tasteB
#define BTN_UP     tasteA
#define BTN_RIGHT  tasteC
#define BTN_DOWN   tasteD

// Display Objekt
LGFX lcd;

// Spielfeld-Dimensionen
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// Grid-Konfiguration
#define CELL_SIZE 10
#define GRID_WIDTH  (SCREEN_WIDTH / CELL_SIZE)   // 32
#define GRID_HEIGHT (SCREEN_HEIGHT / CELL_SIZE)  // 24
#define PLAY_OFFSET_Y 8  // UI oben, Spielfeld startet bei Y=8

// Farben
#define COLOR_BG        0x0000  // Schwarz
#define COLOR_ROAD      0x4208  // Dunkelgrau
#define COLOR_GRASS     0x2444  // Grün
#define COLOR_WATER     0x001F  // Blau
#define COLOR_FROG      0x07E0  // Grün
#define COLOR_CAR_RED   0xF800  // Rot
#define COLOR_CAR_BLUE  0x001F  // Blau
#define COLOR_LOG       0xD69A  // Hellbraun
#define COLOR_TURTLE    0x07E0  // Grün
#define COLOR_HOME      0xFFE0  // Gelb
#define COLOR_TEXT      0xFFFF  // Weiß

// Spieler (Frosch)
struct Frog {
  float x, y;            // Grid-Position (float für smooth movement auf Baumstämmen)
  int maxY;              // Weiteste Y-Position erreicht
  bool alive;
  int invulnerable;      // Frames invulnerabel
};

// Auto
struct Car {
  int lane;              // Fahrspur (Y=2, 4, 6, 8 - Basis-Position)
  float x;               // Position
  int w;                 // Breite in Zellen (X-Richtung = Länge)
  int h;                 // Höhe in Zellen (Y-Richtung = BREITE, wie gewünscht!)
  float speed;           // Geschwindigkeit (Zellen pro Frame)
  uint16_t color;        // Farbe (rot oder blau)
  bool active;
};

// Baumstamm (WICHTIG: Baumstämme RETTEN den Frosch und bewegen ihn mit!)
struct Log {
  int lane;              // Reihe (Y=12, 14, 16, 18 - Basis-Position)
  float x;               // Position
  int w;                 // Breite in Zellen (X-Richtung = Länge)
  int h;                 // Höhe in Zellen (Y-Richtung = BREITE!)
  float speed;           // Geschwindigkeit
  bool active;
};

// Häuser (Ziele)
struct Home {
  int x;                 // X-Position
  bool filled;           // Schon erreicht?
};

// Game Objekte
Frog frog;

#define MAX_CARS 8
Car cars[MAX_CARS];

#define MAX_LOGS 8
Log logs[MAX_LOGS];

#define NUM_HOMES 5
Home homes[NUM_HOMES];

// Game State
int score = 0;
int lives = 3;
int level = 1;
bool gameOver = false;

// Alte Position für Redraw
float oldFrogX = -1;
float oldFrogY = -1;

// Input Debouncing
unsigned long lastMoveTime = 0;
#define MOVE_DELAY 150  // ms zwischen Bewegungen

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== FROGGER ===\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);
  lcd.fillScreen(COLOR_BG);
  lcd.setBrightness(255);

  // Button Inputs konfigurieren
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);

  // Zufallsgenerator
  randomSeed(analogRead(35) + micros());

  // Spiel initialisieren
  initGame();

  Serial.println("Frogger gestartet!");
}

void loop() {
  if (gameOver) {
    // Warte auf Neustart
    if (digitalRead(BTN_UP) == LOW) {
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

  // Frosch initialisieren
  frog.x = GRID_WIDTH / 2.0;
  frog.y = 0;  // Startposition unten
  frog.maxY = 0;
  frog.alive = true;
  frog.invulnerable = 0;
  oldFrogX = -1;
  oldFrogY = -1;

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
  frog.maxY = 0;
  frog.alive = true;
  frog.invulnerable = 30;  // 0.5 Sekunden
  oldFrogX = -1;
  oldFrogY = -1;

  // Häuser initialisieren (oben, Y=23)
  int homeSpacing = GRID_WIDTH / (NUM_HOMES + 1);
  for (int i = 0; i < NUM_HOMES; i++) {
    homes[i].x = homeSpacing * (i + 1);
    homes[i].filled = false;
  }

  // Autos initialisieren (4 Spuren, je 2 Reihen hoch: Y=2-3, 4-5, 6-7, 8-9)
  int carCount = 0;
  int carLanes[] = {2, 4, 6, 8};  // Basis-Y für jede Spur
  for (int i = 0; i < 4; i++) {
    int lane = carLanes[i];
    int numCars = 2;  // 2 Autos pro Spur
    bool dirRight = (lane % 4 == 0);  // Abwechselnde Richtungen

    for (int j = 0; j < numCars; j++) {
      if (carCount < MAX_CARS) {
        cars[carCount].lane = lane;
        cars[carCount].x = (GRID_WIDTH / numCars) * j;  // Gleichmäßig verteilt
        cars[carCount].w = 3 + random(0, 2);  // 3-4 Zellen lang
        cars[carCount].h = 2;  // 2 Reihen HOCH (= breit!)
        cars[carCount].speed = (0.15 + level * 0.03) * (dirRight ? 1 : -1);
        cars[carCount].color = (random(0, 2) == 0) ? COLOR_CAR_RED : COLOR_CAR_BLUE;
        cars[carCount].active = true;
        carCount++;
      }
    }
  }

  // Baumstämme initialisieren (4 Spuren, je 2 Reihen hoch: Y=12-13, 14-15, 16-17, 18-19)
  // WICHTIG: Baumstämme RETTEN den Frosch und bewegen ihn mit!
  int logCount = 0;
  int logLanes[] = {12, 14, 16, 18};  // Basis-Y für jede Spur
  for (int i = 0; i < 4; i++) {
    int lane = logLanes[i];
    int numLogs = 2;  // 2 Baumstämme pro Spur (reduziert von 3)
    bool dirRight = (lane % 4 == 0);

    for (int j = 0; j < numLogs; j++) {
      if (logCount < MAX_LOGS) {
        logs[logCount].lane = lane;
        logs[logCount].x = (GRID_WIDTH / numLogs) * j;
        logs[logCount].w = 5 + random(0, 3);  // 5-7 Zellen lang (kürzer!)
        logs[logCount].h = 2;  // 2 Reihen HOCH (= breit!)
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
  if (now - lastMoveTime < MOVE_DELAY) return;

  float newX = frog.x;
  float newY = frog.y;
  bool moved = false;

  if (digitalRead(BTN_LEFT) == LOW) {
    newX--;
    moved = true;
  } else if (digitalRead(BTN_RIGHT) == LOW) {
    newX++;
    moved = true;
  } else if (digitalRead(BTN_UP) == LOW) {
    newY++;
    moved = true;
  } else if (digitalRead(BTN_DOWN) == LOW) {
    newY--;
    moved = true;
  }

  if (moved) {
    // Grenzen prüfen
    if (newX >= 0 && newX < GRID_WIDTH && newY >= 0 && newY < GRID_HEIGHT) {
      frog.x = newX;
      frog.y = newY;

      // Punkte für Vorwärtsbewegung
      if ((int)newY > frog.maxY) {
        score += 10;
        frog.maxY = (int)newY;
        drawUI();
      }

      lastMoveTime = now;
      Serial.printf("Frog moved to (%.1f, %.1f)\n", frog.x, frog.y);
    }
  }
}

// ===== UPDATE =====

void updateCars() {
  for (int i = 0; i < MAX_CARS; i++) {
    if (!cars[i].active) continue;

    // Alte Position löschen
    eraseCar(i);

    // Bewegen
    cars[i].x += cars[i].speed;

    // Screen wrapping
    if (cars[i].speed > 0 && cars[i].x >= GRID_WIDTH) {
      cars[i].x = -cars[i].w;
    } else if (cars[i].speed < 0 && cars[i].x + cars[i].w <= 0) {
      cars[i].x = GRID_WIDTH;
    }

    // Neue Position zeichnen
    drawCar(i);
  }
}

void updateLogs() {
  for (int i = 0; i < MAX_LOGS; i++) {
    if (!logs[i].active) continue;

    // Alte Position löschen
    eraseLog(i);

    // Bewegen
    logs[i].x += logs[i].speed;

    // Screen wrapping
    if (logs[i].speed > 0 && logs[i].x >= GRID_WIDTH) {
      logs[i].x = -logs[i].w;
    } else if (logs[i].speed < 0 && logs[i].x + logs[i].w <= 0) {
      logs[i].x = GRID_WIDTH;
    }

    // Neue Position zeichnen
    drawLog(i);
  }

  // Frosch auf Baumstamm mitbewegen (Y=12-19)
  if (frog.alive && (int)frog.y >= 12 && (int)frog.y <= 19) {
    for (int i = 0; i < MAX_LOGS; i++) {
      if (!logs[i].active) continue;

      // Prüfe ob Frosch in Y-Bereich des Baumstamms ist (lane bis lane+h-1)
      if ((int)frog.y < logs[i].lane || (int)frog.y >= logs[i].lane + logs[i].h) continue;

      if (frog.x >= logs[i].x && frog.x < logs[i].x + logs[i].w) {
        // Frosch mitbewegen (WICHTIG: float-Speed verwenden, nicht (int)!)
        frog.x += logs[i].speed;
        frog.x = constrain(frog.x, 0, GRID_WIDTH - 1);
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

  // Straße (Y=2-9): Kollision mit Autos
  if ((int)frog.y >= 2 && (int)frog.y <= 9) {
    for (int i = 0; i < MAX_CARS; i++) {
      if (!cars[i].active) continue;

      // Prüfe ob Frosch in Y-Bereich des Autos ist (lane bis lane+h-1)
      if ((int)frog.y < cars[i].lane || (int)frog.y >= cars[i].lane + cars[i].h) continue;

      if (frog.x >= cars[i].x && frog.x < cars[i].x + cars[i].w) {
        // Von Auto überfahren!
        frogDied();
        return;
      }
    }
  }

  // Fluss (Y=12-19): Frosch ERTRINKT im Wasser, muss auf Baumstamm sein!
  if ((int)frog.y >= 12 && (int)frog.y <= 19) {
    bool onLog = false;

    // Prüfe ob auf Baumstamm
    for (int i = 0; i < MAX_LOGS; i++) {
      if (!logs[i].active) continue;

      // Prüfe ob Frosch in Y-Bereich des Baumstamms ist (lane bis lane+h-1)
      if ((int)frog.y < logs[i].lane || (int)frog.y >= logs[i].lane + logs[i].h) continue;

      if (frog.x >= logs[i].x && frog.x < logs[i].x + logs[i].w) {
        onLog = true;
        break;
      }
    }

    if (!onLog) {
      // Ertrunken im Wasser!
      frogDied();
      return;
    }
  }
}

void checkGoal() {
  if (!frog.alive) return;

  // Ziel-Zeile erreicht? (Y=23)
  if ((int)frog.y == 23) {
    // Ist Frosch in einem Haus?
    bool inHome = false;
    for (int i = 0; i < NUM_HOMES; i++) {
      if (abs((int)frog.x - homes[i].x) <= 1 && !homes[i].filled) {
        // Haus erreicht!
        homes[i].filled = true;
        score += 50;
        Serial.printf("Haus %d erreicht! Score: %d\n", i + 1, score);
        drawHomes();
        drawUI();

        // Frosch zurücksetzen
        frog.x = GRID_WIDTH / 2.0;
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
      // Kein Haus getroffen
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
    frog.maxY = 0;
    frog.alive = true;
    frog.invulnerable = 60;  // 1 Sekunde
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
  lcd.fillRect(SCREEN_WIDTH/4 - 10, SCREEN_HEIGHT/2 - 45,
               SCREEN_WIDTH/2 + 20, 90, 0x2104);
  lcd.drawRect(SCREEN_WIDTH/4 - 10, SCREEN_HEIGHT/2 - 45,
               SCREEN_WIDTH/2 + 20, 90, COLOR_TEXT);

  lcd.setTextSize(3);
  lcd.setTextColor(COLOR_TEXT, 0x2104);
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
  // UI-Bereich oben (8px)
  lcd.fillRect(0, 0, SCREEN_WIDTH, PLAY_OFFSET_Y, COLOR_BG);

  // Spielfeld startet ab Y=PLAY_OFFSET_Y
  // Layout: 1/3 Straße, 1/3 Fluss

  // Ziel-Zone (Y=20-23): Gras - 4 Reihen
  lcd.fillRect(0, PLAY_OFFSET_Y, SCREEN_WIDTH, 4 * CELL_SIZE, COLOR_GRASS);

  // Fluss (Y=12-19): Blau - 8 Reihen = 1/3 der Spielhöhe (80px)
  lcd.fillRect(0, PLAY_OFFSET_Y + 4 * CELL_SIZE, SCREEN_WIDTH, 8 * CELL_SIZE, COLOR_WATER);

  // Erholungsspuren (Y=10-11): Gras - 2 Reihen (sichere Zone)
  lcd.fillRect(0, PLAY_OFFSET_Y + 12 * CELL_SIZE, SCREEN_WIDTH, 2 * CELL_SIZE, COLOR_GRASS);

  // Straße (Y=2-9): Grau - 8 Reihen = 1/3 der Spielhöhe (80px)
  lcd.fillRect(0, PLAY_OFFSET_Y + 14 * CELL_SIZE, SCREEN_WIDTH, 8 * CELL_SIZE, COLOR_ROAD);

  // Startzone (Y=0-1): Gras - 2 Reihen
  lcd.fillRect(0, PLAY_OFFSET_Y + 22 * CELL_SIZE, SCREEN_WIDTH, 2 * CELL_SIZE, COLOR_GRASS);
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
  int screenY = PLAY_OFFSET_Y + (GRID_HEIGHT - c->lane - c->h) * CELL_SIZE;  // Angepasst für Höhe
  int width = c->w * CELL_SIZE;
  int height = c->h * CELL_SIZE;  // Höhe in Pixel

  // Auto-Karosserie (rot oder blau) - Hauptteil
  lcd.fillRect(screenX + 1, screenY + 2, width - 2, height - 4, c->color);

  // Abgerundete Ecken vorne (links bei Rechtsbewegung)
  lcd.fillRect(screenX + 2, screenY + 1, 2, 1, c->color);
  lcd.fillRect(screenX + 2, screenY + height - 2, 2, 1, c->color);

  // Abgerundete Ecken hinten (rechts)
  if (width >= 20) {
    lcd.fillRect(screenX + width - 4, screenY + 1, 2, 1, c->color);
    lcd.fillRect(screenX + width - 4, screenY + height - 2, 2, 1, c->color);
  }

  // Dach/Kabine (dunkler, in der Mitte)
  uint16_t roofColor = (c->color == COLOR_CAR_RED) ? 0x8000 : 0x0010;  // Dunkelrot oder Dunkelblau
  if (width >= 25) {
    int roofStart = screenX + width / 3;
    int roofWidth = width / 3;
    lcd.fillRect(roofStart, screenY + 3, roofWidth, height - 6, roofColor);
  }

  // Fenster (hellblau/cyan für Glas-Effekt)
  if (width >= 25) {
    int windowStart = screenX + width / 3 + 2;
    int windowWidth = width / 3 - 4;
    if (windowWidth > 0) {
      lcd.fillRect(windowStart, screenY + 5, windowWidth, height - 10, 0x7BEF);  // Hellgrau/cyan
    }
  }

  // Räder (schwarz, rund)
  int wheelRadius = 2;
  lcd.fillCircle(screenX + 5, screenY + height - 3, wheelRadius, COLOR_BG);
  if (width >= 20) {
    lcd.fillCircle(screenX + width - 6, screenY + height - 3, wheelRadius, COLOR_BG);
  }

  // Scheinwerfer (gelb, vorne)
  if (width >= 15) {
    lcd.fillRect(screenX + 1, screenY + 4, 2, 3, 0xFFE0);  // Gelb
    lcd.fillRect(screenX + 1, screenY + height - 7, 2, 3, 0xFFE0);
  }
}

void eraseCar(int index) {
  Car* c = &cars[index];
  int screenX = (int)c->x * CELL_SIZE;
  int screenY = PLAY_OFFSET_Y + (GRID_HEIGHT - c->lane - c->h) * CELL_SIZE;  // Angepasst für Höhe
  int width = c->w * CELL_SIZE;
  int height = c->h * CELL_SIZE;  // Höhe in Pixel

  lcd.fillRect(screenX, screenY, width, height, COLOR_ROAD);
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
  int screenY = PLAY_OFFSET_Y + (GRID_HEIGHT - l->lane - l->h) * CELL_SIZE;  // Angepasst für Höhe
  int width = l->w * CELL_SIZE;
  int height = l->h * CELL_SIZE;  // Höhe in Pixel

  // Baumstamm-Grundfarbe - mehrere Reihen hoch!
  lcd.fillRect(screenX, screenY, width, height, COLOR_LOG);

  // Jahresringe/Textur (hellere Streifen)
  for (int i = 0; i < width; i += 8) {
    lcd.drawFastVLine(screenX + i, screenY + 1, height - 2, 0x9B60);  // Helles Braun
  }

  // Rand oben/unten dunkler
  lcd.drawFastHLine(screenX, screenY, width, 0x5280);  // Dunkelbraun
  lcd.drawFastHLine(screenX, screenY + height - 1, width, 0x5280);
}

void eraseLog(int index) {
  Log* l = &logs[index];
  int screenX = (int)l->x * CELL_SIZE;
  int screenY = PLAY_OFFSET_Y + (GRID_HEIGHT - l->lane - l->h) * CELL_SIZE;  // Angepasst für Höhe
  int width = l->w * CELL_SIZE;
  int height = l->h * CELL_SIZE;  // Höhe in Pixel

  lcd.fillRect(screenX, screenY, width, height, COLOR_WATER);
}

void drawHomes() {
  for (int i = 0; i < NUM_HOMES; i++) {
    int screenX = homes[i].x * CELL_SIZE;
    int screenY = PLAY_OFFSET_Y;  // Oben im Spielfeld (Y=23 → 8px + 0)

    if (homes[i].filled) {
      lcd.fillRect(screenX - CELL_SIZE, screenY, CELL_SIZE * 2, CELL_SIZE * 2, COLOR_HOME);
    } else {
      lcd.drawRect(screenX - CELL_SIZE, screenY, CELL_SIZE * 2, CELL_SIZE * 2, COLOR_TEXT);
    }
  }
}

void drawFrog() {
  if (!frog.alive) return;

  // Blinken wenn invulnerabel
  if (frog.invulnerable > 0 && (frog.invulnerable / 5) % 2 == 0) {
    return;
  }

  int screenX = (int)frog.x * CELL_SIZE;
  int screenY = PLAY_OFFSET_Y + (GRID_HEIGHT - (int)frog.y - 1) * CELL_SIZE;

  // Frosch-Körper (runder)
  lcd.fillCircle(screenX + CELL_SIZE/2, screenY + CELL_SIZE/2, CELL_SIZE/2 - 1, COLOR_FROG);

  // Augen (weiß mit schwarzem Punkt)
  lcd.fillCircle(screenX + 3, screenY + 3, 2, COLOR_TEXT);  // Linkes Auge
  lcd.fillCircle(screenX + CELL_SIZE - 3, screenY + 3, 2, COLOR_TEXT);  // Rechtes Auge
  lcd.drawPixel(screenX + 3, screenY + 3, COLOR_BG);  // Pupille links
  lcd.drawPixel(screenX + CELL_SIZE - 3, screenY + 3, COLOR_BG);  // Pupille rechts

  // Beine andeuten (kleine Rechtecke)
  lcd.fillRect(screenX + 1, screenY + CELL_SIZE - 3, 2, 2, 0x05E0);  // Vorne links
  lcd.fillRect(screenX + CELL_SIZE - 3, screenY + CELL_SIZE - 3, 2, 2, 0x05E0);  // Vorne rechts
}

void eraseFrog() {
  if (oldFrogX < 0 || oldFrogY < 0) return;

  int screenX = (int)oldFrogX * CELL_SIZE;
  int screenY = PLAY_OFFSET_Y + (GRID_HEIGHT - (int)oldFrogY - 1) * CELL_SIZE;

  // Hintergrundfarbe abhängig von Zone
  uint16_t bgColor = COLOR_GRASS;
  if ((int)oldFrogY >= 2 && (int)oldFrogY <= 9) bgColor = COLOR_ROAD;  // Straßenzone
  else if ((int)oldFrogY >= 12 && (int)oldFrogY <= 19) bgColor = COLOR_WATER;  // Flusszone

  lcd.fillRect(screenX, screenY, CELL_SIZE, CELL_SIZE, bgColor);
}

void drawUI() {
  // UI-Bereich oben löschen
  lcd.fillRect(0, 0, SCREEN_WIDTH, 8, COLOR_BG);

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT, COLOR_BG);

  // Score (links)
  lcd.setCursor(2, 0);
  lcd.printf("S:%d", score);

  // Level (Mitte)
  lcd.setCursor(SCREEN_WIDTH/2 - 15, 0);
  lcd.printf("L:%d", level);

  // Lives (rechts)
  lcd.setCursor(SCREEN_WIDTH - 40, 0);
  lcd.printf("Lvs:%d", lives);
}

void drawGame() {
  eraseFrog();
  drawFrog();

  oldFrogX = frog.x;
  oldFrogY = frog.y;
}
