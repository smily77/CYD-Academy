/*
  CYD Modern Games Menu - Alle modernen Arcade-Spiele in einem Programm

  Hardware:
  - CYD Display (320x240 im Landscape-Modus)
  - Touch Screen für Menu-Auswahl
  - Buttons & Potentiometer für Spiele-Steuerung

  Enthaltene Spiele (Moderne Versionen):
  - Pong Modern (2-Spieler mit Neon-Effekten & Partikeln)
  - Snake Modern (Mit Gradient-Hintergrund & Trail)
  - Breakout Modern (3D-Blöcke & Partikel-Explosionen)
  - Space Invaders Modern (Glow-Effekte & animierte Schilde)
  - Asteroids Modern (Partikel-Explosionen & Bullet-Trails)
  - Frogger Modern (Animiertes Wasser & smooth Movement)
  - Tetris Modern (Ghost-Piece & Rainbow-Effekte - Portrait-Modus!)

  Verwendung:
  1. Touch ein Spiel im Menu
  2. Spiel wird geladen und gestartet
  3. Bei Game Over: Zurück zum Menu

  Hinweis:
  - Alle Spiele sind in der CYD_Games Library gekapselt
  - Du kannst jedes Spiel auch einzeln kompilieren (siehe examples/12b_Pong_Modern/, etc.)
  - Tetris wechselt automatisch zu Portrait-Modus (240x320)
*/

#include <CYD_Display_Config.h>
#include <CYD_Input.h>
#include <PongGameModern.h>
#include <SnakeGameModern.h>
#include <BreakoutGameModern.h>
#include <SpaceInvadersGameModern.h>
#include <AsteroidsGameModern.h>
#include <FroggerGameModern.h>
#include <TetrisGameModern.h>

// Display Objekt
LGFX lcd;

// Spiel-Instanzen (Modern)
PongGameModern pongGame;
SnakeGameModern snakeGame;
BreakoutGameModern breakoutGame;
SpaceInvadersGameModern spaceInvadersGame;
AsteroidsGameModern asteroidsGame;
FroggerGameModern froggerGame;
TetrisGameModern tetrisGame;

// Game State
enum GameState {
  MENU,
  GAME_PONG,
  GAME_SNAKE,
  GAME_BREAKOUT,
  GAME_SPACEINVADERS,
  GAME_ASTEROIDS,
  GAME_FROGGER,
  GAME_TETRIS
};

GameState currentState = MENU;
GameState lastState = MENU;
bool lastTouchState = false;

// Menu-Layout
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#define MENU_ITEM_WIDTH  280
#define MENU_ITEM_HEIGHT 26  // Optimiert für 7 Spiele
#define MENU_ITEM_SPACING 3  // Minimaler Abstand
#define MENU_START_Y 33

// Farben (Modern Theme - Neon & Gradient)
#define COLOR_BG        0x0000  // Schwarz
#define COLOR_TEXT      0xFFFF  // Weiß
#define COLOR_HEADER    0x4015  // Dunkles Lila
#define COLOR_MENU_ITEM 0x2104  // Dunkelgrau
#define COLOR_BORDER    0x8410  // Hellgrau

// Spiel-Definitionen
struct MenuItem {
  const char* name;
  const char* description;
  uint16_t color;
  GameState state;
};

// Menu wird dynamisch basierend auf Hardware-Verfügbarkeit aufgebaut
MenuItem menuItems[7];  // Maximal 7 Spiele
int menuItemCount = 0;

