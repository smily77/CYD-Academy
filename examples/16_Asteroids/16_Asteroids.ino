/*
  Asteroids - Klassisches Arcade-Spiel

  Hardware:
  - CYD Display (320x240)
  - 4x Digitale Buttons:
    * GPIO 5:  Links drehen
    * GPIO 16: Rechts drehen
    * GPIO 17: Schießen
    * GPIO 18: Schub (Thrust)

  Steuerung:
  - Links/Rechts: Rotiert Raumschiff
  - Schub: Beschleunigt in Blickrichtung
  - Schießen: Feuert Geschoss

  Spielregeln:
  - Zerstöre alle Asteroids
  - 3 Leben
  - Große Asteroids zerbrechen in 2 mittlere
  - Mittlere Asteroids zerbrechen in 2 kleine
  - Screen-Wrapping (Objekte gehen durch Bildschirmränder)
  - Punkte: Klein=100, Mittel=50, Groß=20
*/

#include <CYD_Display_Config.h>
#include <math.h>

// Button Pins
#define BTN_LEFT   5
#define BTN_RIGHT  16
#define BTN_SHOOT  17
#define BTN_THRUST 18

// Display Objekt
LGFX lcd;

// Display-Dimensionen
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// Farben
#define COLOR_BG      0x0000  // Schwarz
#define COLOR_SHIP    0x07FF  // Cyan
#define COLOR_THRUST  0xFD20  // Orange (Schub-Flamme)
#define COLOR_BULLET  0xFFFF  // Weiß
#define COLOR_ASTEROID 0xFFFF // Weiß
#define COLOR_TEXT    0xFFFF  // Weiß

// Spieler-Schiff
struct Ship {
  float x, y;           // Position
  float vx, vy;         // Geschwindigkeit
  float angle;          // Rotation in Grad (0 = nach oben)
  int size;             // Größe
  bool alive;
  int invulnerable;     // Frames invulnerabel nach Tod
};

// Asteroid
struct Asteroid {
  float x, y;           // Position
  float vx, vy;         // Geschwindigkeit
  int size;             // 0=klein, 1=mittel, 2=groß
  int radius;           // Radius für Kollision
  bool active;
};

// Geschoss
struct Bullet {
  float x, y;           // Position
  float vx, vy;         // Geschwindigkeit
  int life;             // Verbleibende Lebensdauer (Frames)
  bool active;
};

// Game Objekte
Ship ship;

#define MAX_ASTEROIDS 20
Asteroid asteroids[MAX_ASTEROIDS];

#define MAX_BULLETS 5
Bullet bullets[MAX_BULLETS];

// Game State
int score = 0;
int lives = 3;
int level = 1;
bool gameOver = false;

// Konstanten
#define SHIP_SIZE 8
#define SHIP_ACCELERATION 0.15
#define SHIP_MAX_SPEED 5.0
#define SHIP_ROTATION_SPEED 5.0
#define SHIP_FRICTION 0.98

#define BULLET_SPEED 6.0
#define BULLET_LIFE 60  // Frames

#define ASTEROID_LARGE_RADIUS 20
#define ASTEROID_MEDIUM_RADIUS 12
#define ASTEROID_SMALL_RADIUS 6

// FPS Control
unsigned long lastFrame = 0;
#define FRAME_DELAY 16  // ~60 FPS

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ASTEROIDS ===\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);
  lcd.fillScreen(COLOR_BG);
  lcd.setBrightness(255);

  // Button Inputs konfigurieren
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_SHOOT, INPUT_PULLUP);
  pinMode(BTN_THRUST, INPUT_PULLUP);

  // Zufallsgenerator
  randomSeed(analogRead(34) + analogRead(35) + micros());

  // Spiel initialisieren
  initGame();

  Serial.println("Asteroids gestartet!");
  Serial.println("Steuerung:");
  Serial.println("  Links:    GPIO 5");
  Serial.println("  Rechts:   GPIO 16");
  Serial.println("  Schießen: GPIO 17");
  Serial.println("  Schub:    GPIO 18");
}

void loop() {
  // Frame-Timing
  unsigned long now = millis();
  if (now - lastFrame < FRAME_DELAY) {
    return;
  }
  lastFrame = now;

  if (gameOver) {
    // Warte auf Neustart-Button
    if (digitalRead(BTN_SHOOT) == LOW) {
      initGame();
      gameOver = false;
      delay(300);
    }
    return;
  }

  // Input
  handleInput();

  // Update
  updateShip();
  updateBullets();
  updateAsteroids();

  // Kollisionen
  checkCollisions();

  // Zeichnen
  drawGame();

  // Level komplett?
  if (checkLevelComplete()) {
    level++;
    Serial.printf("Level %d!\n", level);
    initLevel();
    delay(2000);
  }
}

