/*
  Tetris - Klassisches Puzzle-Spiel

  Hardware:
  - CYD Display (240x320 im Portrait-Modus!)
  - 4x Digitale Buttons:
    * GPIO 5:  Links
    * GPIO 16: Rechts
    * GPIO 17: Drehen
    * GPIO 18: Runter (Schnell-Fall)

  Steuerung:
  - Links/Rechts: Stein bewegen
  - Drehen: Stein rotieren
  - Runter: Schneller fallen lassen

  Spielregeln:
  - Fülle horizontale Reihen um Punkte zu bekommen
  - Je mehr Reihen gleichzeitig, desto mehr Punkte
  - Spiel endet wenn Steine oben ankommen
  - Level steigt alle 10 Reihen (schneller)
*/

#include <CYD_Display_Config.h>

// Pin-Mapping: Physische Pins -> Logische Namen (Portrait-Modus!)
// Im Portrait-Modus (setRotation(0)) sind die Tasten um 90° gedreht:
// - tasteA (war oben) → jetzt links
// - tasteB (war links) → jetzt unten
// - tasteC (war rechts) → jetzt oben (Rotate)
// - tasteD (war unten) → jetzt rechts
#define BTN_LEFT   tasteA
#define BTN_DOWN   tasteB
#define BTN_RIGHT  tasteD
#define BTN_ROTATE tasteC

// Display Objekt
LGFX lcd;

// Display-Konfiguration (PORTRAIT!)
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

// Spielfeld-Dimensionen
#define GRID_WIDTH  10
#define GRID_HEIGHT 20
#define BLOCK_SIZE  12

// Spielfeld-Position auf dem Screen
#define FIELD_X 20
#define FIELD_Y 40

// Farben
#define COLOR_BG        0x0000  // Schwarz
#define COLOR_GRID      0x2104  // Dunkelgrau
#define COLOR_TEXT      0xFFFF  // Weiß

// Tetromino-Farben (klassisch)
#define COLOR_I 0x07FF  // Cyan
#define COLOR_O 0xFFE0  // Gelb
#define COLOR_T 0xF81F  // Magenta
#define COLOR_S 0x07E0  // Grün
#define COLOR_Z 0xF800  // Rot
#define COLOR_J 0x001F  // Blau
#define COLOR_L 0xFD20  // Orange

// Tetromino-Typen
enum TetrominoType { I, O, T, S, Z, J, L };

