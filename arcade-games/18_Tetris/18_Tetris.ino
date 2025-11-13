/*
  Tetris - Klassisches Puzzle-Spiel

  Verwendet die gekapselte CYD_Games Library.
  Diese Datei ist ein einfacher Wrapper zum Testen/Debuggen des Spiels.

  WICHTIG: Dieses Spiel verwendet Portrait-Modus (240x320)!

  Hardware:
  - CYD Display (240x320 im Portrait-Modus!)
  - 4x Digitale Buttons (im Portrait-Modus):
    * tasteB:  Drehen (oben)
    * tasteC: Runter (unten, Schnell-Fall)
    * tasteA: Rechts (rechts)
    * tasteD: Links (links)

  Hinweis: Alle Pins sind in CYD_Display_Config.h definiert.

  Steuerung:
  - Links/Rechts: Stein bewegen
  - Drehen: Stein rotieren
  - Runter: Schneller fallen lassen

  Spielregeln:
  - Fülle horizontale Reihen um Punkte zu bekommen
  - Je mehr Reihen gleichzeitig, desto mehr Punkte
  - Spiel endet wenn Steine oben ankommen
  - Level steigt alle 10 Reihen (schneller)
*/

#include <CYD_Display_Config.h>
#include <TetrisGame.h>

// Display Objekt
LGFX lcd;

// Game Objekt
TetrisGame game;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== TETRIS ===\n");

  // Display initialisieren (PORTRAIT!)
  lcd.init();
  lcd.setRotation(0);  // Portrait-Modus!
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

  Serial.println("Tetris gestartet!");
  Serial.println("Steuerung (Portrait-Modus):");
  Serial.println("  Links:  GPIO 18 (tasteD)");
  Serial.println("  Rechts: GPIO 17 (tasteA)");
  Serial.println("  Runter: GPIO 16 (tasteC)");
  Serial.println("  Drehen: GPIO 5  (tasteB)");
}

void loop() {
  game.update();
}
