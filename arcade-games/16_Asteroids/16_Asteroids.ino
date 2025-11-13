/*
  Asteroids - Klassisches Arcade-Spiel

  Verwendet die gekapselte CYD_Games Library.
  Diese Datei ist ein einfacher Wrapper zum Testen/Debuggen des Spiels.

  Hardware:
  - CYD Display (320x240)
  - 1x Potentiometer (potiLeft): Optional für Rotation
  - 4x Digitale Buttons:
    * tasteB:  Links drehen
    * tasteC: Rechts drehen
    * tasteA: Schießen
    * tasteD: Schub (Thrust)

  Hinweis: Alle Pins sind in CYD_Display_Config.h definiert.

  Konfiguration:
  - USE_POT_ROTATION: Auskommentiert = Tasten, Einkommentiert = Poti+Tasten

  Steuerung:
  - Links/Rechts: Rotiert Raumschiff (Tasten-Modus)
  - Poti: Direktsteuerung Rotation 0-360° (Poti-Modus)
  - Schub: Beschleunigt in Blickrichtung
  - Schießen: Feuert Geschoss

  Spielregeln:
  - Zerstöre alle Asteroids
  - 3 Leben
  - Große Asteroids zerbrechen in 2 mittlere
  - Mittlere Asteroids zerbrechen in 2 kleine
  - Screen-Wrapping (Objekte gehen durch Bildschirmränder)
  - Punkte: Klein=100, Mittel=50, Groß=20
*/

#include <CYD_Display_Config.h>
#include <AsteroidsGame.h>

// ===== KONFIGURATION =====
// Auskommentieren für Tasten-Steuerung, einkommentieren für Poti-Steuerung
// #define USE_POT_ROTATION

// Display Objekt
LGFX lcd;

// Game Objekt
AsteroidsGame game;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ASTEROIDS ===\n");

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

#ifdef USE_POT_ROTATION
  // ADC konfigurieren (0-1V Bereich)
  analogSetAttenuation(ADC_0db);  // 0-1V Range
  pinMode(potiLeft, INPUT);
#endif

  // Zufallsgenerator
  randomSeed(analogRead(35) + micros());

  // Spiel initialisieren
#ifdef USE_POT_ROTATION
  game.init(&lcd, true);  // Mit Potentiometer-Steuerung
#else
  game.init(&lcd, false);  // Nur Tasten-Steuerung
#endif

  Serial.println("Asteroids gestartet!");
  Serial.println("Steuerung:");
#ifdef USE_POT_ROTATION
  Serial.println("  Poti:     GPIO 34 (Rotation 0-360°)");
  Serial.println("  Links:    GPIO 5  (tasteB) - Backup");
  Serial.println("  Rechts:   GPIO 16 (tasteC) - Backup");
#else
  Serial.println("  Links:    GPIO 5  (tasteB)");
  Serial.println("  Rechts:   GPIO 16 (tasteC)");
#endif
  Serial.println("  Schießen: GPIO 17 (tasteA)");
  Serial.println("  Schub:    GPIO 18 (tasteD)");
}

void loop() {
  game.update();
}
