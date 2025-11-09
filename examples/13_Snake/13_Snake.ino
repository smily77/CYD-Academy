/*
  Snake Game - Klassisches Snake-Spiel mit Hardware-Buttons

  Hardware:
  - CYD Display (320x240)
  - 4x Hardware-Buttons:
    * GPIO 5:  Links
    * GPIO 17: Oben
    * GPIO 16: Rechts
    * GPIO 18: Unten
    (Buttons sind LOW wenn gedrückt)

  Steuerung:
  - Buttons steuern die Bewegungsrichtung
  - Sammle rote Food-Items
  - Vermeide Wände und deinen eigenen Körper
  - Je länger die Schlange, desto höher der Score

  Spielregeln:
  - Jedes Food-Item lässt die Schlange wachsen
  - Kollision mit Wand oder eigenem Körper = Game Over
  - Bei Game Over: Beliebigen Button drücken zum Neustart
*/

#include <CYD_Display_Config.h>

// Button Pins
#define BTN_LEFT  5
#define BTN_UP    17
#define BTN_RIGHT 16
#define BTN_DOWN  18

// Display Objekt
LGFX lcd;

// Spielfeld-Dimensionen
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// Grid-Konfiguration
#define GRID_SIZE 10  // Größe jedes Quadrats in Pixeln
#define GRID_WIDTH  (SCREEN_WIDTH / GRID_SIZE)   // 32
#define GRID_HEIGHT (SCREEN_HEIGHT / GRID_SIZE)  // 24

// Farben
#define COLOR_BG      0x0000  // Schwarz
#define COLOR_SNAKE   0x07E0  // Grün
#define COLOR_HEAD    0x07FF  // Cyan (Kopf)
#define COLOR_FOOD    0xF800  // Rot
#define COLOR_WALL    0x4208  // Dunkelgrau
#define COLOR_TEXT    0xFFFF  // Weiß

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

// Game State
#define MAX_SNAKE_LENGTH (GRID_WIDTH * GRID_HEIGHT)
Position snake[MAX_SNAKE_LENGTH];
int snakeLength = 3;
Direction currentDirection = DIR_RIGHT;
Direction nextDirection = DIR_RIGHT;

Position food;
int score = 0;
bool gameOver = false;

// Timing
unsigned long lastMoveTime = 0;
int moveDelay = 200;  // Millisekunden zwischen Bewegungen

// Button Debouncing
unsigned long lastButtonPress = 0;
#define BUTTON_DEBOUNCE 50

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== SNAKE GAME ===\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);  // Landscape
  lcd.fillScreen(COLOR_BG);
  lcd.setBrightness(255);

  // Button Pins konfigurieren
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);

  // Spiel initialisieren
  initGame();

  Serial.println("Spiel gestartet!");
  Serial.println("Steuerung:");
  Serial.println("  GPIO 5:  Links");
  Serial.println("  GPIO 16: Oben");
  Serial.println("  GPIO 17: Rechts");
  Serial.println("  GPIO 18: Unten");
}

void loop() {
  if (gameOver) {
    // Warte auf Button-Druck zum Neustart
    if (checkAnyButton()) {
      initGame();
      gameOver = false;
      delay(300);  // Debounce
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

  // Food platzieren
  spawnFood();

  // Bildschirm aufbauen
  lcd.fillScreen(COLOR_BG);
  drawBorder();
  drawScore();
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

  // Altes Tail löschen (nur wenn Snake nicht gewachsen ist)
  drawGridCell(oldTail.x, oldTail.y, COLOR_BG);
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
  // Snake verlängern
  snakeLength++;
  score += 10;

  // Geschwindigkeit erhöhen (aber nicht zu schnell)
  if (moveDelay > 80) {
    moveDelay -= 5;
  }

  Serial.printf("Score: %d, Länge: %d, Speed: %dms\n", score, snakeLength, moveDelay);

  // Neues Food spawnen
  spawnFood();
  drawScore();
}

void handleGameOver() {
  gameOver = true;
  Serial.println("\n=== GAME OVER ===");
  Serial.printf("Final Score: %d\n", score);
  Serial.printf("Snake Length: %d\n", snakeLength);

  // Game Over Bildschirm
  lcd.fillRect(SCREEN_WIDTH/4, SCREEN_HEIGHT/2 - 40,
               SCREEN_WIDTH/2, 80, 0x4208);
  lcd.drawRect(SCREEN_WIDTH/4, SCREEN_HEIGHT/2 - 40,
               SCREEN_WIDTH/2, 80, COLOR_TEXT);

  lcd.setTextSize(3);
  lcd.setTextColor(COLOR_TEXT, 0x4208);
  lcd.setCursor(SCREEN_WIDTH/2 - 75, SCREEN_HEIGHT/2 - 25);
  lcd.println("GAME OVER");

  lcd.setTextSize(2);
  lcd.setCursor(SCREEN_WIDTH/2 - 50, SCREEN_HEIGHT/2 + 5);
  lcd.printf("Score: %d", score);

  lcd.setTextSize(1);
  lcd.setCursor(SCREEN_WIDTH/2 - 60, SCREEN_HEIGHT/2 + 30);
  lcd.println("Press any button");
}

// ===== ZEICHNEN =====

void drawBorder() {
  // Rahmen um das Spielfeld
  for (int x = 0; x < GRID_WIDTH; x++) {
    drawGridCell(x, 0, COLOR_WALL);
    drawGridCell(x, GRID_HEIGHT - 1, COLOR_WALL);
  }
  for (int y = 0; y < GRID_HEIGHT; y++) {
    drawGridCell(0, y, COLOR_WALL);
    drawGridCell(GRID_WIDTH - 1, y, COLOR_WALL);
  }
}

void drawScore() {
  // Score-Bereich löschen
  lcd.fillRect(0, 0, GRID_SIZE * 8, GRID_SIZE, COLOR_BG);

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT, COLOR_BG);
  lcd.setCursor(GRID_SIZE + 2, 2);
  lcd.printf("Score:%d L:%d", score, snakeLength);
}

void drawGame() {
  // Snake zeichnen
  for (int i = 0; i < snakeLength; i++) {
    uint16_t color = (i == 0) ? COLOR_HEAD : COLOR_SNAKE;
    drawGridCell(snake[i].x, snake[i].y, color);
  }

  // Food zeichnen
  drawGridCell(food.x, food.y, COLOR_FOOD);
}

void drawGridCell(int gridX, int gridY, uint16_t color) {
  int pixelX = gridX * GRID_SIZE;
  int pixelY = gridY * GRID_SIZE;

  // Zelle mit kleinem Rand zeichnen (für Grid-Effekt)
  lcd.fillRect(pixelX + 1, pixelY + 1, GRID_SIZE - 2, GRID_SIZE - 2, color);
}