// ===== INITIALISIERUNG =====

void initGame() {
  Serial.println("Initialisiere Spiel...");

  score = 0;
  level = 1;
  lives = 3;
  gameOver = false;

  // Schiff initialisieren
  ship.x = SCREEN_WIDTH / 2;
  ship.y = SCREEN_HEIGHT / 2;
  ship.vx = 0;
  ship.vy = 0;
  ship.angle = 0;  // Nach oben
  ship.size = SHIP_SIZE;
  ship.alive = true;
  ship.invulnerable = 0;

  // Bullets initialisieren
  for (int i = 0; i < MAX_BULLETS; i++) {
    bullets[i].active = false;
  }

  // Asteroids initialisieren
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    asteroids[i].active = false;
  }

  // Bildschirm aufbauen
  lcd.fillScreen(COLOR_BG);
  drawUI();

  // Level starten
  initLevel();
}

void initLevel() {
  Serial.printf("Level %d gestartet!\n", level);

  // Schiff zurücksetzen
  ship.x = SCREEN_WIDTH / 2;
  ship.y = SCREEN_HEIGHT / 2;
  ship.vx = 0;
  ship.vy = 0;
  ship.angle = 0;
  ship.alive = true;
  ship.invulnerable = 60;  // 1 Sekunde invulnerabel

  // Asteroids spawnen (3 + Level)
  int numAsteroids = 3 + level;
  if (numAsteroids > 10) numAsteroids = 10;

  for (int i = 0; i < numAsteroids; i++) {
    spawnAsteroid(-1, -1, 2);  // Große Asteroids an zufälliger Position
  }
}

void spawnAsteroid(float x, float y, int size) {
  // Freien Slot finden
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    if (!asteroids[i].active) {
      asteroids[i].active = true;
      asteroids[i].size = size;

      // Radius basierend auf Größe
      if (size == 2) asteroids[i].radius = ASTEROID_LARGE_RADIUS;
      else if (size == 1) asteroids[i].radius = ASTEROID_MEDIUM_RADIUS;
      else asteroids[i].radius = ASTEROID_SMALL_RADIUS;

      // Position
      if (x < 0 || y < 0) {
        // Zufällige Position am Rand
        int edge = random(0, 4);
        if (edge == 0) { // Oben
          asteroids[i].x = random(0, SCREEN_WIDTH);
          asteroids[i].y = -asteroids[i].radius;
        } else if (edge == 1) { // Rechts
          asteroids[i].x = SCREEN_WIDTH + asteroids[i].radius;
          asteroids[i].y = random(0, SCREEN_HEIGHT);
        } else if (edge == 2) { // Unten
          asteroids[i].x = random(0, SCREEN_WIDTH);
          asteroids[i].y = SCREEN_HEIGHT + asteroids[i].radius;
        } else { // Links
          asteroids[i].x = -asteroids[i].radius;
          asteroids[i].y = random(0, SCREEN_HEIGHT);
        }
      } else {
        asteroids[i].x = x;
        asteroids[i].y = y;
      }

      // Zufällige Geschwindigkeit
      float speed = random(5, 15) / 10.0;
      float angle = random(0, 628) / 100.0;  // 0 bis 2*PI
      asteroids[i].vx = cos(angle) * speed;
      asteroids[i].vy = sin(angle) * speed;

      return;
    }
  }
}

// ===== INPUT =====

void handleInput() {
  static int lastShootState = HIGH;

  // Rotation
  if (digitalRead(BTN_LEFT) == LOW) {
    ship.angle -= SHIP_ROTATION_SPEED;
    if (ship.angle < 0) ship.angle += 360;
  }
  if (digitalRead(BTN_RIGHT) == LOW) {
    ship.angle += SHIP_ROTATION_SPEED;
    if (ship.angle >= 360) ship.angle -= 360;
  }

  // Schub
  if (digitalRead(BTN_THRUST) == LOW) {
    float rad = (ship.angle - 90) * PI / 180.0;  // -90 weil 0° = nach oben
    ship.vx += cos(rad) * SHIP_ACCELERATION;
    ship.vy += sin(rad) * SHIP_ACCELERATION;

    // Max speed begrenzen
    float speed = sqrt(ship.vx * ship.vx + ship.vy * ship.vy);
    if (speed > SHIP_MAX_SPEED) {
      ship.vx = (ship.vx / speed) * SHIP_MAX_SPEED;
      ship.vy = (ship.vy / speed) * SHIP_MAX_SPEED;
    }
  }

  // Schießen (mit Entprellen)
  int shootState = digitalRead(BTN_SHOOT);
  if (shootState == LOW && lastShootState == HIGH) {
    shootBullet();
  }
  lastShootState = shootState;
}

