/*
  Asteroids Modern - Klassisches Arcade-Spiel mit modernen Effekten

  Hardware:
  - CYD Display (320x240)
  - 4x Digitale Buttons:
    * tasteB:  Links drehen
    * tasteC: Rechts drehen
    * tasteA: Schießen
    * tasteD: Schub (Thrust)

  Hinweis: Alle Pins sind in CYD_Display_Config.h definiert.

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

  Moderne Features:
  - Gradient-Hintergrund (Weltraum - dunkles Blau zu Schwarz)
  - Glow-Effekte für Schiff und Asteroids
  - Partikel-Explosionen beim Zerstören
  - Bullet-Trails für schönere Geschosse
  - Moderne UI mit besserer Grafik
  - Smoothe Animationen
*/

#include <CYD_Display_Config.h>

// Pin-Mapping: Physische Pins -> Logische Namen
#define BTN_LEFT   tasteB
#define BTN_RIGHT  tasteC
#define BTN_SHOOT  tasteA
#define BTN_THRUST tasteD

// Display Objekt
LGFX lcd;

// Display-Dimensionen
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// Farben - Moderne Palette
#define COLOR_BG_TOP      0x0015  // Dunkles Blau (oben)
#define COLOR_BG_BOTTOM   0x0000  // Schwarz (unten)
#define COLOR_SHIP        0x07FF  // Cyan
#define COLOR_SHIP_GLOW   0x0410  // Dunkles Cyan
#define COLOR_THRUST      0xFD20  // Orange (Schub-Flamme)
#define COLOR_THRUST_GLOW 0x8200  // Dunkles Orange
#define COLOR_BULLET      0xFFFF  // Weiß
#define COLOR_BULLET_GLOW 0x8410  // Grau
#define COLOR_ASTEROID    0xA514  // Grau-Weiß
#define COLOR_ASTEROID_GLOW 0x4208 // Dunkles Grau
#define COLOR_TEXT        0xFFFF  // Weiß
#define COLOR_PARTICLE1   0xFFE0  // Gelb
#define COLOR_PARTICLE2   0xFD20  // Orange
#define COLOR_PARTICLE3   0xF800  // Rot

// Spieler-Schiff
struct Ship {
  float x, y;           // Position
  float vx, vy;         // Geschwindigkeit
  float angle;          // Rotation in Grad (0 = nach oben)
  int size;             // Größe
  bool alive;
  int invulnerable;     // Frames invulnerabel nach Tod
  float oldX, oldY;     // Alte Position für Löschen
  float oldAngle;       // Alte Rotation
};

// Asteroid
struct Asteroid {
  float x, y;           // Position
  float vx, vy;         // Geschwindigkeit
  int size;             // 0=klein, 1=mittel, 2=groß
  int radius;           // Radius für Kollision
  bool active;
  float oldX, oldY;     // Alte Position für Löschen
};

// Geschoss
struct Bullet {
  float x, y;           // Position
  float vx, vy;         // Geschwindigkeit
  int life;             // Verbleibende Lebensdauer (Frames)
  bool active;
  float oldX, oldY;     // Alte Position für Trail
};

// Partikel für Explosionen
struct Particle {
  float x, y;
  float vx, vy;
  uint16_t color;
  uint8_t life;  // 0-100
  bool active;
};

// Array-Größen
#define MAX_ASTEROIDS 20
#define MAX_BULLETS 5
#define MAX_PARTICLES 40

// Game Objekte
Ship ship;
Asteroid asteroids[MAX_ASTEROIDS];
Bullet bullets[MAX_BULLETS];
Particle particles[MAX_PARTICLES];

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

#define FRAME_DELAY 16  // ~60 FPS

