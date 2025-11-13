/*
  Tetris Modern - Klassisches Puzzle-Spiel mit modernen Effekten

  Hardware:
  - CYD Display (240x320 im Portrait-Modus!)
  - 4x Digitale Buttons:
    * tasteB:  Drehen (oben)
    * tasteC: Runter (unten, Schnell-Fall)
    * tasteA: Rechts (rechts)
    * tasteD: Links (links)

  Hinweis: Alle Pins sind in CYD_Display_Config.h definiert.

  Steuerung:
  - Links/Rechts: Stein bewegen
  - Drehen: Stein rotieren
  - Runter: Schneller fallen lassen

  Moderne Features:
  - Gradient-Hintergrund (Dunkellila zu Dunkelblau)
  - Glow-Effekte auf aktiven Tetrominos
  - Ghost-Piece Preview (zeigt wo der Stein landet)
  - Partikel-Effekte beim Linien-Löschen
  - 3D-Block-Rendering mit Highlights und Schatten
  - Sanfte Drop-Animationen
  - Rainbow-Farb-Effekte für vollständige Linien vor dem Löschen
  - Moderne UI mit Gradienten
*/

#include <CYD_Display_Config.h>
#include <CYD_Input.h>

// Pin-Mapping: Portrait-Modus
#define BTN_LEFT   tasteD
#define BTN_RIGHT  tasteA
#define BTN_DOWN   tasteC
#define BTN_ROTATE tasteB

// Display Objekt
LGFX lcd;

// Display-Dimensionen (PORTRAIT!)
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

// Spielfeld-Dimensionen
#define GRID_WIDTH  10
#define GRID_HEIGHT 20
#define BLOCK_SIZE  12

// Spielfeld-Position
#define FIELD_X 20
#define FIELD_Y 40

// Farben - Moderne Palette mit Gradienten
#define COLOR_BG_TOP      0x4015  // Dunkles Lila
#define COLOR_BG_BOTTOM   0x0015  // Dunkles Blau
#define COLOR_BG          0x0000  // Schwarz
#define COLOR_GRID        0x2104  // Dunkelgrau
#define COLOR_TEXT        0xFFFF  // Weiß
#define COLOR_GHOST       0x4228  // Grau für Ghost-Piece

// Tetromino-Farben (leuchtender)
#define COLOR_I 0x07FF  // Cyan
#define COLOR_O 0xFFE0  // Gelb
#define COLOR_T 0xF81F  // Magenta
#define COLOR_S 0x07E0  // Grün
#define COLOR_Z 0xF800  // Rot
#define COLOR_J 0x001F  // Blau
#define COLOR_L 0xFD20  // Orange

// Partikel-Farben
#define COLOR_PARTICLE1 0xFFE0  // Gelb
#define COLOR_PARTICLE2 0xFD20  // Orange
#define COLOR_PARTICLE3 0xF800  // Rot
#define COLOR_PARTICLE4 0xF81F  // Magenta
#define COLOR_PARTICLE5 0x07FF  // Cyan

// Tetromino-Typen
enum TetrominoType { I, O, T, S, Z, J, L };

