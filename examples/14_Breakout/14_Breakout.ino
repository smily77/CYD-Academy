/*
  Breakout Game - Klassisches Breakout/Arkanoid Spiel

  Hardware:
  - CYD Display (320x240)
  - 1x Analog-Regler (Potentiometer):
    * GPIO 34: Schläger-Steuerung (Links/Rechts)
  - Optional: Touch zum Ball-Start

  Steuerung:
  - Analog-Regler: Bewegt Schläger horizontal
  - Touch Screen: Startet Ball (wenn am Schläger)

  Spielregeln:
  - Zerstöre alle Steine mit dem Ball
  - Ball darf nicht unten rausfallen (3 Leben)
  - Verschiedene Stein-Farben = verschiedene Punkte
  - Level komplett = Nächstes Level (Steine regenerieren)
*/

#include <CYD_Display_Config.h>

// Analog Input Pin
#define POT_PADDLE  34

// Button Pins
#define BTN_START   17  // Ball starten
#define BTN_PAUSE   18  // Pause

// Display Objekt
LGFX lcd;

// Spielfeld-Dimensionen
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// Farben
#define COLOR_BG      0x0000  // Schwarz
#define COLOR_PADDLE  0xFFFF  // Weiß
#define COLOR_BALL    0xFFFF  // Weiß
#define COLOR_TEXT    0xFFFF  // Weiß

// Stein-Farben (von oben nach unten)
#define COLOR_BRICK_RED     0xF800  // Rot - 7 Punkte
#define COLOR_BRICK_ORANGE  0xFD20  // Orange - 6 Punkte
#define COLOR_BRICK_YELLOW  0xFFE0  // Gelb - 5 Punkte
#define COLOR_BRICK_GREEN   0x07E0  // Grün - 4 Punkte
#define COLOR_BRICK_CYAN    0x07FF  // Cyan - 3 Punkte
#define COLOR_BRICK_BLUE    0x001F  // Blau - 2 Punkte

// Schläger
struct Paddle {
  int x, y;
  int w, h;
  uint16_t color;
};

// Ball
struct Ball {
  float x, y;
  float vx, vy;
  int size;
  uint16_t color;
  bool stuck;  // Am Schläger festgeklebt
};

// Stein
struct Brick {
  int x, y;
  int w, h;
  uint16_t color;
  int points;
  bool active;
};

// Game Objekte
Paddle paddle;
Ball ball;

// Steine (8 Reihen x 10 Spalten)
#define BRICK_ROWS 6
#define BRICK_COLS 10
#define BRICK_COUNT (BRICK_ROWS * BRICK_COLS)
Brick bricks[BRICK_COUNT];

// Game State
int score = 0;
int lives = 3;
int level = 1;
bool gameOver = false;
bool levelComplete = false;
bool paused = false;

// Alte Positionen
int oldPaddleX = 0;
float oldBallX = 0;
float oldBallY = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== BREAKOUT GAME ===\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);
  lcd.fillScreen(COLOR_BG);
  lcd.setBrightness(255);

  // Analog Input konfigurieren
  pinMode(POT_PADDLE, INPUT);

  // Button Inputs konfigurieren
  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(BTN_PAUSE, INPUT_PULLUP);

  // Zufallsgenerator
  randomSeed(analogRead(POT_PADDLE) + analogRead(35) + micros());

  // Spiel initialisieren
  initGame();

  Serial.println("Breakout gestartet!");
  Serial.println("Steuerung: Analog-Regler GPIO 34");
  Serial.println("Ball-Start: GPIO 17");
  Serial.println("Pause: GPIO 18");
}

void loop() {
  if (gameOver) {
    // Warte auf Button zum Neustart
    if (digitalRead(BTN_START) == LOW) {
      initGame();
      gameOver = false;
      delay(300);
    }
    return;
  }

  // Pause-Button
  static int lastPauseState = HIGH;
  int pauseState = digitalRead(BTN_PAUSE);
  if (pauseState == LOW && lastPauseState == HIGH) {
    paused = !paused;
    Serial.println(paused ? "PAUSE" : "WEITER");
    delay(200);
  }
  lastPauseState = pauseState;

  if (paused) {
    // Zeige Pause-Text
    lcd.setTextSize(3);
    lcd.setTextColor(COLOR_TEXT, COLOR_BG);
    lcd.setCursor(SCREEN_WIDTH/2 - 60, SCREEN_HEIGHT/2 - 15);
    lcd.println("PAUSE");
    delay(100);
    return;
  }

  if (levelComplete) {
    // Nächstes Level starten
    delay(2000);
    level++;
    initLevel();
    levelComplete = false;
  }

  // Schläger aktualisieren
  updatePaddle();

  // Ball-Start mit Button
  if (ball.stuck) {
    if (digitalRead(BTN_START) == LOW) {
      launchBall();
      delay(200);
    }
  } else {
    // Ball bewegen
    updateBall();

    // Kollisionen prüfen
    checkCollisions();
  }

  // Zeichnen
  drawGame();

  // Frame-Rate ~60 FPS
  delay(16);
}

// ===== INITIALISIERUNG =====

