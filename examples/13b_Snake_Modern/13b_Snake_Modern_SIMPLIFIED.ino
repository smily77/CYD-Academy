/*
  Snake Game MODERN - Simplified version using SnakeGameModern class

  Hardware:
  - CYD Display (320x240)
  - 4x Hardware-Buttons: GPIO 5 (Links), GPIO 17 (Oben), GPIO 16 (Rechts), GPIO 18 (Unten)

  Design-Features:
  - Gradient-Hintergrund, 3D-Schlange mit Schatten
  - Neon-Glow-Effekte, Pulsierendes Food, Partikeleffekt
*/

#include <CYD_Display_Config.h>
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

  // Button Pins konfigurieren
  pinMode(tasteB, INPUT_PULLUP);
  pinMode(tasteA, INPUT_PULLUP);
  pinMode(tasteC, INPUT_PULLUP);
  pinMode(tasteD, INPUT_PULLUP);

  // Zufallsgenerator
  randomSeed(analogRead(34) + micros());

  // Spiel initialisieren
  game.init(&lcd);

  Serial.println("Modern Snake gestartet!");
}

void loop() {
  game.update();
  delay(16);  // ~60 FPS
}
