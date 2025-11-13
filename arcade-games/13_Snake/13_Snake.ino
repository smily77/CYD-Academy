/*
  Snake Game - Klassisches Snake-Spiel (Standalone Version)

  Hardware:
  - CYD Display (320x240)
  - 4x Hardware-Buttons:
    * GPIO 5 (tasteB):  Links
    * GPIO 17 (tasteA): Oben
    * GPIO 16 (tasteC): Rechts
    * GPIO 18 (tasteD): Unten

  Steuerung:
  - Buttons steuern die Bewegungsrichtung
  - Sammle rote Food-Items
  - Vermeide Wände und deinen eigenen Körper

  Hinweis:
  - Dieses Programm ist ein Wrapper für SnakeGame.h aus der CYD_Games Library
  - Der komplette Spiel-Code ist in libraries/CYD_Games/SnakeGame.h
  - Für ein Menu mit allen Spielen: siehe examples/00_RetroGames/
*/

#include <CYD_Display_Config.h>
#include <SnakeGame.h>

// Display Objekt
LGFX lcd;

// Spiel-Instanz
SnakeGame game;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== SNAKE GAME (Standalone) ===\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);  // Landscape
  lcd.fillScreen(0x0000);
  lcd.setBrightness(255);

  // Button Pins konfigurieren
  pinMode(tasteA, INPUT_PULLUP);
  pinMode(tasteB, INPUT_PULLUP);
  pinMode(tasteC, INPUT_PULLUP);
  pinMode(tasteD, INPUT_PULLUP);

  // Zufallsgenerator
  randomSeed(analogRead(35) + micros());

  // Spiel initialisieren
  game.init(&lcd);

  Serial.println("Snake gestartet! (Standalone Version)");
  Serial.println("Steuerung:");
  Serial.println("  tasteB (GPIO 5):  Links");
  Serial.println("  tasteA (GPIO 17): Oben");
  Serial.println("  tasteC (GPIO 16): Rechts");
  Serial.println("  tasteD (GPIO 18): Unten");
}

void loop() {
  // Spiel aktualisieren
  game.update();
}
