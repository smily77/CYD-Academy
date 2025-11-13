/*
  Frogger - Klassisches Arcade-Spiel

  Verwendet die gekapselte CYD_Games Library.
  Diese Datei ist ein einfacher Wrapper zum Testen/Debuggen des Spiels.

  Hardware:
  - CYD Display (320x240)
  - 4x Digitale Buttons:
    * GPIO 5:  Links
    * GPIO 16: Rechts
    * GPIO 17: Oben
    * GPIO 18: Unten

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

  // Button Inputs konfigurieren (wird auch in game.init() gemacht, aber schadet nicht)
  pinMode(tasteA, INPUT_PULLUP);
  pinMode(tasteB, INPUT_PULLUP);
  pinMode(tasteC, INPUT_PULLUP);
  pinMode(tasteD, INPUT_PULLUP);

  // Zufallsgenerator
  randomSeed(analogRead(35) + micros());

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
