/*
 * CYD-Academy - Spiel 02: Sudoku
 *
 * Das klassische Logik-Puzzle für Zahlenfreunde!
 * 9x9 Grid mit 3x3 Boxen, fülle alle Felder mit Zahlen 1-9.
 *
 * Regeln:
 * - Jede Zeile muss die Zahlen 1-9 enthalten (keine Duplikate)
 * - Jede Spalte muss die Zahlen 1-9 enthalten (keine Duplikate)
 * - Jede 3x3 Box muss die Zahlen 1-9 enthalten (keine Duplikate)
 *
 * Hardware:
 * - ESP32-2432S028R (CYD - Cheap Yellow Display)
 * - Touch-Screen für Spielsteuerung
 *
 * Steuerung:
 * - Touch: Tippe auf eine Zelle um sie auszuwählen
 * - Numpad: Tippe auf 1-9 um Zahl einzutragen
 * - Touch-Button "NEU": Neues Puzzle generieren
 * - Touch-Button "EASY": Schwierigkeit ändern (Easy/Medium/Hard)
 * - Touch-Button "NOTIZ": Notizen-Modus (kleine Kandidaten)
 * - Touch-Button "DEL": Gewählte Zelle löschen
 * - Touch-Button "CHECK": Prüfe auf Fehler
 *
 * Features:
 * - 3 Schwierigkeitsgrade (Easy: 35 Zahlen, Medium: 30, Hard: 25)
 * - Puzzle-Generator mit Backtracking
 * - Fehler-Detektion (rot markiert)
 * - Notizen-Modus für Kandidaten
 * - Timer und Fehler-Zähler
 * - Auto-Prüfung bei Eingabe
 * - Gewinn-Animation
 * - Original Sudoku-Design
 *
 * Grafik:
 * - Klassisches Grid-Layout
 * - 3x3 Boxen dick umrandet
 * - Ausgewählte Zelle hervorgehoben
 * - Fehler in Rot
 * - Vorgegebene Zahlen in Schwarz (nicht änderbar)
 * - Eingegebene Zahlen in Blau
 * - Notizen klein und grau
 *
 * Autor: CYD-Academy
 * Lizenz: MIT
 */

#include <CYD_Display_Config.h>

// ========== Display Setup ==========
LGFX lcd;

// ========== Sudoku-Konfiguration ==========
#define GRID_SIZE 9
#define BOX_SIZE 3
#define CELL_COUNT (GRID_SIZE * GRID_SIZE)

// ========== Farben (RGB565) ==========
#define COLOR_BACKGROUND 0xFFFF      // Weiß
#define COLOR_GRID_THIN 0xCE59       // Hellgrau (normale Linien)
#define COLOR_GRID_THICK 0x0000      // Schwarz (Box-Linien)
#define COLOR_SELECTED 0xFED7        // Hellgelb (ausgewählte Zelle)
#define COLOR_ERROR 0xF800           // Rot (Fehler)
#define COLOR_GIVEN 0x0000           // Schwarz (vorgegebene Zahlen)
#define COLOR_USER 0x001F            // Blau (eingegebene Zahlen)
#define COLOR_NOTE 0x8410            // Grau (Notizen)
#define COLOR_BUTTON_BG 0x4A49       // Dunkelgrau
#define COLOR_BUTTON_TEXT 0xFFFF     // Weiß

// ========== Schwierigkeitsgrade ==========
enum Difficulty {
  EASY = 35,    // 35 vorgegebene Zahlen
  MEDIUM = 30,  // 30 vorgegebene Zahlen
  HARD = 25     // 25 vorgegebene Zahlen
};

// ========== Globale Variablen ==========
// Spielfeld-Dimensionen
int16_t GRID_X, GRID_Y;      // Position des Grids
int16_t CELL_SIZE;            // Größe einer Zelle
int16_t NUMPAD_X, NUMPAD_Y;   // Position des Numpads
int16_t NUMPAD_SIZE;          // Größe eines Numpad-Buttons
int16_t BUTTON_X, BUTTON_Y;   // Position der Control-Buttons
int16_t BUTTON_W, BUTTON_H;   // Größe der Control-Buttons