void shootBullet() {
  // Freies Geschoss finden
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) {
      float rad = (ship.angle - 90) * PI / 180.0;

      bullets[i].x = ship.x;
      bullets[i].y = ship.y;
      bullets[i].vx = cos(rad) * BULLET_SPEED + ship.vx;
      bullets[i].vy = sin(rad) * BULLET_SPEED + ship.vy;
      bullets[i].life = BULLET_LIFE;
      bullets[i].active = true;

      Serial.println("Shoot!");
      return;
    }
  }
}

// ===== UPDATE =====

void updateShip() {
  if (!ship.alive) return;

  // Invulnerabilität reduzieren
  if (ship.invulnerable > 0) {
    ship.invulnerable--;
  }

  // Alte Position löschen (Schiff + mögliche Thrust-Flamme)
  eraseShip();

  // Reibung
  ship.vx *= SHIP_FRICTION;
  ship.vy *= SHIP_FRICTION;

  // Bewegung
  ship.x += ship.vx;
  ship.y += ship.vy;

  // Screen wrapping
  if (ship.x < -SHIP_SIZE) ship.x = SCREEN_WIDTH + SHIP_SIZE;
  if (ship.x > SCREEN_WIDTH + SHIP_SIZE) ship.x = -SHIP_SIZE;
  if (ship.y < -SHIP_SIZE) ship.y = SCREEN_HEIGHT + SHIP_SIZE;
  if (ship.y > SCREEN_HEIGHT + SHIP_SIZE) ship.y = -SHIP_SIZE;
}

void updateBullets() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) continue;

    // Alte Position löschen
    lcd.fillCircle((int)bullets[i].x, (int)bullets[i].y, 1, COLOR_BG);

    // Bewegung
    bullets[i].x += bullets[i].vx;
    bullets[i].y += bullets[i].vy;
    bullets[i].life--;

    // Screen wrapping
    if (bullets[i].x < 0) bullets[i].x = SCREEN_WIDTH;
    if (bullets[i].x > SCREEN_WIDTH) bullets[i].x = 0;
    if (bullets[i].y < 0) bullets[i].y = SCREEN_HEIGHT;
    if (bullets[i].y > SCREEN_HEIGHT) bullets[i].y = 0;

    // Deaktivieren wenn Leben abgelaufen
    if (bullets[i].life <= 0) {
      bullets[i].active = false;
      continue;
    }

    // Neue Position zeichnen
    lcd.fillCircle((int)bullets[i].x, (int)bullets[i].y, 1, COLOR_BULLET);
  }
}

void updateAsteroids() {
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    if (!asteroids[i].active) continue;

    // Alte Position löschen
    lcd.drawCircle((int)asteroids[i].x, (int)asteroids[i].y,
                   asteroids[i].radius, COLOR_BG);

    // Bewegung
    asteroids[i].x += asteroids[i].vx;
    asteroids[i].y += asteroids[i].vy;

    // Screen wrapping
    if (asteroids[i].x < -asteroids[i].radius * 2)
      asteroids[i].x = SCREEN_WIDTH + asteroids[i].radius * 2;
    if (asteroids[i].x > SCREEN_WIDTH + asteroids[i].radius * 2)
      asteroids[i].x = -asteroids[i].radius * 2;
    if (asteroids[i].y < -asteroids[i].radius * 2)
      asteroids[i].y = SCREEN_HEIGHT + asteroids[i].radius * 2;
    if (asteroids[i].y > SCREEN_HEIGHT + asteroids[i].radius * 2)
      asteroids[i].y = -asteroids[i].radius * 2;

    // Neue Position zeichnen
    lcd.drawCircle((int)asteroids[i].x, (int)asteroids[i].y,
                   asteroids[i].radius, COLOR_ASTEROID);
  }
}

// ===== KOLLISIONEN =====

void checkCollisions() {
  // Bullets vs Asteroids
  for (int b = 0; b < MAX_BULLETS; b++) {
    if (!bullets[b].active) continue;

    for (int a = 0; a < MAX_ASTEROIDS; a++) {
      if (!asteroids[a].active) continue;

      float dx = bullets[b].x - asteroids[a].x;
      float dy = bullets[b].y - asteroids[a].y;
      float dist = sqrt(dx * dx + dy * dy);

      if (dist < asteroids[a].radius) {
        // Treffer!
        bullets[b].active = false;
        lcd.fillCircle((int)bullets[b].x, (int)bullets[b].y, 1, COLOR_BG);

        // Asteroid zerstören
        destroyAsteroid(a);
        break;
      }
    }
  }

  // Ship vs Asteroids
  if (ship.alive && ship.invulnerable == 0) {
    for (int a = 0; a < MAX_ASTEROIDS; a++) {
      if (!asteroids[a].active) continue;

      float dx = ship.x - asteroids[a].x;
      float dy = ship.y - asteroids[a].y;
      float dist = sqrt(dx * dx + dy * dy);

      if (dist < asteroids[a].radius + SHIP_SIZE) {
        // Kollision!
        shipDestroyed();
        break;
      }
    }
  }
}

