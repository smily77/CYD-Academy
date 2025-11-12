/*
  CYD Retro Games Menu - Alle klassischen Arcade-Spiele in einem Programm

  Hardware:
  - CYD Display (320x240 im Landscape-Modus)
  - Touch Screen für Menu-Auswahl
  - Buttons & Potentiometer für Spiele-Steuerung

  Enthaltene Spiele:
  - Pong (Klassisches 2-Spieler Paddle-Spiel)
  - Snake (Klassische Schlange)
  - Breakout (Brick-Breaker)

  Verwendung:
  1. Touch ein Spiel im Menu
  2. Spiel wird geladen und gestartet
  3. Bei Game Over: Zurück zum Menu

  Hinweis:
  - Alle Spiele sind in der CYD_Games Library gekapselt
  - Du kannst jedes Spiel auch einzeln kompilieren (siehe examples/12_Pong/, 13_Snake/, 14_Breakout/)
*/

#include <CYD_Display_Config.h>
#include <SnakeGame.h>
#include <PongGame.h>
#include <BreakoutGame.h>

// Display Objekt
LGFX lcd;

// Spiel-Instanzen
SnakeGame snakeGame;
PongGame pongGame;
BreakoutGame breakoutGame;

// Game State
enum GameState {
  MENU,
  GAME_SNAKE,
  GAME_PONG,
  GAME_BREAKOUT
};

GameState currentState = MENU;
bool lastTouchState = false;

// Menu-Layout
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#define MENU_ITEM_WIDTH  280
#define MENU_ITEM_HEIGHT 50
#define MENU_ITEM_SPACING 15
#define MENU_START_Y 40

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

MenuItem menuItems[] = {
  {"PONG", "2-Spieler Paddle-Klassiker", 0x07FF, GAME_PONG},
  {"SNAKE", "Sammle Food, werde länger!", 0x07E0, GAME_SNAKE},
  {"BREAKOUT", "Zerstöre alle Steine!", 0xF800, GAME_BREAKOUT}
};

const int menuItemCount = sizeof(menuItems) / sizeof(menuItems[0]);

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== CYD RETRO GAMES ===\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);  // Landscape
  lcd.fillScreen(COLOR_BG);
  lcd.setBrightness(255);

  // Button Pins konfigurieren (für alle Spiele)
  pinMode(tasteA, INPUT_PULLUP);
  pinMode(tasteB, INPUT_PULLUP);
  pinMode(tasteC, INPUT_PULLUP);
  pinMode(tasteD, INPUT_PULLUP);

  // Zufallsgenerator
  randomSeed(analogRead(35) + micros());

  // Menu zeichnen
  drawMenu();

  Serial.println("Retro Games Menu bereit!");
  Serial.println("Touch ein Spiel um zu starten");
}

void loop() {
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
      // Pong hat kein Game Over, läuft endlos
      // TODO: Mit extra Button zurück zum Menu
      break;

    case GAME_BREAKOUT:
      breakoutGame.update();
      // Zurück zum Menu bei Game Over
      if (breakoutGame.isGameOver()) {
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

  // Header
  lcd.fillRect(0, 0, SCREEN_WIDTH, 35, COLOR_HEADER);
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT, COLOR_HEADER);
  lcd.setCursor(60, 10);
  lcd.println("RETRO ARCADE GAMES");

  // Menu-Items
  int y = MENU_START_Y;

  for (int i = 0; i < menuItemCount; i++) {
    int x = (SCREEN_WIDTH - MENU_ITEM_WIDTH) / 2;

    // Item-Hintergrund
    lcd.fillRect(x, y, MENU_ITEM_WIDTH, MENU_ITEM_HEIGHT, COLOR_MENU_ITEM);
    lcd.drawRect(x, y, MENU_ITEM_WIDTH, MENU_ITEM_HEIGHT, COLOR_BORDER);

    // Farbiges Symbol
    lcd.fillRect(x + 10, y + 10, 30, 30, menuItems[i].color);
    lcd.drawRect(x + 10, y + 10, 30, 30, COLOR_TEXT);

    // Spiel-Name
    lcd.setTextSize(2);
    lcd.setTextColor(COLOR_TEXT, COLOR_MENU_ITEM);
    lcd.setCursor(x + 50, y + 8);
    lcd.println(menuItems[i].name);

    // Beschreibung
    lcd.setTextSize(1);
    lcd.setTextColor(0xBDF7, COLOR_MENU_ITEM);  // Hellgrau
    lcd.setCursor(x + 50, y + 28);
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

      // Display löschen und Spiel initialisieren
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
        default:
          break;
      }

      delay(300);  // Debounce
      return;
    }

    menuY += MENU_ITEM_HEIGHT + MENU_ITEM_SPACING;
  }
}