// Sudoku-Daten
uint8_t grid[GRID_SIZE][GRID_SIZE];           // Aktuelles Grid (0 = leer)
uint8_t solution[GRID_SIZE][GRID_SIZE];       // Lösung
bool isGiven[GRID_SIZE][GRID_SIZE];           // Vorgegebene Zahlen (nicht änderbar)
bool notes[GRID_SIZE][GRID_SIZE][9];          // Notizen (Kandidaten 1-9)

// Spielzustand
int8_t selectedRow = -1, selectedCol = -1;    // Ausgewählte Zelle (-1 = keine)
Difficulty difficulty = MEDIUM;               // Aktueller Schwierigkeitsgrad
bool noteMode = false;                        // Notizen-Modus aktiv?
bool gameWon = false;                         // Spiel gewonnen?
uint16_t errorCount = 0;                      // Anzahl Fehler
unsigned long gameStartTime = 0;
unsigned long gameDuration = 0;
unsigned long lastUpdateTime = 0;

// Touch-Debounce
unsigned long lastTouchTime = 0;
const unsigned long TOUCH_DEBOUNCE = 200;

// ========== Forward Declarations ==========
void initGame();
void generatePuzzle();
bool solveSudoku(int row, int col);
void removeNumbers(int count);
void drawGrid();
void drawCell(int row, int col);
void drawNumpad();
void drawButtons();
void drawStats();
void handleTouch();
bool isValid(int row, int col, int num);
bool checkWin();
void showWinAnimation();
void clearNotes(int row, int col);

// ========== Setup ==========
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== CYD Spiel 02: Sudoku ===");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);  // Landscape
  lcd.fillScreen(COLOR_BACKGROUND);
  lcd.setTextDatum(MC_DATUM);

  // Layout berechnen
  int16_t screenWidth = lcd.width();
  int16_t screenHeight = lcd.height();

  // Grid links, Numpad + Buttons rechts
  int16_t availableHeight = screenHeight - 60;  // 60px für Stats
  int16_t gridSize = (availableHeight < screenWidth - 120) ? availableHeight : (screenWidth - 120);
  gridSize = (gridSize / GRID_SIZE) * GRID_SIZE;  // Auf Grid abrunden

  CELL_SIZE = gridSize / GRID_SIZE;
  GRID_X = 10;
  GRID_Y = 60 + (availableHeight - gridSize) / 2;

  // Numpad rechts (3x3 Grid für 1-9)
  NUMPAD_SIZE = (screenWidth - GRID_X - gridSize - 30) / 3;
  NUMPAD_X = GRID_X + gridSize + 10;
  NUMPAD_Y = GRID_Y;

  // Control-Buttons unter Numpad
  BUTTON_W = NUMPAD_SIZE * 3;
  BUTTON_H = 30;
  BUTTON_X = NUMPAD_X;
  BUTTON_Y = NUMPAD_Y + NUMPAD_SIZE * 3 + 10;

  Serial.printf("Grid: %dx%d at (%d,%d), Cell: %d\n",
                gridSize, gridSize, GRID_X, GRID_Y, CELL_SIZE);
  Serial.printf("Numpad: %d at (%d,%d)\n", NUMPAD_SIZE, NUMPAD_X, NUMPAD_Y);

  // Spiel starten
  initGame();

  Serial.println("Setup abgeschlossen!");
}

// ========== Main Loop ==========
void loop() {
  if (!gameWon) {
    // Timer aktualisieren
    unsigned long now = millis();
    if (now - lastUpdateTime >= 1000) {
      lastUpdateTime = now;
      gameDuration = (now - gameStartTime) / 1000;
      drawStats();
    }

    handleTouch();
  } else {
    handleTouch();  // Nur für Neustart
  }

  delay(10);
}

// ========== Spiel initialisieren ==========
void initGame() {
  // Zurücksetzen
  memset(grid, 0, sizeof(grid));
  memset(solution, 0, sizeof(solution));
  memset(isGiven, false, sizeof(isGiven));
  memset(notes, false, sizeof(notes));

  selectedRow = -1;
  selectedCol = -1;
  noteMode = false;
  gameWon = false;
  errorCount = 0;
  gameStartTime = millis();
  lastUpdateTime = millis();
  gameDuration = 0;

  // UI zeichnen
  lcd.fillScreen(COLOR_BACKGROUND);
  drawGrid();
  drawNumpad();
  drawButtons();
  drawStats();

  // Puzzle generieren
  generatePuzzle();

  Serial.println("Neues Sudoku generiert!");
}

