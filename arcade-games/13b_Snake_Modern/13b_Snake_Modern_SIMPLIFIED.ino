/*
  Snake Game MODERN - Simplified version using SnakeGameModern class

  Hardware:
  - CYD Display (320x240)
  - 4x Hardware-Buttons: tasteB (Links), tasteA (Oben), tasteC (Rechts), tasteD (Unten)

  Hinweis: Alle Pins sind in CYD_Display_Config.h definiert.

  Design-Features:
  - Gradient-Hintergrund, 3D-Schlange mit Schatten
  - Neon-Glow-Effekte, Pulsierendes Food, Partikeleffekt
*/

#include <CYD_Display_Config.h>
#include <CYD_Input.h>
#include <SnakeGameModern.h>

LGFX lcd;
SnakeGameModern game;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== SNAKE GAME MODERN ===\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);
  lcd.setBrightness(255);

  // Input initialisieren
  CYD_Input::init();

  // Zufallsgenerator
  randomSeed(CYD_Input::readPoti(CYD_POTI_LEFT) + micros());

  // Spiel initialisieren
  game.init(&lcd);

  Serial.println("Modern Snake gestartet!");
}

void loop() {
  game.update();
  delay(16);  // ~60 FPS
}