void initGame() {
  Serial.println("Initialisiere Spiel...");

  score = 0;
  lives = 3;
  level = 1;
  gameOver = false;
  levelComplete = false;

  // Schläger initialisieren
  paddle.w = 60;
  paddle.h = 8;
  paddle.x = SCREEN_WIDTH / 2 - paddle.w / 2;
  paddle.y = SCREEN_HEIGHT - 20;
  paddle.color = COLOR_PADDLE;
  oldPaddleX = paddle.x;

  // Ball initialisieren
  resetBall();

  // Steine initialisieren
  initLevel();

  // Bildschirm aufbauen
  lcd.fillScreen(COLOR_BG);
  drawBricks();
  drawUI();
}

void initLevel() {
  Serial.printf("Level %d gestartet!\n", level);

  // Stein-Dimensionen
  int brickW = 28;
  int brickH = 10;
  int brickSpacing = 4;
  int startX = 10;
  int startY = 40;

  // Farben für Reihen
  uint16_t rowColors[BRICK_ROWS] = {
    COLOR_BRICK_RED,
    COLOR_BRICK_ORANGE,
    COLOR_BRICK_YELLOW,
    COLOR_BRICK_GREEN,
    COLOR_BRICK_CYAN,
    COLOR_BRICK_BLUE
  };

  int rowPoints[BRICK_ROWS] = {7, 6, 5, 4, 3, 2};

  // Steine erstellen
  int index = 0;
  for (int row = 0; row < BRICK_ROWS; row++) {
    for (int col = 0; col < BRICK_COLS; col++) {
      bricks[index].x = startX + col * (brickW + brickSpacing);
      bricks[index].y = startY + row * (brickH + brickSpacing);
      bricks[index].w = brickW;
      bricks[index].h = brickH;
      bricks[index].color = rowColors[row];
      bricks[index].points = rowPoints[row];
      bricks[index].active = true;
      index++;
    }
  }

  resetBall();
  lcd.fillScreen(COLOR_BG);
  drawBricks();
  drawUI();
}

void resetBall() {
  ball.size = 6;
  ball.color = COLOR_BALL;
  ball.stuck = true;
  ball.x = paddle.x + paddle.w / 2 - ball.size / 2;
  ball.y = paddle.y - ball.size - 1;
  ball.vx = 0;
  ball.vy = 0;

  oldBallX = ball.x;
  oldBallY = ball.y;
}

void launchBall() {
  ball.stuck = false;

  // Zufällige Start-Richtung
  float angle = random(30, 150);  // Grad
  float rad = angle * 3.14159 / 180.0;

  // Langsamer Start, wird mit Score schneller
  float baseSpeed = 2.5;
  float speedBoost = score / 100.0;  // +1.0 pro 100 Punkte
  float speed = baseSpeed + speedBoost;
  speed = constrain(speed, 2.5, 5.0);  // Max 5.0

  ball.vx = cos(rad) * speed;
  ball.vy = -abs(sin(rad)) * speed;  // Immer nach oben

  Serial.printf("Ball gestartet! Speed: %.1f\n", speed);
}

// ===== UPDATE FUNKTIONEN =====

void updatePaddle() {
  oldPaddleX = paddle.x;

  // Analog-Wert lesen (0-4095 vom ESP32 ADC)
  int potValue = analogRead(POT_PADDLE);
  paddle.x = map(potValue, 0, 4095, 0, SCREEN_WIDTH - paddle.w);

  // Grenzen
  paddle.x = constrain(paddle.x, 0, SCREEN_WIDTH - paddle.w);

  // Ball bewegen wenn festgeklebt
  if (ball.stuck) {
    oldBallX = ball.x;  // Alte Position merken BEVOR wir bewegen
    ball.x = paddle.x + paddle.w / 2 - ball.size / 2;
  }
}

void updateBall() {
  oldBallX = ball.x;
  oldBallY = ball.y;

  ball.x += ball.vx;
  ball.y += ball.vy;

  // Kollision mit Wänden
  if (ball.x <= 0 || ball.x >= SCREEN_WIDTH - ball.size) {
    ball.vx = -ball.vx;
    ball.x = constrain(ball.x, 0, SCREEN_WIDTH - ball.size);
  }

  // Kollision mit oberer Wand
  if (ball.y <= 0) {
    ball.vy = -ball.vy;
    ball.y = 0;
  }

  // Ball unten raus - Leben verlieren
  if (ball.y > SCREEN_HEIGHT) {
    lives--;
    Serial.printf("Leben verloren! Verbleibend: %d\n", lives);

    if (lives <= 0) {
      handleGameOver();
    } else {
      resetBall();
      drawUI();
    }
  }
}

