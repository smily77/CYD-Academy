/*
  Space Invaders Modern - Arcade-Spiel mit modernen Effekten

  Hardware:
  - CYD Display (320x240)
  - 4x Digitale Buttons:
    * GPIO 5:  Links
    * GPIO 16: Rechts
    * GPIO 17: Schießen
    * GPIO 18: Pause

  Steuerung:
  - Links/Rechts: Bewegt Raumschiff horizontal
  - Schießen: Feuert Geschoss
  - Pause: Pausiert das Spiel

  Moderne Features:
  - Gradient-Hintergrund (Weltraum)
  - Glow-Effekte für Spieler und Aliens
  - Partikel-Explosionen
  - Moderne UI
  - Animierte Schilde
  - Schönere Bullets mit Trails
*/

#include <CYD_Display_Config.h>

// Pin-Mapping: Physische Pins -> Logische Namen
#define BTN_LEFT   tasteB
#define BTN_RIGHT  tasteC
#define BTN_SHOOT  tasteA
#define BTN_PAUSE  tasteD

// Display Objekt
LGFX lcd;

// Display-Dimensionen
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// Farben - Moderne Palette
#define COLOR_BG_TOP      0x0015  // Dunkles Blau (oben)
#define COLOR_BG_BOTTOM   0x0000  // Schwarz (unten)
#define COLOR_PLAYER      0x07FF  // Cyan
#define COLOR_PLAYER_GLOW 0x0410  // Dunkles Cyan
#define COLOR_ALIEN1      0xF800  // Rot
#define COLOR_ALIEN2      0xFD20  // Orange
#define COLOR_ALIEN3      0xFFE0  // Gelb
#define COLOR_ALIEN4      0x07E0  // Grün
#define COLOR_ALIEN5      0x07FF  // Cyan
#define COLOR_BULLET      0xFFFF  // Weiß
#define COLOR_BULLET_GLOW 0x8410  // Grau
#define COLOR_SHIELD      0x07E0  // Grün
#define COLOR_SHIELD_GLOW 0x0400  // Dunkles Grün
#define COLOR_TEXT        0xFFFF  // Weiß
#define COLOR_PARTICLE1   0xFFE0  // Gelb
#define COLOR_PARTICLE2   0xFD20  // Orange
#define COLOR_PARTICLE3   0xF800  // Rot

// Spieler
struct Player {
  int x, y;
  int w, h;
  uint16_t color;
  int lives;
};

// Alien
struct Alien {
  int x, y;
  int w, h;
  uint16_t color;
  int points;
  bool active;
};

// Geschoss
struct Bullet {
  int x, y;
  int w, h;
  int vy;
  bool active;
};

// Schild
struct Shield {
  int x, y;
  int w, h;
  uint8_t health;  // 0-100
};

// Partikel für Explosionen
struct Particle {
  float x, y;
  float vx, vy;
  uint16_t color;
  uint8_t life;  // 0-100
  bool active;
};

// Game Objekte
Player player;

// Aliens (5 Reihen x 11 Spalten = 55)
#define ALIEN_ROWS 5
#define ALIEN_COLS 11
#define ALIEN_COUNT (ALIEN_ROWS * ALIEN_COLS)
Alien aliens[ALIEN_COUNT];

// Geschosse
#define MAX_PLAYER_BULLETS 3
#define MAX_ALIEN_BULLETS 5
Bullet playerBullets[MAX_PLAYER_BULLETS];
Bullet alienBullets[MAX_ALIEN_BULLETS];

// Schilde (4 Stück)
#define SHIELD_COUNT 4
Shield shields[SHIELD_COUNT];

// Partikel
#define MAX_PARTICLES 30
Particle particles[MAX_PARTICLES];

// Alien-Bewegung
int alienDirection = 1;  // 1 = rechts, -1 = links
int alienStepX = 4;
int alienStepY = 8;
unsigned long lastAlienMove = 0;
int alienMoveDelay = 1000;  // ms zwischen Alien-Bewegungen