// Tetromino-Shapes
static const bool SHAPES[7][4][4][4] = {
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

static const uint16_t COLORS[7] = {
  COLOR_I, COLOR_O, COLOR_T, COLOR_S, COLOR_Z, COLOR_J, COLOR_L
};

// Strukturen
struct Tetromino {
  TetrominoType type;
  int x, y;
  int rotation;
  uint16_t color;
};

// Partikel für Line-Clear Effekte
struct Particle {
  float x, y;
  float vx, vy;
  uint16_t color;
  uint8_t life;
  bool active;
};

// Game State
uint16_t grid[GRID_HEIGHT][GRID_WIDTH];
Tetromino currentPiece;
Tetromino nextPiece;

int score = 0;
int lines = 0;
int level = 1;
bool gameOver = false;

unsigned long lastFallTime = 0;
unsigned long fallDelay = 1000;
unsigned long lastInputTime = 0;

#define INPUT_DELAY 150

// Partikel
#define MAX_PARTICLES 50
Particle particles[MAX_PARTICLES];

// Flags für Line-Clear Animation
bool lineClearActive = false;
int clearingLines[4];
int clearingLineCount = 0;
unsigned long lineClearStartTime = 0;
#define LINE_CLEAR_DURATION 800

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

// Hintergrundfarbe an Position
uint16_t getBgColor(int y) {
  return lerpColor(COLOR_BG_TOP, COLOR_BG_BOTTOM, (float)y / SCREEN_HEIGHT);
}

// Rainbow-Farbe für Animation (0-255 Hue)
uint16_t rainbowColor(uint8_t hue) {
  uint16_t colors[] = {
    0xF800,  // Rot
    0xFD20,  // Orange
    0xFFE0,  // Gelb
    0x07E0,  // Grün
    0x07FF,  // Cyan
    0x001F,  // Blau
    0xF81F   // Magenta
  };
  return colors[(hue / 36) % 7];
}

// Dunklere Farbe für Schatten
uint16_t darkenColor(uint16_t color, float factor) {
  uint8_t r = (color >> 11) & 0x1F;
  uint8_t g = (color >> 5) & 0x3F;
  uint8_t b = color & 0x1F;

  r = r * factor;
  g = g * factor;
  b = b * factor;

  return (r << 11) | (g << 5) | b;
}

// Hellere Farbe für Highlights
uint16_t brightenColor(uint16_t color, float factor) {
  uint8_t r = (color >> 11) & 0x1F;
  uint8_t g = (color >> 5) & 0x3F;
  uint8_t b = color & 0x1F;

  r = min(31, (int)(r * factor));
  g = min(63, (int)(g * factor));
  b = min(31, (int)(b * factor));

  return (r << 11) | (g << 5) | b;
}

// ===== SETUP & LOOP =====

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== TETRIS MODERN ===\n");

  // Display initialisieren (PORTRAIT!)
  lcd.init();
  lcd.setRotation(0);  // Portrait-Modus!
  lcd.fillScreen(COLOR_BG);
  lcd.setBrightness(255);

  // Input initialisieren
  CYD_Input::init();

  // Zufallsgenerator
  randomSeed(CYD_Input::readPoti(CYD_POTI_RIGHT) + micros());

  // Spiel initialisieren
  initGame();

  Serial.println("Tetris Modern gestartet!");
  Serial.println("Steuerung (Portrait-Modus):");
  Serial.println("  Links:  GPIO 18 (tasteD)");
  Serial.println("  Rechts: GPIO 17 (tasteA)");
  Serial.println("  Runter: GPIO 16 (tasteC)");
  Serial.println("  Drehen: GPIO 5  (tasteB)");
}

void loop() {
  if (gameOver) {
    // Warte auf Neustart
    if (CYD_Input::readButton(CYD_BTN_B)) {
      initGame();
      delay(300);
    }
    return;
  }

  // Line-Clear Animation aktiv?
  if (lineClearActive) {
    updateLineClearAnimation();
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
      checkAndClearLines();
      spawnPiece();

      // Prüfe ob Game Over
      if (checkCollision(currentPiece.x, currentPiece.y, currentPiece.rotation)) {
        handleGameOver();
      }
    }
    lastFallTime = now;
  }

  // Partikel aktualisieren
  updateParticles();

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
  lastFallTime = 0;
  lastInputTime = 0;
  lineClearActive = false;

  // Spielfeld leeren
  for (int y = 0; y < GRID_HEIGHT; y++) {
    for (int x = 0; x < GRID_WIDTH; x++) {
      grid[y][x] = COLOR_BG;
    }
  }

  // Partikel initialisieren
  for (int i = 0; i < MAX_PARTICLES; i++) {
    particles[i].active = false;
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

  if (CYD_Input::readButton(CYD_BTN_D)) {
    moved = movePiece(-1, 0);
  } else if (CYD_Input::readButton(CYD_BTN_A)) {
    moved = movePiece(1, 0);
  } else if (CYD_Input::readButton(CYD_BTN_C)) {
    moved = movePiece(0, 1);
    if (moved) score += 1;
  } else if (CYD_Input::readButton(CYD_BTN_B)) {
    moved = rotatePiece();
  }

  if (moved) {
    lastInputTime = now;
  }
}

