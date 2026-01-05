/*
 * CYD-Academy - Spiel 01: 15-Puzzle (Schiebepuzzle)
 *
 * Das klassische 15-Puzzle im Original-Design von 1874!
 * Rote und weiße Kacheln mit goldenen Zahlen, wie das Original.
 *
 * Ziel: Ordne die 15 nummerierten Kacheln (1-15) in der richtigen
 * Reihenfolge, indem du sie in das leere Feld schiebst.
 *
 * Hardware:
 * - ESP32-2432S028R (CYD - Cheap Yellow Display)
 * - Touch-Screen für Spielsteuerung
 *
 * Steuerung:
 * - Touch: Tippe auf eine Kachel neben dem leeren Feld zum Verschieben
 * - Touch-Button "NEU": Neues Spiel (Shuffle)
 * - Touch-Button "UNDO": Letzten Zug rückgängig machen
 * - Touch-Button "PAUSE": Spiel pausieren/fortsetzen
 * - Touch-Button "HELP": Zeige Hinweis
 *
 * Features:
 * - Original 15-Puzzle Design (Rot/Weiß mit goldenen Zahlen)
 * - Realistische 3D-Grafik mit Schatten und Highlights
 * - Glatte Schiebe-Animationen
 * - Holzrahmen um das Spielfeld
 * - 4 Touch-Buttons rechts vom Puzzle
 * - Züge-Zähler und Timer (ohne Flackern)
 * - Bester Score wird gespeichert
 * - Gewinn-Animation mit Feuerwerk
 * - Garantiert lösbares Puzzle nach Shuffle
 * - Undo-Funktion für letzte Züge
 *
 * Original-Farbschema:
 * - Kacheln 1,3,6,8,9,11,14: ROT
 * - Kacheln 2,4,5,7,10,12,13,15: WEISS
 * - Zahlen: GOLD mit Schatten
 *
 * Autor: CYD-Academy
 * Lizenz: MIT
 */

#include <CYD_Display_Config.h>
#include <vector>

// ========== Display Setup ==========
LGFX lcd;

// ========== Spielfeld-Konfiguration ==========
#define GRID_SIZE 4
#define TILE_COUNT (GRID_SIZE * GRID_SIZE)

// ========== Farben (RGB565) - Original Design ==========
#define COLOR_BACKGROUND 0x7BCF      // Helles Grau
#define COLOR_WOOD_DARK 0x5980       // Dunkelbraun (Holz)
#define COLOR_WOOD_LIGHT 0xAD55      // Hellbraun (Holz)
#define COLOR_WOOD_MID 0x8C51        // Mittelbraun (Holz)
#define COLOR_EMPTY 0x2945           // Dunkelgrau (leeres Feld)

// Original 15-Puzzle Farben
#define COLOR_TILE_RED 0xC800        // Rot (wie Original)
#define COLOR_TILE_WHITE 0xFFFF      // Weiß
#define COLOR_NUMBER_GOLD 0xFD20     // Gold
#define COLOR_SHADOW 0x2104          // Schatten (sehr dunkel)
#define COLOR_WIN_TEXT 0xFFE0        // Gelb

// Touch-Button Farben
#define COLOR_BUTTON_BG 0x4A49       // Dunkelgrau
#define COLOR_BUTTON_BORDER 0x8410   // Grau
#define COLOR_BUTTON_TEXT 0xFFFF     // Weiß
#define COLOR_BUTTON_PRESSED 0x2945  // Noch dunkler

// Welche Kacheln sind rot? (1,3,6,8,9,11,14)
const bool IS_RED_TILE[15] = {
  true,  false, true,  false, // 1,2,3,4
  false, true,  false, true,  // 5,6,7,8
  true,  false, true,  false, // 9,10,11,12
  false, true,  false         // 13,14,15
};

// ========== Globale Variablen ==========
// Spielfeld-Dimensionen (werden in setup() berechnet)
int16_t FIELD_SIZE;        // Größe des gesamten Spielfelds
int16_t TILE_SIZE;         // Größe einer Kachel
int16_t FIELD_X;           // X-Position des Spielfelds
int16_t FIELD_Y;           // Y-Position des Spielfelds
int16_t FRAME_WIDTH;       // Breite des Holzrahmens
int16_t BUTTON_WIDTH;      // Breite der Touch-Buttons
int16_t BUTTON_HEIGHT;     // Höhe der Touch-Buttons
int16_t BUTTON_X;          // X-Position der Buttons
int16_t BUTTON_Y;          // Y-Position der Buttons