void buildMenu() {
  menuItemCount = 0;
  bool hasPoti = CYD_Input::hasPotis();
  bool hasEncoder = CYD_Input::hasEncoder();

  // Pong (mit Potis, Tasten oder Encoder)
  menuItems[menuItemCount++] = {"PONG", "Neon-Paddle mit Partikeln", 0x07FF, GAME_PONG};

  // Snake (NICHT mit Encoder)
  if (!hasEncoder) {
    menuItems[menuItemCount++] = {"SNAKE", "Gradient-Schlange & Trail", 0x07E0, GAME_SNAKE};
  }

  // Breakout (mit Potis, Tasten oder Encoder)
  menuItems[menuItemCount++] = {"BREAKOUT", "3D-Blöcke & Explosionen", 0xFD20, GAME_BREAKOUT};

  // Space Invaders (mit Tasten oder Encoder)
  menuItems[menuItemCount++] = {"SPACE INVADERS", "Glow-Aliens & Schilde", 0xF81F, GAME_SPACEINVADERS};

  // Asteroids (NICHT mit Encoder)
  if (!hasEncoder) {
    menuItems[menuItemCount++] = {"ASTEROIDS", "Partikel & Bullet-Trails", 0x07FF, GAME_ASTEROIDS};
  }

  // Frogger (mit Tasten oder Encoder)
  menuItems[menuItemCount++] = {"FROGGER", "Animiertes Wasser & Smooth", 0xFFE0, GAME_FROGGER};

  // Tetris (mit Tasten oder Encoder)
  menuItems[menuItemCount++] = {"TETRIS", "Ghost-Piece & Rainbow!", 0xF81F, GAME_TETRIS};

  Serial.printf("Menu aufgebaut: %d Spiele verfügbar (Potis: %s, Encoder: %s)\n",
                menuItemCount, hasPoti ? "JA" : "NEIN", hasEncoder ? "JA" : "NEIN");
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== CYD MODERN GAMES ===\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);  // Landscape
  lcd.fillScreen(COLOR_BG);
  lcd.setBrightness(255);

  // Input-System initialisieren
  CYD_Input::init();

  // Zufallsgenerator
  randomSeed(analogRead(34) + analogRead(35) + micros());

  // Menu basierend auf verfügbarer Hardware aufbauen
  buildMenu();

  // Menu zeichnen
  drawMenu();

  Serial.println("Modern Games Menu bereit!");
  Serial.println("Touch ein Spiel um zu starten");
}

void loop() {
  // Bei Wechsel von/zu Tetris: Rotation ändern
  if (currentState != lastState) {
    if (currentState == GAME_TETRIS && lastState != GAME_TETRIS) {
      // Wechsel ZU Tetris: Portrait-Modus
      lcd.setRotation(0);
      lcd.fillScreen(COLOR_BG);
      tetrisGame.init(&lcd);
    } else if (lastState == GAME_TETRIS && currentState == MENU) {
      // Wechsel VON Tetris zurück zum Menu: Landscape-Modus
      lcd.setRotation(1);
      lcd.fillScreen(COLOR_BG);
      drawMenu();
    }
    lastState = currentState;
  }

  switch (currentState) {
    case MENU:
      handleMenu();
      break;

    case GAME_PONG:
      pongGame.update();
      // Zurück zum Menu mit Taste D (oder B+C ohne Potis)
      if (pongGame.shouldReturnToMenu()) {
        delay(500);
        currentState = MENU;
        drawMenu();
      }
      break;

    case GAME_SNAKE:
      snakeGame.update();
      if (snakeGame.isGameOver()) {
        delay(2000);
        currentState = MENU;
        drawMenu();
      }
      break;

    case GAME_BREAKOUT:
      breakoutGame.update();
      if (breakoutGame.isGameOver()) {
        delay(2000);
        currentState = MENU;
        drawMenu();
      }
      break;

    case GAME_SPACEINVADERS:
      spaceInvadersGame.update();
      if (spaceInvadersGame.isGameOver()) {
        delay(2000);
        currentState = MENU;
        drawMenu();
      }
      break;

    case GAME_ASTEROIDS:
      asteroidsGame.update();
      if (asteroidsGame.isGameOver()) {
        delay(2000);
        currentState = MENU;
        drawMenu();
      }
      break;

    case GAME_FROGGER:
      froggerGame.update();
      if (froggerGame.isGameOver()) {
        delay(2000);
        currentState = MENU;
        drawMenu();
      }
      break;

    case GAME_TETRIS:
      tetrisGame.update();
      if (tetrisGame.isGameOver()) {
        delay(2000);
        currentState = MENU;
      }
      break;
  }

  delay(10);
}

// ===== MENU =====