bool movePiece(int dx, int dy) {
  // Alte Position löschen (inkl. Ghost)
  eraseCurrentPieceWithGhost();

  // Neue Position prüfen
  int newX = currentPiece.x + dx;
  int newY = currentPiece.y + dy;

  if (!checkCollision(newX, newY, currentPiece.rotation)) {
    currentPiece.x = newX;
    currentPiece.y = newY;
    drawPieceWithGhost();
    return true;
  } else {
    // Kollision - alte Position wieder zeichnen
    drawPieceWithGhost();
    return false;
  }
}

bool rotatePiece() {
  // Alte Position löschen (inkl. Ghost)
  eraseCurrentPieceWithGhost();

  // Neue Rotation prüfen
  int newRotation = (currentPiece.rotation + 1) % 4;

  if (!checkCollision(currentPiece.x, currentPiece.y, newRotation)) {
    currentPiece.rotation = newRotation;
    drawPieceWithGhost();
    return true;
  } else {
    // Kollision - alte Position wieder zeichnen
    drawPieceWithGhost();
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

// Ghost-Piece Position berechnen
int calculateGhostY() {
  int ghostY = currentPiece.y;

  // So weit fallen lassen bis Kollision
  while (!checkCollision(currentPiece.x, ghostY + 1, currentPiece.rotation)) {
    ghostY++;
  }

  return ghostY;
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

void checkAndClearLines() {
  clearingLineCount = 0;

  // Volle Linien finden
  for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
    bool fullLine = true;

    for (int x = 0; x < GRID_WIDTH; x++) {
      if (grid[y][x] == COLOR_BG) {
        fullLine = false;
        break;
      }
    }

    if (fullLine) {
      clearingLines[clearingLineCount++] = y;
    }
  }

  // Wenn Linien gefunden, Animation starten
  if (clearingLineCount > 0) {
    lineClearActive = true;
    lineClearStartTime = millis();
    Serial.printf("Line-Clear Animation gestartet: %d Linien\n", clearingLineCount);
  }
}

void updateLineClearAnimation() {
  unsigned long elapsed = millis() - lineClearStartTime;

  if (elapsed < LINE_CLEAR_DURATION) {
    // Rainbow-Animation auf den Linien
    uint8_t hue = (elapsed / 10) % 256;

    for (int i = 0; i < clearingLineCount; i++) {
      int y = clearingLines[i];
      uint16_t color = rainbowColor(hue + i * 30);

      for (int x = 0; x < GRID_WIDTH; x++) {
        drawBlock(x, y, color);
      }
    }

    delay(16);
  } else {
    // Animation fertig - Linien löschen
    clearLines();
    lineClearActive = false;
  }
}

void clearLines() {
  // Partikel spawnen für jede gelöschte Linie
  for (int i = 0; i < clearingLineCount; i++) {
    int y = clearingLines[i];
    spawnLineClearParticles(y);
  }

  // Linien tatsächlich löschen
  for (int i = 0; i < clearingLineCount; i++) {
    int y = clearingLines[i];

    // Alle Zeilen darüber nach unten
    for (int yy = y; yy > 0; yy--) {
      for (int x = 0; x < GRID_WIDTH; x++) {
        grid[yy][x] = grid[yy - 1][x];
      }
    }

    // Oberste Zeile löschen
    for (int x = 0; x < GRID_WIDTH; x++) {
      grid[0][x] = COLOR_BG;
    }

    // Nächste Linien anpassen (sie sind nach unten gerutscht)
    for (int j = i + 1; j < clearingLineCount; j++) {
      if (clearingLines[j] < y) {
        clearingLines[j]++;
      }
    }
  }

  // Score berechnen
  int points[] = {0, 40, 100, 300, 1200};
  score += points[clearingLineCount] * level;
  lines += clearingLineCount;

  // Level erhöhen
  level = (lines / 10) + 1;
  fallDelay = max(100, 1000 - (level - 1) * 100);

  // UI updaten
  drawUI();
  drawGrid();
  drawPieceWithGhost();

  Serial.printf("Lines: %d, Score: %d, Level: %d\n", lines, score, level);
}

void handleGameOver() {
  gameOver = true;
  Serial.println("\n=== GAME OVER ===");
  Serial.printf("Score: %d\n", score);
  Serial.printf("Lines: %d\n", lines);
  Serial.printf("Level: %d\n", level);

  // Game Over Screen mit Gradient
  uint16_t boxColor = lerpColor(COLOR_BG_TOP, COLOR_BG_BOTTOM, 0.5);
  lcd.fillRect(20, SCREEN_HEIGHT/2 - 50, SCREEN_WIDTH - 40, 100, boxColor);
  lcd.drawRect(20, SCREEN_HEIGHT/2 - 50, SCREEN_WIDTH - 40, 100, COLOR_TEXT);
  lcd.drawRect(21, SCREEN_HEIGHT/2 - 49, SCREEN_WIDTH - 42, 98, brightenColor(boxColor, 1.5));

  lcd.setTextSize(3);
  lcd.setTextColor(COLOR_TEXT, boxColor);
  lcd.setCursor(40, SCREEN_HEIGHT/2 - 30);
  lcd.println("GAME OVER");

  lcd.setTextSize(2);
  lcd.setCursor(50, SCREEN_HEIGHT/2 + 5);
  lcd.printf("Score: %d", score);

  lcd.setTextSize(1);
  lcd.setCursor(35, SCREEN_HEIGHT/2 + 30);
  lcd.println("Press ROTATE to restart");
}

// ===== PARTIKEL =====

void spawnLineClearParticles(int lineY) {
  // 20-30 Partikel für die Linie spawnen
  int numParticles = random(20, 31);

  for (int i = 0; i < numParticles; i++) {
    // Freies Partikel finden
    for (int p = 0; p < MAX_PARTICLES; p++) {
      if (!particles[p].active) {
        // Position auf der Linie
        particles[p].x = FIELD_X + random(0, GRID_WIDTH * BLOCK_SIZE);
        particles[p].y = FIELD_Y + lineY * BLOCK_SIZE + BLOCK_SIZE / 2;

        // Zufällige Geschwindigkeit
        float angle = random(0, 628) / 100.0;
        float speed = random(20, 50) / 10.0;
        particles[p].vx = cos(angle) * speed;
        particles[p].vy = sin(angle) * speed - 2;

        // Zufällige Farbe
        int colorChoice = random(0, 5);
        switch(colorChoice) {
          case 0: particles[p].color = COLOR_PARTICLE1; break;
          case 1: particles[p].color = COLOR_PARTICLE2; break;
          case 2: particles[p].color = COLOR_PARTICLE3; break;
          case 3: particles[p].color = COLOR_PARTICLE4; break;
          case 4: particles[p].color = COLOR_PARTICLE5; break;
        }

        particles[p].life = 100;
        particles[p].active = true;
        break;
      }
    }
  }
}

void updateParticles() {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (!particles[i].active) continue;

    // Alte Position löschen
    int oldX = (int)particles[i].x;
    int oldY = (int)particles[i].y;

    // Im Spielfeld?
    if (oldY >= FIELD_Y && oldY < FIELD_Y + GRID_HEIGHT * BLOCK_SIZE &&
        oldX >= FIELD_X && oldX < FIELD_X + GRID_WIDTH * BLOCK_SIZE) {
      uint16_t bgColor = COLOR_BG;
      lcd.fillCircle(oldX, oldY, 1, bgColor);
    }

    // Bewegen
    particles[i].x += particles[i].vx;
    particles[i].y += particles[i].vy;
    particles[i].vy += 0.3;  // Gravitation
    particles[i].life -= 3;

    // Deaktivieren
    if (particles[i].life <= 0 || particles[i].y > SCREEN_HEIGHT) {
      particles[i].active = false;
      continue;
    }

    // Neue Position zeichnen
    int newX = (int)particles[i].x;
    int newY = (int)particles[i].y;
    lcd.fillCircle(newX, newY, 1, particles[i].color);
  }
}

// ===== ZEICHNEN =====

void drawBackground() {
  // Gradient von oben (Lila) nach unten (Blau)
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    uint16_t color = lerpColor(COLOR_BG_TOP, COLOR_BG_BOTTOM, (float)y / SCREEN_HEIGHT);
    lcd.drawFastHLine(0, y, SCREEN_WIDTH, color);
  }

  // Titel mit Glow
  lcd.setTextSize(2);
  // Glow
  lcd.setTextColor(darkenColor(COLOR_TEXT, 0.3), getBgColor(15));
  lcd.setCursor(51, 6);
  lcd.println("TETRIS");
  lcd.setCursor(49, 6);
  lcd.println("TETRIS");
  lcd.setCursor(50, 7);
  lcd.println("TETRIS");
  lcd.setCursor(50, 5);
  lcd.println("TETRIS");
  // Haupttext
  lcd.setTextColor(COLOR_TEXT, getBgColor(5));
  lcd.setCursor(50, 6);
  lcd.println("TETRIS");

  // Rahmen um Spielfeld mit Glow
  uint16_t frameGlow = brightenColor(COLOR_TEXT, 0.7);
  lcd.drawRect(FIELD_X - 3, FIELD_Y - 3, GRID_WIDTH * BLOCK_SIZE + 6, GRID_HEIGHT * BLOCK_SIZE + 6, frameGlow);
  lcd.drawRect(FIELD_X - 2, FIELD_Y - 2, GRID_WIDTH * BLOCK_SIZE + 4, GRID_HEIGHT * BLOCK_SIZE + 4, COLOR_TEXT);
}