// ========== Puzzle generieren ==========
void generatePuzzle() {
  Serial.println("Generiere Sudoku-Puzzle...");

  // 1. Leeres Grid
  memset(solution, 0, sizeof(solution));

  // 2. Fülle Diagonale 3x3 Boxen (unabhängig voneinander)
  for (int box = 0; box < 3; box++) {
    int startRow = box * 3;
    int startCol = box * 3;

    // Zufällige Zahlen 1-9 in Box
    uint8_t nums[9] = {1,2,3,4,5,6,7,8,9};
    for (int i = 8; i > 0; i--) {
      int j = random(i + 1);
      uint8_t temp = nums[i];
      nums[i] = nums[j];
      nums[j] = temp;
    }

    // In Box eintragen
    int idx = 0;
    for (int r = 0; r < 3; r++) {
      for (int c = 0; c < 3; c++) {
        solution[startRow + r][startCol + c] = nums[idx++];
      }
    }
  }

  // 3. Löse Rest mit Backtracking
  solveSudoku(0, 0);

  // 4. Kopiere Lösung zu Grid
  memcpy(grid, solution, sizeof(grid));

  // 5. Entferne Zahlen basierend auf Schwierigkeit
  removeNumbers(81 - difficulty);

  // 6. Markiere vorgegebene Zahlen
  for (int r = 0; r < GRID_SIZE; r++) {
    for (int c = 0; c < GRID_SIZE; c++) {
      isGiven[r][c] = (grid[r][c] != 0);
    }
  }

  // Grid neu zeichnen
  drawGrid();

  Serial.printf("Puzzle generiert! Schwierigkeit: %d vorgegebene Zahlen\n", difficulty);
}

// ========== Sudoku-Solver (Backtracking) ==========
bool solveSudoku(int row, int col) {
  // Ende erreicht?
  if (row == GRID_SIZE) return true;

  // Nächste Position
  int nextRow = (col == GRID_SIZE - 1) ? row + 1 : row;
  int nextCol = (col == GRID_SIZE - 1) ? 0 : col + 1;

  // Zelle schon gefüllt?
  if (solution[row][col] != 0) {
    return solveSudoku(nextRow, nextCol);
  }

  // Probiere Zahlen 1-9
  for (int num = 1; num <= 9; num++) {
    if (isValidSolution(row, col, num)) {
      solution[row][col] = num;

      if (solveSudoku(nextRow, nextCol)) {
        return true;
      }

      solution[row][col] = 0;  // Backtrack
    }
  }

  return false;
}

// ========== Validierung für Lösung ==========
bool isValidSolution(int row, int col, int num) {
  // Prüfe Zeile
  for (int c = 0; c < GRID_SIZE; c++) {
    if (solution[row][c] == num) return false;
  }

  // Prüfe Spalte
  for (int r = 0; r < GRID_SIZE; r++) {
    if (solution[r][col] == num) return false;
  }

  // Prüfe 3x3 Box
  int boxRow = (row / 3) * 3;
  int boxCol = (col / 3) * 3;
  for (int r = 0; r < 3; r++) {
    for (int c = 0; c < 3; c++) {
      if (solution[boxRow + r][boxCol + c] == num) return false;
    }
  }

  return true;
}

// ========== Zahlen entfernen ==========
void removeNumbers(int count) {
  int removed = 0;
  while (removed < count) {
    int row = random(GRID_SIZE);
    int col = random(GRID_SIZE);

    if (grid[row][col] != 0) {
      grid[row][col] = 0;
      removed++;
    }
  }
}

