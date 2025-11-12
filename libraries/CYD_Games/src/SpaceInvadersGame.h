/*
  SpaceInvadersGame.h - Encapsulated Space Invaders Game
  Teil der CYD_Games Library
*/

#ifndef SPACEINVADERSGAME_H
#define SPACEINVADERSGAME_H

#include <Arduino.h>
#include <CYD_Display_Config.h>

// Pin-Mapping (muss VOR der Klasse definiert sein)
#define SI_BTN_LEFT   tasteB
#define SI_BTN_RIGHT  tasteC
#define SI_BTN_SHOOT  tasteA
#define SI_BTN_PAUSE  tasteD

// Display-Dimensionen
#define SI_SCREEN_WIDTH  320
#define SI_SCREEN_HEIGHT 240

// Farben
#define SI_COLOR_BG       0x0000  // Schwarz
#define SI_COLOR_PLAYER   0x07FF  // Cyan
#define SI_COLOR_ALIEN1   0xF800  // Rot (oberste Reihe)
#define SI_COLOR_ALIEN2   0xFD20  // Orange
#define SI_COLOR_ALIEN3   0xFFE0  // Gelb
#define SI_COLOR_ALIEN4   0x07E0  // Grün
#define SI_COLOR_ALIEN5   0x001F  // Blau (unterste Reihe)
#define SI_COLOR_BULLET   0xFFFF  // Weiß
#define SI_COLOR_SHIELD   0x07E0  // Grün
#define SI_COLOR_TEXT     0xFFFF  // Weiß

// Strukturen (müssen VOR der Klasse definiert sein)
struct SI_Player {
  int x, y;
  int w, h;
  uint16_t color;
  int lives;
};

struct SI_Alien {
  int x, y;
  int w, h;
  uint16_t color;
  int points;
  bool active;
};

struct SI_Bullet {
  int x, y;
  int w, h;
  int vy;
  bool active;
};

struct SI_Shield {
  int x, y;
  int w, h;
  uint8_t health;  // 0-100
};

// Array-Größen
#define SI_ALIEN_ROWS 5
#define SI_ALIEN_COLS 11
#define SI_ALIEN_COUNT (SI_ALIEN_ROWS * SI_ALIEN_COLS)
#define SI_MAX_PLAYER_BULLETS 3
#define SI_MAX_ALIEN_BULLETS 5
#define SI_SHIELD_COUNT 4

class SpaceInvadersGame {
public:
  SpaceInvadersGame() :
    lcd(nullptr),
    score(0),
    level(1),
    gameOver(false),
    paused(false),
    alienDirection(1),
    alienStepX(4),
    alienStepY(8),
    lastAlienMove(0),
    alienMoveDelay(1000),
    lastAlienShot(0),
    lastPauseState(HIGH),
    oldPlayerX(-1)
  {
    // Konstruktor - Arrays werden in init() initialisiert
  }

  void init(LGFX* display) {
    lcd = display;

    score = 0;
    level = 1;
    gameOver = false;
    paused = false;

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

    // Schilde initialisieren
    initShields();

    // Level initialisieren
    initLevel();

    // Button Inputs konfigurieren
    pinMode(SI_BTN_LEFT, INPUT_PULLUP);
    pinMode(SI_BTN_RIGHT, INPUT_PULLUP);
    pinMode(SI_BTN_SHOOT, INPUT_PULLUP);
    pinMode(SI_BTN_PAUSE, INPUT_PULLUP);

    // Bildschirm aufbauen
    lcd->fillScreen(SI_COLOR_BG);
    drawShields();
    drawUI();

    Serial.println("Space Invaders gestartet!");
  }