// Spiellogik
uint8_t tiles[TILE_COUNT]; // 0 = leeres Feld, 1-15 = nummerierte Kacheln
uint8_t emptyPos;          // Position des leeren Feldes (0-15)
uint16_t moveCount = 0;    // Anzahl der Züge
uint16_t bestScore = 9999; // Bester Score (wenigste Züge)
bool gameWon = false;      // Spiel gewonnen?
bool isPaused = false;     // Spiel pausiert?

// Animation
int8_t animTile = -1;      // Kachel die gerade animiert wird (-1 = keine)
float animProgress = 0.0;  // Animations-Fortschritt (0.0 - 1.0)
int8_t animStartX, animStartY; // Start-Position der Animation
int8_t animEndX, animEndY;     // End-Position der Animation
const float ANIM_SPEED = 0.15; // Animations-Geschwindigkeit

// Timer
unsigned long gameStartTime = 0;
unsigned long gameDuration = 0;
unsigned long lastUpdateTime = 0;

// Undo-Stack
struct Move {
  uint8_t tile;
  uint8_t from;
  uint8_t to;
};
std::vector<Move> moveHistory;
const int MAX_HISTORY = 100;

// Touch-Debounce
unsigned long lastTouchTime = 0;
const unsigned long TOUCH_DEBOUNCE = 200;

// Stats-Cache (gegen Flackern)
uint16_t lastDrawnMoves = 9999;
uint32_t lastDrawnTime = 9999;

// ========== Forward Declarations ==========
void initGame();
void shufflePuzzle();
void drawBoard();
void drawTile(uint8_t pos, bool immediate = false);
void drawFrame();
void drawStats();
void drawButtons();
bool checkButtonPress(uint16_t x, uint16_t y, int buttonIndex);
void handleTouch();
void showHint();
bool checkWin();
void showWinAnimation();
uint8_t getTileAt(int8_t x, int8_t y);
void setTileAt(int8_t x, int8_t y, uint8_t tile);
void getXY(uint8_t pos, int8_t &x, int8_t &y);
uint8_t getPos(int8_t x, int8_t y);
bool canMove(uint8_t pos);
void moveTile(uint8_t pos, bool addToHistory = true);
void undoMove();
bool isSolvable();
void updateAnimation();
uint16_t interpolateColor(uint16_t color1, uint16_t color2, float t);

// ========== Setup ==========
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== CYD Spiel 01: 15-Puzzle ===");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);  // Landscape
  lcd.fillScreen(COLOR_BACKGROUND);
  lcd.setTextDatum(MC_DATUM);  // Text zentriert

  // Spielfeld-Dimensionen berechnen (abhängig von Display-Größe)
  int16_t screenWidth = lcd.width();
  int16_t screenHeight = lcd.height();

  Serial.printf("Display: %dx%d\n", screenWidth, screenHeight);

  // Platz für Buttons rechts reservieren (70px)
  int16_t availableWidth = screenWidth - 80;   // 70px für Buttons + 10px Margin
  int16_t availableHeight = screenHeight - 60; // 60px für Stats oben

  int16_t maxSize = (availableWidth - 40 < availableHeight) ? (availableWidth - 40) : availableHeight;

  FIELD_SIZE = (maxSize / GRID_SIZE) * GRID_SIZE;  // Auf Grid-Größe abrunden
  TILE_SIZE = FIELD_SIZE / GRID_SIZE;
  FRAME_WIDTH = TILE_SIZE / 6;  // Etwas breiter für bessere Sichtbarkeit

  FIELD_X = 10;  // Links ausgerichtet (mit Margin)
  FIELD_Y = 60 + (availableHeight - FIELD_SIZE - FRAME_WIDTH * 2) / 2;

  // Touch-Buttons rechts
  BUTTON_WIDTH = 60;
  BUTTON_HEIGHT = (FIELD_SIZE + FRAME_WIDTH * 2) / 4 - 10;
  BUTTON_X = FIELD_X + FIELD_SIZE + FRAME_WIDTH * 2 + 10;
  BUTTON_Y = FIELD_Y;

  Serial.printf("Field: %dx%d at (%d,%d)\n", FIELD_SIZE, FIELD_SIZE, FIELD_X, FIELD_Y);
  Serial.printf("Tile: %dx%d, Frame: %d\n", TILE_SIZE, TILE_SIZE, FRAME_WIDTH);
  Serial.printf("Buttons: %dx%d at (%d,%d)\n", BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_X, BUTTON_Y);

  // Spiel initialisieren
  initGame();

  Serial.println("Setup abgeschlossen!");
  Serial.println("Viel Spaß beim Puzzeln!");
}

