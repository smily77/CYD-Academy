/*
  TetrisGame.h - Encapsulated Tetris Game
  Teil der CYD_Games Library

  WICHTIG:
  - Dieses Spiel verwendet Portrait-Modus (setRotation(0))!
  - CYD_Display_Config.h muss VOR diesem Header inkludiert werden!
*/

#ifndef TETRISGAME_H
#define TETRISGAME_H

#include <Arduino.h>
#include <CYD_Input.h>
// Forward declaration - LGFX muss bereits definiert sein
class LGFX;

// Pin-Mapping (muss VOR der Klasse definiert sein)
// Portrait-Modus: Tasten um 90° gegen Uhrzeigersinn gedreht
#define TET_BTN_LEFT   tasteD
#define TET_BTN_RIGHT  tasteA
#define TET_BTN_DOWN   tasteC
#define TET_BTN_ROTATE tasteB

// Display-Konfiguration (PORTRAIT!)
#define TET_SCREEN_WIDTH  240
#define TET_SCREEN_HEIGHT 320

// Spielfeld-Dimensionen
#define TET_GRID_WIDTH  10
#define TET_GRID_HEIGHT 20
#define TET_BLOCK_SIZE  12

// Spielfeld-Position auf dem Screen
#define TET_FIELD_X 20
#define TET_FIELD_Y 40

// Farben
#define TET_COLOR_BG        0x0000  // Schwarz
#define TET_COLOR_GRID      0x2104  // Dunkelgrau
#define TET_COLOR_TEXT      0xFFFF  // Weiß

// Tetromino-Farben (klassisch)
#define TET_COLOR_I 0x07FF  // Cyan
#define TET_COLOR_O 0xFFE0  // Gelb
#define TET_COLOR_T 0xF81F  // Magenta
#define TET_COLOR_S 0x07E0  // Grün
#define TET_COLOR_Z 0xF800  // Rot
#define TET_COLOR_J 0x001F  // Blau
#define TET_COLOR_L 0xFD20  // Orange

// Tetromino-Typen
enum TET_TetrominoType { TET_I, TET_O, TET_T, TET_S, TET_Z, TET_J, TET_L };