// Game State
int score = 0;
int lives = 3;
int level = 1;
bool gameOver = false;
unsigned long lastFrame = 0;
int lastShootState = HIGH;
bool backgroundDrawn = false;

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

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ASTEROIDS MODERN ===\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);
  lcd.fillScreen(COLOR_BG_BOTTOM);
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

  Serial.println("Asteroids Modern gestartet!");
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
  updateParticles();

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
  backgroundDrawn = false;
  lastFrame = 0;
  lastShootState = HIGH;

  // Schiff initialisieren
  ship.x = SCREEN_WIDTH / 2;
  ship.y = SCREEN_HEIGHT / 2;
  ship.vx = 0;
  ship.vy = 0;
  ship.angle = 0;  // Nach oben
  ship.size = SHIP_SIZE;
  ship.alive = true;
  ship.invulnerable = 0;
  ship.oldX = -100;
  ship.oldY = -100;
  ship.oldAngle = 0;

  // Bullets initialisieren
  for (int i = 0; i < MAX_BULLETS; i++) {
    bullets[i].active = false;
  }

  // Asteroids initialisieren
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    asteroids[i].active = false;
  }

  // Partikel initialisieren
  for (int i = 0; i < MAX_PARTICLES; i++) {
    particles[i].active = false;
  }

  // Bildschirm aufbauen
  drawBackground();
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
  ship.oldX = -100;
  ship.oldY = -100;

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

      asteroids[i].oldX = asteroids[i].x;
      asteroids[i].oldY = asteroids[i].y;

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
  // Rotation via Buttons
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
      bullets[i].oldX = ship.x;
      bullets[i].oldY = ship.y;
      bullets[i].vx = cos(rad) * BULLET_SPEED + ship.vx;
      bullets[i].vy = sin(rad) * BULLET_SPEED + ship.vy;
      bullets[i].life = BULLET_LIFE;
      bullets[i].active = true;
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

    // Alte Position speichern für Trail
    bullets[i].oldX = bullets[i].x;
    bullets[i].oldY = bullets[i].y;

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
    }
  }
}

void updateAsteroids() {
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    if (!asteroids[i].active) continue;

    // Alte Position speichern
    asteroids[i].oldX = asteroids[i].x;
    asteroids[i].oldY = asteroids[i].y;

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
  }
}

void updateParticles() {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (!particles[i].active) continue;

    // Alte Position löschen
    uint16_t bgColor = getBgColor((int)particles[i].y);
    lcd.fillCircle((int)particles[i].x, (int)particles[i].y, 1, bgColor);

    // Bewegen
    particles[i].x += particles[i].vx;
    particles[i].y += particles[i].vy;
    particles[i].vy += 0.2;  // Gravitation
    particles[i].life -= 5;

    // Deaktivieren wenn tot oder außerhalb
    if (particles[i].life <= 0 ||
        particles[i].y > SCREEN_HEIGHT ||
        particles[i].x < 0 || particles[i].x > SCREEN_WIDTH) {
      particles[i].active = false;
      continue;
    }

    // Neue Position zeichnen
    lcd.fillCircle((int)particles[i].x, (int)particles[i].y, 1, particles[i].color);
  }
}

