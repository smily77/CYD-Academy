/*
  SpaceInvadersGameModern.h - Arcade-Spiel mit modernen Effekten als Klasse gekapselt

  Teil der CYD_Games Library für CYD Display

  Verwendung:
    SpaceInvadersGameModern game;
    game.init(&lcd);
    // In loop():
    game.update();
*/

#ifndef SPACEINVADERSGAMEMODERN_H
#define SPACEINVADERSGAMEMODERN_H

#include <Arduino.h>
#include <LovyanGFX.hpp>

// Forward declaration
class LGFX;

// ===== PIN DEFINITIONS =====
#define SI_BTN_LEFT   5   // tasteB
#define SI_BTN_RIGHT  16  // tasteC
#define SI_BTN_SHOOT  17  // tasteA
#define SI_BTN_PAUSE  18  // tasteD

// ===== DISPLAY DIMENSIONS =====
#define SI_SCREEN_WIDTH  320
#define SI_SCREEN_HEIGHT 240

// ===== MODERNE FARBPALETTE =====
#define SI_COLOR_BG_TOP      0x0015  // Dunkles Blau (oben)
#define SI_COLOR_BG_BOTTOM   0x0000  // Schwarz (unten)
#define SI_COLOR_PLAYER      0x07FF  // Cyan
#define SI_COLOR_PLAYER_GLOW 0x0410  // Dunkles Cyan
#define SI_COLOR_ALIEN1      0xF800  // Rot
#define SI_COLOR_ALIEN2      0xFD20  // Orange
#define SI_COLOR_ALIEN3      0xFFE0  // Gelb
#define SI_COLOR_ALIEN4      0x07E0  // Grün
#define SI_COLOR_ALIEN5      0x07FF  // Cyan
#define SI_COLOR_BULLET      0xFFFF  // Weiß
#define SI_COLOR_BULLET_GLOW 0x8410  // Grau
#define SI_COLOR_SHIELD      0x07E0  // Grün
#define SI_COLOR_SHIELD_GLOW 0x0400  // Dunkles Grün
#define SI_COLOR_TEXT        0xFFFF  // Weiß
#define SI_COLOR_PARTICLE1   0xFFE0  // Gelb
#define SI_COLOR_PARTICLE2   0xFD20  // Orange
#define SI_COLOR_PARTICLE3   0xF800  // Rot

// ===== GAME CONSTANTS =====
#define SI_ALIEN_ROWS 5
#define SI_ALIEN_COLS 11
#define SI_ALIEN_COUNT (SI_ALIEN_ROWS * SI_ALIEN_COLS)
#define SI_MAX_PLAYER_BULLETS 3
#define SI_MAX_ALIEN_BULLETS 5
#define SI_SHIELD_COUNT 4
#define SI_MAX_PARTICLES 30

// ===== STRUCTS =====
struct SpaceInvPlayer {
  int x, y;
  int w, h;
  uint16_t color;
  int lives;
};

struct SpaceInvAlien {
  int x, y;
  int w, h;
  uint16_t color;
  int points;
  bool active;
};

struct SpaceInvBullet {
  int x, y;
  int w, h;
  int vy;
  bool active;
};

struct SpaceInvShield {
  int x, y;
  int w, h;
  uint8_t health;  // 0-100
};

struct SpaceInvParticle {
  float x, y;
  float vx, vy;
  uint16_t color;
  uint8_t life;  // 0-100
  bool active;
};

// ===== CLASS DEFINITION =====
class SpaceInvadersGameModern {
public:
  SpaceInvadersGameModern() :
    lcd(nullptr),
    alienDirection(1),
    alienStepX(4),
    alienStepY(8),
    lastAlienMove(0),
    alienMoveDelay(1000),
    score(0),
    level(1),
    gameOver(false),
    paused(false),
    lastAlienShot(0),
    oldPlayerX(-1),
    backgroundDrawn(false),
    lastPauseState(HIGH)
  {
    // Arrays werden im init() initialisiert
  }

