/*
  Tetris Modern - Simplified version using TetrisGameModern class

  Hardware:
  - CYD Display (240x320 im Portrait-Modus!)
  - 4x Digitale Buttons:
    * GPIO 5 (tasteB): Drehen
    * GPIO 16 (tasteC): Runter
    * GPIO 17 (tasteA): Rechts
    * GPIO 18 (tasteD): Links

  Moderne Features:
  - Gradient-Hintergrund, Glow-Effekte, Ghost-Piece Preview
  - Partikel-Effekte, 3D-Block-Rendering, Rainbow-Farben
*/

#include <CYD_Display_Config.h>
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

  // Button Inputs konfigurieren
  pinMode(tasteD, INPUT_PULLUP);
  pinMode(tasteA, INPUT_PULLUP);
  pinMode(tasteB, INPUT_PULLUP);
  pinMode(tasteC, INPUT_PULLUP);

  // Zufallsgenerator
  randomSeed(analogRead(35) + micros());

  // Spiel initialisieren
  game.init(&lcd);

  Serial.println("Tetris Modern gestartet!");
}

void loop() {
  game.update();
  delay(16);  // ~60 FPS
}