// ========== Main Loop ==========
void loop() {
  // Animations-Update
  if (animTile >= 0) {
    updateAnimation();
    return;  // Während Animation keine anderen Updates
  }

  if (isPaused) {
    handleTouch();  // Touch-Buttons auch in Pause
    delay(50);
    return;
  }

  if (!gameWon) {
    // Timer aktualisieren (alle 100ms)
    unsigned long now = millis();
    if (now - lastUpdateTime >= 100) {
      lastUpdateTime = now;
      gameDuration = (now - gameStartTime) / 1000;
      drawStats();  // Stats neu zeichnen (nur wenn geändert)
    }

    handleTouch();
  } else {
    handleTouch();  // Nach Gewinn nur Touch (für Neustart)
  }

  delay(10);
}

// ========== Spiel initialisieren ==========
void initGame() {
  // Gelöstes Puzzle erstellen
  for (uint8_t i = 0; i < TILE_COUNT - 1; i++) {
    tiles[i] = i + 1;
  }
  tiles[TILE_COUNT - 1] = 0;  // Letztes Feld leer
  emptyPos = TILE_COUNT - 1;

  // Zurücksetzen
  moveCount = 0;
  gameWon = false;
  isPaused = false;
  animTile = -1;
  moveHistory.clear();
  gameStartTime = millis();
  lastUpdateTime = millis();
  gameDuration = 0;
  lastDrawnMoves = 9999;
  lastDrawnTime = 9999;

  // UI zeichnen
  lcd.fillScreen(COLOR_BACKGROUND);
  drawFrame();
  drawButtons();
  drawBoard();
  drawStats();

  delay(500);  // Kurz gelöstes Puzzle zeigen

  // Puzzle mischen
  shufflePuzzle();

  Serial.println("Neues Spiel gestartet!");
}

// ========== Puzzle mischen (garantiert lösbar) ==========
void shufflePuzzle() {
  // Methode: Zufällige gültige Züge (garantiert lösbar!)
  randomSeed(millis());

  int8_t lastMove = -1;
  for (int i = 0; i < 200; i++) {  // 200 zufällige Züge
    // Finde alle möglichen Züge
    int8_t x, y;
    getXY(emptyPos, x, y);

    std::vector<uint8_t> possibleMoves;
    if (x > 0 && getPos(x - 1, y) != lastMove) possibleMoves.push_back(getPos(x - 1, y));
    if (x < GRID_SIZE - 1 && getPos(x + 1, y) != lastMove) possibleMoves.push_back(getPos(x + 1, y));
    if (y > 0 && getPos(x, y - 1) != lastMove) possibleMoves.push_back(getPos(x, y - 1));
    if (y < GRID_SIZE - 1 && getPos(x, y + 1) != lastMove) possibleMoves.push_back(getPos(x, y + 1));

    if (possibleMoves.size() > 0) {
      uint8_t movePos = possibleMoves[random(possibleMoves.size())];
      lastMove = emptyPos;

      // Sofort bewegen (ohne Animation und History)
      uint8_t temp = tiles[movePos];
      tiles[movePos] = tiles[emptyPos];
      tiles[emptyPos] = temp;
      emptyPos = movePos;
    }
  }

  // Prüfe ob lösbar (sollte immer der Fall sein)
  if (!isSolvable()) {
    Serial.println("WARNUNG: Puzzle nicht lösbar! Nochmal mischen...");
    shufflePuzzle();
    return;
  }

  // Board neu zeichnen
  moveCount = 0;
  moveHistory.clear();
  gameStartTime = millis();
  lastDrawnMoves = 9999;
  lastDrawnTime = 9999;
  drawBoard();
  drawStats();

  Serial.println("Puzzle gemischt!");
}

// ========== Prüfe ob Puzzle lösbar ist ==========
bool isSolvable() {
  // Zähle Inversionen (wie viele Zahlen sind in falscher Reihenfolge?)
  int inversions = 0;
  for (int i = 0; i < TILE_COUNT - 1; i++) {
    if (tiles[i] == 0) continue;
    for (int j = i + 1; j < TILE_COUNT; j++) {
      if (tiles[j] == 0) continue;
      if (tiles[i] > tiles[j]) inversions++;
    }
  }

  // Bei 4x4: Lösbar wenn (Inversionen + Zeile des leeren Feldes) ungerade
  int8_t x, y;
  getXY(emptyPos, x, y);
  int emptyRow = y;

  return ((inversions + emptyRow) % 2 == 1);
}

// ========== Board zeichnen ==========
void drawBoard() {
  for (uint8_t i = 0; i < TILE_COUNT; i++) {
    drawTile(i, true);
  }
}