// Game State
int score = 0;
int level = 1;
bool gameOver = false;
bool paused = false;
unsigned long lastAlienShot = 0;

// Alte Position für Player
int oldPlayerX = -1;

// Background gradient flag
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
  Serial.println("\n=== SPACE INVADERS MODERN ===\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);
  lcd.fillScreen(COLOR_BG_BOTTOM);
  lcd.setBrightness(255);

  // Button Inputs konfigurieren
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_SHOOT, INPUT_PULLUP);
  pinMode(BTN_PAUSE, INPUT_PULLUP);

  // Zufallsgenerator
  randomSeed(analogRead(34) + analogRead(35) + micros());

  // Spiel initialisieren
  initGame();

  Serial.println("Space Invaders Modern gestartet!");
  Serial.println("Steuerung:");
  Serial.println("  Links:    GPIO 5");
  Serial.println("  Rechts:   GPIO 16");
  Serial.println("  Schießen: GPIO 17");
  Serial.println("  Pause:    GPIO 18");
}

void loop() {
  if (gameOver) {
    // Warte auf Neustart-Button
    if (digitalRead(BTN_SHOOT) == LOW) {
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
    lcd.setTextColor(COLOR_TEXT, COLOR_BG_TOP);
    lcd.setCursor(SCREEN_WIDTH/2 - 60, SCREEN_HEIGHT/2 - 15);
    lcd.println("PAUSE");
    delay(100);
    return;
  }

  // Spieler bewegen
  updatePlayer();

  // Spieler schießen
  if (digitalRead(BTN_SHOOT) == LOW) {
    shootPlayerBullet();
    delay(100);  // Entprellen
  }

  // Aliens bewegen (zeitgesteuert)
  if (millis() - lastAlienMove > alienMoveDelay) {
    updateAliens();
    lastAlienMove = millis();
  }

  // Alien-Schüsse (zufällig)
  if (millis() - lastAlienShot > 1000) {
    shootAlienBullet();
    lastAlienShot = millis();
  }

  // Geschosse aktualisieren
  updateBullets();

  // Partikel aktualisieren
  updateParticles();

  // Kollisionen prüfen
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

  delay(16);  // ~60 FPS
}

// ===== INITIALISIERUNG =====

void initGame() {
  Serial.println("Initialisiere Spiel...");

  score = 0;
  level = 1;
  gameOver = false;
  paused = false;
  backgroundDrawn = false;

  // Spieler initialisieren
  player.w = 16;
  player.h = 10;
  player.x = SCREEN_WIDTH / 2 - player.w / 2;
  player.y = SCREEN_HEIGHT - 30;
  player.color = COLOR_PLAYER;
  player.lives = 3;
  oldPlayerX = -1;

  // Geschosse initialisieren
  for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
    playerBullets[i].active = false;
  }
  for (int i = 0; i < MAX_ALIEN_BULLETS; i++) {
    alienBullets[i].active = false;
  }

  // Partikel initialisieren
  for (int i = 0; i < MAX_PARTICLES; i++) {
    particles[i].active = false;
  }

  // Schilde initialisieren
  initShields();

  // Level initialisieren
  initLevel();

  // Bildschirm aufbauen
  drawBackground();
  drawShields();
  drawUI();
}

