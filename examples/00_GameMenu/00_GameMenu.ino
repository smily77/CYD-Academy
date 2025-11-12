/*
  CYD Game Menu - Hauptmenü für alle Arcade-Spiele

  Hardware:
  - CYD Display (320x240 im Landscape-Modus)
  - Touch Screen

  Funktion:
  - Zeigt eine Liste aller verfügbaren Spiele
  - Touch auf ein Spiel zeigt Details und Upload-Anleitung

  Verfügbare Spiele (examples/):
  - 12_Pong / 12b_Pong_Modern
  - 13_Snake / 13b_Snake_Modern
  - 14_Breakout / 14b_Breakout_Modern
  - 15_SpaceInvaders / 15b_SpaceInvaders_Modern
  - 16_Asteroids
  - 17_Frogger
  - 18_Tetris

  Hinweis:
  - Um ein Spiel zu spielen, lade die entsprechende .ino Datei
    auf dein CYD Board mit der Arduino IDE oder PlatformIO
  - Verwende das sync-games.ps1 Script um aktualisierte Versionen
    der Spiele zu übernehmen
*/

#include <CYD_Display_Config.h>

// Display Objekt
LGFX lcd;

// Display-Konfiguration
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// Farben
#define COLOR_BG        0x0000  // Schwarz
#define COLOR_TEXT      0xFFFF  // Weiß
#define COLOR_HEADER    0x001F  // Blau
#define COLOR_MENU_ITEM 0x2104  // Dunkelgrau
#define COLOR_SELECTED  0x07E0  // Grün
#define COLOR_BORDER    0x4208  // Mittelgrau

// Spiel-Definitionen
struct Game {
  const char* name;
  const char* folder;
  const char* description;
  uint16_t color;
};

Game games[] = {
  {"Pong", "12_Pong", "Klassisches 2-Spieler Paddle-Spiel", 0xFFE0},
  {"Pong Modern", "12b_Pong_Modern", "Pong mit modernem Design", 0xFD20},
  {"Snake", "13_Snake", "Klassische Schlange", 0x07E0},
  {"Snake Modern", "13b_Snake_Modern", "Snake mit Power-Ups", 0x07F0},
  {"Breakout", "14_Breakout", "Klassisches Brick-Breaker", 0xF800},
  {"Breakout Modern", "14b_Breakout_Modern", "Breakout mit Extras", 0xF810},
  {"Space Invaders", "15_SpaceInvaders", "Klassischer Shooter", 0x001F},
  {"Space Inv. Modern", "15b_SpaceInvaders_Modern", "Invaders mit Upgrades", 0x003F},
  {"Asteroids", "16_Asteroids", "Weltraum-Shooter", 0xF81F},
  {"Frogger", "17_Frogger", "Überquere Straße & Fluss", 0x07E0},
  {"Tetris", "18_Tetris", "Puzzle-Klassiker (Portrait!)", 0x07FF}
};

const int gameCount = sizeof(games) / sizeof(games[0]);

// Scroll-Position
int scrollOffset = 0;
int selectedGame = -1;
bool showDetails = false;

// Touch-State
bool lastTouchState = false;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== CYD GAME MENU ===\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);  // Landscape
  lcd.fillScreen(COLOR_BG);
  lcd.setBrightness(255);

  // Hauptmenü zeichnen
  drawMenu();

  Serial.println("Game Menu bereit!");
  Serial.println("Touch ein Spiel um Details zu sehen");
}

void loop() {
  // Touch-Input prüfen
  uint16_t x, y;
  bool touched = lcd.getTouch(&x, &y);

  if (touched) {
    // Nur auf neuen Touch reagieren (nicht gedrückt halten)
    if (!lastTouchState) {
      handleTouch(x, y);
      lastTouchState = true;
    }
  } else {
    lastTouchState = false;
  }

  delay(50);
}

// ===== ZEICHNEN =====