  void init(LGFX* display) {
    lcd = display;

    Serial.println("Initialisiere Space Invaders Modern...");

    // Button Inputs konfigurieren
    pinMode(SI_BTN_LEFT, INPUT_PULLUP);
    pinMode(SI_BTN_RIGHT, INPUT_PULLUP);
    pinMode(SI_BTN_SHOOT, INPUT_PULLUP);
    pinMode(SI_BTN_PAUSE, INPUT_PULLUP);

    // Zufallsgenerator
    randomSeed(analogRead(34) + analogRead(35) + micros());

    // Spiel initialisieren
    initGame();

    Serial.println("Space Invaders Modern bereit!");
  }

  void update() {
    if (gameOver) {
      // Warte auf Neustart-Button
      if (digitalRead(SI_BTN_SHOOT) == LOW) {
        initGame();
        gameOver = false;
        delay(300);
      }
      return;
    }

    // Pause-Button
    int pauseState = digitalRead(SI_BTN_PAUSE);
    if (pauseState == LOW && lastPauseState == HIGH) {
      paused = !paused;
      Serial.println(paused ? "PAUSE" : "WEITER");
      delay(200);
    }
    lastPauseState = pauseState;

    if (paused) {
      // Zeige Pause-Text
      lcd->setTextSize(3);
      lcd->setTextColor(SI_COLOR_TEXT, SI_COLOR_BG_TOP);
      lcd->setCursor(SI_SCREEN_WIDTH/2 - 60, SI_SCREEN_HEIGHT/2 - 15);
      lcd->println("PAUSE");
      delay(100);
      return;
    }

    // Spieler bewegen
    updatePlayer();

    // Spieler schießen
    if (digitalRead(SI_BTN_SHOOT) == LOW) {
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

  bool isGameOver() { return gameOver; }
  int getScore() { return score; }
  int getLevel() { return level; }
  int getLives() { return player.lives; }

private:
  LGFX* lcd;

  // Game objects
  SpaceInvPlayer player;
  SpaceInvAlien aliens[SI_ALIEN_COUNT];
  SpaceInvBullet playerBullets[SI_MAX_PLAYER_BULLETS];
  SpaceInvBullet alienBullets[SI_MAX_ALIEN_BULLETS];
  SpaceInvShield shields[SI_SHIELD_COUNT];
  SpaceInvParticle particles[SI_MAX_PARTICLES];

  // Alien-Bewegung
  int alienDirection;  // 1 = rechts, -1 = links
  int alienStepX;
  int alienStepY;
  unsigned long lastAlienMove;
  int alienMoveDelay;  // ms zwischen Alien-Bewegungen

  // Game State
  int score;
  int level;
  bool gameOver;
  bool paused;
  unsigned long lastAlienShot;

  // Alte Position für Player
  int oldPlayerX;

  // Background gradient flag
  bool backgroundDrawn;

  // Button states
  int lastPauseState;

  // === HILFSFUNKTIONEN ===

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
    return lerpColor(SI_COLOR_BG_TOP, SI_COLOR_BG_BOTTOM, (float)y / SI_SCREEN_HEIGHT);
  }

  // === INITIALISIERUNG ===

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
    player.x = SI_SCREEN_WIDTH / 2 - player.w / 2;
    player.y = SI_SCREEN_HEIGHT - 30;
    player.color = SI_COLOR_PLAYER;
    player.lives = 3;
    oldPlayerX = -1;

    // Geschosse initialisieren
    for (int i = 0; i < SI_MAX_PLAYER_BULLETS; i++) {
      playerBullets[i].active = false;
    }
    for (int i = 0; i < SI_MAX_ALIEN_BULLETS; i++) {
      alienBullets[i].active = false;
    }

    // Partikel initialisieren
    for (int i = 0; i < SI_MAX_PARTICLES; i++) {
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
    uint16_t rowColors[SI_ALIEN_ROWS] = {
      SI_COLOR_ALIEN1,  // Rot - 30 Punkte
      SI_COLOR_ALIEN2,  // Orange - 20 Punkte
      SI_COLOR_ALIEN3,  // Gelb - 20 Punkte
      SI_COLOR_ALIEN4,  // Grün - 10 Punkte
      SI_COLOR_ALIEN5   // Cyan - 10 Punkte
    };

    int rowPoints[SI_ALIEN_ROWS] = {30, 20, 20, 10, 10};

    // Aliens erstellen
    int index = 0;
    for (int row = 0; row < SI_ALIEN_ROWS; row++) {
      for (int col = 0; col < SI_ALIEN_COLS; col++) {
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
    int shieldY = SI_SCREEN_HEIGHT - 70;
    int spacing = (SI_SCREEN_WIDTH - SI_SHIELD_COUNT * shieldW) / (SI_SHIELD_COUNT + 1);

    for (int i = 0; i < SI_SHIELD_COUNT; i++) {
      shields[i].x = spacing + i * (shieldW + spacing);
      shields[i].y = shieldY;
      shields[i].w = shieldW;
      shields[i].h = shieldH;
      shields[i].health = 100;
    }
  }

  // === UPDATE FUNKTIONEN ===

  void updatePlayer() {
    int newX = player.x;

    if (digitalRead(SI_BTN_LEFT) == LOW) {
      newX -= 1;
    }
    if (digitalRead(SI_BTN_RIGHT) == LOW) {
      newX += 1;
    }

    newX = constrain(newX, 0, SI_SCREEN_WIDTH - player.w);

    if (newX != player.x) {
      player.x = newX;
    }
  }

  void updateAliens() {
    // Grenzen finden
    int leftMost = SI_SCREEN_WIDTH;
    int rightMost = 0;
    bool needMoveDown = false;

    for (int i = 0; i < SI_ALIEN_COUNT; i++) {
      if (!aliens[i].active) continue;

      if (aliens[i].x < leftMost) leftMost = aliens[i].x;
      if (aliens[i].x + aliens[i].w > rightMost) rightMost = aliens[i].x + aliens[i].w;
    }

    // Richtungswechsel prüfen
    if (alienDirection == 1 && rightMost + alienStepX >= SI_SCREEN_WIDTH) {
      alienDirection = -1;
      needMoveDown = true;
    } else if (alienDirection == -1 && leftMost - alienStepX <= 0) {
      alienDirection = 1;
      needMoveDown = true;
    }

    // Aliens bewegen
    for (int i = 0; i < SI_ALIEN_COUNT; i++) {
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
    for (int i = 0; i < SI_MAX_PLAYER_BULLETS; i++) {
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
    int activeAliens[SI_ALIEN_COUNT];
    int activeCount = 0;

    for (int i = 0; i < SI_ALIEN_COUNT; i++) {
      if (aliens[i].active) {
        activeAliens[activeCount++] = i;
      }
    }

    if (activeCount == 0) return;

    int shooterIndex = activeAliens[random(activeCount)];
    SpaceInvAlien* shooter = &aliens[shooterIndex];

    // Freies Geschoss finden
    for (int i = 0; i < SI_MAX_ALIEN_BULLETS; i++) {
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
    for (int i = 0; i < SI_MAX_PLAYER_BULLETS; i++) {
      if (!playerBullets[i].active) continue;

      // Alte Position mit Glow löschen
      int oldY = playerBullets[i].y;
      uint16_t bgColor = getBgColor(oldY + playerBullets[i].h / 2);
      lcd->fillRect(playerBullets[i].x - 1, oldY - 1,
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
      lcd->fillRect(playerBullets[i].x - 1, newY - 1,
                   playerBullets[i].w + 2, playerBullets[i].h + 2, SI_COLOR_BULLET_GLOW);
      // Core
      lcd->fillRect(playerBullets[i].x, newY,
                   playerBullets[i].w, playerBullets[i].h, SI_COLOR_BULLET);
    }

    // Alien-Geschosse
    for (int i = 0; i < SI_MAX_ALIEN_BULLETS; i++) {
      if (!alienBullets[i].active) continue;

      // Alte Position löschen
      int oldY = alienBullets[i].y;
      uint16_t bgColor = getBgColor(oldY + alienBullets[i].h / 2);
      lcd->fillRect(alienBullets[i].x - 1, oldY - 1,
                   alienBullets[i].w + 2, alienBullets[i].h + 2, bgColor);

      alienBullets[i].y += alienBullets[i].vy;

      // Außerhalb des Bildschirms?
      if (alienBullets[i].y > SI_SCREEN_HEIGHT) {
        alienBullets[i].active = false;
        continue;
      }

      // Neue Position mit Glow zeichnen
      int newY = alienBullets[i].y;
      // Glow
      lcd->fillRect(alienBullets[i].x - 1, newY - 1,
                   alienBullets[i].w + 2, alienBullets[i].h + 2, SI_COLOR_BULLET_GLOW);
      // Core
      lcd->fillRect(alienBullets[i].x, newY,
                   alienBullets[i].w, alienBullets[i].h, SI_COLOR_BULLET);
    }
  }

  void updateParticles() {
    for (int i = 0; i < SI_MAX_PARTICLES; i++) {
      if (!particles[i].active) continue;

      // Alte Position löschen
      uint16_t bgColor = getBgColor((int)particles[i].y);
      lcd->fillCircle((int)particles[i].x, (int)particles[i].y, 1, bgColor);

      // Bewegen
      particles[i].x += particles[i].vx;
      particles[i].y += particles[i].vy;
      particles[i].vy += 0.2;  // Gravitation
      particles[i].life -= 5;

      // Deaktivieren wenn tot oder außerhalb
      if (particles[i].life <= 0 ||
          particles[i].y > SI_SCREEN_HEIGHT ||
          particles[i].x < 0 || particles[i].x > SI_SCREEN_WIDTH) {
        particles[i].active = false;
        continue;
      }

      // Neue Position zeichnen
      lcd->fillCircle((int)particles[i].x, (int)particles[i].y, 1, particles[i].color);
    }
  }

  void spawnExplosion(int x, int y, uint16_t color) {
    // 8-12 Partikel spawnen
    int numParticles = random(8, 13);

    for (int i = 0; i < numParticles; i++) {
      // Freies Partikel finden
      for (int p = 0; p < SI_MAX_PARTICLES; p++) {
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
          if (colorChoice == 0) particles[p].color = SI_COLOR_PARTICLE1;
          else if (colorChoice == 1) particles[p].color = SI_COLOR_PARTICLE2;
          else particles[p].color = SI_COLOR_PARTICLE3;

          particles[p].life = 100;
          particles[p].active = true;
          break;
        }
      }
    }
  }

  // === KOLLISIONEN ===

  void checkCollisions() {
    // Spieler-Geschosse vs Aliens
    for (int b = 0; b < SI_MAX_PLAYER_BULLETS; b++) {
      if (!playerBullets[b].active) continue;

      for (int a = 0; a < SI_ALIEN_COUNT; a++) {
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
          lcd->fillRect(playerBullets[b].x - 1, playerBullets[b].y - 1,
                       playerBullets[b].w + 2, playerBullets[b].h + 2, bgColor);
          playerBullets[b].active = false;

          Serial.printf("Alien zerstört! Score: %d\n", score);
          drawUI();
          break;
        }
      }
    }

    // Spieler-Geschosse vs Schilde
    for (int b = 0; b < SI_MAX_PLAYER_BULLETS; b++) {
      if (!playerBullets[b].active) continue;

      for (int s = 0; s < SI_SHIELD_COUNT; s++) {
        if (shields[s].health == 0) continue;

        if (playerBullets[b].x + playerBullets[b].w >= shields[s].x &&
            playerBullets[b].x <= shields[s].x + shields[s].w &&
            playerBullets[b].y + playerBullets[b].h >= shields[s].y &&
            playerBullets[b].y <= shields[s].y + shields[s].h) {

          // Schild beschädigen
          shields[s].health = max(0, shields[s].health - 10);

          // Geschoss löschen
          uint16_t bgColor = getBgColor(playerBullets[b].y);
          lcd->fillRect(playerBullets[b].x - 1, playerBullets[b].y - 1,
                       playerBullets[b].w + 2, playerBullets[b].h + 2, bgColor);
          playerBullets[b].active = false;

          // Schild neu zeichnen
          drawShield(s);
          break;
        }
      }
    }

    // Alien-Geschosse vs Spieler
    for (int b = 0; b < SI_MAX_ALIEN_BULLETS; b++) {
      if (!alienBullets[b].active) continue;

      if (alienBullets[b].x + alienBullets[b].w >= player.x &&
          alienBullets[b].x <= player.x + player.w &&
          alienBullets[b].y + alienBullets[b].h >= player.y &&
          alienBullets[b].y <= player.y + player.h) {

        // Explosion spawnen
        spawnExplosion(player.x + player.w / 2, player.y + player.h / 2, SI_COLOR_PLAYER);

        // Geschoss löschen
        uint16_t bgColor = getBgColor(alienBullets[b].y);
        lcd->fillRect(alienBullets[b].x - 1, alienBullets[b].y - 1,
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
    for (int b = 0; b < SI_MAX_ALIEN_BULLETS; b++) {
      if (!alienBullets[b].active) continue;

      for (int s = 0; s < SI_SHIELD_COUNT; s++) {
        if (shields[s].health == 0) continue;

        if (alienBullets[b].x + alienBullets[b].w >= shields[s].x &&
            alienBullets[b].x <= shields[s].x + shields[s].w &&
            alienBullets[b].y + alienBullets[b].h >= shields[s].y &&
            alienBullets[b].y <= shields[s].y + shields[s].h) {

          // Schild beschädigen
          shields[s].health = max(0, shields[s].health - 10);

          // Geschoss löschen
          uint16_t bgColor = getBgColor(alienBullets[b].y);
          lcd->fillRect(alienBullets[b].x - 1, alienBullets[b].y - 1,
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
    for (int i = 0; i < SI_ALIEN_COUNT; i++) {
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
    uint16_t boxColor = lerpColor(SI_COLOR_BG_TOP, SI_COLOR_BG_BOTTOM, 0.5);
    lcd->fillRect(SI_SCREEN_WIDTH/4 - 10, SI_SCREEN_HEIGHT/2 - 55,
                 SI_SCREEN_WIDTH/2 + 20, 110, boxColor);
    lcd->drawRect(SI_SCREEN_WIDTH/4 - 10, SI_SCREEN_HEIGHT/2 - 55,
                 SI_SCREEN_WIDTH/2 + 20, 110, SI_COLOR_TEXT);

    lcd->setTextSize(3);
    lcd->setTextColor(SI_COLOR_TEXT, boxColor);
    lcd->setCursor(SI_SCREEN_WIDTH/2 - 75, SI_SCREEN_HEIGHT/2 - 30);
    lcd->println("GAME OVER");

    lcd->setTextSize(2);
    lcd->setCursor(SI_SCREEN_WIDTH/2 - 55, SI_SCREEN_HEIGHT/2 + 5);
    lcd->printf("Score: %d", score);

    lcd->setTextSize(1);
    lcd->setCursor(SI_SCREEN_WIDTH/2 - 55, SI_SCREEN_HEIGHT/2 + 30);
    lcd->printf("Level: %d", level);

    lcd->setCursor(SI_SCREEN_WIDTH/2 - 55, SI_SCREEN_HEIGHT/2 + 45);
    lcd->println("Press SHOOT to restart");
  }

  // === ZEICHNEN ===

  void drawBackground() {
    // Gradient von oben (dunkelblau) nach unten (schwarz)
    for (int y = 0; y < SI_SCREEN_HEIGHT; y++) {
      uint16_t color = lerpColor(SI_COLOR_BG_TOP, SI_COLOR_BG_BOTTOM, (float)y / SI_SCREEN_HEIGHT);
      lcd->drawFastHLine(0, y, SI_SCREEN_WIDTH, color);
    }
    backgroundDrawn = true;
  }

  void drawAliens() {
    for (int i = 0; i < SI_ALIEN_COUNT; i++) {
      if (aliens[i].active) {
        drawAlien(i);
      }
    }
  }

  void drawAlien(int index) {
    SpaceInvAlien* a = &aliens[index];

    // Glow-Effekt (1 Pixel größer, halbtransparent simuliert durch dunklere Farbe)
    uint16_t glowColor = lerpColor(a->color, getBgColor(a->y + a->h / 2), 0.7);
    lcd->fillRect(a->x - 1, a->y - 1, a->w + 2, a->h + 2, glowColor);

    // Alien
    lcd->fillRect(a->x, a->y, a->w, a->h, a->color);
  }

  void eraseAlien(int index) {
    SpaceInvAlien* a = &aliens[index];

    // Alien mit Glow löschen (inkl. 1 Pixel Rand)
    for (int y = a->y - 1; y <= a->y + a->h; y++) {
      uint16_t bgColor = getBgColor(y);
      lcd->drawFastHLine(a->x - 1, y, a->w + 2, bgColor);
    }
  }

  void drawShields() {
    for (int i = 0; i < SI_SHIELD_COUNT; i++) {
      drawShield(i);
    }
  }

  void drawShield(int index) {
    SpaceInvShield* s = &shields[index];

    if (s->health == 0) {
      // Komplett zerstört - mit Gradient löschen
      for (int y = s->y; y < s->y + s->h; y++) {
        uint16_t bgColor = getBgColor(y);
        lcd->drawFastHLine(s->x, y, s->w, bgColor);
      }
    } else {
      // Zeichnen mit Transparenz-Effekt (weniger opak bei weniger Health)
      if (s->health < 50) {
        // Beschädigt - mit Löchern zeichnen
        for (int y = 0; y < s->h; y++) {
          uint16_t bgColor = getBgColor(s->y + y);
          for (int x = 0; x < s->w; x++) {
            if ((x + y) % 2 == 0) {
              lcd->drawPixel(s->x + x, s->y + y, SI_COLOR_SHIELD);
            } else {
              lcd->drawPixel(s->x + x, s->y + y, bgColor);
            }
          }
        }
      } else {
        // Gesund - mit Glow
        // Glow (außen)
        for (int y = s->y; y < s->y + s->h; y++) {
          lcd->drawPixel(s->x - 1, y, SI_COLOR_SHIELD_GLOW);
          lcd->drawPixel(s->x + s->w, y, SI_COLOR_SHIELD_GLOW);
        }
        lcd->drawFastHLine(s->x - 1, s->y - 1, s->w + 2, SI_COLOR_SHIELD_GLOW);
        lcd->drawFastHLine(s->x - 1, s->y + s->h, s->w + 2, SI_COLOR_SHIELD_GLOW);

        // Solider Schild
        lcd->fillRect(s->x, s->y, s->w, s->h, SI_COLOR_SHIELD);
      }
    }
  }

  void drawUI() {
    // Oberer Bereich mit Gradient löschen
    for (int y = 0; y < 25; y++) {
      uint16_t bgColor = getBgColor(y);
      lcd->drawFastHLine(0, y, SI_SCREEN_WIDTH, bgColor);
    }

    lcd->setTextSize(2);
    lcd->setTextColor(SI_COLOR_TEXT, SI_COLOR_BG_TOP);

    // Score (links)
    lcd->setCursor(5, 5);
    lcd->printf("S:%d", score);

    // Level (Mitte)
    lcd->setCursor(SI_SCREEN_WIDTH/2 - 30, 5);
    lcd->printf("L:%d", level);

    // Lives (rechts)
    lcd->setCursor(SI_SCREEN_WIDTH - 80, 5);
    lcd->printf("Lvs:%d", player.lives);
  }

  void drawGame() {
    // Spieler zeichnen - nur wenn Position sich geändert hat
    if (oldPlayerX != player.x) {
      // Alten mit Glow löschen
      if (oldPlayerX >= 0) {
        for (int y = player.y - 2; y <= player.y + player.h + 1; y++) {
          uint16_t bgColor = getBgColor(y);
          lcd->drawFastHLine(oldPlayerX - 2, y, player.w + 4, bgColor);
        }
      }

      // Neuen mit Glow zeichnen
      // Glow (größer, dunklere Farbe)
      lcd->fillRect(player.x - 2, player.y - 2, player.w + 4, player.h + 4, SI_COLOR_PLAYER_GLOW);

      // Player
      lcd->fillRect(player.x, player.y, player.w, player.h, player.color);

      oldPlayerX = player.x;
    }
  }
};

#endif // SPACEINVADERSGAMEMODERN_H