// ========== Holzrahmen zeichnen (realistischer) ==========
void drawFrame() {
  int16_t x1 = FIELD_X;
  int16_t y1 = FIELD_Y;
  int16_t x2 = FIELD_X + FIELD_SIZE + FRAME_WIDTH * 2;
  int16_t y2 = FIELD_Y + FIELD_SIZE + FRAME_WIDTH * 2;

  // Hauptrahmen (mittelbraun)
  lcd.fillRect(x1, y1, x2 - x1, y2 - y1, COLOR_WOOD_MID);

  // Holzmaserung simulieren (horizontale Linien in verschiedenen Brauntönen)
  for (int16_t i = 0; i < (y2 - y1); i += 3) {
    uint16_t woodColor = (i % 6 == 0) ? COLOR_WOOD_DARK :
                         (i % 6 == 3) ? COLOR_WOOD_LIGHT : COLOR_WOOD_MID;
    lcd.drawFastHLine(x1, y1 + i, x2 - x1, woodColor);
  }

  // 3D-Effekt: Helle Kante oben/links
  for (int16_t i = 0; i < FRAME_WIDTH / 3; i++) {
    lcd.drawFastHLine(x1 + i, y1 + i, x2 - x1 - i * 2, COLOR_WOOD_LIGHT);
    lcd.drawFastVLine(x1 + i, y1 + i, y2 - y1 - i * 2, COLOR_WOOD_LIGHT);
  }

  // 3D-Effekt: Dunkle Kante unten/rechts
  for (int16_t i = 0; i < FRAME_WIDTH / 3; i++) {
    lcd.drawFastHLine(x1 + i, y2 - 1 - i, x2 - x1 - i * 2, COLOR_WOOD_DARK);
    lcd.drawFastVLine(x2 - 1 - i, y1 + i, y2 - y1 - i * 2, COLOR_WOOD_DARK);
  }

  // Innerer schwarzer Rand (Vertiefung)
  int16_t innerX = x1 + FRAME_WIDTH;
  int16_t innerY = y1 + FRAME_WIDTH;
  int16_t innerW = FIELD_SIZE;
  int16_t innerH = FIELD_SIZE;

  lcd.drawRect(innerX, innerY, innerW, innerH, TFT_BLACK);
  lcd.drawRect(innerX + 1, innerY + 1, innerW - 2, innerH - 2, COLOR_WOOD_DARK);
}