void destroyAsteroid(int index) {
  Asteroid* ast = &asteroids[index];

  // Punkte
  if (ast->size == 2) score += 20;      // Groß
  else if (ast->size == 1) score += 50;  // Mittel
  else score += 100;                     // Klein

  Serial.printf("Asteroid zerstört! Score: %d\n", score);

  // Asteroid löschen
  lcd.drawCircle((int)ast->x, (int)ast->y, ast->radius, COLOR_BG);

  // In kleinere zerbrechen
  if (ast->size > 0) {
    // 2 kleinere Asteroids spawnen
    spawnAsteroid(ast->x, ast->y, ast->size - 1);
    spawnAsteroid(ast->x, ast->y, ast->size - 1);
  }

  ast->active = false;
  drawUI();
}

void shipDestroyed() {
  Serial.println("Schiff zerstört!");

  ship.alive = false;
  eraseShip();

  lives--;
  drawUI();

  delay(1000);

  if (lives <= 0) {
    handleGameOver();
  } else {
    // Respawn
    ship.x = SCREEN_WIDTH / 2;
    ship.y = SCREEN_HEIGHT / 2;
    ship.vx = 0;
    ship.vy = 0;
    ship.angle = 0;
    ship.alive = true;
    ship.invulnerable = 120;  // 2 Sekunden
  }
}

bool checkLevelComplete() {
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    if (asteroids[i].active) return false;
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

  lcd.setCursor(SCREEN_WIDTH/2 - 55, SCREEN_HEIGHT/2 + 45);
  lcd.println("Press SHOOT to restart");
}

// ===== ZEICHNEN =====

void drawShip() {
  if (!ship.alive) return;

  // Blinken wenn invulnerabel
  if (ship.invulnerable > 0 && (ship.invulnerable / 5) % 2 == 0) {
    return;  // Nicht zeichnen (Blink-Effekt)
  }

  float rad = (ship.angle - 90) * PI / 180.0;

  // Dreieck-Punkte berechnen
  int x1 = ship.x + cos(rad) * SHIP_SIZE;
  int y1 = ship.y + sin(rad) * SHIP_SIZE;

  int x2 = ship.x + cos(rad + 2.5) * SHIP_SIZE;
  int y2 = ship.y + sin(rad + 2.5) * SHIP_SIZE;

  int x3 = ship.x + cos(rad - 2.5) * SHIP_SIZE;
  int y3 = ship.y + sin(rad - 2.5) * SHIP_SIZE;

  // Dreieck zeichnen
  lcd.drawLine(x1, y1, x2, y2, COLOR_SHIP);
  lcd.drawLine(x2, y2, x3, y3, COLOR_SHIP);
  lcd.drawLine(x3, y3, x1, y1, COLOR_SHIP);

  // Schub-Flamme wenn Thrust gedrückt
  if (digitalRead(BTN_THRUST) == LOW) {
    int fx1 = ship.x + cos(rad + PI) * (SHIP_SIZE - 2);
    int fy1 = ship.y + sin(rad + PI) * (SHIP_SIZE - 2);

    int fx2 = ship.x + cos(rad + PI) * (SHIP_SIZE + 4);
    int fy2 = ship.y + sin(rad + PI) * (SHIP_SIZE + 4);

    lcd.drawLine(fx1, fy1, fx2, fy2, COLOR_THRUST);
  }
}

void eraseShip() {
  if (!ship.alive && ship.invulnerable == 0) return;

  // Bereich um Schiff löschen (inkl. Thrust-Flamme)
  int eraseSize = SHIP_SIZE + 6;
  lcd.fillRect(ship.x - eraseSize, ship.y - eraseSize,
               eraseSize * 2, eraseSize * 2, COLOR_BG);
}

void drawUI() {
  // Oberer Bereich löschen
  lcd.fillRect(0, 0, SCREEN_WIDTH, 20, COLOR_BG);

  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT, COLOR_BG);

  // Score (links)
  lcd.setCursor(5, 2);
  lcd.printf("S:%d", score);

  // Level (Mitte)
  lcd.setCursor(SCREEN_WIDTH/2 - 30, 2);
  lcd.printf("L:%d", level);

  // Lives (rechts)
  lcd.setCursor(SCREEN_WIDTH - 80, 2);
  lcd.printf("Lvs:%d", lives);
}

void drawGame() {
  drawShip();
}
