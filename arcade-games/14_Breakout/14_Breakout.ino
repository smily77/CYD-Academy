/*
  Breakout Game - Klassisches Breakout/Arkanoid Spiel (Standalone Version)

  Hardware:
  - CYD Display (320x240)
  - 1x Analog-Regler (Potentiometer):
    * GPIO 34 (potiLeft): Schläger-Steuerung (Links/Rechts)
  - 2x Taster:
    * GPIO 17 (tasteA): Ball-Start
    * GPIO 18 (tasteD): Pause

  Steuerung:
  - Analog-Regler: Bewegt Schläger horizontal
  - tasteA: Startet Ball (wenn am Schläger)
  - tasteD: Pause

  Spielregeln:
  - Zerstöre alle Steine mit dem Ball
  - Ball darf nicht unten rausfallen (3 Leben)
  - Verschiedene Stein-Farben = verschiedene Punkte
  - Level komplett = Nächstes Level

  Hinweis:
  - Dieses Programm ist ein Wrapper für BreakoutGame.h aus der CYD_Games Library
  - Der komplette Spiel-Code ist in libraries/CYD_Games/BreakoutGame.h
  - Für ein Menu mit allen Spielen: siehe examples/00_RetroGames/
*/

#include <CYD_Display_Config.h>
#include <BreakoutGame.h>

// Display Objekt
LGFX lcd;

// Spiel-Instanz
BreakoutGame game;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== BREAKOUT GAME (Standalone) ===\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);  // Landscape
  lcd.fillScreen(0x0000);
  lcd.setBrightness(255);

  // Analog Input konfigurieren
  pinMode(potiLeft, INPUT);

  // Button Inputs konfigurieren
  pinMode(tasteA, INPUT_PULLUP);
  pinMode(tasteD, INPUT_PULLUP);

  // Zufallsgenerator
  randomSeed(analogRead(potiLeft) + analogRead(35) + micros());

  // Spiel initialisieren
  game.init(&lcd);

  Serial.println("Breakout gestartet! (Standalone Version)");
  Serial.println("Steuerung:");
  Serial.println("  potiLeft (GPIO 34): Schläger-Steuerung");
  Serial.println("  tasteA (GPIO 17):   Ball-Start");
  Serial.println("  tasteD (GPIO 18):   Pause");
}

void loop() {
  // Spiel aktualisieren
  game.update();
}