// Tetromino-Shapes (4x4 Grid für jedes Shape, 4 Rotationen)
const bool SHAPES[7][4][4][4] = {
  // I
  {
    {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
    {{0,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,1,0,0}},
    {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
    {{0,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,1,0,0}}
  },
  // O
  {
    {{0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0}}
  },
  // T
  {
    {{0,0,0,0}, {0,1,0,0}, {1,1,1,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,1,0,0}, {0,1,1,0}, {0,1,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {1,1,1,0}, {0,1,0,0}},
    {{0,0,0,0}, {0,1,0,0}, {1,1,0,0}, {0,1,0,0}}
  },
  // S
  {
    {{0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,1,0,0}, {0,1,1,0}, {0,0,1,0}},
    {{0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,1,0,0}, {0,1,1,0}, {0,0,1,0}}
  },
  // Z
  {
    {{0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,0,1,0}, {0,1,1,0}, {0,1,0,0}},
    {{0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,0,1,0}, {0,1,1,0}, {0,1,0,0}}
  },
  // J
  {
    {{0,0,0,0}, {1,0,0,0}, {1,1,1,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,1,1,0}, {0,1,0,0}, {0,1,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {1,1,1,0}, {0,0,1,0}},
    {{0,0,0,0}, {0,1,0,0}, {0,1,0,0}, {1,1,0,0}}
  },
  // L
  {
    {{0,0,0,0}, {0,0,1,0}, {1,1,1,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,1,0,0}, {0,1,0,0}, {0,1,1,0}},
    {{0,0,0,0}, {0,0,0,0}, {1,1,1,0}, {1,0,0,0}},
    {{0,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,1,0,0}}
  }
};

const uint16_t COLORS[7] = {
  COLOR_I, COLOR_O, COLOR_T, COLOR_S, COLOR_Z, COLOR_J, COLOR_L
};

// Aktueller Tetromino
struct Tetromino {
  TetrominoType type;
  int x, y;
  int rotation;
  uint16_t color;
};

// Spielfeld
uint16_t grid[GRID_HEIGHT][GRID_WIDTH];

// Game State
Tetromino currentPiece;
Tetromino nextPiece;
int score = 0;
int lines = 0;
int level = 1;
bool gameOver = false;

// Timing
unsigned long lastFallTime = 0;
unsigned long fallDelay = 1000;  // ms
unsigned long lastInputTime = 0;
#define INPUT_DELAY 150

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== TETRIS ===\n");

  // Display initialisieren (PORTRAIT!)
  lcd.init();
  lcd.setRotation(0);  // Portrait-Modus!
  lcd.fillScreen(COLOR_BG);
  lcd.setBrightness(255);

  // Button Inputs konfigurieren
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_ROTATE, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);

  // Zufallsgenerator
  randomSeed(analogRead(35) + micros());

  // Spiel initialisieren
  initGame();

  Serial.println("Tetris gestartet!");
}

void loop() {
  if (gameOver) {
    // Warte auf Neustart
    if (digitalRead(BTN_ROTATE) == LOW) {
      initGame();
      gameOver = false;
      delay(300);
    }
    return;
  }

  // Input
  handleInput();

  // Auto-Fall
  unsigned long now = millis();
  if (now - lastFallTime > fallDelay) {
    if (!movePiece(0, 1)) {
      // Kann nicht weiter fallen - festsetzen
      lockPiece();
      clearLines();
      spawnPiece();

      // Prüfe ob Game Over
      if (checkCollision(currentPiece.x, currentPiece.y, currentPiece.rotation)) {
        handleGameOver();
      }
    }
    lastFallTime = now;
  }

  delay(16);  // ~60 FPS
}

// ===== INITIALISIERUNG =====

void initGame() {
  Serial.println("Initialisiere Spiel...");

  score = 0;
  lines = 0;
  level = 1;
  gameOver = false;
  fallDelay = 1000;

  // Spielfeld leeren
  for (int y = 0; y < GRID_HEIGHT; y++) {
    for (int x = 0; x < GRID_WIDTH; x++) {
      grid[y][x] = COLOR_BG;
    }
  }

  // UI zeichnen
  drawBackground();
  drawUI();
  drawGrid();

  // Nächstes Stück vorbereiten
  nextPiece = createRandomPiece();

  // Erstes Stück spawnen
  spawnPiece();
}

Tetromino createRandomPiece() {
  Tetromino piece;
  piece.type = (TetrominoType)random(0, 7);
  piece.x = GRID_WIDTH / 2 - 2;
  piece.y = 0;
  piece.rotation = 0;
  piece.color = COLORS[piece.type];
  return piece;
}

void spawnPiece() {
  currentPiece = nextPiece;
  nextPiece = createRandomPiece();

  drawPiece(currentPiece, true);
  drawNextPiece();
}

// ===== INPUT =====

void handleInput() {
  unsigned long now = millis();
  if (now - lastInputTime < INPUT_DELAY) return;

  bool moved = false;

  if (digitalRead(BTN_LEFT) == LOW) {
    moved = movePiece(-1, 0);
  } else if (digitalRead(BTN_RIGHT) == LOW) {
    moved = movePiece(1, 0);
  } else if (digitalRead(BTN_DOWN) == LOW) {
    moved = movePiece(0, 1);
    if (moved) score += 1;  // Bonus für schnelles Fallen
  } else if (digitalRead(BTN_ROTATE) == LOW) {
    moved = rotatePiece();
  }

  if (moved) {
    lastInputTime = now;
  }
}

bool movePiece(int dx, int dy) {
  // Alte Position löschen
  drawPiece(currentPiece, false);

  // Neue Position prüfen
  int newX = currentPiece.x + dx;
  int newY = currentPiece.y + dy;

  if (!checkCollision(newX, newY, currentPiece.rotation)) {
    currentPiece.x = newX;
    currentPiece.y = newY;
    drawPiece(currentPiece, true);
    return true;
  } else {
    // Kollision - alte Position wieder zeichnen
    drawPiece(currentPiece, true);
    return false;
  }
}

bool rotatePiece() {
  // Alte Position löschen
  drawPiece(currentPiece, false);

  // Neue Rotation prüfen
  int newRotation = (currentPiece.rotation + 1) % 4;

  if (!checkCollision(currentPiece.x, currentPiece.y, newRotation)) {
    currentPiece.rotation = newRotation;
    drawPiece(currentPiece, true);
    return true;
  } else {
    // Kollision - alte Position wieder zeichnen
    drawPiece(currentPiece, true);
    return false;
  }
}

// ===== KOLLISION =====

bool checkCollision(int x, int y, int rotation) {
  const bool (*shape)[4] = SHAPES[currentPiece.type][rotation];

  for (int py = 0; py < 4; py++) {
    for (int px = 0; px < 4; px++) {
      if (shape[py][px]) {
        int gridX = x + px;
        int gridY = y + py;

        // Außerhalb?
        if (gridX < 0 || gridX >= GRID_WIDTH || gridY >= GRID_HEIGHT) {
          return true;
        }

        // Kollision mit bestehendem Block?
        if (gridY >= 0 && grid[gridY][gridX] != COLOR_BG) {
          return true;
        }
      }
    }
  }

  return false;
}

// ===== PIECE MANAGEMENT =====

void lockPiece() {
  const bool (*shape)[4] = SHAPES[currentPiece.type][currentPiece.rotation];

  for (int py = 0; py < 4; py++) {
    for (int px = 0; px < 4; px++) {
      if (shape[py][px]) {
        int gridX = currentPiece.x + px;
        int gridY = currentPiece.y + py;

        if (gridY >= 0 && gridY < GRID_HEIGHT && gridX >= 0 && gridX < GRID_WIDTH) {
          grid[gridY][gridX] = currentPiece.color;
        }
      }
    }
  }
}

void clearLines() {
  int linesCleared = 0;

  for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
    bool fullLine = true;

    for (int x = 0; x < GRID_WIDTH; x++) {
      if (grid[y][x] == COLOR_BG) {
        fullLine = false;
        break;
      }
    }

    if (fullLine) {
      linesCleared++;

      // Zeile löschen (alle Zeilen darüber nach unten)
      for (int yy = y; yy > 0; yy--) {
        for (int x = 0; x < GRID_WIDTH; x++) {
          grid[yy][x] = grid[yy - 1][x];
        }
      }

      // Oberste Zeile löschen
      for (int x = 0; x < GRID_WIDTH; x++) {
        grid[0][x] = COLOR_BG;
      }

      y++;  // Nochmal prüfen (Zeile ist nach unten gerutscht)
    }
  }

  if (linesCleared > 0) {
    // Score berechnen
    int points[] = {0, 40, 100, 300, 1200};
    score += points[linesCleared] * level;
    lines += linesCleared;

    // Level erhöhen
    level = (lines / 10) + 1;
    fallDelay = max(100, 1000 - (level - 1) * 100);

    // UI updaten
    drawUI();
    drawGrid();  // Komplettes Grid neu zeichnen

    Serial.printf("Lines: %d, Score: %d, Level: %d\n", lines, score, level);
  }
}

void handleGameOver() {
  gameOver = true;
  Serial.println("\n=== GAME OVER ===");
  Serial.printf("Score: %d\n", score);
  Serial.printf("Lines: %d\n", lines);
  Serial.printf("Level: %d\n", level);

  // Game Over Screen
  lcd.fillRect(20, SCREEN_HEIGHT/2 - 50, SCREEN_WIDTH - 40, 100, 0x2104);
  lcd.drawRect(20, SCREEN_HEIGHT/2 - 50, SCREEN_WIDTH - 40, 100, COLOR_TEXT);

  lcd.setTextSize(3);
  lcd.setTextColor(COLOR_TEXT, 0x2104);
  lcd.setCursor(40, SCREEN_HEIGHT/2 - 30);
  lcd.println("GAME OVER");

  lcd.setTextSize(2);
  lcd.setCursor(50, SCREEN_HEIGHT/2 + 5);
  lcd.printf("Score: %d", score);

  lcd.setTextSize(1);
  lcd.setCursor(35, SCREEN_HEIGHT/2 + 30);
  lcd.println("Press ROTATE to restart");
}

// ===== ZEICHNEN =====

void drawBackground() {
  lcd.fillScreen(COLOR_BG);

  // Titel
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT, COLOR_BG);
  lcd.setCursor(60, 5);
  lcd.println("TETRIS");

  // Rahmen um Spielfeld
  lcd.drawRect(FIELD_X - 2, FIELD_Y - 2, GRID_WIDTH * BLOCK_SIZE + 4, GRID_HEIGHT * BLOCK_SIZE + 4, COLOR_TEXT);
}

