/*
  CYD Retro Games Menu (LARGE) - Touchfreundliche Version für kleine Displays

  Diese Version ist optimiert für kleine CYD-Displays mit:
  - GRÖßEREN Touch-Bereichen (einfacher zu treffen)
  - NUR Spiel-Titel (keine Beschreibung)
  - GRÖßERER Schrift für bessere Lesbarkeit

  Hardware:
  - CYD Display (320x240 im Landscape-Modus)
  - Touch Screen für Menu-Auswahl
  - Buttons & Potentiometer für Spiele-Steuerung

  Enthaltene Spiele:
  - Pong (Klassisches 2-Spieler Paddle-Spiel)
  - Snake (Klassische Schlange)
  - Breakout (Brick-Breaker)
  - Space Invaders (Alien-Shooter)
  - Asteroids (Weltraum-Shooter)
  - Frogger (Straßen-Überquerungs-Klassiker)
  - Tetris (Puzzle-Klassiker - Portrait-Modus!)

  Verwendung:
  1. Touch ein Spiel im Menu (große Buttons!)
  2. Spiel wird geladen und gestartet
  3. Bei Game Over: Zurück zum Menu

  Unterschied zu 00_RetroGames:
  - Buttons sind HÖHER (40px statt 26px)
  - Nur TITEL wird angezeigt (keine Beschreibung)
  - GRÖSSERE Schrift (TextSize 2 statt 1)
  - Weniger Spiele passen auf Bildschirm (aber besser zu treffen!)
*/

#include <CYD_Display_Config.h>
#include <CYD_Input.h>
#include <SnakeGame.h>
#include <PongGame.h>
#include <BreakoutGame.h>
#include <SpaceInvadersGame.h>
#include <AsteroidsGame.h>
#include <FroggerGame.h>
#include <TetrisGame.h>

// Display Objekt
LGFX lcd;

// Spiel-Instanzen
SnakeGame snakeGame;
PongGame pongGame;
BreakoutGame breakoutGame;
SpaceInvadersGame spaceInvadersGame;
AsteroidsGame asteroidsGame;
FroggerGame froggerGame;
TetrisGame tetrisGame;

// Game State
enum GameState {
  MENU,
  GAME_SNAKE,
  GAME_PONG,
  GAME_BREAKOUT,
  GAME_SPACEINVADERS,
  GAME_ASTEROIDS,
  GAME_FROGGER,
  GAME_TETRIS
};

GameState currentState = MENU;
GameState lastState = MENU;
bool lastTouchState = false;

// Menu-Layout (LARGE VERSION - Portrait-Modus für mehr Platz!)
#define SCREEN_WIDTH  240  // Portrait: schmaler
#define SCREEN_HEIGHT 320  // Portrait: höher (mehr Platz für Buttons!)

#define MENU_ITEM_WIDTH  220  // Angepasst für Portrait
#define MENU_ITEM_HEIGHT 38   // Groß für besseres Treffen
#define MENU_ITEM_SPACING 3   // Abstand zwischen Buttons
#define MENU_START_Y 45       // Start nach Header

// Farben
#define COLOR_BG        0x0000  // Schwarz
#define COLOR_TEXT      0xFFFF  // Weiß
#define COLOR_HEADER    0x001F  // Blau
#define COLOR_MENU_ITEM 0x2104  // Dunkelgrau
#define COLOR_BORDER    0x4208  // Mittelgrau

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
  menuItems[menuItemCount++] = {"PONG", "2-Spieler Paddle-Klassiker", 0x07FF, GAME_PONG};

  // Snake (NICHT mit Encoder)
  if (!hasEncoder) {
    menuItems[menuItemCount++] = {"SNAKE", "Sammle Food, werde länger!", 0x07E0, GAME_SNAKE};
  }

  // Breakout (mit Potis, Tasten oder Encoder)
  menuItems[menuItemCount++] = {"BREAKOUT", "Zerstöre alle Steine!", 0xF800, GAME_BREAKOUT};

  // Space Invaders (mit Tasten oder Encoder)
  menuItems[menuItemCount++] = {"SPACE INVADERS", "Verteidige gegen Aliens!", 0xF81F, GAME_SPACEINVADERS};

  // Asteroids (NICHT mit Encoder)
  if (!hasEncoder) {
    menuItems[menuItemCount++] = {"ASTEROIDS", "Zerstöre Asteroids!", 0x07FF, GAME_ASTEROIDS};
  }

  // Frogger (mit Tasten oder Encoder)
  menuItems[menuItemCount++] = {"FROGGER", "Überquere die Strasse!", 0xFFE0, GAME_FROGGER};

  // Tetris (mit Tasten oder Encoder)
  menuItems[menuItemCount++] = {"TETRIS", "Puzzle-Klassiker!", 0xF81F, GAME_TETRIS};

  Serial.printf("Menu aufgebaut: %d Spiele verfügbar (Potis: %s, Encoder: %s)\n",
                menuItemCount, hasPoti ? "JA" : "NEIN", hasEncoder ? "JA" : "NEIN");
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== CYD RETRO GAMES (LARGE) ===\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(0);  // Portrait für Menu (mehr Höhe für Buttons!)
  lcd.fillScreen(COLOR_BG);
  lcd.setBrightness(255);

  // Input-System initialisieren
  CYD_Input::init();

  // Zufallsgenerator
  randomSeed(analogRead(35) + micros());

  // Menu basierend auf verfügbarer Hardware aufbauen
  buildMenu();

  // Menu zeichnen
  drawMenu();

  Serial.println("Retro Games Menu bereit!");
  Serial.println("Touch ein Spiel um zu starten");
}