void initLevel() {
  Serial.printf("Level %d gestartet!\n", level);

  // Alien-Dimensionen
  int alienW = 14;
  int alienH = 10;
  int alienSpacingX = 6;
  int alienSpacingY = 6;
  int startX = 30;
  int startY = 40;

  // Farben für Reihen
  uint16_t rowColors[ALIEN_ROWS] = {
    COLOR_ALIEN1,  // Rot - 30 Punkte
    COLOR_ALIEN2,  // Orange - 20 Punkte
    COLOR_ALIEN3,  // Gelb - 20 Punkte
    COLOR_ALIEN4,  // Grün - 10 Punkte
    COLOR_ALIEN5   // Cyan - 10 Punkte
  };

  int rowPoints[ALIEN_ROWS] = {30, 20, 20, 10, 10};

  // Aliens erstellen
  int index = 0;
  for (int row = 0; row < ALIEN_ROWS; row++) {
    for (int col = 0; col < ALIEN_COLS; col++) {
      aliens[index].x = startX + col * (alienW + alienSpacingX);
      aliens[index].y = startY + row * (alienH + alienSpacingY);
      aliens[index].w = alienW;
      aliens[index].h = alienH;
      aliens[index].color = rowColors[row];
      aliens[index].points = rowPoints[row];
      aliens[index].active = true;
      index++;
    }
  }

  // Alien-Bewegung zurücksetzen
  alienDirection = 1;
  alienMoveDelay = max(300, 1000 - (level - 1) * 100);  // Schneller mit jedem Level

  lastAlienMove = millis();
  lastAlienShot = millis();

  // Aliens zeichnen
  drawAliens();
}

void initShields() {
  int shieldW = 40;
  int shieldH = 30;
  int shieldY = SCREEN_HEIGHT - 70;
  int spacing = (SCREEN_WIDTH - SHIELD_COUNT * shieldW) / (SHIELD_COUNT + 1);

  for (int i = 0; i < SHIELD_COUNT; i++) {
    shields[i].x = spacing + i * (shieldW + spacing);
    shields[i].y = shieldY;
    shields[i].w = shieldW;
    shields[i].h = shieldH;
    shields[i].health = 100;
  }
}

// ===== UPDATE FUNKTIONEN =====

void updatePlayer() {
  int newX = player.x;

  if (digitalRead(BTN_LEFT) == LOW) {
    newX -= 1;
  }
  if (digitalRead(BTN_RIGHT) == LOW) {
    newX += 1;
  }

  newX = constrain(newX, 0, SCREEN_WIDTH - player.w);

  if (newX != player.x) {
    player.x = newX;
  }
}

void updateAliens() {
  // Grenzen finden
  int leftMost = SCREEN_WIDTH;
  int rightMost = 0;
  bool needMoveDown = false;

  for (int i = 0; i < ALIEN_COUNT; i++) {
    if (!aliens[i].active) continue;

    if (aliens[i].x < leftMost) leftMost = aliens[i].x;
    if (aliens[i].x + aliens[i].w > rightMost) rightMost = aliens[i].x + aliens[i].w;
  }

  // Richtungswechsel prüfen
  if (alienDirection == 1 && rightMost + alienStepX >= SCREEN_WIDTH) {
    alienDirection = -1;
    needMoveDown = true;
  } else if (alienDirection == -1 && leftMost - alienStepX <= 0) {
    alienDirection = 1;
    needMoveDown = true;
  }

  // Aliens bewegen
  for (int i = 0; i < ALIEN_COUNT; i++) {
    if (!aliens[i].active) continue;

    // Alte Position mit Glow löschen
    eraseAlien(i);

    if (needMoveDown) {
      aliens[i].y += alienStepY;
    } else {
      aliens[i].x += alienStepX * alienDirection;
    }

    // Neue Position zeichnen
    drawAlien(i);

    // Game Over wenn Aliens unten ankommen
    if (aliens[i].y + aliens[i].h >= player.y) {
      handleGameOver();
      return;
    }
  }
}

void shootPlayerBullet() {
  // Freies Geschoss finden
  for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
    if (!playerBullets[i].active) {
      playerBullets[i].x = player.x + player.w / 2 - 1;
      playerBullets[i].y = player.y - 5;
      playerBullets[i].w = 2;
      playerBullets[i].h = 5;
      playerBullets[i].vy = -5;
      playerBullets[i].active = true;
      Serial.println("Player shoot!");
      return;
    }
  }
}

