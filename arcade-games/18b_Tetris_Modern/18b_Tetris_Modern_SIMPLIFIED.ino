/*
  Tetris Modern - Simplified version using TetrisGameModern class

  Hardware:
  - CYD Display (240x320 im Portrait-Modus!)
  - 4x Digitale Buttons:
    * tasteB: Drehen
    * tasteC: Runter
    * tasteA: Rechts
    * tasteD: Links

  Hinweis: Alle Pins sind in CYD_Display_Config.h definiert.

  Moderne Features:
  - Gradient-Hintergrund, Glow-Effekte, Ghost-Piece Preview
  - Partikel-Effekte, 3D-Block-Rendering, Rainbow-Farben
*/

#include <CYD_Display_Config.h>
#include <CYD_Input.h>
#include <TetrisGameModern.h>

LGFX lcd;
TetrisGameModern game;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== TETRIS MODERN ===\n");

  // Display initialisieren (PORTRAIT!)
  lcd.init();
  lcd.setRotation(0);  // Portrait-Modus!
  lcd.setBrightness(255);

  // Input initialisieren
  CYD_Input::init();

  // Zufallsgenerator
  randomSeed(CYD_Input::readPoti(CYD_POTI_RIGHT) + micros());

  // Spiel initialisieren
  game.init(&lcd);

  Serial.println("Tetris Modern gestartet!");
}

void loop() {
  game.update();
  delay(16);  // ~60 FPS
}