// ========== Kachel zeichnen (Original-Design: Rot/Weiß mit Gold) ==========
void drawTile(uint8_t pos, bool immediate) {
  uint8_t tile = tiles[pos];

  int8_t x, y;
  getXY(pos, x, y);

  int16_t px = FIELD_X + FRAME_WIDTH + x * TILE_SIZE;
  int16_t py = FIELD_Y + FRAME_WIDTH + y * TILE_SIZE;

  // Während Animation: Position interpolieren
  if (!immediate && animTile == tile && animProgress < 1.0) {
    int16_t startPx = FIELD_X + FRAME_WIDTH + animStartX * TILE_SIZE;
    int16_t startPy = FIELD_Y + FRAME_WIDTH + animStartY * TILE_SIZE;
    int16_t endPx = FIELD_X + FRAME_WIDTH + animEndX * TILE_SIZE;
    int16_t endPy = FIELD_Y + FRAME_WIDTH + animEndY * TILE_SIZE;

    // Ease-in-out interpolation
    float t = animProgress;
    t = t < 0.5 ? 2 * t * t : 1 - pow(-2 * t + 2, 2) / 2;

    px = startPx + (endPx - startPx) * t;
    py = startPy + (endPy - startPy) * t;
  }

  if (tile == 0) {
    // Leeres Feld - dunkler Hintergrund
    lcd.fillRect(px, py, TILE_SIZE, TILE_SIZE, COLOR_EMPTY);

    // Subtile Vertiefung zeichnen
    lcd.drawRect(px, py, TILE_SIZE, TILE_SIZE, COLOR_SHADOW);
    lcd.drawRect(px + 1, py + 1, TILE_SIZE - 2, TILE_SIZE - 2, COLOR_SHADOW);
  } else {
    // Original-Farbschema: 1,3,6,8,9,11,14 = ROT, Rest = WEISS
    bool isRed = IS_RED_TILE[tile - 1];
    uint16_t tileColor = isRed ? COLOR_TILE_RED : COLOR_TILE_WHITE;

    // Schatten (rechts und unten) - größer für besseren 3D-Effekt
    int16_t shadowOffset = (TILE_SIZE / 20 > 3) ? (TILE_SIZE / 20) : 3;
    lcd.fillRect(px + shadowOffset, py + shadowOffset,
                 TILE_SIZE - shadowOffset + 1, TILE_SIZE - shadowOffset + 1,
                 COLOR_SHADOW);

    // Hauptkachel
    lcd.fillRect(px, py, TILE_SIZE - shadowOffset, TILE_SIZE - shadowOffset, tileColor);

    // 3D-Kanten für Tiefe
    int16_t edgeSize = (TILE_SIZE / 12 > 2) ? (TILE_SIZE / 12) : 2;

    // Helle Kante (oben, links) - für 3D-Effekt
    uint16_t lightEdge = isRed ? 0xF800 : TFT_WHITE;  // Helles Rot oder Weiß
    for (int16_t i = 0; i < edgeSize; i++) {
      lcd.drawFastHLine(px + i, py + i, TILE_SIZE - shadowOffset - i * 2, lightEdge);
      lcd.drawFastVLine(px + i, py + i, TILE_SIZE - shadowOffset - i * 2, lightEdge);
    }

    // Dunkle Kante (unten, rechts)
    uint16_t darkEdge = isRed ? 0x8000 : 0xCE59;  // Dunkles Rot oder Hellgrau
    for (int16_t i = 0; i < edgeSize; i++) {
      lcd.drawFastHLine(px + i, py + TILE_SIZE - shadowOffset - 1 - i,
                       TILE_SIZE - shadowOffset - i * 2, darkEdge);
      lcd.drawFastVLine(px + TILE_SIZE - shadowOffset - 1 - i, py + i,
                       TILE_SIZE - shadowOffset - i * 2, darkEdge);
    }

    // Goldene Zahl zeichnen
    int fontScale = (TILE_SIZE / 35 > 1) ? (TILE_SIZE / 35) : 1;

    // Text-Schatten (schwarz)
    lcd.setTextColor(COLOR_SHADOW);
    lcd.drawNumber(tile, px + TILE_SIZE / 2 + 2, py + TILE_SIZE / 2 + 2, 4 * fontScale);

    // Haupttext (gold)
    lcd.setTextColor(COLOR_NUMBER_GOLD);
    lcd.drawNumber(tile, px + TILE_SIZE / 2, py + TILE_SIZE / 2, 4 * fontScale);
  }
}

// ========== Touch-Buttons zeichnen ==========
void drawButtons() {
  const char* labels[4] = {"NEU", "UNDO", "PAUSE", "HELP"};

  for (int i = 0; i < 4; i++) {
    int16_t by = BUTTON_Y + i * (BUTTON_HEIGHT + 10);

    // Button-Hintergrund
    lcd.fillRect(BUTTON_X, by, BUTTON_WIDTH, BUTTON_HEIGHT, COLOR_BUTTON_BG);

    // 3D-Rahmen
    lcd.drawRect(BUTTON_X, by, BUTTON_WIDTH, BUTTON_HEIGHT, COLOR_BUTTON_BORDER);

    // Helle Kante oben/links (3D-Effekt)
    lcd.drawFastHLine(BUTTON_X + 1, by + 1, BUTTON_WIDTH - 2, TFT_WHITE);
    lcd.drawFastVLine(BUTTON_X + 1, by + 1, BUTTON_HEIGHT - 2, TFT_WHITE);

    // Dunkle Kante unten/rechts
    lcd.drawFastHLine(BUTTON_X + 1, by + BUTTON_HEIGHT - 2, BUTTON_WIDTH - 2, TFT_BLACK);
    lcd.drawFastVLine(BUTTON_X + BUTTON_WIDTH - 2, by + 1, BUTTON_HEIGHT - 2, TFT_BLACK);

    // Text
    lcd.setTextColor(COLOR_BUTTON_TEXT);
    lcd.setTextDatum(MC_DATUM);

    int fontSize = (BUTTON_HEIGHT > 40) ? 2 : 1;
    lcd.drawString(labels[i], BUTTON_X + BUTTON_WIDTH / 2, by + BUTTON_HEIGHT / 2, fontSize);
  }
}