void drawUI() {
  // UI rechts neben dem Spielfeld
  int uiX = FIELD_X + GRID_WIDTH * BLOCK_SIZE + 15;

  // Bereich löschen mit Gradient
  for (int y = 50; y < 200; y++) {
    uint16_t bgColor = getBgColor(y);
    lcd.drawFastHLine(uiX, y, 70, bgColor);
  }

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT, getBgColor(60));

  // Score
  lcd.setCursor(uiX, 50);
  lcd.println("SCORE");
  lcd.setTextColor(brightenColor(COLOR_O, 1.2), getBgColor(65));
  lcd.setCursor(uiX, 65);
  lcd.printf("%d", score);

  // Lines
  lcd.setTextColor(COLOR_TEXT, getBgColor(90));
  lcd.setCursor(uiX, 90);
  lcd.println("LINES");
  lcd.setTextColor(brightenColor(COLOR_I, 1.2), getBgColor(105));
  lcd.setCursor(uiX, 105);
  lcd.printf("%d", lines);

  // Level
  lcd.setTextColor(COLOR_TEXT, getBgColor(130));
  lcd.setCursor(uiX, 130);
  lcd.println("LEVEL");
  lcd.setTextColor(brightenColor(COLOR_T, 1.2), getBgColor(145));
  lcd.setCursor(uiX, 145);
  lcd.printf("%d", level);

  // Next Label
  lcd.setTextColor(COLOR_TEXT, getBgColor(170));
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
    // Gefüllter Block mit 3D-Effekt und Glow

    // Glow (außen, 1 Pixel größer)
    uint16_t glowColor = darkenColor(color, 0.4);
    lcd.fillRect(screenX - 1, screenY - 1, BLOCK_SIZE + 2, BLOCK_SIZE + 2, glowColor);

    // Hauptfarbe
    lcd.fillRect(screenX, screenY, BLOCK_SIZE, BLOCK_SIZE, color);

    // Highlight (oben links) - 3D Effekt
    uint16_t lightColor = brightenColor(color, 1.4);
    lcd.drawFastHLine(screenX + 1, screenY + 1, BLOCK_SIZE - 2, lightColor);
    lcd.drawFastVLine(screenX + 1, screenY + 1, BLOCK_SIZE - 2, lightColor);
    lcd.drawFastHLine(screenX + 2, screenY + 2, BLOCK_SIZE - 4, lightColor);
    lcd.drawFastVLine(screenX + 2, screenY + 2, BLOCK_SIZE - 4, lightColor);

    // Shadow (unten rechts) - 3D Effekt
    uint16_t darkColor = darkenColor(color, 0.6);
    lcd.drawFastHLine(screenX + 1, screenY + BLOCK_SIZE - 2, BLOCK_SIZE - 2, darkColor);
    lcd.drawFastVLine(screenX + BLOCK_SIZE - 2, screenY + 1, BLOCK_SIZE - 2, darkColor);
    lcd.drawFastHLine(screenX + 2, screenY + BLOCK_SIZE - 3, BLOCK_SIZE - 4, darkColor);
    lcd.drawFastVLine(screenX + BLOCK_SIZE - 3, screenY + 2, BLOCK_SIZE - 4, darkColor);
  }
}