void checkCollisions() {
  // Kollision mit Schläger
  if (ball.y + ball.size >= paddle.y &&
      ball.y + ball.size <= paddle.y + paddle.h &&
      ball.x + ball.size >= paddle.x &&
      ball.x <= paddle.x + paddle.w) {

    ball.vy = -abs(ball.vy);
    ball.y = paddle.y - ball.size;

    // Winkel abhängig von Treffpunkt
    float hitPos = (ball.x + ball.size/2) - (paddle.x + paddle.w/2);
    ball.vx += hitPos * 0.1;

    // Geschwindigkeit begrenzen
    ball.vx = constrain(ball.vx, -5, 5);

    Serial.println("Hit: Schläger");
  }

  // Kollision mit Steinen
  for (int i = 0; i < BRICK_COUNT; i++) {
    if (!bricks[i].active) continue;

    if (ball.x + ball.size >= bricks[i].x &&
        ball.x <= bricks[i].x + bricks[i].w &&
        ball.y + ball.size >= bricks[i].y &&
        ball.y <= bricks[i].y + bricks[i].h) {

      // Stein zerstören
      bricks[i].active = false;
      score += bricks[i].points;

      // Stein löschen
      lcd.fillRect(bricks[i].x, bricks[i].y, bricks[i].w, bricks[i].h, COLOR_BG);

      // Ball abprallen lassen
      // Einfache Logik: von welcher Seite kam der Ball?
      float ballCenterX = ball.x + ball.size / 2;
      float ballCenterY = ball.y + ball.size / 2;
      float brickCenterX = bricks[i].x + bricks[i].w / 2;
      float brickCenterY = bricks[i].y + bricks[i].h / 2;

      if (abs(ballCenterX - brickCenterX) > abs(ballCenterY - brickCenterY)) {
        // Seitliche Kollision
        ball.vx = -ball.vx;
      } else {
        // Vertikale Kollision
        ball.vy = -ball.vy;
      }

      Serial.printf("Stein zerstört! Score: %d\n", score);
      drawUI();

      // Level komplett?
      if (checkLevelComplete()) {
        levelComplete = true;
        Serial.println("Level komplett!");
      }

      break;  // Nur ein Stein pro Frame
    }
  }
}

bool checkLevelComplete() {
  for (int i = 0; i < BRICK_COUNT; i++) {
    if (bricks[i].active) return false;
  }
  return true;
}

void handleGameOver() {
  gameOver = true;
  Serial.println("\n=== GAME OVER ===");
  Serial.printf("Final Score: %d\n", score);
  Serial.printf("Level erreicht: %d\n", level);

  // Game Over Screen
  lcd.fillRect(SCREEN_WIDTH/4 - 10, SCREEN_HEIGHT/2 - 55,
               SCREEN_WIDTH/2 + 20, 110, 0x2104);
  lcd.drawRect(SCREEN_WIDTH/4 - 10, SCREEN_HEIGHT/2 - 55,
               SCREEN_WIDTH/2 + 20, 110, COLOR_TEXT);

  lcd.setTextSize(3);
  lcd.setTextColor(COLOR_TEXT, 0x2104);
  lcd.setCursor(SCREEN_WIDTH/2 - 75, SCREEN_HEIGHT/2 - 30);
  lcd.println("GAME OVER");

  lcd.setTextSize(2);
  lcd.setCursor(SCREEN_WIDTH/2 - 55, SCREEN_HEIGHT/2 + 5);
  lcd.printf("Score: %d", score);

  lcd.setTextSize(1);
  lcd.setCursor(SCREEN_WIDTH/2 - 55, SCREEN_HEIGHT/2 + 30);
  lcd.printf("Level: %d", level);

  lcd.setCursor(SCREEN_WIDTH/2 - 40, SCREEN_HEIGHT/2 + 45);
  lcd.println("Touch to restart");
}

// ===== ZEICHNEN =====

void drawBricks() {
  for (int i = 0; i < BRICK_COUNT; i++) {
    if (bricks[i].active) {
      drawBrick(i);
    }
  }
}

void drawBrick(int index) {
  Brick* b = &bricks[index];

  // Stein ohne Rahmen (Retro-Look)
  lcd.fillRect(b->x, b->y, b->w, b->h, b->color);
}

void drawUI() {
  // Oberer Bereich löschen
  lcd.fillRect(0, 0, SCREEN_WIDTH, 30, COLOR_BG);

  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT, COLOR_BG);

  // Score
  lcd.setCursor(10, 5);
  lcd.printf("Score:%d", score);

  // Level
  lcd.setCursor(SCREEN_WIDTH/2 - 40, 5);
  lcd.printf("Level:%d", level);

  // Leben
  lcd.setCursor(SCREEN_WIDTH - 90, 5);
  lcd.printf("Lives:%d", lives);
}

void drawGame() {
  // Schläger zeichnen (immer neu zeichnen um Doppelungen zu vermeiden)
  // Alten löschen
  lcd.fillRect(oldPaddleX, paddle.y, paddle.w, paddle.h, COLOR_BG);
  // Neuen zeichnen
  lcd.fillRect(paddle.x, paddle.y, paddle.w, paddle.h, paddle.color);
  oldPaddleX = paddle.x;

  // Ball zeichnen - alte Position IMMER löschen (auch wenn am Schläger)
  lcd.fillRect(oldBallX, oldBallY, ball.size, ball.size, COLOR_BG);
  lcd.fillRect(ball.x, ball.y, ball.size, ball.size, ball.color);
}