// ========== Statistiken zeichnen (OHNE Flackern) ==========
void drawStats() {
  // Nur neu zeichnen wenn sich etwas geändert hat!
  if (moveCount == lastDrawnMoves && gameDuration == lastDrawnTime && !isPaused) {
    return;  // Nichts geändert, nicht neu zeichnen
  }

  // Header-Bereich löschen (nur den Teil der sich ändert)
  lcd.fillRect(0, 0, lcd.width(), 55, COLOR_BACKGROUND);

  lcd.setTextDatum(TL_DATUM);
  lcd.setTextColor(TFT_BLACK);

  // Titel (nur einmal zeichnen, ändert sich nicht)
  lcd.drawString("15-PUZZLE", 10, 5, 4);

  // Züge (rechts oben)
  lcd.setTextDatum(TR_DATUM);
  String moves = "Zuege: " + String(moveCount);
  lcd.drawString(moves, lcd.width() - 10, 5, 2);

  // Timer
  int minutes = gameDuration / 60;
  int seconds = gameDuration % 60;
  String timeStr = "Zeit: " + String(minutes) + ":" + (seconds < 10 ? "0" : "") + String(seconds);
  lcd.drawString(timeStr, lcd.width() - 10, 25, 2);

  // Bester Score
  if (bestScore < 9999) {
    String best = "Best: " + String(bestScore) + " Zuege";
    lcd.drawString(best, lcd.width() - 10, 43, 2);
  }

  // Pause-Anzeige
  if (isPaused) {
    lcd.setTextDatum(MC_DATUM);
    lcd.setTextColor(TFT_RED);
    lcd.drawString("PAUSE", lcd.width() / 2, 37, 4);  // 7 Pixel tiefer
  }

  // Cache aktualisieren
  lastDrawnMoves = moveCount;
  lastDrawnTime = gameDuration;

  lcd.setTextDatum(MC_DATUM);  // Zurücksetzen
}

// ========== Button-Press Check ==========
bool checkButtonPress(uint16_t x, uint16_t y, int buttonIndex) {
  int16_t by = BUTTON_Y + buttonIndex * (BUTTON_HEIGHT + 10);

  return (x >= BUTTON_X && x <= BUTTON_X + BUTTON_WIDTH &&
          y >= by && y <= by + BUTTON_HEIGHT);
}

// ========== Touch-Eingabe verarbeiten ==========
void handleTouch() {
  if (animTile >= 0) return;  // Während Animation kein Touch

  uint16_t touchX, touchY;
  if (lcd.getTouch(&touchX, &touchY)) {
    // Debounce
    if (millis() - lastTouchTime < TOUCH_DEBOUNCE) return;
    lastTouchTime = millis();

    // Touch-Buttons prüfen
    if (touchX >= BUTTON_X && touchX <= BUTTON_X + BUTTON_WIDTH) {
      if (checkButtonPress(touchX, touchY, 0)) {
        // Button "NEU" gedrückt
        Serial.println("Button NEU gedrückt");
        initGame();
        return;
      } else if (checkButtonPress(touchX, touchY, 1)) {
        // Button "UNDO" gedrückt
        Serial.println("Button UNDO gedrückt");
        undoMove();
        return;
      } else if (checkButtonPress(touchX, touchY, 2)) {
        // Button "PAUSE" gedrückt
        Serial.println("Button PAUSE gedrückt");
        isPaused = !isPaused;
        lastDrawnTime = 9999;  // Force redraw
        drawStats();
        return;
      } else if (checkButtonPress(touchX, touchY, 3)) {
        // Button "HELP" gedrückt
        Serial.println("Button HELP gedrückt");
        showHint();
        return;
      }
    }

    // Prüfe ob Touch im Spielfeld
    if (touchX < FIELD_X + FRAME_WIDTH ||
        touchX > FIELD_X + FRAME_WIDTH + FIELD_SIZE ||
        touchY < FIELD_Y + FRAME_WIDTH ||
        touchY > FIELD_Y + FRAME_WIDTH + FIELD_SIZE) {
      return;
    }

    // Welche Kachel wurde berührt?
    int8_t tileX = (touchX - FIELD_X - FRAME_WIDTH) / TILE_SIZE;
    int8_t tileY = (touchY - FIELD_Y - FRAME_WIDTH) / TILE_SIZE;

    if (tileX >= 0 && tileX < GRID_SIZE && tileY >= 0 && tileY < GRID_SIZE) {
      uint8_t pos = getPos(tileX, tileY);

      if (canMove(pos)) {
        moveTile(pos);
      }
    }
  }
}

