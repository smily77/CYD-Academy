/*
  Pong Game - Klassisches 2-Spieler Pong (Standalone Version)

  Hardware:
  - CYD Display (320x240)
  - 2x Analog-Regler (Potentiometer):
    * GPIO 34 (potiLeft): Linker Spieler
    * GPIO 35 (potiRight): Rechter Spieler
  - 3x Taster:
    * GPIO 16 (tasteC): Auto-Modus linker Spieler EIN
    * GPIO 5 (tasteB): Auto-Modus linker Spieler AUS
    * GPIO 17 (tasteA): Spielstand auf 0 setzen

  Steuerung:
  - Linker Regler: Bewegt linken Schläger (oder AI im Auto-Modus)
  - Rechter Regler: Bewegt rechten Schläger

  Hinweis:
  - Dieses Programm ist ein Wrapper für PongGame.h aus der CYD_Games Library
  - Der komplette Spiel-Code ist in libraries/CYD_Games/PongGame.h
  - Für ein Menu mit allen Spielen: siehe examples/00_RetroGames/
*/

#include <CYD_Display_Config.h>
#include <PongGame.h>

// Display Objekt
LGFX lcd;

// Spiel-Instanz
PongGame game;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== PONG GAME (Standalone) ===\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);  // Landscape
  lcd.fillScreen(0x0000);
  lcd.setBrightness(255);

  // Analog Inputs konfigurieren
  pinMode(potiLeft, INPUT);
  pinMode(potiRight, INPUT);

  // Taster Inputs konfigurieren
  pinMode(tasteA, INPUT_PULLUP);
  pinMode(tasteB, INPUT_PULLUP);
  pinMode(tasteC, INPUT_PULLUP);

  // Zufallsgenerator
  randomSeed(analogRead(potiLeft) + analogRead(potiRight) + micros());

  // Spiel initialisieren
  game.init(&lcd);

  Serial.println("Pong gestartet! (Standalone Version)");
  Serial.println("Steuerung:");
  Serial.println("  potiLeft (GPIO 34):   Linker Schläger");
  Serial.println("  potiRight (GPIO 35):  Rechter Schläger");
  Serial.println("  tasteC (GPIO 16):     Auto-Modus EIN");
  Serial.println("  tasteB (GPIO 5):      Auto-Modus AUS");
  Serial.println("  tasteA (GPIO 17):     Score Reset");
}

void loop() {
  // Spiel aktualisieren
  game.update();
}