void drawUI() {
  // UI rechts neben dem Spielfeld
  int uiX = FIELD_X + GRID_WIDTH * BLOCK_SIZE + 15;

  lcd.fillRect(uiX, 50, 70, 150, COLOR_BG);

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT, COLOR_BG);

  // Score
  lcd.setCursor(uiX, 50);
  lcd.println("SCORE");
  lcd.setCursor(uiX, 65);
  lcd.printf("%d", score);

  // Lines
  lcd.setCursor(uiX, 90);
  lcd.println("LINES");
  lcd.setCursor(uiX, 105);
  lcd.printf("%d", lines);

  // Level
  lcd.setCursor(uiX, 130);
  lcd.println("LEVEL");
  lcd.setCursor(uiX, 145);
  lcd.printf("%d", level);

  // Next Label
  lcd.setCursor(uiX, 170);
  lcd.println("NEXT:");
}

void drawGrid() {
  for (int y = 0; y < GRID_HEIGHT; y++) {
    for (int x = 0; x < GRID_WIDTH; x++) {
      drawBlock(x, y, grid[y][x]);
    }
  }
}

void drawBlock(int x, int y, uint16_t color) {
  int screenX = FIELD_X + x * BLOCK_SIZE;
  int screenY = FIELD_Y + y * BLOCK_SIZE;

  if (color == COLOR_BG) {
    // Leerer Block - Grid-Linie
    lcd.fillRect(screenX, screenY, BLOCK_SIZE, BLOCK_SIZE, COLOR_BG);
    lcd.drawRect(screenX, screenY, BLOCK_SIZE, BLOCK_SIZE, COLOR_GRID);
  } else {
    // Gefüllter Block mit 3D-Effekt
    lcd.fillRect(screenX, screenY, BLOCK_SIZE, BLOCK_SIZE, color);

    // Hellerer Rand oben/links
    uint16_t lightColor = color + 0x2104;
    lcd.drawFastHLine(screenX, screenY, BLOCK_SIZE, lightColor);
    lcd.drawFastVLine(screenX, screenY, BLOCK_SIZE, lightColor);

    // Dunklerer Rand unten/rechts
    uint16_t darkColor = color - 0x2104;
    lcd.drawFastHLine(screenX, screenY + BLOCK_SIZE - 1, BLOCK_SIZE, darkColor);
    lcd.drawFastVLine(screenX + BLOCK_SIZE - 1, screenY, BLOCK_SIZE, darkColor);
  }
}

void drawPiece(Tetromino piece, bool show) {
  const bool (*shape)[4] = SHAPES[piece.type][piece.rotation];

  for (int py = 0; py < 4; py++) {
    for (int px = 0; px < 4; px++) {
      if (shape[py][px]) {
        int gridX = piece.x + px;
        int gridY = piece.y + py;

        if (gridY >= 0 && gridY < GRID_HEIGHT && gridX >= 0 && gridX < GRID_WIDTH) {
          drawBlock(gridX, gridY, show ? piece.color : COLOR_BG);
        }
      }
    }
  }
}

void drawNextPiece() {
  int uiX = FIELD_X + GRID_WIDTH * BLOCK_SIZE + 15;
  int nextY = 190;

  // Bereich löschen
  lcd.fillRect(uiX, nextY, 50, 50, COLOR_BG);

  // Nächstes Stück zeichnen
  const bool (*shape)[4] = SHAPES[nextPiece.type][0];

  for (int py = 0; py < 4; py++) {
    for (int px = 0; px < 4; px++) {
      if (shape[py][px]) {
        int screenX = uiX + px * 10;
        int screenY = nextY + py * 10;

        lcd.fillRect(screenX, screenY, 9, 9, nextPiece.color);
      }
    }
  }
}