void drawMenu() {
  lcd.fillScreen(COLOR_BG);

  // Header
  lcd.fillRect(0, 0, SCREEN_WIDTH, 35, COLOR_HEADER);
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT, COLOR_HEADER);
  lcd.setCursor(60, 10);
  lcd.println("CYD ARCADE GAMES");

  // Spiele-Liste
  int y = 45;
  int itemHeight = 35;

  for (int i = 0; i < gameCount; i++) {
    int itemY = y + i * itemHeight - scrollOffset;

    // Nur zeichnen wenn sichtbar
    if (itemY < 35 || itemY > SCREEN_HEIGHT) continue;

    // Item-Hintergrund
    uint16_t bgColor = COLOR_MENU_ITEM;
    if (i == selectedGame) {
      bgColor = COLOR_SELECTED;
    }

    lcd.fillRect(10, itemY, SCREEN_WIDTH - 20, itemHeight - 3, bgColor);
    lcd.drawRect(10, itemY, SCREEN_WIDTH - 20, itemHeight - 3, COLOR_BORDER);

    // Spiel-Name (farbiges Symbol + Text)
    lcd.fillRect(15, itemY + 5, 25, 25, games[i].color);
    lcd.drawRect(15, itemY + 5, 25, 25, COLOR_TEXT);

    lcd.setTextSize(2);
    lcd.setTextColor(COLOR_TEXT, bgColor);
    lcd.setCursor(50, itemY + 10);
    lcd.println(games[i].name);
  }

  // Scroll-Indikator
  if (gameCount * itemHeight > SCREEN_HEIGHT - 45) {
    int scrollbarHeight = 150;
    int scrollbarY = 45 + (scrollOffset * scrollbarHeight) / ((gameCount * itemHeight) - (SCREEN_HEIGHT - 45));
    lcd.fillRect(SCREEN_WIDTH - 5, 45, 3, scrollbarHeight, COLOR_BORDER);
    lcd.fillRect(SCREEN_WIDTH - 5, 45 + scrollbarY, 3, 20, COLOR_SELECTED);
  }

  // Hinweis unten
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT, COLOR_BG);
  lcd.setCursor(10, SCREEN_HEIGHT - 15);
  lcd.println("Touch Spiel für Details");
}

void drawDetails() {
  lcd.fillScreen(COLOR_BG);

  // Header
  lcd.fillRect(0, 0, SCREEN_WIDTH, 35, games[selectedGame].color);
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT, games[selectedGame].color);

  // Spiel-Name zentriert
  int textWidth = strlen(games[selectedGame].name) * 12;
  lcd.setCursor((SCREEN_WIDTH - textWidth) / 2, 10);
  lcd.println(games[selectedGame].name);

  // Details-Bereich
  int y = 50;

  // Beschreibung
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT, COLOR_BG);
  lcd.setCursor(20, y);
  lcd.println("Beschreibung:");

  lcd.setCursor(20, y + 20);
  lcd.setTextColor(0xBDF7, COLOR_BG);  // Hellgrau
  lcd.println(games[selectedGame].description);

  // Ordner-Info
  y += 60;
  lcd.setTextColor(COLOR_TEXT, COLOR_BG);
  lcd.setCursor(20, y);
  lcd.println("Ordner:");

  lcd.setCursor(20, y + 20);
  lcd.setTextColor(0xBDF7, COLOR_BG);
  lcd.print("examples/");
  lcd.println(games[selectedGame].folder);

  // Upload-Anleitung
  y += 60;
  lcd.setTextColor(COLOR_TEXT, COLOR_BG);
  lcd.setCursor(20, y);
  lcd.println("Wie spielen:");

  lcd.setTextSize(1);
  lcd.setTextColor(0xBDF7, COLOR_BG);
  lcd.setCursor(20, y + 20);
  lcd.println("1. Öffne Arduino IDE / PlatformIO");
  lcd.setCursor(20, y + 35);
  lcd.println("2. Lade die .ino Datei aus dem");
  lcd.setCursor(20, y + 50);
  lcd.print("   examples/");
  lcd.print(games[selectedGame].folder);
  lcd.println("/ Ordner");
  lcd.setCursor(20, y + 65);
  lcd.println("3. Upload auf dein CYD Board");

  // Zurück-Button
  lcd.fillRect(10, SCREEN_HEIGHT - 40, 100, 30, COLOR_HEADER);
  lcd.drawRect(10, SCREEN_HEIGHT - 40, 100, 30, COLOR_TEXT);
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT, COLOR_HEADER);
  lcd.setCursor(25, SCREEN_HEIGHT - 32);
  lcd.println("ZURÜCK");
}

// ===== TOUCH-HANDLING =====

void handleTouch(int x, int y) {
  Serial.printf("Touch: x=%d, y=%d\n", x, y);

  if (showDetails) {
    // Zurück-Button (unten links)
    if (x >= 10 && x <= 110 && y >= SCREEN_HEIGHT - 40 && y <= SCREEN_HEIGHT - 10) {
      showDetails = false;
      selectedGame = -1;
      drawMenu();
      Serial.println("Zurück zum Menu");
    }
  } else {
    // Spiel-Auswahl in der Liste
    int itemHeight = 35;
    int listY = y - 45;  // Relativ zur Liste

    if (listY > 0) {
      int index = (listY + scrollOffset) / itemHeight;

      if (index >= 0 && index < gameCount) {
        selectedGame = index;
        showDetails = true;
        drawDetails();

        Serial.print("Spiel gewählt: ");
        Serial.println(games[selectedGame].name);
      }
    }
  }
}