// Tetromino-Shapes (muss VOR der Klasse sein)
static const bool TET_SHAPES[7][4][4][4] = {
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

static const uint16_t TET_COLORS[7] = {
  TET_COLOR_I, TET_COLOR_O, TET_COLOR_T, TET_COLOR_S, TET_COLOR_Z, TET_COLOR_J, TET_COLOR_L
};

// Strukturen
struct TET_Tetromino {
  TET_TetrominoType type;
  int x, y;
  int rotation;
  uint16_t color;
};

#define TET_INPUT_DELAY 150

class TetrisGame {
public:
  TetrisGame() :
    lcd(nullptr),
    score(0),
    lines(0),
    level(1),
    gameOver(false),
    lastFallTime(0),
    fallDelay(1000),
    lastInputTime(0)
  {
    // Konstruktor
  }

  void init(LGFX* display) {
    lcd = display;

    score = 0;
    lines = 0;
    level = 1;
    gameOver = false;
    fallDelay = 1000;
    lastFallTime = 0;
    lastInputTime = 0;

    // Spielfeld leeren
    for (int y = 0; y < TET_GRID_HEIGHT; y++) {
      for (int x = 0; x < TET_GRID_WIDTH; x++) {
        grid[y][x] = TET_COLOR_BG;
      }
    }

    // Input initialisieren
    CYD_Input::init();

    // UI zeichnen
    drawBackground();
    drawUI();
    drawGrid();

    // Nächstes Stück vorbereiten
    nextPiece = createRandomPiece();

    // Erstes Stück spawnen
    spawnPiece();

    Serial.println("Tetris gestartet!");
  }

  void update() {
    if (gameOver) {
      // Warte auf Neustart
      if (CYD_Input::readButton(CYD_BTN_B)) {
        init(lcd);
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

  // Status-Abfragen
  bool isGameOver() const { return gameOver; }
  int getScore() const { return score; }
  int getLines() const { return lines; }
  int getLevel() const { return level; }

private:
  LGFX* lcd;

  // Game State
  uint16_t grid[TET_GRID_HEIGHT][TET_GRID_WIDTH];
  TET_Tetromino currentPiece;
  TET_Tetromino nextPiece;

  int score;
  int lines;
  int level;
  bool gameOver;

  unsigned long lastFallTime;
  unsigned long fallDelay;
  unsigned long lastInputTime;

  // ===== INITIALISIERUNG =====

  TET_Tetromino createRandomPiece() {
    TET_Tetromino piece;
    piece.type = (TET_TetrominoType)random(0, 7);
    piece.x = TET_GRID_WIDTH / 2 - 2;
    piece.y = 0;
    piece.rotation = 0;
    piece.color = TET_COLORS[piece.type];
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
    if (now - lastInputTime < TET_INPUT_DELAY) return;

    bool moved = false;
    bool hasEncoder = CYD_Input::hasEncoder();

    if (hasEncoder) {
      // Mit Encoder: Drehung = links/rechts, Button = drehen, KEIN Schnellabsenken
      int delta = CYD_Input::readEncoderDelta();
      if (delta < 0) {
        moved = movePiece(-1, 0);  // Links
      } else if (delta > 0) {
        moved = movePiece(1, 0);   // Rechts
      } else if (CYD_Input::readEncoderButton()) {
        moved = rotatePiece();
      }
    } else {
      // Mit Buttons: D = links, A = rechts, C = runter, B = drehen
      if (CYD_Input::readButton(CYD_BTN_D)) {
        moved = movePiece(-1, 0);
      } else if (CYD_Input::readButton(CYD_BTN_A)) {
        moved = movePiece(1, 0);
      } else if (CYD_Input::readButton(CYD_BTN_C)) {
        moved = movePiece(0, 1);
        if (moved) score += 1;  // Bonus für schnelles Fallen
      } else if (CYD_Input::readButton(CYD_BTN_B)) {
        moved = rotatePiece();
      }
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
    const bool (*shape)[4] = TET_SHAPES[currentPiece.type][rotation];

    for (int py = 0; py < 4; py++) {
      for (int px = 0; px < 4; px++) {
        if (shape[py][px]) {
          int gridX = x + px;
          int gridY = y + py;

          // Außerhalb?
          if (gridX < 0 || gridX >= TET_GRID_WIDTH || gridY >= TET_GRID_HEIGHT) {
            return true;
          }

          // Kollision mit bestehendem Block?
          if (gridY >= 0 && grid[gridY][gridX] != TET_COLOR_BG) {
            return true;
          }
        }
      }
    }

    return false;
  }

  // ===== PIECE MANAGEMENT =====

  void lockPiece() {
    const bool (*shape)[4] = TET_SHAPES[currentPiece.type][currentPiece.rotation];

    for (int py = 0; py < 4; py++) {
      for (int px = 0; px < 4; px++) {
        if (shape[py][px]) {
          int gridX = currentPiece.x + px;
          int gridY = currentPiece.y + py;

          if (gridY >= 0 && gridY < TET_GRID_HEIGHT && gridX >= 0 && gridX < TET_GRID_WIDTH) {
            grid[gridY][gridX] = currentPiece.color;
          }
        }
      }
    }
  }

  void clearLines() {
    int linesCleared = 0;

    for (int y = TET_GRID_HEIGHT - 1; y >= 0; y--) {
      bool fullLine = true;

      for (int x = 0; x < TET_GRID_WIDTH; x++) {
        if (grid[y][x] == TET_COLOR_BG) {
          fullLine = false;
          break;
        }
      }

      if (fullLine) {
        linesCleared++;

        // Zeile löschen (alle Zeilen darüber nach unten)
        for (int yy = y; yy > 0; yy--) {
          for (int x = 0; x < TET_GRID_WIDTH; x++) {
            grid[yy][x] = grid[yy - 1][x];
          }
        }

        // Oberste Zeile löschen
        for (int x = 0; x < TET_GRID_WIDTH; x++) {
          grid[0][x] = TET_COLOR_BG;
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
    lcd->fillRect(20, TET_SCREEN_HEIGHT/2 - 50, TET_SCREEN_WIDTH - 40, 100, 0x2104);
    lcd->drawRect(20, TET_SCREEN_HEIGHT/2 - 50, TET_SCREEN_WIDTH - 40, 100, TET_COLOR_TEXT);

    lcd->setTextSize(3);
    lcd->setTextColor(TET_COLOR_TEXT, 0x2104);
    lcd->setCursor(40, TET_SCREEN_HEIGHT/2 - 30);
    lcd->println("GAME OVER");

    lcd->setTextSize(2);
    lcd->setCursor(50, TET_SCREEN_HEIGHT/2 + 5);
    lcd->printf("Score: %d", score);

    lcd->setTextSize(1);
    lcd->setCursor(35, TET_SCREEN_HEIGHT/2 + 30);
    lcd->println("Press ROTATE to restart");
  }

  // ===== ZEICHNEN =====

  void drawBackground() {
    lcd->fillScreen(TET_COLOR_BG);

    // Titel
    lcd->setTextSize(2);
    lcd->setTextColor(TET_COLOR_TEXT, TET_COLOR_BG);
    lcd->setCursor(60, 5);
    lcd->println("TETRIS");

    // Rahmen um Spielfeld
    lcd->drawRect(TET_FIELD_X - 2, TET_FIELD_Y - 2, TET_GRID_WIDTH * TET_BLOCK_SIZE + 4, TET_GRID_HEIGHT * TET_BLOCK_SIZE + 4, TET_COLOR_TEXT);
  }

  void drawUI() {
    // UI rechts neben dem Spielfeld
    int uiX = TET_FIELD_X + TET_GRID_WIDTH * TET_BLOCK_SIZE + 15;

    lcd->fillRect(uiX, 50, 70, 150, TET_COLOR_BG);

    lcd->setTextSize(1);
    lcd->setTextColor(TET_COLOR_TEXT, TET_COLOR_BG);

    // Score
    lcd->setCursor(uiX, 50);
    lcd->println("SCORE");
    lcd->setCursor(uiX, 65);
    lcd->printf("%d", score);

    // Lines
    lcd->setCursor(uiX, 90);
    lcd->println("LINES");
    lcd->setCursor(uiX, 105);
    lcd->printf("%d", lines);

    // Level
    lcd->setCursor(uiX, 130);
    lcd->println("LEVEL");
    lcd->setCursor(uiX, 145);
    lcd->printf("%d", level);

    // Next Label
    lcd->setCursor(uiX, 170);
    lcd->println("NEXT:");
  }

  void drawGrid() {
    for (int y = 0; y < TET_GRID_HEIGHT; y++) {
      for (int x = 0; x < TET_GRID_WIDTH; x++) {
        drawBlock(x, y, grid[y][x]);
      }
    }
  }

  void drawBlock(int x, int y, uint16_t color) {
    int screenX = TET_FIELD_X + x * TET_BLOCK_SIZE;
    int screenY = TET_FIELD_Y + y * TET_BLOCK_SIZE;

    if (color == TET_COLOR_BG) {
      // Leerer Block - Grid-Linie
      lcd->fillRect(screenX, screenY, TET_BLOCK_SIZE, TET_BLOCK_SIZE, TET_COLOR_BG);
      lcd->drawRect(screenX, screenY, TET_BLOCK_SIZE, TET_BLOCK_SIZE, TET_COLOR_GRID);
    } else {
      // Gefüllter Block mit 3D-Effekt
      lcd->fillRect(screenX, screenY, TET_BLOCK_SIZE, TET_BLOCK_SIZE, color);

      // Hellerer Rand oben/links
      uint16_t lightColor = color + 0x2104;
      lcd->drawFastHLine(screenX, screenY, TET_BLOCK_SIZE, lightColor);
      lcd->drawFastVLine(screenX, screenY, TET_BLOCK_SIZE, lightColor);

      // Dunklerer Rand unten/rechts
      uint16_t darkColor = color - 0x2104;
      lcd->drawFastHLine(screenX, screenY + TET_BLOCK_SIZE - 1, TET_BLOCK_SIZE, darkColor);
      lcd->drawFastVLine(screenX + TET_BLOCK_SIZE - 1, screenY, TET_BLOCK_SIZE, darkColor);
    }
  }

  void drawPiece(TET_Tetromino piece, bool show) {
    const bool (*shape)[4] = TET_SHAPES[piece.type][piece.rotation];

    for (int py = 0; py < 4; py++) {
      for (int px = 0; px < 4; px++) {
        if (shape[py][px]) {
          int gridX = piece.x + px;
          int gridY = piece.y + py;

          if (gridY >= 0 && gridY < TET_GRID_HEIGHT && gridX >= 0 && gridX < TET_GRID_WIDTH) {
            drawBlock(gridX, gridY, show ? piece.color : TET_COLOR_BG);
          }
        }
      }
    }
  }

  void drawNextPiece() {
    int uiX = TET_FIELD_X + TET_GRID_WIDTH * TET_BLOCK_SIZE + 15;
    int nextY = 190;

    // Bereich löschen
    lcd->fillRect(uiX, nextY, 50, 50, TET_COLOR_BG);

    // Nächstes Stück zeichnen
    const bool (*shape)[4] = TET_SHAPES[nextPiece.type][0];

    for (int py = 0; py < 4; py++) {
      for (int px = 0; px < 4; px++) {
        if (shape[py][px]) {
          int screenX = uiX + px * 10;
          int screenY = nextY + py * 10;

          lcd->fillRect(screenX, screenY, 9, 9, nextPiece.color);
        }
      }
    }
  }
};

#endif // TETRISGAME_H