void loop() {
  // Bei Zustandswechsel: Rotation und Spiel-Init
  if (currentState != lastState) {
    // Wechsel ZU einem Spiel (außer Tetris): Landscape
    if (currentState != MENU && currentState != GAME_TETRIS) {
      lcd.setRotation(1);  // Landscape für alle Spiele außer Tetris
      lcd.fillScreen(COLOR_BG);

      // Init je nach Spiel
      if (currentState == GAME_SNAKE) snakeGame.init(&lcd);
      else if (currentState == GAME_PONG) pongGame.init(&lcd);
      else if (currentState == GAME_BREAKOUT) breakoutGame.init(&lcd);
      else if (currentState == GAME_SPACEINVADERS) spaceInvadersGame.init(&lcd);
      else if (currentState == GAME_ASTEROIDS) asteroidsGame.init(&lcd);
      else if (currentState == GAME_FROGGER) froggerGame.init(&lcd);
    }
    // Wechsel ZU Tetris: bleibt Portrait
    else if (currentState == GAME_TETRIS) {
      lcd.setRotation(0);  // Portrait (wie Menu)
      lcd.fillScreen(COLOR_BG);
      tetrisGame.init(&lcd);
    }
    // Wechsel ZU Menu: Portrait
    else if (currentState == MENU) {
      lcd.setRotation(0);  // Portrait für Menu
      lcd.fillScreen(COLOR_BG);
      drawMenu();
    }

    lastState = currentState;
  }

  switch (currentState) {
    case MENU:
      handleMenu();
      break;

    case GAME_SNAKE:
      snakeGame.update();
      // Zurück zum Menu bei Game Over
      if (snakeGame.isGameOver()) {
        // Warte kurz, dann zurück zum Menu
        delay(2000);
        currentState = MENU;
        drawMenu();
      }
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
        drawMenu();
      }
      break;
  }

  delay(10);
}

// ===== MENU =====

void drawMenu() {
  lcd.fillScreen(COLOR_BG);

  // Header (Portrait-Modus angepasst)
  lcd.fillRect(0, 0, SCREEN_WIDTH, 42, COLOR_HEADER);
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT, COLOR_HEADER);
  lcd.setCursor(10, 8);
  lcd.println("RETRO ARCADE");
  lcd.setCursor(45, 24);
  lcd.println("GAMES");

  // Menu-Items (LARGE VERSION - Portrait, nur Titel!)
  int y = MENU_START_Y;

  for (int i = 0; i < menuItemCount; i++) {
    int x = (SCREEN_WIDTH - MENU_ITEM_WIDTH) / 2;

    // Item-Hintergrund
    lcd.fillRect(x, y, MENU_ITEM_WIDTH, MENU_ITEM_HEIGHT, COLOR_MENU_ITEM);
    lcd.drawRect(x, y, MENU_ITEM_WIDTH, MENU_ITEM_HEIGHT, COLOR_BORDER);

    // Farbiges Symbol (linksbündig)
    lcd.fillRect(x + 6, y + 7, 24, 24, menuItems[i].color);
    lcd.drawRect(x + 6, y + 7, 24, 24, COLOR_TEXT);

    // Spiel-Name (GRÖßERE Schrift, vertikal zentriert)
    lcd.setTextSize(2);
    lcd.setTextColor(COLOR_TEXT, COLOR_MENU_ITEM);
    lcd.setCursor(x + 36, y + 11);  // Vertikal zentriert
    lcd.println(menuItems[i].name);

    y += MENU_ITEM_HEIGHT + MENU_ITEM_SPACING;
  }

  // Hinweis unten
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT, COLOR_BG);
  lcd.setCursor(55, SCREEN_HEIGHT - 10);
  lcd.println("Touch zum Starten");
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
        case GAME_SNAKE:
          snakeGame.init(&lcd);
          break;
        case GAME_PONG:
          pongGame.init(&lcd);
          break;
        case GAME_BREAKOUT:
          breakoutGame.init(&lcd);
          break;
        case GAME_SPACEINVADERS:
          spaceInvadersGame.init(&lcd);
          break;
        case GAME_ASTEROIDS:
          asteroidsGame.init(&lcd, false);  // Ohne Potentiometer
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