// ========== Hinweis zeigen (markiert bewegbare Kacheln) ==========
void showHint() {
  Serial.println("Zeige Hinweis: Markiere bewegbare Kacheln");

  // Finde alle bewegbaren Kacheln
  std::vector<uint8_t> movableTiles;
  for (uint8_t i = 0; i < TILE_COUNT; i++) {
    if (tiles[i] != 0 && canMove(i)) {
      movableTiles.push_back(i);
    }
  }

  if (movableTiles.empty()) {
    Serial.println("Keine bewegbaren Kacheln gefunden!");
    return;
  }

  // Blinke 3 mal
  for (int blink = 0; blink < 3; blink++) {
    // Grünen Rahmen um bewegbare Kacheln zeichnen
    for (uint8_t pos : movableTiles) {
      int8_t x, y;
      getXY(pos, x, y);

      int16_t px = FIELD_X + FRAME_WIDTH + x * TILE_SIZE;
      int16_t py = FIELD_Y + FRAME_WIDTH + y * TILE_SIZE;

      // Dicker grüner Rahmen (4 Pixel)
      for (int i = 0; i < 4; i++) {
        lcd.drawRect(px + i, py + i, TILE_SIZE - i * 2, TILE_SIZE - i * 2, TFT_GREEN);
      }
    }

    delay(400);

    // Rahmen wieder entfernen (Kacheln neu zeichnen)
    for (uint8_t pos : movableTiles) {
      drawTile(pos, true);
    }

    delay(200);
  }

  Serial.printf("Hinweis: %d bewegbare Kacheln gefunden\n", movableTiles.size());
}

// ========== Prüfe ob gewonnen ==========
bool checkWin() {
  for (uint8_t i = 0; i < TILE_COUNT - 1; i++) {
    if (tiles[i] != i + 1) return false;
  }
  return (tiles[TILE_COUNT - 1] == 0);
}

// ========== Gewinn-Animation ==========
void showWinAnimation() {
  gameWon = true;

  // Update bester Score
  if (moveCount < bestScore) {
    bestScore = moveCount;
  }

  Serial.printf("GEWONNEN in %d Zügen! (%d:%02d)\n",
                moveCount, gameDuration / 60, gameDuration % 60);

  // Feuerwerk-Animation
  for (int round = 0; round < 3; round++) {
    for (int i = 0; i < 15; i++) {
      int16_t x = random(FIELD_X + FRAME_WIDTH, FIELD_X + FRAME_WIDTH + FIELD_SIZE);
      int16_t y = random(FIELD_Y + FRAME_WIDTH, FIELD_Y + FRAME_WIDTH + FIELD_SIZE);
      int16_t radius = random(3, 10);
      uint16_t color = (i % 2 == 0) ? COLOR_TILE_RED : COLOR_NUMBER_GOLD;

      lcd.fillCircle(x, y, radius, color);
      delay(40);
    }
    delay(150);
    drawBoard();  // Board neu zeichnen
  }

  // Gewinn-Text
  lcd.setTextDatum(MC_DATUM);
  lcd.setTextColor(COLOR_WIN_TEXT);
  lcd.fillRect(FIELD_X + FRAME_WIDTH, FIELD_Y + FIELD_SIZE / 2 - 30,
               FIELD_SIZE, 70, COLOR_BACKGROUND);
  lcd.drawString("GEWONNEN!", FIELD_X + FRAME_WIDTH + FIELD_SIZE / 2,
                 FIELD_Y + FIELD_SIZE / 2 - 10, 4);

  String stats = String(moveCount) + " Zuege in " +
                 String(gameDuration / 60) + ":" +
                 (gameDuration % 60 < 10 ? "0" : "") +
                 String(gameDuration % 60);
  lcd.setTextColor(TFT_BLACK);
  lcd.drawString(stats, FIELD_X + FRAME_WIDTH + FIELD_SIZE / 2,
                 FIELD_Y + FIELD_SIZE / 2 + 20, 2);

  lcd.drawString("Tippe NEU zum Neustart", FIELD_X + FRAME_WIDTH + FIELD_SIZE / 2,
                 FIELD_Y + FIELD_SIZE / 2 + 40, 2);

  // Warte auf NEU-Button
  delay(1000);
}

// ========== Hilfsfunktionen ==========

// Position -> X,Y Koordinaten
void getXY(uint8_t pos, int8_t &x, int8_t &y) {
  x = pos % GRID_SIZE;
  y = pos / GRID_SIZE;
}

// X,Y Koordinaten -> Position
uint8_t getPos(int8_t x, int8_t y) {
  return y * GRID_SIZE + x;
}

// Prüfe ob Kachel bewegt werden kann
bool canMove(uint8_t pos) {
  int8_t x, y;
  getXY(pos, x, y);

  int8_t emptyX, emptyY;
  getXY(emptyPos, emptyX, emptyY);

  // Gleiche Zeile oder Spalte?
  if (x == emptyX) {
    return (abs(y - emptyY) == 1);
  } else if (y == emptyY) {
    return (abs(x - emptyX) == 1);
  }

  return false;
}