void drawMenu() {
  lcd.fillScreen(COLOR_BG);

  // Header mit Gradient-Effekt
  lcd.fillRect(0, 0, SCREEN_WIDTH, 35, COLOR_HEADER);
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT, COLOR_HEADER);
  lcd.setCursor(45, 10);
  lcd.println("MODERN ARCADE GAMES");

  // Menu-Items
  int y = MENU_START_Y;

  for (int i = 0; i < menuItemCount; i++) {
    int x = (SCREEN_WIDTH - MENU_ITEM_WIDTH) / 2;

    // Item-Hintergrund
    lcd.fillRect(x, y, MENU_ITEM_WIDTH, MENU_ITEM_HEIGHT, COLOR_MENU_ITEM);
    lcd.drawRect(x, y, MENU_ITEM_WIDTH, MENU_ITEM_HEIGHT, COLOR_BORDER);

    // Farbiges Symbol (mit Glow-Effekt simuliert durch doppelten Rahmen)
    lcd.fillRect(x + 5, y + 5, 20, 20, menuItems[i].color);
    lcd.drawRect(x + 4, y + 4, 22, 22, dimColor(menuItems[i].color, 0.5));
    lcd.drawRect(x + 5, y + 5, 20, 20, COLOR_TEXT);

    // Spiel-Name
    lcd.setTextSize(1);
    lcd.setTextColor(COLOR_TEXT, COLOR_MENU_ITEM);
    lcd.setCursor(x + 30, y + 5);
    lcd.println(menuItems[i].name);

    // Beschreibung
    lcd.setTextSize(1);
    lcd.setTextColor(0xBDF7, COLOR_MENU_ITEM);  // Hellgrau
    lcd.setCursor(x + 30, y + 17);
    lcd.println(menuItems[i].description);

    y += MENU_ITEM_HEIGHT + MENU_ITEM_SPACING;
  }

  // Hinweis unten
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT, COLOR_BG);
  lcd.setCursor(80, SCREEN_HEIGHT - 15);
  lcd.println("Touch Spiel zum Starten");
}

void handleMenu() {
  // Touch-Input prüfen
  uint16_t x, y;
  bool touched = lcd.getTouch(&x, &y);

  if (touched) {
    // Nur auf neuen Touch reagieren
    if (!lastTouchState) {
      handleMenuTouch(x, y);
      lastTouchState = true;
    }
  } else {
    lastTouchState = false;
  }

  delay(50);
}

void handleMenuTouch(int x, int y) {
  Serial.printf("Menu Touch: x=%d, y=%d\n", x, y);

  int menuY = MENU_START_Y;
  int menuX = (SCREEN_WIDTH - MENU_ITEM_WIDTH) / 2;

  for (int i = 0; i < menuItemCount; i++) {
    // Prüfe ob Touch innerhalb des Menu-Items
    if (x >= menuX && x <= menuX + MENU_ITEM_WIDTH &&
        y >= menuY && y <= menuY + MENU_ITEM_HEIGHT) {

      Serial.printf("Spiel gewählt: %s\n", menuItems[i].name);

      // Spiel starten
      currentState = menuItems[i].state;

      // Tetris wird in loop() mit Rotation-Wechsel behandelt
      if (currentState == GAME_TETRIS) {
        delay(300);  // Debounce
        return;
      }

      // Display löschen und Spiel initialisieren (für Landscape-Spiele)
      lcd.fillScreen(COLOR_BG);

      switch (currentState) {
        case GAME_PONG:
          pongGame.init(&lcd);
          break;
        case GAME_SNAKE:
          snakeGame.init(&lcd);
          break;
        case GAME_BREAKOUT:
          breakoutGame.init(&lcd);
          break;
        case GAME_SPACEINVADERS:
          spaceInvadersGame.init(&lcd);
          break;
        case GAME_ASTEROIDS:
          asteroidsGame.init(&lcd);
          break;
        case GAME_FROGGER:
          froggerGame.init(&lcd);
          break;
        default:
          break;
      }

      delay(300);  // Debounce
      return;
    }

    menuY += MENU_ITEM_HEIGHT + MENU_ITEM_SPACING;
  }
}

// Helper: Dimme eine Farbe für Glow-Effekt
uint16_t dimColor(uint16_t color, float factor) {
  factor = constrain(factor, 0.0, 1.0);

  uint8_t r = ((color >> 11) & 0x1F) * factor;
  uint8_t g = ((color >> 5) & 0x3F) * factor;
  uint8_t b = (color & 0x1F) * factor;

  return (r << 11) | (g << 5) | b;
}