// ========== Grid zeichnen ==========
void drawGrid() {
  // Hintergrund
  lcd.fillRect(GRID_X, GRID_Y, CELL_SIZE * GRID_SIZE, CELL_SIZE * GRID_SIZE, COLOR_BACKGROUND);

  // Alle Zellen zeichnen
  for (int r = 0; r < GRID_SIZE; r++) {
    for (int c = 0; c < GRID_SIZE; c++) {
      drawCell(r, c);
    }
  }

  // Grid-Linien
  for (int i = 0; i <= GRID_SIZE; i++) {
    bool isBoxLine = (i % 3 == 0);
    uint16_t color = isBoxLine ? COLOR_GRID_THICK : COLOR_GRID_THIN;
    int thickness = isBoxLine ? 2 : 1;

    // Horizontale Linie
    for (int t = 0; t < thickness; t++) {
      lcd.drawFastHLine(GRID_X, GRID_Y + i * CELL_SIZE + t,
                       CELL_SIZE * GRID_SIZE, color);
    }

    // Vertikale Linie
    for (int t = 0; t < thickness; t++) {
      lcd.drawFastVLine(GRID_X + i * CELL_SIZE + t, GRID_Y,
                       CELL_SIZE * GRID_SIZE, color);
    }
  }
}

// ========== Einzelne Zelle zeichnen ==========
void drawCell(int row, int col) {
  int16_t x = GRID_X + col * CELL_SIZE + 2;
  int16_t y = GRID_Y + row * CELL_SIZE + 2;
  int16_t w = CELL_SIZE - 3;
  int16_t h = CELL_SIZE - 3;

  // Hintergrund
  uint16_t bgColor = COLOR_BACKGROUND;
  if (row == selectedRow && col == selectedCol) {
    bgColor = COLOR_SELECTED;  // Ausgewählte Zelle
  }
  lcd.fillRect(x, y, w, h, bgColor);

  // Zahl zeichnen
  if (grid[row][col] != 0) {
    uint16_t textColor = isGiven[row][col] ? COLOR_GIVEN : COLOR_USER;

    // Prüfe auf Fehler
    if (!isGiven[row][col] && !isValid(row, col, grid[row][col])) {
      textColor = COLOR_ERROR;
    }

    lcd.setTextColor(textColor);
    lcd.setTextDatum(MC_DATUM);

    int fontSize = (CELL_SIZE > 25) ? 4 : 2;
    lcd.drawNumber(grid[row][col], x + w / 2, y + h / 2, fontSize);
  } else if (!noteMode || (row != selectedRow || col != selectedCol)) {
    // Notizen zeichnen (3x3 Grid)
    for (int i = 0; i < 9; i++) {
      if (notes[row][col][i]) {
        int noteRow = i / 3;
        int noteCol = i % 3;
        int16_t nx = x + noteCol * (w / 3) + w / 6;
        int16_t ny = y + noteRow * (h / 3) + h / 6;

        lcd.setTextColor(COLOR_NOTE);
        lcd.setTextDatum(MC_DATUM);
        lcd.drawNumber(i + 1, nx, ny, 1);
      }
    }
  }
}

// ========== Numpad zeichnen ==========
void drawNumpad() {
  for (int i = 0; i < 9; i++) {
    int row = i / 3;
    int col = i % 3;
    int16_t x = NUMPAD_X + col * NUMPAD_SIZE;
    int16_t y = NUMPAD_Y + row * NUMPAD_SIZE;

    // Button
    lcd.fillRect(x + 2, y + 2, NUMPAD_SIZE - 4, NUMPAD_SIZE - 4, COLOR_BUTTON_BG);
    lcd.drawRect(x + 2, y + 2, NUMPAD_SIZE - 4, NUMPAD_SIZE - 4, TFT_WHITE);

    // Zahl
    lcd.setTextColor(COLOR_BUTTON_TEXT);
    lcd.setTextDatum(MC_DATUM);
    int fontSize = (NUMPAD_SIZE > 30) ? 4 : 2;
    lcd.drawNumber(i + 1, x + NUMPAD_SIZE / 2, y + NUMPAD_SIZE / 2, fontSize);
  }
}