void shootAlienBullet() {
  // Zufälligen aktiven Alien finden
  int activeAliens[ALIEN_COUNT];
  int activeCount = 0;

  for (int i = 0; i < ALIEN_COUNT; i++) {
    if (aliens[i].active) {
      activeAliens[activeCount++] = i;
    }
  }

  if (activeCount == 0) return;

  int shooterIndex = activeAliens[random(activeCount)];
  Alien* shooter = &aliens[shooterIndex];

  // Freies Geschoss finden
  for (int i = 0; i < MAX_ALIEN_BULLETS; i++) {
    if (!alienBullets[i].active) {
      alienBullets[i].x = shooter->x + shooter->w / 2 - 1;
      alienBullets[i].y = shooter->y + shooter->h;
      alienBullets[i].w = 2;
      alienBullets[i].h = 5;
      alienBullets[i].vy = 3;
      alienBullets[i].active = true;
      return;
    }
  }
}

void updateBullets() {
  // Spieler-Geschosse
  for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
    if (!playerBullets[i].active) continue;

    // Alte Position mit Glow löschen
    int oldY = playerBullets[i].y;
    uint16_t bgColor = getBgColor(oldY + playerBullets[i].h / 2);
    lcd.fillRect(playerBullets[i].x - 1, oldY - 1,
                 playerBullets[i].w + 2, playerBullets[i].h + 2, bgColor);

    playerBullets[i].y += playerBullets[i].vy;

    // Außerhalb des Bildschirms?
    if (playerBullets[i].y < 0) {
      playerBullets[i].active = false;
      continue;
    }

    // Neue Position mit Glow zeichnen
    int newY = playerBullets[i].y;
    // Glow
    lcd.fillRect(playerBullets[i].x - 1, newY - 1,
                 playerBullets[i].w + 2, playerBullets[i].h + 2, COLOR_BULLET_GLOW);
    // Core
    lcd.fillRect(playerBullets[i].x, newY,
                 playerBullets[i].w, playerBullets[i].h, COLOR_BULLET);
  }

  // Alien-Geschosse
  for (int i = 0; i < MAX_ALIEN_BULLETS; i++) {
    if (!alienBullets[i].active) continue;

    // Alte Position löschen
    int oldY = alienBullets[i].y;
    uint16_t bgColor = getBgColor(oldY + alienBullets[i].h / 2);
    lcd.fillRect(alienBullets[i].x - 1, oldY - 1,
                 alienBullets[i].w + 2, alienBullets[i].h + 2, bgColor);

    alienBullets[i].y += alienBullets[i].vy;

    // Außerhalb des Bildschirms?
    if (alienBullets[i].y > SCREEN_HEIGHT) {
      alienBullets[i].active = false;
      continue;
    }

    // Neue Position mit Glow zeichnen
    int newY = alienBullets[i].y;
    // Glow
    lcd.fillRect(alienBullets[i].x - 1, newY - 1,
                 alienBullets[i].w + 2, alienBullets[i].h + 2, COLOR_BULLET_GLOW);
    // Core
    lcd.fillRect(alienBullets[i].x, newY,
                 alienBullets[i].w, alienBullets[i].h, COLOR_BULLET);
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

void spawnExplosion(int x, int y, uint16_t color) {
  // 8-12 Partikel spawnen
  int numParticles = random(8, 13);

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
  // Spieler-Geschosse vs Aliens
  for (int b = 0; b < MAX_PLAYER_BULLETS; b++) {
    if (!playerBullets[b].active) continue;

    for (int a = 0; a < ALIEN_COUNT; a++) {
      if (!aliens[a].active) continue;

      if (playerBullets[b].x + playerBullets[b].w >= aliens[a].x &&
          playerBullets[b].x <= aliens[a].x + aliens[a].w &&
          playerBullets[b].y + playerBullets[b].h >= aliens[a].y &&
          playerBullets[b].y <= aliens[a].y + aliens[a].h) {

        // Explosion spawnen
        spawnExplosion(aliens[a].x + aliens[a].w / 2,
                      aliens[a].y + aliens[a].h / 2,
                      aliens[a].color);

        // Alien zerstören
        aliens[a].active = false;
        score += aliens[a].points;

        // Alien mit Glow löschen
        eraseAlien(a);

        // Geschoss löschen
        uint16_t bgColor = getBgColor(playerBullets[b].y);
        lcd.fillRect(playerBullets[b].x - 1, playerBullets[b].y - 1,
                     playerBullets[b].w + 2, playerBullets[b].h + 2, bgColor);
        playerBullets[b].active = false;

        Serial.printf("Alien zerstört! Score: %d\n", score);
        drawUI();
        break;
      }
    }
  }

  // Spieler-Geschosse vs Schilde
  for (int b = 0; b < MAX_PLAYER_BULLETS; b++) {
    if (!playerBullets[b].active) continue;

    for (int s = 0; s < SHIELD_COUNT; s++) {
      if (shields[s].health == 0) continue;

      if (playerBullets[b].x + playerBullets[b].w >= shields[s].x &&
          playerBullets[b].x <= shields[s].x + shields[s].w &&
          playerBullets[b].y + playerBullets[b].h >= shields[s].y &&
          playerBullets[b].y <= shields[s].y + shields[s].h) {

        // Schild beschädigen
        shields[s].health = max(0, shields[s].health - 10);

        // Geschoss löschen
        uint16_t bgColor = getBgColor(playerBullets[b].y);
        lcd.fillRect(playerBullets[b].x - 1, playerBullets[b].y - 1,
                     playerBullets[b].w + 2, playerBullets[b].h + 2, bgColor);
        playerBullets[b].active = false;

        // Schild neu zeichnen
        drawShield(s);
        break;
      }
    }
  }

  // Alien-Geschosse vs Spieler
  for (int b = 0; b < MAX_ALIEN_BULLETS; b++) {
    if (!alienBullets[b].active) continue;

    if (alienBullets[b].x + alienBullets[b].w >= player.x &&
        alienBullets[b].x <= player.x + player.w &&
        alienBullets[b].y + alienBullets[b].h >= player.y &&
        alienBullets[b].y <= player.y + player.h) {

      // Explosion spawnen
      spawnExplosion(player.x + player.w / 2, player.y + player.h / 2, COLOR_PLAYER);

      // Geschoss löschen
      uint16_t bgColor = getBgColor(alienBullets[b].y);
      lcd.fillRect(alienBullets[b].x - 1, alienBullets[b].y - 1,
                   alienBullets[b].w + 2, alienBullets[b].h + 2, bgColor);
      alienBullets[b].active = false;

      // Leben verlieren
      player.lives--;
      Serial.printf("Getroffen! Leben: %d\n", player.lives);
      drawUI();

      if (player.lives <= 0) {
        handleGameOver();
      }
      break;
    }
  }

  // Alien-Geschosse vs Schilde
  for (int b = 0; b < MAX_ALIEN_BULLETS; b++) {
    if (!alienBullets[b].active) continue;

    for (int s = 0; s < SHIELD_COUNT; s++) {
      if (shields[s].health == 0) continue;

      if (alienBullets[b].x + alienBullets[b].w >= shields[s].x &&
          alienBullets[b].x <= shields[s].x + shields[s].w &&
          alienBullets[b].y + alienBullets[b].h >= shields[s].y &&
          alienBullets[b].y <= shields[s].y + shields[s].h) {

        // Schild beschädigen
        shields[s].health = max(0, shields[s].health - 10);

        // Geschoss löschen
        uint16_t bgColor = getBgColor(alienBullets[b].y);
        lcd.fillRect(alienBullets[b].x - 1, alienBullets[b].y - 1,
                     alienBullets[b].w + 2, alienBullets[b].h + 2, bgColor);
        alienBullets[b].active = false;

        // Schild neu zeichnen
        drawShield(s);
        break;
      }
    }
  }
}

bool checkLevelComplete() {
  for (int i = 0; i < ALIEN_COUNT; i++) {
    if (aliens[i].active) return false;
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

void drawAliens() {
  for (int i = 0; i < ALIEN_COUNT; i++) {
    if (aliens[i].active) {
      drawAlien(i);
    }
  }
}

void drawAlien(int index) {
  Alien* a = &aliens[index];

  // Glow-Effekt (1 Pixel größer, halbtransparent simuliert durch dunklere Farbe)
  uint16_t glowColor = lerpColor(a->color, getBgColor(a->y + a->h / 2), 0.7);
  lcd.fillRect(a->x - 1, a->y - 1, a->w + 2, a->h + 2, glowColor);

  // Alien
  lcd.fillRect(a->x, a->y, a->w, a->h, a->color);
}

void eraseAlien(int index) {
  Alien* a = &aliens[index];

  // Alien mit Glow löschen (inkl. 1 Pixel Rand)
  for (int y = a->y - 1; y <= a->y + a->h; y++) {
    uint16_t bgColor = getBgColor(y);
    lcd.drawFastHLine(a->x - 1, y, a->w + 2, bgColor);
  }
}

void drawShields() {
  for (int i = 0; i < SHIELD_COUNT; i++) {
    drawShield(i);
  }
}

void drawShield(int index) {
  Shield* s = &shields[index];

  if (s->health == 0) {
    // Komplett zerstört - mit Gradient löschen
    for (int y = s->y; y < s->y + s->h; y++) {
      uint16_t bgColor = getBgColor(y);
      lcd.drawFastHLine(s->x, y, s->w, bgColor);
    }
  } else {
    // Zeichnen mit Transparenz-Effekt (weniger opak bei weniger Health)
    if (s->health < 50) {
      // Beschädigt - mit Löchern zeichnen
      for (int y = 0; y < s->h; y++) {
        uint16_t bgColor = getBgColor(s->y + y);
        for (int x = 0; x < s->w; x++) {
          if ((x + y) % 2 == 0) {
            lcd.drawPixel(s->x + x, s->y + y, COLOR_SHIELD);
          } else {
            lcd.drawPixel(s->x + x, s->y + y, bgColor);
          }
        }
      }
    } else {
      // Gesund - mit Glow
      // Glow (außen)
      for (int y = s->y; y < s->y + s->h; y++) {
        lcd.drawPixel(s->x - 1, y, COLOR_SHIELD_GLOW);
        lcd.drawPixel(s->x + s->w, y, COLOR_SHIELD_GLOW);
      }
      lcd.drawFastHLine(s->x - 1, s->y - 1, s->w + 2, COLOR_SHIELD_GLOW);
      lcd.drawFastHLine(s->x - 1, s->y + s->h, s->w + 2, COLOR_SHIELD_GLOW);

      // Solider Schild
      lcd.fillRect(s->x, s->y, s->w, s->h, COLOR_SHIELD);
    }
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
  lcd.printf("Lvs:%d", player.lives);
}

void drawGame() {
  // Spieler zeichnen - nur wenn Position sich geändert hat
  if (oldPlayerX != player.x) {
    // Alten mit Glow löschen
    if (oldPlayerX >= 0) {
      for (int y = player.y - 2; y <= player.y + player.h + 1; y++) {
        uint16_t bgColor = getBgColor(y);
        lcd.drawFastHLine(oldPlayerX - 2, y, player.w + 4, bgColor);
      }
    }

    // Neuen mit Glow zeichnen
    // Glow (größer, dunklere Farbe)
    lcd.fillRect(player.x - 2, player.y - 2, player.w + 4, player.h + 4, COLOR_PLAYER_GLOW);

    // Player
    lcd.fillRect(player.x, player.y, player.w, player.h, player.color);

    oldPlayerX = player.x;
  }
}