  void update() {
    if (gameOver) {
      // Warte auf Neustart-Button
      if (digitalRead(SI_BTN_SHOOT) == LOW) {
        init(lcd);
        delay(300);
      }
      return;
    }

    // Pause-Button
    int pauseState = digitalRead(SI_BTN_PAUSE);
    if (pauseState == LOW && lastPauseState == HIGH) {
      paused = !paused;
      delay(200);
    }
    lastPauseState = pauseState;

    if (paused) {
      // Zeige Pause-Text
      lcd->setTextSize(3);
      lcd->setTextColor(SI_COLOR_TEXT, SI_COLOR_BG);
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

  // Status-Abfragen
  bool isGameOver() const { return gameOver; }
  int getScore() const { return score; }
  int getLevel() const { return level; }
  int getLives() const { return player.lives; }

private:
  LGFX* lcd;

  // Game State
  SI_Player player;
  SI_Alien aliens[SI_ALIEN_COUNT];
  SI_Bullet playerBullets[SI_MAX_PLAYER_BULLETS];
  SI_Bullet alienBullets[SI_MAX_ALIEN_BULLETS];
  SI_Shield shields[SI_SHIELD_COUNT];

  int alienDirection;  // 1 = rechts, -1 = links
  int alienStepX;
  int alienStepY;
  unsigned long lastAlienMove;
  int alienMoveDelay;
  unsigned long lastAlienShot;

  int score;
  int level;
  bool gameOver;
  bool paused;
  int lastPauseState;
  int oldPlayerX;

  // ===== INITIALISIERUNG =====

  void initLevel() {
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
      SI_COLOR_ALIEN5   // Blau - 10 Punkte
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

  // ===== UPDATE FUNKTIONEN =====

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

      // Alte Position löschen
      lcd->fillRect(aliens[i].x, aliens[i].y, aliens[i].w, aliens[i].h, SI_COLOR_BG);

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
    SI_Alien* shooter = &aliens[shooterIndex];

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

      // Alte Position löschen
      lcd->fillRect(playerBullets[i].x, playerBullets[i].y,
                   playerBullets[i].w, playerBullets[i].h, SI_COLOR_BG);

      playerBullets[i].y += playerBullets[i].vy;

      // Außerhalb des Bildschirms?
      if (playerBullets[i].y < 0) {
        playerBullets[i].active = false;
        continue;
      }

      // Neue Position zeichnen
      lcd->fillRect(playerBullets[i].x, playerBullets[i].y,
                   playerBullets[i].w, playerBullets[i].h, SI_COLOR_BULLET);
    }

    // Alien-Geschosse
    for (int i = 0; i < SI_MAX_ALIEN_BULLETS; i++) {
      if (!alienBullets[i].active) continue;

      // Alte Position löschen
      lcd->fillRect(alienBullets[i].x, alienBullets[i].y,
                   alienBullets[i].w, alienBullets[i].h, SI_COLOR_BG);

      alienBullets[i].y += alienBullets[i].vy;

      // Außerhalb des Bildschirms?
      if (alienBullets[i].y > SI_SCREEN_HEIGHT) {
        alienBullets[i].active = false;
        continue;
      }

      // Neue Position zeichnen
      lcd->fillRect(alienBullets[i].x, alienBullets[i].y,
                   alienBullets[i].w, alienBullets[i].h, SI_COLOR_BULLET);
    }
  }

  // ===== KOLLISIONEN =====

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

          // Alien zerstören
          aliens[a].active = false;
          score += aliens[a].points;

          // Alien löschen
          lcd->fillRect(aliens[a].x, aliens[a].y, aliens[a].w, aliens[a].h, SI_COLOR_BG);

          // Geschoss löschen
          lcd->fillRect(playerBullets[b].x, playerBullets[b].y,
                       playerBullets[b].w, playerBullets[b].h, SI_COLOR_BG);
          playerBullets[b].active = false;

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
          lcd->fillRect(playerBullets[b].x, playerBullets[b].y,
                       playerBullets[b].w, playerBullets[b].h, SI_COLOR_BG);
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

        // Geschoss löschen
        lcd->fillRect(alienBullets[b].x, alienBullets[b].y,
                     alienBullets[b].w, alienBullets[b].h, SI_COLOR_BG);
        alienBullets[b].active = false;

        // Leben verlieren
        player.lives--;
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
          lcd->fillRect(alienBullets[b].x, alienBullets[b].y,
                       alienBullets[b].w, alienBullets[b].h, SI_COLOR_BG);
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

    // Game Over Screen
    lcd->fillRect(SI_SCREEN_WIDTH/4 - 10, SI_SCREEN_HEIGHT/2 - 55,
                 SI_SCREEN_WIDTH/2 + 20, 110, 0x2104);
    lcd->drawRect(SI_SCREEN_WIDTH/4 - 10, SI_SCREEN_HEIGHT/2 - 55,
                 SI_SCREEN_WIDTH/2 + 20, 110, SI_COLOR_TEXT);

    lcd->setTextSize(3);
    lcd->setTextColor(SI_COLOR_TEXT, 0x2104);
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

  // ===== ZEICHNEN =====

  void drawAliens() {
    for (int i = 0; i < SI_ALIEN_COUNT; i++) {
      if (aliens[i].active) {
        drawAlien(i);
      }
    }
  }

  void drawAlien(int index) {
    SI_Alien* a = &aliens[index];
    lcd->fillRect(a->x, a->y, a->w, a->h, a->color);
  }

  void drawShields() {
    for (int i = 0; i < SI_SHIELD_COUNT; i++) {
      drawShield(i);
    }
  }

  void drawShield(int index) {
    SI_Shield* s = &shields[index];

    if (s->health == 0) {
      // Komplett zerstört - löschen
      lcd->fillRect(s->x, s->y, s->w, s->h, SI_COLOR_BG);
    } else {
      // Zeichnen mit Transparenz-Effekt (weniger opak bei weniger Health)
      if (s->health < 50) {
        // Beschädigt - mit Löchern zeichnen
        for (int y = 0; y < s->h; y++) {
          for (int x = 0; x < s->w; x++) {
            if ((x + y) % 2 == 0) {
              lcd->drawPixel(s->x + x, s->y + y, SI_COLOR_SHIELD);
            } else {
              lcd->drawPixel(s->x + x, s->y + y, SI_COLOR_BG);
            }
          }
        }
      } else {
        // Gesund - solide
        lcd->fillRect(s->x, s->y, s->w, s->h, SI_COLOR_SHIELD);
      }
    }
  }

  void drawUI() {
    // Oberer Bereich löschen
    lcd->fillRect(0, 0, SI_SCREEN_WIDTH, 25, SI_COLOR_BG);

    lcd->setTextSize(2);
    lcd->setTextColor(SI_COLOR_TEXT, SI_COLOR_BG);

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
      // Alten löschen
      if (oldPlayerX >= 0) {
        lcd->fillRect(oldPlayerX, player.y, player.w, player.h, SI_COLOR_BG);
      }
      // Neuen zeichnen
      lcd->fillRect(player.x, player.y, player.w, player.h, player.color);
      oldPlayerX = player.x;
    }
  }
};

#endif // SPACEINVADERSGAME_H