void spawnExplosion(int x, int y, uint16_t baseColor) {
  // 10-15 Partikel spawnen
  int numParticles = random(10, 16);

  for (int i = 0; i < numParticles; i++) {
    // Freies Partikel finden
    for (int p = 0; p < MAX_PARTICLES; p++) {
      if (!particles[p].active) {
        particles[p].x = x;
        particles[p].y = y;

        // Zufällige Geschwindigkeit in alle Richtungen
        float angle = random(0, 628) / 100.0;  // 0 bis 2*PI
        float speed = random(10, 30) / 10.0;
        particles[p].vx = cos(angle) * speed;
        particles[p].vy = sin(angle) * speed - 1;  // Leicht nach oben

        // Farbe variieren
        int colorChoice = random(0, 3);
        if (colorChoice == 0) particles[p].color = COLOR_PARTICLE1;
        else if (colorChoice == 1) particles[p].color = COLOR_PARTICLE2;
        else particles[p].color = COLOR_PARTICLE3;

        particles[p].life = 100;
        particles[p].active = true;
        break;
      }
    }
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

  // Explosion spawnen
  spawnExplosion((int)ast->x, (int)ast->y, COLOR_ASTEROID);

  // Punkte
  if (ast->size == 2) score += 20;      // Groß
  else if (ast->size == 1) score += 50;  // Mittel
  else score += 100;                     // Klein

  // In kleinere zerbrechen
  if (ast->size > 0) {
    // 2 kleinere Asteroids spawnen
    spawnAsteroid(ast->x, ast->y, ast->size - 1);
    spawnAsteroid(ast->x, ast->y, ast->size - 1);
  }

  ast->active = false;
  drawUI();

  Serial.printf("Asteroid zerstört! Score: %d\n", score);
}

void shipDestroyed() {
  Serial.println("Schiff zerstört!");

  // Explosion spawnen
  spawnExplosion((int)ship.x, (int)ship.y, COLOR_SHIP);

  ship.alive = false;
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
    ship.oldX = -100;
    ship.oldY = -100;
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

  // Game Over Screen mit Gradient-Hintergrund
  uint16_t boxColor = lerpColor(COLOR_BG_TOP, COLOR_BG_BOTTOM, 0.5);
  lcd.fillRect(SCREEN_WIDTH/4 - 10, SCREEN_HEIGHT/2 - 55,
               SCREEN_WIDTH/2 + 20, 110, boxColor);
  lcd.drawRect(SCREEN_WIDTH/4 - 10, SCREEN_HEIGHT/2 - 55,
               SCREEN_WIDTH/2 + 20, 110, COLOR_TEXT);

  lcd.setTextSize(3);
  lcd.setTextColor(COLOR_TEXT, boxColor);
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

void drawBackground() {
  // Gradient von oben (dunkelblau) nach unten (schwarz)
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    uint16_t color = lerpColor(COLOR_BG_TOP, COLOR_BG_BOTTOM, (float)y / SCREEN_HEIGHT);
    lcd.drawFastHLine(0, y, SCREEN_WIDTH, color);
  }
  backgroundDrawn = true;
}

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

  // Glow-Effekt (dickere Linien, dunklere Farbe)
  lcd.drawLine(x1, y1, x2, y2, COLOR_SHIP_GLOW);
  lcd.drawLine(x2, y2, x3, y3, COLOR_SHIP_GLOW);
  lcd.drawLine(x3, y3, x1, y1, COLOR_SHIP_GLOW);

  // Schiff-Dreieck (heller)
  lcd.drawLine(x1-1, y1, x2-1, y2, COLOR_SHIP);
  lcd.drawLine(x2, y2-1, x3, y3-1, COLOR_SHIP);
  lcd.drawLine(x3+1, y3, x1+1, y1, COLOR_SHIP);

  // Schub-Flamme wenn Thrust gedrückt
  if (digitalRead(BTN_THRUST) == LOW) {
    int fx1 = ship.x + cos(rad + PI) * (SHIP_SIZE - 2);
    int fy1 = ship.y + sin(rad + PI) * (SHIP_SIZE - 2);

    int fx2 = ship.x + cos(rad + PI) * (SHIP_SIZE + 5);
    int fy2 = ship.y + sin(rad + PI) * (SHIP_SIZE + 5);

    // Glow für Flamme
    lcd.drawLine(fx1, fy1, fx2, fy2, COLOR_THRUST_GLOW);
    lcd.drawLine(fx1-1, fy1, fx2-1, fy2, COLOR_THRUST);
    lcd.drawLine(fx1+1, fy1, fx2+1, fy2, COLOR_THRUST);
  }
}

void eraseShip() {
  // Bereich um alte Schiff-Position löschen (inkl. Thrust-Flamme)
  int eraseSize = SHIP_SIZE + 7;
  for (int y = ship.oldY - eraseSize; y <= ship.oldY + eraseSize; y++) {
    if (y < 0 || y >= SCREEN_HEIGHT) continue;
    uint16_t bgColor = getBgColor(y);
    lcd.drawFastHLine(ship.oldX - eraseSize, y, eraseSize * 2, bgColor);
  }
}

