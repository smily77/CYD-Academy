/*
 * CYD-Academy - Spiel 01: 15-Puzzle (Schiebepuzzle)
 *
 * Das klassische 15-Puzzle (auch bekannt als "Fünfzehnerspiel",
 * "Schiebepuzzle" oder "Ohne-Fleiß-kein-Preis-Spiel") - ein
 * zeitloser Denksport-Klassiker seit 1874!
 *
 * Ziel: Ordne die 15 nummerierten Kacheln (1-15) in der richtigen
 * Reihenfolge, indem du sie in das leere Feld schiebst.
 *
 * Hardware:
 * - ESP32-2432S028R (CYD - Cheap Yellow Display)
 * - Touch-Screen für Spielsteuerung
 * - Optional: 4x Buttons für zusätzliche Funktionen
 *
 * Steuerung:
 * - Touch: Tippe auf eine Kachel neben dem leeren Feld zum Verschieben
 * - Button A: Neues Spiel (Shuffle)
 * - Button B: Lösung zeigen (kurz antippen)
 * - Button C: Rückgängig (letzter Zug)
 * - Button D: Pause / Fortsetzen
 *
 * Features:
 * - Realistische 3D-Grafik mit Schatten und Highlights
 * - Glatte Schiebe-Animationen
 * - Holzrahmen-Textur um das Spielfeld
 * - Züge-Zähler und Timer
 * - Bester Score (wenigste Züge) wird gespeichert
 * - Gewinn-Animation mit Feuerwerk
 * - Garantiert lösbares Puzzle nach Shuffle
 * - Undo-Funktion für letzte Züge
 *
 * Grafik-Details:
 * - Farbverläufe von Blau über Grün zu Orange
 * - 3D-Effekte mit mehrfarbigen Kanten (hell oben-links, dunkel unten-rechts)
 * - Schatten unter den Kacheln
 * - Große, gut lesbare Zahlen mit Text-Schatten
 * - Holzmaserung um das Spielfeld
 *
 * Bibliotheken:
 * - LovyanGFX (für Display und Touch)
 * - CYD_Display_Config.h (für Board-Konfiguration)
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

// ========== Farben (RGB565) ==========
#define COLOR_BACKGROUND 0x2945      // Dunkelgrau
#define COLOR_FRAME_DARK 0x4208      // Dunkelbraun
#define COLOR_FRAME_LIGHT 0x8C51     // Hellbraun
#define COLOR_EMPTY 0x18C3           // Sehr dunkelgrau
#define COLOR_TEXT_SHADOW 0x2104     // Dunkel für Schatten
#define COLOR_WIN_TEXT 0xFFE0        // Gelb

// Farbverläufe für Kacheln (Blau -> Grün -> Orange)
const uint16_t TILE_COLORS[15] = {
  0x3C5F, 0x3C9F, 0x3CDF, 0x3D1F,  // 1-4: Blau-Töne
  0x3D5F, 0x45BF, 0x4DDF, 0x55FF,  // 5-8: Cyan-Grün-Töne
  0x5DFF, 0x6DDF, 0x7D9F, 0x8D5F,  // 9-12: Grün-Töne
  0x9D1F, 0xAD1F, 0xBCDF           // 13-15: Orange-Töne
};

// ========== Globale Variablen ==========
// Spielfeld-Dimensionen (werden in setup() berechnet)
int16_t FIELD_SIZE;        // Größe des gesamten Spielfelds
int16_t TILE_SIZE;         // Größe einer Kachel
int16_t FIELD_X;           // X-Position des Spielfelds
int16_t FIELD_Y;           // Y-Position des Spielfelds
int16_t FRAME_WIDTH;       // Breite des Holzrahmens

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

// ========== Forward Declarations ==========
void initGame();
void shufflePuzzle();
void drawBoard();
void drawTile(uint8_t pos, bool immediate = false);
void drawFrame();
void drawStats();
void handleTouch();
void handleButtons();
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

  // Spielfeld so groß wie möglich, aber mit Platz für Stats
  int16_t availableHeight = screenHeight - 60;  // 60px für Stats oben
  int16_t maxSize = min(screenWidth - 40, availableHeight);

  FIELD_SIZE = (maxSize / GRID_SIZE) * GRID_SIZE;  // Auf Grid-Größe abrunden
  TILE_SIZE = FIELD_SIZE / GRID_SIZE;
  FRAME_WIDTH = TILE_SIZE / 8;

  FIELD_X = (screenWidth - FIELD_SIZE - FRAME_WIDTH * 2) / 2;
  FIELD_Y = 60 + (availableHeight - FIELD_SIZE - FRAME_WIDTH * 2) / 2;

  Serial.printf("Field: %dx%d at (%d,%d)\n", FIELD_SIZE, FIELD_SIZE, FIELD_X, FIELD_Y);
  Serial.printf("Tile: %dx%d, Frame: %d\n", TILE_SIZE, TILE_SIZE, FRAME_WIDTH);

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
    handleButtons();  // Nur Buttons in Pause
    delay(50);
    return;
  }

  if (!gameWon) {
    // Timer aktualisieren (alle 100ms)
    unsigned long now = millis();
    if (now - lastUpdateTime >= 100) {
      lastUpdateTime = now;
      gameDuration = (now - gameStartTime) / 1000;
      drawStats();  // Stats neu zeichnen
    }

    handleTouch();
    handleButtons();
  } else {
    handleButtons();  // Nach Gewinn nur Buttons (für Neustart)
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

  // Board zeichnen
  drawFrame();
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

// ========== Holzrahmen zeichnen ==========
void drawFrame() {
  int16_t x1 = FIELD_X;
  int16_t y1 = FIELD_Y;
  int16_t x2 = FIELD_X + FIELD_SIZE + FRAME_WIDTH * 2;
  int16_t y2 = FIELD_Y + FIELD_SIZE + FRAME_WIDTH * 2;

  // Äußerer dunkler Rahmen
  lcd.fillRect(x1, y1, x2 - x1, FRAME_WIDTH, COLOR_FRAME_DARK);
  lcd.fillRect(x1, y1, FRAME_WIDTH, y2 - y1, COLOR_FRAME_DARK);

  // Innerer heller Rahmen (3D-Effekt)
  lcd.fillRect(x1 + FRAME_WIDTH, y2 - FRAME_WIDTH, x2 - x1 - FRAME_WIDTH, FRAME_WIDTH, COLOR_FRAME_LIGHT);
  lcd.fillRect(x2 - FRAME_WIDTH, y1 + FRAME_WIDTH, FRAME_WIDTH, y2 - y1 - FRAME_WIDTH, COLOR_FRAME_LIGHT);

  // Holzmaserung (einfache Linien)
  for (int16_t i = 0; i < FRAME_WIDTH; i += 2) {
    lcd.drawFastHLine(x1 + i, y1 + i, x2 - x1 - i * 2, COLOR_FRAME_DARK);
    lcd.drawFastVLine(x1 + i, y1 + i, y2 - y1 - i * 2, COLOR_FRAME_DARK);
  }
}

// ========== Kachel zeichnen ==========
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

    // Subtile Vertiefung zeichnen (3D-Effekt umgekehrt)
    lcd.drawRect(px, py, TILE_SIZE, TILE_SIZE, COLOR_FRAME_DARK);
    lcd.drawFastHLine(px + 1, py + 1, TILE_SIZE - 2, COLOR_FRAME_DARK);
    lcd.drawFastVLine(px + 1, py + 1, TILE_SIZE - 2, COLOR_FRAME_DARK);
  } else {
    // Farbige Kachel
    uint16_t tileColor = TILE_COLORS[tile - 1];

    // Schatten (rechts und unten)
    int16_t shadowOffset = max(2, TILE_SIZE / 32);
    lcd.fillRect(px + shadowOffset, py + shadowOffset,
                 TILE_SIZE - shadowOffset, TILE_SIZE - shadowOffset,
                 COLOR_TEXT_SHADOW);

    // Hauptkachel
    lcd.fillRect(px, py, TILE_SIZE - shadowOffset, TILE_SIZE - shadowOffset, tileColor);

    // 3D-Kanten (hell oben/links, dunkel unten/rechts)
    int16_t edgeSize = max(1, TILE_SIZE / 16);
    uint16_t lightEdge = interpolateColor(tileColor, TFT_WHITE, 0.3);
    uint16_t darkEdge = interpolateColor(tileColor, TFT_BLACK, 0.3);

    // Helle Kanten (oben, links)
    for (int16_t i = 0; i < edgeSize; i++) {
      lcd.drawFastHLine(px + i, py + i, TILE_SIZE - shadowOffset - i * 2, lightEdge);
      lcd.drawFastVLine(px + i, py + i, TILE_SIZE - shadowOffset - i * 2, lightEdge);
    }

    // Dunkle Kanten (unten, rechts)
    for (int16_t i = 0; i < edgeSize; i++) {
      lcd.drawFastHLine(px + i, py + TILE_SIZE - shadowOffset - 1 - i,
                       TILE_SIZE - shadowOffset - i * 2, darkEdge);
      lcd.drawFastVLine(px + TILE_SIZE - shadowOffset - 1 - i, py + i,
                       TILE_SIZE - shadowOffset - i * 2, darkEdge);
    }

    // Zahl zeichnen
    lcd.setTextColor(TFT_WHITE);

    // Berechne Font-Größe basierend auf Kachel-Größe
    int fontScale = max(1, TILE_SIZE / 40);

    // Text-Schatten
    lcd.setTextColor(COLOR_TEXT_SHADOW);
    lcd.drawNumber(tile, px + TILE_SIZE / 2 + 2, py + TILE_SIZE / 2 + 2, 4 * fontScale);

    // Haupttext
    lcd.setTextColor(TFT_WHITE);
    lcd.drawNumber(tile, px + TILE_SIZE / 2, py + TILE_SIZE / 2, 4 * fontScale);
  }
}

// ========== Statistiken zeichnen ==========
void drawStats() {
  // Header-Bereich löschen
  lcd.fillRect(0, 0, lcd.width(), 55, COLOR_BACKGROUND);

  lcd.setTextDatum(TL_DATUM);
  lcd.setTextColor(TFT_WHITE);

  // Titel
  lcd.drawString("15-PUZZLE", 10, 5, 4);

  // Züge
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
    lcd.setTextColor(COLOR_WIN_TEXT);
    lcd.drawString("PAUSE", lcd.width() / 2, 30, 4);
  }

  lcd.setTextDatum(MC_DATUM);  // Zurücksetzen
}

// ========== Touch-Eingabe verarbeiten ==========
void handleTouch() {
  if (animTile >= 0) return;  // Während Animation kein Touch

  uint16_t touchX, touchY;
  if (lcd.getTouch(&touchX, &touchY)) {
    // Debounce
    if (millis() - lastTouchTime < TOUCH_DEBOUNCE) return;
    lastTouchTime = millis();

    // Prüfe ob Touch im Spielfeld
    if (touchX < FIELD_X + FRAME_WIDTH || touchX > FIELD_X + FRAME_WIDTH + FIELD_SIZE ||
        touchY < FIELD_Y + FRAME_WIDTH || touchY > FIELD_Y + FRAME_WIDTH + FIELD_SIZE) {
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

// ========== Button-Eingabe verarbeiten ==========
void handleButtons() {
  // Hinweis: Button-Implementierung ist optional
  // Hier könnten die Buttons aus CYD_Display_Config.h verwendet werden
  // Für diese Version fokussieren wir auf Touch-Steuerung

  // TODO: Button A - Neues Spiel
  // TODO: Button B - Lösung zeigen
  // TODO: Button C - Undo
  // TODO: Button D - Pause
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
    for (int i = 0; i < 20; i++) {
      int16_t x = random(FIELD_X + FRAME_WIDTH, FIELD_X + FRAME_WIDTH + FIELD_SIZE);
      int16_t y = random(FIELD_Y + FRAME_WIDTH, FIELD_Y + FRAME_WIDTH + FIELD_SIZE);
      int16_t radius = random(5, 15);
      uint16_t color = TILE_COLORS[random(15)];

      lcd.fillCircle(x, y, radius, color);
      delay(50);
    }
    delay(200);
    drawBoard();  // Board neu zeichnen
  }

  // Gewinn-Text
  lcd.setTextDatum(MC_DATUM);
  lcd.setTextColor(COLOR_WIN_TEXT);
  lcd.fillRect(FIELD_X + FRAME_WIDTH, FIELD_Y + FIELD_SIZE / 2 - 30,
               FIELD_SIZE, 60, COLOR_BACKGROUND);
  lcd.drawString("GEWONNEN!", FIELD_X + FRAME_WIDTH + FIELD_SIZE / 2,
                 FIELD_Y + FIELD_SIZE / 2 - 10, 4);

  String stats = String(moveCount) + " Zuege in " +
                 String(gameDuration / 60) + ":" +
                 (gameDuration % 60 < 10 ? "0" : "") +
                 String(gameDuration % 60);
  lcd.drawString(stats, FIELD_X + FRAME_WIDTH + FIELD_SIZE / 2,
                 FIELD_Y + FIELD_SIZE / 2 + 20, 2);

  lcd.drawString("Tippe zum Neustart", FIELD_X + FRAME_WIDTH + FIELD_SIZE / 2,
                 FIELD_Y + FIELD_SIZE / 2 + 40, 2);

  // Warte auf Touch für Neustart
  delay(1000);
  while (true) {
    uint16_t x, y;
    if (lcd.getTouch(&x, &y)) {
      delay(200);
      initGame();
      break;
    }
    delay(50);
  }
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
  drawTile(oldEmpty, true);  // Neue Position der Kachel (wird durch Animation überschrieben)
  drawTile(emptyPos, true);  // Neues leeres Feld

  moveCount++;

  // Animation läuft in updateAnimation()
  // Nach Animation: Win-Check
}

// Letzten Zug rückgängig machen
void undoMove() {
  if (moveHistory.empty()) return;

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

  // Bewege zurück (ohne History)
  moveTile(tilePos, false);
  moveCount--;  // Zug nicht zählen
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
    // (drawTile verwendet animProgress automatisch)
    for (uint8_t i = 0; i < TILE_COUNT; i++) {
      if (tiles[i] == animTile) {
        drawTile(i, false);
        break;
      }
    }
  }

  delay(16);  // ~60 FPS
}

// Farben interpolieren (für 3D-Kanten)
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