void drawGhostBlock(int x, int y, uint16_t color) {
  int screenX = FIELD_X + x * BLOCK_SIZE;
  int screenY = FIELD_Y + y * BLOCK_SIZE;

  // Ghost-Block (nur Outline)
  uint16_t ghostColor = lerpColor(COLOR_GHOST, color, 0.3);
  lcd.drawRect(screenX + 1, screenY + 1, BLOCK_SIZE - 2, BLOCK_SIZE - 2, ghostColor);
  lcd.drawRect(screenX + 2, screenY + 2, BLOCK_SIZE - 4, BLOCK_SIZE - 4, ghostColor);
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

void drawGhostPiece(int ghostY) {
  const bool (*shape)[4] = SHAPES[currentPiece.type][currentPiece.rotation];

  for (int py = 0; py < 4; py++) {
    for (int px = 0; px < 4; px++) {
      if (shape[py][px]) {
        int gridX = currentPiece.x + px;
        int gridY = ghostY + py;

        if (gridY >= 0 && gridY < GRID_HEIGHT && gridX >= 0 && gridX < GRID_WIDTH) {
          // Nur zeichnen wenn nicht vom aktuellen Piece überdeckt
          if (gridY != currentPiece.y + py || gridX != currentPiece.x + px) {
            drawGhostBlock(gridX, gridY, currentPiece.color);
          }
        }
      }
    }
  }
}

void eraseGhostPiece(int ghostY) {
  const bool (*shape)[4] = SHAPES[currentPiece.type][currentPiece.rotation];

  for (int py = 0; py < 4; py++) {
    for (int px = 0; px < 4; px++) {
      if (shape[py][px]) {
        int gridX = currentPiece.x + px;
        int gridY = ghostY + py;

        if (gridY >= 0 && gridY < GRID_HEIGHT && gridX >= 0 && gridX < GRID_WIDTH) {
          // Nur löschen wenn nicht vom aktuellen Piece überdeckt
          if (gridY != currentPiece.y + py || gridX != currentPiece.x + px) {
            drawBlock(gridX, gridY, COLOR_BG);
          }
        }
      }
    }
  }
}

void drawPieceWithGhost() {
  int ghostY = calculateGhostY();

  // Zuerst Ghost zeichnen
  if (ghostY != currentPiece.y) {
    drawGhostPiece(ghostY);
  }

  // Dann aktuelles Piece darüber
  drawPiece(currentPiece, true);
}

void eraseCurrentPieceWithGhost() {
  int ghostY = calculateGhostY();

  // Ghost löschen
  if (ghostY != currentPiece.y) {
    eraseGhostPiece(ghostY);
  }

  // Aktuelles Piece löschen
  drawPiece(currentPiece, false);
}

void drawNextPiece() {
  int uiX = FIELD_X + GRID_WIDTH * BLOCK_SIZE + 15;
  int nextY = 190;

  // Bereich löschen mit Gradient
  for (int y = nextY; y < nextY + 50; y++) {
    uint16_t bgColor = getBgColor(y);
    lcd.drawFastHLine(uiX, y, 50, bgColor);
  }

  // Nächstes Stück zeichnen (kleinere Blocks)
  const bool (*shape)[4] = SHAPES[nextPiece.type][0];

  for (int py = 0; py < 4; py++) {
    for (int px = 0; px < 4; px++) {
      if (shape[py][px]) {
        int screenX = uiX + px * 10 + 5;
        int screenY = nextY + py * 10 + 5;

        // Mini-Block mit Glow
        uint16_t glowColor = darkenColor(nextPiece.color, 0.4);
        lcd.fillRect(screenX - 1, screenY - 1, 11, 11, glowColor);
        lcd.fillRect(screenX, screenY, 9, 9, nextPiece.color);

        // Mini 3D-Effekt
        lcd.drawFastHLine(screenX + 1, screenY + 1, 7, brightenColor(nextPiece.color, 1.3));
        lcd.drawFastVLine(screenX + 1, screenY + 1, 7, brightenColor(nextPiece.color, 1.3));
        lcd.drawFastHLine(screenX + 1, screenY + 7, 7, darkenColor(nextPiece.color, 0.7));
        lcd.drawFastVLine(screenX + 7, screenY + 1, 7, darkenColor(nextPiece.color, 0.7));
      }
    }
  }
}