// Kachel bewegen
void moveTile(uint8_t pos, bool addToHistory) {
  if (!canMove(pos)) return;

  // Animation vorbereiten
  animTile = tiles[pos];
  getXY(pos, animStartX, animStartY);
  getXY(emptyPos, animEndX, animEndY);
  animProgress = 0.0;

  // History speichern
  if (addToHistory && moveHistory.size() < MAX_HISTORY) {
    Move move;
    move.tile = tiles[pos];
    move.from = pos;
    move.to = emptyPos;
    moveHistory.push_back(move);
  }

  // Tausche Kacheln
  uint8_t temp = tiles[pos];
  tiles[pos] = tiles[emptyPos];
  tiles[emptyPos] = temp;

  uint8_t oldEmpty = emptyPos;
  emptyPos = pos;

  // Beide Positionen neu zeichnen (Hintergrund)
  drawTile(oldEmpty, true);  // Neue Position der Kachel
  drawTile(emptyPos, true);  // Neues leeres Feld

  moveCount++;
  lastDrawnMoves = 9999;  // Force stats redraw
}

// Letzten Zug rückgängig machen
void undoMove() {
  if (moveHistory.empty()) {
    Serial.println("Keine Züge zum Rückgängigmachen");
    return;
  }

  Move lastMove = moveHistory.back();
  moveHistory.pop_back();

  // Finde die Kachel
  uint8_t tilePos = 0;
  for (uint8_t i = 0; i < TILE_COUNT; i++) {
    if (tiles[i] == lastMove.tile) {
      tilePos = i;
      break;
    }
  }

  // Bewege zurück (ohne History und ohne Animation)
  uint8_t temp = tiles[tilePos];
  tiles[tilePos] = tiles[emptyPos];
  tiles[emptyPos] = temp;
  emptyPos = tilePos;

  moveCount--;

  // Board neu zeichnen
  drawBoard();
  lastDrawnMoves = 9999;  // Force stats redraw
  drawStats();

  Serial.println("Zug rückgängig gemacht");
}

// Animation aktualisieren
void updateAnimation() {
  if (animTile < 0) return;

  animProgress += ANIM_SPEED;

  if (animProgress >= 1.0) {
    // Animation beendet
    animProgress = 1.0;

    // Finale Positionen zeichnen
    for (uint8_t i = 0; i < TILE_COUNT; i++) {
      if (tiles[i] == animTile || tiles[i] == 0) {
        drawTile(i, true);
      }
    }

    animTile = -1;

    // Win-Check
    if (checkWin()) {
      showWinAnimation();
    } else {
      drawStats();  // Stats aktualisieren
    }
  } else {
    // Animation läuft - Kachel an interpolierter Position zeichnen

    // Alte Position löschen (leeres Feld zeichnen)
    int16_t startPx = FIELD_X + FRAME_WIDTH + animStartX * TILE_SIZE;
    int16_t startPy = FIELD_Y + FRAME_WIDTH + animStartY * TILE_SIZE;
    lcd.fillRect(startPx, startPy, TILE_SIZE, TILE_SIZE, COLOR_EMPTY);

    // Neue Position löschen (leeres Feld)
    int16_t endPx = FIELD_X + FRAME_WIDTH + animEndX * TILE_SIZE;
    int16_t endPy = FIELD_Y + FRAME_WIDTH + animEndY * TILE_SIZE;
    lcd.fillRect(endPx, endPy, TILE_SIZE, TILE_SIZE, COLOR_EMPTY);

    // Kachel an interpolierter Position zeichnen
    for (uint8_t i = 0; i < TILE_COUNT; i++) {
      if (tiles[i] == animTile) {
        drawTile(i, false);
        break;
      }
    }
  }

  delay(16);  // ~60 FPS
}

// Farben interpolieren
uint16_t interpolateColor(uint16_t color1, uint16_t color2, float t) {
  // RGB565 -> RGB888
  uint8_t r1 = ((color1 >> 11) & 0x1F) << 3;
  uint8_t g1 = ((color1 >> 5) & 0x3F) << 2;
  uint8_t b1 = (color1 & 0x1F) << 3;

  uint8_t r2 = ((color2 >> 11) & 0x1F) << 3;
  uint8_t g2 = ((color2 >> 5) & 0x3F) << 2;
  uint8_t b2 = (color2 & 0x1F) << 3;

  // Interpolieren
  uint8_t r = r1 + (r2 - r1) * t;
  uint8_t g = g1 + (g2 - g1) * t;
  uint8_t b = b1 + (b2 - b1) * t;

  // RGB888 -> RGB565
  return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}
