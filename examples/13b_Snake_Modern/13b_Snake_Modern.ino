/*
  Snake Game MODERN - Modernes Snake mit Fancy Design

  Hardware:
  - CYD Display (320x240)
  - 4x Hardware-Buttons:
    * GPIO 5:  Links
    * GPIO 17: Oben
    * GPIO 16: Rechts
    * GPIO 18: Unten
    (Buttons sind LOW wenn gedrückt)

  Design-Features:
  - Gradient-Hintergrund (Dunkelgrün bis Schwarz)
  - 3D-Schlange mit Schatten und Gradient
  - Neon-Glow-Effekte für Schlangenkopf
  - Pulsierendes Food mit Glow-Animationen
  - Partikeleffekt beim Food-Essen
  - Moderne Neon-Wände mit Glow
  - Animierte UI-Elemente
  - Moderner Game Over Screen
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
#define GRID_SIZE 10
#define GRID_WIDTH  (SCREEN_WIDTH / GRID_SIZE)   // 32
#define GRID_HEIGHT (SCREEN_HEIGHT / GRID_SIZE)  // 24

// ===== MODERNE FARBPALETTE =====
// Hintergrund: Dunkler Gradient
#define COLOR_BG_TOP     0x0410  // Dunkelgrün
#define COLOR_BG_BOTTOM  0x0000  // Schwarz

// Neon-Farben
#define COLOR_NEON_GREEN   0x07E0  // Helles Grün
#define COLOR_NEON_LIME    0xBFE0  // Limette
#define COLOR_NEON_CYAN    0x07FF  // Cyan
#define COLOR_NEON_ORANGE  0xFD20  // Orange
#define COLOR_NEON_RED     0xF800  // Rot
#define COLOR_NEON_PINK    0xF81F  // Pink
#define COLOR_NEON_YELLOW  0xFFE0  // Gelb

// Spielelemente
#define COLOR_SNAKE_BODY   COLOR_NEON_GREEN
#define COLOR_SNAKE_HEAD   COLOR_NEON_CYAN
#define COLOR_FOOD_BASE    COLOR_NEON_ORANGE
#define COLOR_WALL         COLOR_NEON_CYAN
#define COLOR_TEXT         0xFFFF

// Schatten und Highlights
#define COLOR_SHADOW       0x2104
#define COLOR_HIGHLIGHT    0xFFFF

// Richtungen
enum Direction {
  DIR_NONE = 0,
  DIR_UP,
  DIR_DOWN,
  DIR_LEFT,
  DIR_RIGHT
};

// Snake-Segment Position
struct Position {
  int x, y;
};

// Partikel für Food-Eat-Effekt
struct Particle {
  float x, y;
  float oldX, oldY;  // Alte Position zum Löschen
  float vx, vy;
  int life;
  uint16_t color;
  bool active;
};

// Game State
#define MAX_SNAKE_LENGTH (GRID_WIDTH * GRID_HEIGHT)
Position snake[MAX_SNAKE_LENGTH];
int snakeLength = 3;
Direction currentDirection = DIR_RIGHT;
Direction nextDirection = DIR_RIGHT;

Position food;
int score = 0;
int lastScore = -1;  // Für Score-Update-Erkennung
int lastSnakeLength = -1;
int lastMoveDelay = -1;
bool gameOver = false;

// Partikel-System
#define MAX_PARTICLES 30
Particle particles[MAX_PARTICLES];

// Timing
unsigned long lastMoveTime = 0;
int moveDelay = 200;

// Button Debouncing
unsigned long lastButtonPress = 0;
#define BUTTON_DEBOUNCE 50

// Animationen
float glowPhase = 0;
float foodPulse = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== SNAKE GAME MODERN ===\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);
  lcd.setBrightness(255);

  // Button Pins konfigurieren
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);

  // Zufallsgenerator
  randomSeed(analogRead(34) + micros());

  // Partikel initialisieren
  for (int i = 0; i < MAX_PARTICLES; i++) {
    particles[i].active = false;
  }

  // Spiel initialisieren
  initGame();

  Serial.println("Modern Snake gestartet!");
  Serial.println("Steuerung: GPIO 5, 17, 16, 18");
}

void loop() {
  // Animationen aktualisieren
  glowPhase += 0.08;
  if (glowPhase > 6.28) glowPhase = 0;

  foodPulse += 0.1;
  if (foodPulse > 6.28) foodPulse = 0;

  if (gameOver) {
    // Warte auf Button-Druck zum Neustart
    if (checkAnyButton()) {
      initGame();
      gameOver = false;
      delay(300);
    }
    return;
  }

  // Button-Input verarbeiten
  readButtons();

  // Snake bewegen (zeitgesteuert)
  if (millis() - lastMoveTime > moveDelay) {
    lastMoveTime = millis();

    currentDirection = nextDirection;
    moveSnake();

    if (checkCollision()) {
      handleGameOver();
      return;
    }

    if (checkFood()) {
      eatFood();
    }

    drawGame();
  }

  // Partikel aktualisieren und zeichnen (immer)
  updateParticles();
  drawParticles();

  // Wände neu zeichnen (damit Partikel sie nicht löschen)
  // Rahmen hat Aussparung für Score, daher kein Konflikt mehr
  drawModernBorder();

  // Score nur neu zeichnen wenn sich etwas geändert hat
  if (score != lastScore || snakeLength != lastSnakeLength || moveDelay != lastMoveDelay) {
    drawModernScore();
    lastScore = score;
    lastSnakeLength = snakeLength;
    lastMoveDelay = moveDelay;
  }

  delay(16);  // ~60 FPS
}

// ===== INITIALISIERUNG =====

void initGame() {
  Serial.println("Initialisiere Spiel...");

  // Snake in der Mitte starten
  snakeLength = 3;
  snake[0].x = GRID_WIDTH / 2;
  snake[0].y = GRID_HEIGHT / 2;
  snake[1].x = snake[0].x - 1;
  snake[1].y = snake[0].y;
  snake[2].x = snake[1].x - 1;
  snake[2].y = snake[1].y;

  currentDirection = DIR_RIGHT;
  nextDirection = DIR_RIGHT;

  score = 0;
  moveDelay = 200;

  // Score-Tracking zurücksetzen
  lastScore = score;
  lastSnakeLength = snakeLength;
  lastMoveDelay = moveDelay;

  // Food platzieren
  spawnFood();

  // Bildschirm aufbauen
  drawGradientBackground();
  drawModernBorder();
  drawModernScore();
  drawGame();

  Serial.printf("Snake Position: (%d, %d)\n", snake[0].x, snake[0].y);
  Serial.printf("Food Position: (%d, %d)\n", food.x, food.y);
}

void spawnFood() {
  bool validPosition = false;

  while (!validPosition) {
    food.x = random(1, GRID_WIDTH - 1);
    food.y = random(1, GRID_HEIGHT - 1);

    // Prüfen ob Food nicht auf Snake liegt
    validPosition = true;
    for (int i = 0; i < snakeLength; i++) {
      if (snake[i].x == food.x && snake[i].y == food.y) {
        validPosition = false;
        break;
      }
    }
  }

  Serial.printf("Neues Food: (%d, %d)\n", food.x, food.y);
}

// ===== BUTTON INPUT =====

void readButtons() {
  unsigned long now = millis();
  if (now - lastButtonPress < BUTTON_DEBOUNCE) {
    return;
  }

  // Verhindere 180-Grad Wendungen
  if (digitalRead(BTN_UP) == LOW && currentDirection != DIR_DOWN) {
    nextDirection = DIR_UP;
    lastButtonPress = now;
    Serial.println("Button: UP");
  }
  else if (digitalRead(BTN_DOWN) == LOW && currentDirection != DIR_UP) {
    nextDirection = DIR_DOWN;
    lastButtonPress = now;
    Serial.println("Button: DOWN");
  }
  else if (digitalRead(BTN_LEFT) == LOW && currentDirection != DIR_RIGHT) {
    nextDirection = DIR_LEFT;
    lastButtonPress = now;
    Serial.println("Button: LEFT");
  }
  else if (digitalRead(BTN_RIGHT) == LOW && currentDirection != DIR_LEFT) {
    nextDirection = DIR_RIGHT;
    lastButtonPress = now;
    Serial.println("Button: RIGHT");
  }
}

bool checkAnyButton() {
  return (digitalRead(BTN_UP) == LOW ||
          digitalRead(BTN_DOWN) == LOW ||
          digitalRead(BTN_LEFT) == LOW ||
          digitalRead(BTN_RIGHT) == LOW);
}

// ===== GAME LOGIC =====

void moveSnake() {
  // Alte Tail-Position merken (zum Löschen)
  Position oldTail = snake[snakeLength - 1];

  // Alle Segmente nach hinten verschieben
  for (int i = snakeLength - 1; i > 0; i--) {
    snake[i] = snake[i - 1];
  }

  // Kopf in neue Richtung bewegen
  switch (currentDirection) {
    case DIR_UP:
      snake[0].y--;
      break;
    case DIR_DOWN:
      snake[0].y++;
      break;
    case DIR_LEFT:
      snake[0].x--;
      break;
    case DIR_RIGHT:
      snake[0].x++;
      break;
    default:
      break;
  }

  // Altes Tail löschen
  clearGridCell(oldTail.x, oldTail.y);
}

bool checkCollision() {
  // Kollision mit Wänden
  if (snake[0].x <= 0 || snake[0].x >= GRID_WIDTH - 1 ||
      snake[0].y <= 0 || snake[0].y >= GRID_HEIGHT - 1) {
    Serial.println("Kollision: Wand!");
    return true;
  }

  // Kollision mit eigenem Körper
  for (int i = 1; i < snakeLength; i++) {
    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
      Serial.println("Kollision: Eigener Körper!");
      return true;
    }
  }

  return false;
}

bool checkFood() {
  if (snake[0].x == food.x && snake[0].y == food.y) {
    Serial.println("Food gegessen!");
    return true;
  }
  return false;
}

void eatFood() {
  // Partikel spawnen am Food
  int foodCenterX = food.x * GRID_SIZE + GRID_SIZE/2;
  int foodCenterY = food.y * GRID_SIZE + GRID_SIZE/2;
  spawnParticles(foodCenterX, foodCenterY, COLOR_FOOD_BASE, 15);

  // Snake verlängern
  snakeLength++;
  score += 10;

  // Geschwindigkeit erhöhen
  if (moveDelay > 80) {
    moveDelay -= 5;
  }

  Serial.printf("Score: %d, Länge: %d, Speed: %dms\n", score, snakeLength, moveDelay);

  // Neues Food spawnen
  spawnFood();
  drawModernScore();
}

void handleGameOver() {
  gameOver = true;
  Serial.println("\n=== GAME OVER ===");
  Serial.printf("Final Score: %d\n", score);
  Serial.printf("Snake Length: %d\n", snakeLength);

  // Moderner Game Over Screen
  int boxX = SCREEN_WIDTH/4 - 10;
  int boxY = SCREEN_HEIGHT/2 - 55;
  int boxW = SCREEN_WIDTH/2 + 20;
  int boxH = 110;

  // Box mit Gradient
  for (int y = 0; y < boxH; y++) {
    float factor = (float)y / boxH;
    uint16_t color = lerpColor(0x2104, 0x0000, factor);
    lcd.drawFastHLine(boxX, boxY + y, boxW, color);
  }

  // Neon-Umrandung
  lcd.drawRect(boxX - 1, boxY - 1, boxW + 2, boxH + 2, COLOR_NEON_RED);
  lcd.drawRect(boxX - 2, boxY - 2, boxW + 4, boxH + 4, dimColor(COLOR_NEON_RED, 0.5));

  // Text mit Schatten - zentriert in der Box
  // GAME OVER (9 Zeichen * 18px = 162px breit bei Textgröße 3)
  int centerX = boxX + boxW / 2;

  lcd.setTextSize(3);
  lcd.setTextColor(COLOR_SHADOW);
  lcd.setCursor(centerX - 79, boxY + 22);  // 162/2 = 81, Schatten +2
  lcd.println("GAME OVER");

  lcd.setTextColor(COLOR_NEON_RED);
  lcd.setCursor(centerX - 81, boxY + 20);
  lcd.println("GAME OVER");

  // Score (variiert je nach Punktzahl, ca. 110px bei "Score: 30")
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_SHADOW);
  lcd.setCursor(centerX - 53, boxY + 57);  // Schatten +2
  lcd.printf("Score: %d", score);

  lcd.setTextColor(COLOR_NEON_YELLOW);
  lcd.setCursor(centerX - 55, boxY + 55);
  lcd.printf("Score: %d", score);

  // Anweisung (17 Zeichen * 6px = 102px breit bei Textgröße 1)
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_NEON_CYAN);
  lcd.setCursor(centerX - 51, boxY + 85);  // 102/2 = 51
  lcd.println("Press any button");
}

// ===== PARTIKEL-SYSTEM =====

void spawnParticles(int x, int y, uint16_t color, int count) {
  for (int i = 0; i < count; i++) {
    for (int j = 0; j < MAX_PARTICLES; j++) {
      if (!particles[j].active) {
        particles[j].x = x;
        particles[j].y = y;
        particles[j].oldX = x;
        particles[j].oldY = y;
        particles[j].vx = random(-40, 40) / 10.0;
        particles[j].vy = random(-40, 40) / 10.0;
        particles[j].life = random(15, 30);
        particles[j].color = color;
        particles[j].active = true;
        break;
      }
    }
  }
}

void updateParticles() {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (particles[i].active) {
      // Alte Position speichern
      particles[i].oldX = particles[i].x;
      particles[i].oldY = particles[i].y;

      // Neue Position berechnen
      particles[i].x += particles[i].vx;
      particles[i].y += particles[i].vy;
      particles[i].vy += 0.2;  // Gravitation
      particles[i].vx *= 0.98;  // Reibung
      particles[i].life--;

      if (particles[i].life <= 0) {
        particles[i].active = false;
      }
    }
  }
}

void drawParticles() {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (particles[i].active) {
      // Alte Position löschen (mit Gradient-Hintergrund)
      // NICHT im Score-Bereich (obere Zeile) löschen!
      int oldSize = 1 + (particles[i].life / 10) + 2;  // +2 für Glow
      for (int dy = -oldSize; dy <= oldSize; dy++) {
        for (int dx = -oldSize; dx <= oldSize; dx++) {
          int px = particles[i].oldX + dx;
          int py = particles[i].oldY + dy;
          if (px >= 0 && px < SCREEN_WIDTH && py >= GRID_SIZE && py < SCREEN_HEIGHT) {  // py >= GRID_SIZE!
            uint16_t bgColor = lerpColor(COLOR_BG_TOP, COLOR_BG_BOTTOM, (float)py / SCREEN_HEIGHT);
            lcd.drawPixel(px, py, bgColor);
          }
        }
      }

      // Neue Position zeichnen (nur wenn nicht im Score-Bereich)
      if (particles[i].y >= GRID_SIZE) {
        float alpha = (float)particles[i].life / 30.0;
        uint16_t color = dimColor(particles[i].color, alpha);

        int size = 1 + (particles[i].life / 10);
        lcd.fillCircle(particles[i].x, particles[i].y, size, color);

        // Glow
        if (alpha > 0.5) {
          lcd.drawCircle(particles[i].x, particles[i].y, size + 1, dimColor(color, 0.3));
        }
      }
    }
  }
}

// ===== ZEICHNEN =====

void drawGradientBackground() {
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    uint16_t color = lerpColor(COLOR_BG_TOP, COLOR_BG_BOTTOM, (float)y / SCREEN_HEIGHT);
    lcd.drawFastHLine(0, y, SCREEN_WIDTH, color);
  }
}

void drawModernBorder() {
  // Wände mit Neon-Glow-Effekt
  // Oberste Zeile mit Aussparung für Score und Speed
  // Score: x=1 bis x=11, Speed: x=26 bis x=31
  for (int x = 0; x < GRID_WIDTH; x++) {
    // Aussparungen für Score (x=1-11) und Speed (x=26-31)
    if (x == 0 || (x >= 12 && x <= 25)) {
      drawModernWall(x, 0);
    }
    drawModernWall(x, GRID_HEIGHT - 1);
  }
  for (int y = 1; y < GRID_HEIGHT - 1; y++) {
    drawModernWall(0, y);
    drawModernWall(GRID_WIDTH - 1, y);
  }
}

void drawModernWall(int gridX, int gridY) {
  int pixelX = gridX * GRID_SIZE;
  int pixelY = gridY * GRID_SIZE;

  // Haupt-Block
  lcd.fillRect(pixelX + 2, pixelY + 2, GRID_SIZE - 4, GRID_SIZE - 4, COLOR_WALL);

  // Glow
  lcd.drawRect(pixelX + 1, pixelY + 1, GRID_SIZE - 2, GRID_SIZE - 2, dimColor(COLOR_WALL, 0.5));
  lcd.drawRect(pixelX, pixelY, GRID_SIZE, GRID_SIZE, dimColor(COLOR_WALL, 0.2));
}

void drawModernScore() {
  // Kompakte Löschung nur für Text-Bereiche (verhindert Flackern)
  lcd.fillRect(GRID_SIZE, 1, 100, 8, COLOR_BG_TOP);  // Score und Länge
  lcd.fillRect(SCREEN_WIDTH - 60, 1, 58, 8, COLOR_BG_TOP);  // Speed

  redrawScoreText();  // Text zeichnen
}

void redrawScoreText() {
  // Text mit Schatten (ohne Hintergrund zu löschen)
  lcd.setTextSize(1);

  lcd.setTextColor(COLOR_SHADOW);
  lcd.setCursor(GRID_SIZE + 3, 3);
  lcd.printf("Score:%d L:%d", score, snakeLength);

  lcd.setTextColor(COLOR_NEON_YELLOW);
  lcd.setCursor(GRID_SIZE + 2, 2);
  lcd.printf("Score:%d L:%d", score, snakeLength);

  // Speed-Indikator
  lcd.setTextColor(COLOR_NEON_CYAN);
  lcd.setCursor(SCREEN_WIDTH - 60, 2);
  lcd.printf("Spd:%d", 200 - moveDelay);
}

void drawGame() {
  // Snake zeichnen
  for (int i = 0; i < snakeLength; i++) {
    if (i == 0) {
      drawModernSnakeHead(snake[i].x, snake[i].y);
    } else {
      drawModernSnakeBody(snake[i].x, snake[i].y, i);
    }
  }

  // Food zeichnen (mit Animation)
  drawModernFood();
}

void drawModernSnakeHead(int gridX, int gridY) {
  int pixelX = gridX * GRID_SIZE;
  int pixelY = gridY * GRID_SIZE;

  // Schatten
  lcd.fillRect(pixelX + 2, pixelY + 2, GRID_SIZE - 3, GRID_SIZE - 3, COLOR_SHADOW);

  // Gradient-Kopf
  for (int i = 0; i < GRID_SIZE - 2; i++) {
    float factor = (float)i / (GRID_SIZE - 2);
    uint16_t color = lerpColor(COLOR_SNAKE_HEAD, COLOR_NEON_LIME, factor);
    lcd.drawFastHLine(pixelX + 1, pixelY + 1 + i, GRID_SIZE - 2, color);
  }

  // Pulsierender Glow
  float glow = sin(glowPhase) * 0.5 + 0.5;
  if (glow > 0.3) {
    uint16_t glowColor = dimColor(COLOR_SNAKE_HEAD, glow * 0.6);
    lcd.drawRect(pixelX, pixelY, GRID_SIZE, GRID_SIZE, glowColor);
    if (glow > 0.6) {
      lcd.drawRect(pixelX - 1, pixelY - 1, GRID_SIZE + 2, GRID_SIZE + 2, dimColor(glowColor, 0.4));
    }
  }

  // Augen (je nach Richtung)
  int eye1X = pixelX + GRID_SIZE/2 - 2;
  int eye2X = pixelX + GRID_SIZE/2 + 2;
  int eyeY = pixelY + GRID_SIZE/2;

  if (currentDirection == DIR_LEFT || currentDirection == DIR_RIGHT) {
    eye1X = (currentDirection == DIR_RIGHT) ? pixelX + GRID_SIZE - 3 : pixelX + 2;
    eye2X = eye1X;
    eyeY = pixelY + GRID_SIZE/2 - 1;
    int eyeY2 = pixelY + GRID_SIZE/2 + 1;
    lcd.fillCircle(eye1X, eyeY, 1, COLOR_NEON_RED);
    lcd.fillCircle(eye2X, eyeY2, 1, COLOR_NEON_RED);
  } else {
    eyeY = (currentDirection == DIR_DOWN) ? pixelY + GRID_SIZE - 3 : pixelY + 2;
    lcd.fillCircle(eye1X, eyeY, 1, COLOR_NEON_RED);
    lcd.fillCircle(eye2X, eyeY, 1, COLOR_NEON_RED);
  }
}

void drawModernSnakeBody(int gridX, int gridY, int segmentIndex) {
  int pixelX = gridX * GRID_SIZE;
  int pixelY = gridY * GRID_SIZE;

  // Schatten
  lcd.fillRect(pixelX + 2, pixelY + 2, GRID_SIZE - 3, GRID_SIZE - 3, COLOR_SHADOW);

  // Gradient basierend auf Segment-Position
  float segmentFactor = 1.0 - ((float)segmentIndex / snakeLength) * 0.5;
  uint16_t bodyColor = dimColor(COLOR_SNAKE_BODY, segmentFactor);

  // Segment mit leichtem Gradient
  for (int i = 0; i < GRID_SIZE - 2; i++) {
    float factor = (float)i / (GRID_SIZE - 2) * 0.3;
    uint16_t color = dimColor(bodyColor, 1.0 - factor);
    lcd.drawFastHLine(pixelX + 1, pixelY + 1 + i, GRID_SIZE - 2, color);
  }

  // Highlight oben
  lcd.drawFastHLine(pixelX + 2, pixelY + 1, GRID_SIZE - 4, dimColor(COLOR_HIGHLIGHT, 0.3));
}

void drawModernFood() {
  int pixelX = food.x * GRID_SIZE;
  int pixelY = food.y * GRID_SIZE;

  // Pulsierende Größe
  float pulse = sin(foodPulse) * 0.3 + 0.7;  // 0.7 bis 1.0
  int size = (GRID_SIZE - 2) * pulse;
  int offset = (GRID_SIZE - size) / 2;

  // Multi-Layer Glow
  for (int r = 3; r >= 0; r--) {
    float intensity = 1.0 - (r * 0.25);
    uint16_t glowColor = dimColor(COLOR_FOOD_BASE, intensity * 0.4);
    lcd.drawRect(pixelX + offset - r, pixelY + offset - r,
                 size + r*2, size + r*2, glowColor);
  }

  // Haupt-Food mit Gradient
  for (int i = 0; i < size; i++) {
    float factor = (float)i / size;
    uint16_t color = lerpColor(COLOR_NEON_YELLOW, COLOR_FOOD_BASE, factor);
    lcd.drawFastHLine(pixelX + offset, pixelY + offset + i, size, color);
  }

  // Highlight
  int hlSize = size / 3;
  lcd.fillRect(pixelX + offset + 1, pixelY + offset + 1, hlSize, hlSize, COLOR_HIGHLIGHT);
}

void clearGridCell(int gridX, int gridY) {
  int pixelX = gridX * GRID_SIZE;
  int pixelY = gridY * GRID_SIZE;

  // Mit Gradient-Hintergrund füllen
  for (int y = 0; y < GRID_SIZE; y++) {
    uint16_t bgColor = lerpColor(COLOR_BG_TOP, COLOR_BG_BOTTOM,
                                   (float)(pixelY + y) / SCREEN_HEIGHT);
    lcd.drawFastHLine(pixelX, pixelY + y, GRID_SIZE, bgColor);
  }
}

// ===== HELPER FUNKTIONEN =====

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

uint16_t dimColor(uint16_t color, float factor) {
  factor = constrain(factor, 0.0, 1.0);

  uint8_t r = ((color >> 11) & 0x1F) * factor;
  uint8_t g = ((color >> 5) & 0x3F) * factor;
  uint8_t b = (color & 0x1F) * factor;

  return (r << 11) | (g << 5) | b;
}