// ========== Control-Buttons zeichnen ==========
void drawButtons() {
  const char* labels[5] = {"NEU", "EASY", "NOTIZ", "DEL", "CHECK"};

  // Zeige aktuellen Schwierigkeitsgrad
  String diffLabel = "EASY";
  if (difficulty == MEDIUM) diffLabel = "MEDIUM";
  else if (difficulty == HARD) diffLabel = "HARD";

  for (int i = 0; i < 5; i++) {
    int16_t by = BUTTON_Y + i * (BUTTON_H + 5);

    lcd.fillRect(BUTTON_X, by, BUTTON_W, BUTTON_H, COLOR_BUTTON_BG);
    lcd.drawRect(BUTTON_X, by, BUTTON_W, BUTTON_H, TFT_WHITE);

    lcd.setTextColor(COLOR_BUTTON_TEXT);
    lcd.setTextDatum(MC_DATUM);

    String label = labels[i];
    if (i == 1) label = diffLabel;  // Zeige aktuellen Schwierigkeitsgrad
    if (i == 2 && noteMode) label = "NOTIZ*";  // * wenn aktiv

    lcd.drawString(label, BUTTON_X + BUTTON_W / 2, by + BUTTON_H / 2, 2);
  }
}

// ========== Statistiken zeichnen ==========
void drawStats() {
  lcd.fillRect(0, 0, lcd.width(), 55, COLOR_BACKGROUND);

  lcd.setTextDatum(TL_DATUM);
  lcd.setTextColor(TFT_BLACK);
  lcd.drawString("SUDOKU", 10, 5, 4);

  lcd.setTextDatum(TR_DATUM);

  // Timer
  int minutes = gameDuration / 60;
  int seconds = gameDuration % 60;
  String timeStr = "Zeit: " + String(minutes) + ":" +
                   (seconds < 10 ? "0" : "") + String(seconds);
  lcd.drawString(timeStr, lcd.width() - 10, 5, 2);

  // Fehler
  String errors = "Fehler: " + String(errorCount);
  lcd.drawString(errors, lcd.width() - 10, 25, 2);

  lcd.setTextDatum(MC_DATUM);
}

// ========== Touch-Eingabe ==========
void handleTouch() {
  uint16_t tx, ty;
  if (!lcd.getTouch(&tx, &ty)) return;

  if (millis() - lastTouchTime < TOUCH_DEBOUNCE) return;
  lastTouchTime = millis();

  // Grid-Touch
  if (tx >= GRID_X && tx < GRID_X + CELL_SIZE * GRID_SIZE &&
      ty >= GRID_Y && ty < GRID_Y + CELL_SIZE * GRID_SIZE) {

    int col = (tx - GRID_X) / CELL_SIZE;
    int row = (ty - GRID_Y) / CELL_SIZE;

    if (row >= 0 && row < GRID_SIZE && col >= 0 && col < GRID_SIZE) {
      // Alte Auswahl löschen
      if (selectedRow >= 0) {
        drawCell(selectedRow, selectedCol);
      }

      selectedRow = row;
      selectedCol = col;

      // Neue Auswahl zeichnen
      drawCell(selectedRow, selectedCol);

      Serial.printf("Zelle ausgewählt: (%d,%d)\n", row, col);
    }
    return;
  }

  // Numpad-Touch
  if (tx >= NUMPAD_X && tx < NUMPAD_X + NUMPAD_SIZE * 3 &&
      ty >= NUMPAD_Y && ty < NUMPAD_Y + NUMPAD_SIZE * 3) {

    if (selectedRow < 0 || isGiven[selectedRow][selectedCol]) {
      Serial.println("Keine Zelle ausgewählt oder Zelle vorgegeben");
      return;
    }

    int col = (tx - NUMPAD_X) / NUMPAD_SIZE;
    int row = (ty - NUMPAD_Y) / NUMPAD_SIZE;
    int num = row * 3 + col + 1;

    if (noteMode) {
      // Notiz umschalten
      notes[selectedRow][selectedCol][num - 1] = !notes[selectedRow][selectedCol][num - 1];
    } else {
      // Zahl eintragen
      grid[selectedRow][selectedCol] = num;
      clearNotes(selectedRow, selectedCol);

      // Prüfe auf Fehler
      if (!isValid(selectedRow, selectedCol, num)) {
        errorCount++;
        drawStats();
      }
    }

    drawCell(selectedRow, selectedCol);

    // Prüfe Gewinn
    if (checkWin()) {
      showWinAnimation();
    }

    Serial.printf("Zahl %d eingetragen (Notiz: %d)\n", num, noteMode);
    return;
  }

  // Control-Buttons
  if (tx >= BUTTON_X && tx < BUTTON_X + BUTTON_W) {
    for (int i = 0; i < 5; i++) {
      int16_t by = BUTTON_Y + i * (BUTTON_H + 5);

      if (ty >= by && ty < by + BUTTON_H) {
        if (i == 0) {
          // NEU
          initGame();
        } else if (i == 1) {
          // Schwierigkeit wechseln
          if (difficulty == EASY) difficulty = MEDIUM;
          else if (difficulty == MEDIUM) difficulty = HARD;
          else difficulty = EASY;
          drawButtons();
          Serial.printf("Schwierigkeit: %d\n", difficulty);
        } else if (i == 2) {
          // Notiz-Modus
          noteMode = !noteMode;
          drawButtons();
        } else if (i == 3) {
          // DEL
          if (selectedRow >= 0 && !isGiven[selectedRow][selectedCol]) {
            grid[selectedRow][selectedCol] = 0;
            memset(notes[selectedRow][selectedCol], false, 9);
            drawCell(selectedRow, selectedCol);
          }
        } else if (i == 4) {
          // CHECK
          // Markiere alle Fehler rot (wird bei drawGrid gemacht)
          drawGrid();
        }
        return;
      }
    }
  }
}