void drawAsteroid(int index) {
  Asteroid* ast = &asteroids[index];

  int x = (int)ast->x;
  int y = (int)ast->y;
  int r = ast->radius;

  // Glow-Effekt (größerer Kreis, dunklere Farbe)
  lcd.drawCircle(x, y, r + 1, COLOR_ASTEROID_GLOW);

  // Asteroid (Hauptkreis)
  lcd.drawCircle(x, y, r, COLOR_ASTEROID);

  // Innerer Kreis für mehr Detail
  if (r > 8) {
    lcd.drawCircle(x, y, r - 3, COLOR_ASTEROID);
  }
}

void eraseAsteroid(int index) {
  Asteroid* ast = &asteroids[index];

  int x = (int)ast->oldX;
  int y = (int)ast->oldY;
  int r = ast->radius;

  // Alte Position mit Glow löschen
  for (int dy = -r - 2; dy <= r + 2; dy++) {
    int py = y + dy;
    if (py < 0 || py >= SCREEN_HEIGHT) continue;

    int dx = sqrt((r + 2) * (r + 2) - dy * dy);
    uint16_t bgColor = getBgColor(py);
    lcd.drawFastHLine(x - dx, py, dx * 2 + 1, bgColor);
  }
}

void drawBullet(int index) {
  Bullet* b = &bullets[index];

  int x = (int)b->x;
  int y = (int)b->y;

  // Trail (Linie von alter zu neuer Position)
  if (b->life < BULLET_LIFE - 1) {
    lcd.drawLine((int)b->oldX, (int)b->oldY, x, y, COLOR_BULLET_GLOW);
  }

  // Glow
  lcd.fillCircle(x, y, 2, COLOR_BULLET_GLOW);

  // Heller Kern
  lcd.fillCircle(x, y, 1, COLOR_BULLET);
}

void eraseBullet(int index) {
  Bullet* b = &bullets[index];

  // Trail und Glow löschen
  int x1 = (int)b->oldX;
  int y1 = (int)b->oldY;
  int x2 = (int)b->x;
  int y2 = (int)b->y;

  // Bereich um Trail löschen
  int minX = min(x1, x2) - 3;
  int maxX = max(x1, x2) + 3;
  int minY = min(y1, y2) - 3;
  int maxY = max(y1, y2) + 3;

  for (int y = minY; y <= maxY; y++) {
    if (y < 0 || y >= SCREEN_HEIGHT) continue;
    uint16_t bgColor = getBgColor(y);
    lcd.drawFastHLine(minX, y, maxX - minX + 1, bgColor);
  }
}

void drawUI() {
  // Oberer Bereich mit Gradient löschen
  for (int y = 0; y < 25; y++) {
    uint16_t bgColor = getBgColor(y);
    lcd.drawFastHLine(0, y, SCREEN_WIDTH, bgColor);
  }

  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT, COLOR_BG_TOP);

  // Score (links)
  lcd.setCursor(5, 5);
  lcd.printf("S:%d", score);

  // Level (Mitte)
  lcd.setCursor(SCREEN_WIDTH/2 - 30, 5);
  lcd.printf("L:%d", level);

  // Lives (rechts)
  lcd.setCursor(SCREEN_WIDTH - 80, 5);
  lcd.printf("Lvs:%d", lives);
}

void drawGame() {
  // Altes Schiff löschen (wenn Position sich geändert hat)
  if (ship.oldX != ship.x || ship.oldY != ship.y || ship.oldAngle != ship.angle) {
    eraseShip();
    ship.oldX = ship.x;
    ship.oldY = ship.y;
    ship.oldAngle = ship.angle;
  }

  // Alte Asteroids löschen und neue zeichnen
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    if (!asteroids[i].active) continue;

    if (asteroids[i].oldX != asteroids[i].x || asteroids[i].oldY != asteroids[i].y) {
      eraseAsteroid(i);
    }
    drawAsteroid(i);
  }

  // Alte Bullets löschen und neue zeichnen
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) continue;

    if (bullets[i].oldX != bullets[i].x || bullets[i].oldY != bullets[i].y) {
      eraseBullet(i);
    }
    drawBullet(i);
  }

  // Schiff zeichnen (wird oben gedrawn)
  drawShip();
}
