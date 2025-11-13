/*
  Space Invaders - Klassisches Arcade-Spiel

  Verwendet die gekapselte CYD_Games Library.
  Diese Datei ist ein einfacher Wrapper zum Testen/Debuggen des Spiels.

  Hardware:
  - CYD Display (320x240)
  - 4x Digitale Buttons:
    * tasteB:  Links
    * tasteC: Rechts
    * tasteA: Schießen
    * tasteD: Pause

  Hinweis: Alle Pins sind in CYD_Display_Config.h definiert.

  Steuerung:
  - Links/Rechts: Bewegt Raumschiff horizontal
  - Schießen: Feuert Geschoss
  - Pause: Pausiert das Spiel

  Spielregeln:
  - Zerstöre alle Aliens
  - 3 Leben
  - Aliens schießen zurück
  - Schilde schützen vor Schüssen
  - Level wird schwieriger (schneller)
*/

#include <CYD_Display_Config.h>
#include <SpaceInvadersGame.h>

// Display Objekt
LGFX lcd;

// Game Objekt
SpaceInvadersGame game;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== SPACE INVADERS ===\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);  // Landscape
  lcd.fillScreen(0x0000);
  lcd.setBrightness(255);

  // Button Inputs konfigurieren (wird auch in game.init() gemacht, aber schadet nicht)
  pinMode(tasteA, INPUT_PULLUP);
  pinMode(tasteB, INPUT_PULLUP);
  pinMode(tasteC, INPUT_PULLUP);
  pinMode(tasteD, INPUT_PULLUP);

  // Zufallsgenerator
  randomSeed(analogRead(34) + analogRead(35) + micros());

  // Spiel initialisieren
  game.init(&lcd);

  Serial.println("Space Invaders gestartet!");
  Serial.println("Steuerung:");
  Serial.println("  Links:    GPIO 5  (tasteB)");
  Serial.println("  Rechts:   GPIO 16 (tasteC)");
  Serial.println("  Schießen: GPIO 17 (tasteA)");
  Serial.println("  Pause:    GPIO 18 (tasteD)");
}

void loop() {
  game.update();
}