// ========== Validierung ==========
bool isValid(int row, int col, int num) {
  // Prüfe Zeile
  for (int c = 0; c < GRID_SIZE; c++) {
    if (c != col && grid[row][c] == num) return false;
  }

  // Prüfe Spalte
  for (int r = 0; r < GRID_SIZE; r++) {
    if (r != row && grid[r][col] == num) return false;
  }

  // Prüfe 3x3 Box
  int boxRow = (row / 3) * 3;
  int boxCol = (col / 3) * 3;
  for (int r = 0; r < 3; r++) {
    for (int c = 0; c < 3; c++) {
      int checkRow = boxRow + r;
      int checkCol = boxCol + c;
      if ((checkRow != row || checkCol != col) &&
          grid[checkRow][checkCol] == num) {
        return false;
      }
    }
  }

  return true;
}

// ========== Gewinn-Check ==========
bool checkWin() {
  // Alle Zellen gefüllt?
  for (int r = 0; r < GRID_SIZE; r++) {
    for (int c = 0; c < GRID_SIZE; c++) {
      if (grid[r][c] == 0) return false;
      if (!isValid(r, c, grid[r][c])) return false;
    }
  }
  return true;
}

// ========== Gewinn-Animation ==========
void showWinAnimation() {
  gameWon = true;

  Serial.printf("GEWONNEN in %d:%02d! Fehler: %d\n",
                gameDuration / 60, gameDuration % 60, errorCount);

  // Grüner Rahmen um Grid
  for (int i = 0; i < 5; i++) {
    lcd.drawRect(GRID_X - i, GRID_Y - i,
                CELL_SIZE * GRID_SIZE + i * 2,
                CELL_SIZE * GRID_SIZE + i * 2, TFT_GREEN);
  }

  delay(500);

  // Gewinn-Text
  lcd.fillRect(GRID_X + 50, GRID_Y + CELL_SIZE * 4,
               CELL_SIZE * GRID_SIZE - 100, 60, COLOR_BACKGROUND);

  lcd.setTextColor(TFT_GREEN);
  lcd.setTextDatum(MC_DATUM);
  lcd.drawString("GEWONNEN!", GRID_X + CELL_SIZE * 4 + CELL_SIZE / 2,
                 GRID_Y + CELL_SIZE * 4 + 15, 4);

  String stats = String(gameDuration / 60) + ":" +
                 (gameDuration % 60 < 10 ? "0" : "") +
                 String(gameDuration % 60) + " / " +
                 String(errorCount) + " Fehler";
  lcd.setTextColor(TFT_BLACK);
  lcd.drawString(stats, GRID_X + CELL_SIZE * 4 + CELL_SIZE / 2,
                 GRID_Y + CELL_SIZE * 4 + 40, 2);

  delay(3000);
}

// ========== Notizen löschen ==========
void clearNotes(int row, int col) {
  memset(notes[row][col], false, 9);
}
