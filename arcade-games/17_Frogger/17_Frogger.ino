/*
  Frogger - Klassisches Arcade-Spiel

  Verwendet die gekapselte CYD_Games Library.
  Diese Datei ist ein einfacher Wrapper zum Testen/Debuggen des Spiels.

  Hardware:
  - CYD Display (320x240)
  - 4x Digitale Buttons:
    * tasteB:  Links
    * tasteC: Rechts
    * tasteA: Oben
    * tasteD: Unten

  Hinweis: Alle Pins sind in CYD_Display_Config.h definiert.

  Steuerung:
  - Tasten: Bewege Frosch hoch/runter/links/rechts

  Spielregeln:
  - Bringe den Frosch sicher nach oben zu den Häusern
  - Straße (Y=2-9): Vermeide Autos
  - Erholungsspuren (Y=10-11): Sichere Zone zum Ausruhen
  - Fluss (Y=12-19): Springe auf Baumstämme!
  - WICHTIG: Frosch ERTRINKT im Wasser, Baumstämme retten und tragen dich!
  - Erreiche alle 5 Häuser für nächstes Level
  - 3 Leben
  - Punkte: Vorwärts bewegen = +10, Haus erreichen = +50
*/

#include <CYD_Display_Config.h>
#include <CYD_Input.h>
#include <FroggerGame.h>

// Display Objekt
LGFX lcd;

// Game Objekt
FroggerGame game;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== FROGGER ===\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);  // Landscape
  lcd.fillScreen(0x0000);
  lcd.setBrightness(255);

  // Input initialisieren
  CYD_Input::init();

  // Zufallsgenerator
  randomSeed(CYD_Input::readPoti(CYD_POTI_RIGHT) + micros());

  // Spiel initialisieren
  game.init(&lcd);

  Serial.println("Frogger gestartet!");
  Serial.println("Steuerung:");
  Serial.println("  Links:  GPIO 5  (tasteB)");
  Serial.println("  Oben:   GPIO 17 (tasteA)");
  Serial.println("  Rechts: GPIO 16 (tasteC)");
  Serial.println("  Unten:  GPIO 18 (tasteD)");
}

void loop() {
  game.update();
}
