/*
  Pong Game MODERN - Simplified version using PongGameModern class

  Hardware:
  - CYD Display (320x240)
  - 2x Analog-Regler (Potentiometer): potiLeft (links), potiRight (rechts)
  - 3x Taster: tasteC (Auto ON), tasteB (Auto OFF), tasteA (Reset)

  Hinweis: Alle Pins sind in CYD_Display_Config.h definiert.

  Design-Features:
  - Gradient-Hintergrund, Neon-Glüheffekte, 3D-Schläger
  - Partikeleffekt, Trail-Effekt, Animierte UI
*/

#include <CYD_Display_Config.h>
#include <PongGameModern.h>

LGFX lcd;
PongGameModern game;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== PONG GAME MODERN ===\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);
  lcd.setBrightness(255);

  // Pins konfigurieren
  pinMode(potiLeft, INPUT);
  pinMode(potiRight, INPUT);
  pinMode(tasteA, INPUT_PULLUP);
  pinMode(tasteB, INPUT_PULLUP);
  pinMode(tasteC, INPUT_PULLUP);

  // Zufallsgenerator
  randomSeed(analogRead(potiLeft) + analogRead(potiRight) + micros());

  // Spiel initialisieren
  game.init(&lcd);

  Serial.println("Modern Pong gestartet!");
}

void loop() {
  game.update();
  delay(16);  // ~60 FPS
}
