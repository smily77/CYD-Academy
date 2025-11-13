/*
  CYD_Input.h - Zentrale Input-Abstraktion für CYD Games

  Unterstützt:
  - GPIO-Buttons (direkt am ESP32)
  - I2C-Buttons (über PCF8574 Portextender)
  - GPIO-Potentiometer (optional)

  Konfiguration erfolgt in CYD_Display_Config.h:

  Option 1: Buttons direkt an GPIO
    #define gpioSwitch
    #define tasteA 17
    #define tasteB 5
    #define tasteC 16
    #define tasteD 18

  Option 2: Buttons über I2C Portextender PCF8574
    #define i2cSwitch
    #define tasteA 0    // PCF8574 Pin 0
    #define tasteB 1    // PCF8574 Pin 1
    #define tasteC 2    // PCF8574 Pin 2
    #define tasteD 3    // PCF8574 Pin 3
    #define pcfAddress 0x20
    #define switchInterrupt 22  // Optional: Interrupt Pin

  Potentiometer (optional):
    #define gpioPoti
    #define potiLeft 34
    #define potiRight 35

  Verwendung in Spielen:
    CYD_Input::init();
    if (CYD_Input::readButton(CYD_BTN_A)) { ... }
    int value = CYD_Input::readPoti(CYD_POTI_LEFT);
*/

#ifndef CYD_INPUT_H
#define CYD_INPUT_H

#include <Arduino.h>

// Konfigurationsprüfung
#if defined(gpioSwitch) && defined(i2cSwitch)
  #error "FEHLER: gpioSwitch und i2cSwitch können nicht gleichzeitig definiert sein!"
#endif

#if !defined(gpioSwitch) && !defined(i2cSwitch)
  #error "FEHLER: Entweder gpioSwitch oder i2cSwitch muss in CYD_Display_Config.h definiert sein!"
#endif

// PCF8574 Library nur laden wenn i2cSwitch aktiv
#ifdef i2cSwitch
  #include <PCF8574.h>
#endif

// Button-Konstanten für lesbareren Code
#define CYD_BTN_A 0
#define CYD_BTN_B 1
#define CYD_BTN_C 2
#define CYD_BTN_D 3

// Poti-Konstanten
#define CYD_POTI_LEFT  0
#define CYD_POTI_RIGHT 1

class CYD_Input {
public:
  // ===== INITIALISIERUNG =====

  static bool init() {
    #ifdef gpioSwitch
      // GPIO-Buttons initialisieren
      pinMode(tasteA, INPUT_PULLUP);
      pinMode(tasteB, INPUT_PULLUP);
      pinMode(tasteC, INPUT_PULLUP);
      pinMode(tasteD, INPUT_PULLUP);

      Serial.println("CYD_Input: GPIO-Buttons initialisiert");
      Serial.printf("  tasteA: GPIO %d\n", tasteA);
      Serial.printf("  tasteB: GPIO %d\n", tasteB);
      Serial.printf("  tasteC: GPIO %d\n", tasteC);
      Serial.printf("  tasteD: GPIO %d\n", tasteD);
    #endif

    #ifdef i2cSwitch
      // PCF8574 initialisieren
      #ifndef pcfAddress
        #error "FEHLER: pcfAddress muss definiert sein wenn i2cSwitch aktiv ist!"
      #endif

      pcf = PCF8574(pcfAddress);

      // Alle Pins als Input mit Pullup
      pcf.pinMode(tasteA, INPUT);
      pcf.pinMode(tasteB, INPUT);
      pcf.pinMode(tasteC, INPUT);
      pcf.pinMode(tasteD, INPUT);

      // PCF8574 starten
      if (!pcf.begin()) {
        Serial.println("FEHLER: PCF8574 nicht gefunden!");
        return false;
      }

      Serial.println("CYD_Input: I2C-Buttons (PCF8574) initialisiert");
      Serial.printf("  I2C-Adresse: 0x%02X\n", pcfAddress);
      Serial.printf("  tasteA: PCF Pin %d\n", tasteA);
      Serial.printf("  tasteB: PCF Pin %d\n", tasteB);
      Serial.printf("  tasteC: PCF Pin %d\n", tasteC);
      Serial.printf("  tasteD: PCF Pin %d\n", tasteD);

      #ifdef switchInterrupt
        pinMode(switchInterrupt, INPUT_PULLUP);
        Serial.printf("  Interrupt: GPIO %d\n", switchInterrupt);
      #endif
    #endif

    #ifdef gpioPoti
      // Potis sind analog, brauchen keine Initialisierung
      Serial.println("CYD_Input: GPIO-Potentiometer verfügbar");
      Serial.printf("  potiLeft:  GPIO %d\n", potiLeft);
      Serial.printf("  potiRight: GPIO %d\n", potiRight);
    #else
      Serial.println("CYD_Input: Keine Potentiometer definiert");
    #endif

    return true;
  }

  // ===== BUTTON-INPUT =====

  // Liest Button-Status (LOW = gedrückt, HIGH = nicht gedrückt)
  static bool readButton(int button) {
    #ifdef gpioSwitch
      switch (button) {
        case CYD_BTN_A: return digitalRead(tasteA) == LOW;
        case CYD_BTN_B: return digitalRead(tasteB) == LOW;
        case CYD_BTN_C: return digitalRead(tasteC) == LOW;
        case CYD_BTN_D: return digitalRead(tasteD) == LOW;
        default: return false;
      }
    #endif

    #ifdef i2cSwitch
      // Optional: Interrupt-optimiert
      #ifdef switchInterrupt
        // Nur lesen wenn Interrupt aktiv (LOW = Button gedrückt)
        if (digitalRead(switchInterrupt) == HIGH) {
          return false;  // Keine Buttons gedrückt
        }
      #endif

      switch (button) {
        case CYD_BTN_A: return pcf.digitalRead(tasteA) == LOW;
        case CYD_BTN_B: return pcf.digitalRead(tasteB) == LOW;
        case CYD_BTN_C: return pcf.digitalRead(tasteC) == LOW;
        case CYD_BTN_D: return pcf.digitalRead(tasteD) == LOW;
        default: return false;
      }
    #endif

    return false;
  }

  // Kompatibilität: Direkter Zugriff auf Button-Pins für legacy Code
  static bool readButtonRaw(int button) {
    return readButton(button);
  }

  // ===== POTENTIOMETER-INPUT =====

  // Prüft ob Potentiometer verfügbar sind
  static bool hasPotis() {
    #ifdef gpioPoti
      return true;
    #else
      return false;
    #endif
  }

  // Liest Potentiometer-Wert (0-1000 für Kompatibilität mit bestehenden Spielen)
  static int readPoti(int poti) {
    #ifdef gpioPoti
      int rawValue = 0;

      switch (poti) {
        case CYD_POTI_LEFT:
          rawValue = analogRead(potiLeft);
          break;
        case CYD_POTI_RIGHT:
          rawValue = analogRead(potiRight);
          break;
        default:
          return 0;
      }

      // ESP32 ADC: Angepasst für 1V-Potis (1V / 3,3V * 4095 ≈ 1241)
      // Statt 0-4095 verwenden wir 0-1241 für 1V-Potis
      // constrain() stellt sicher, dass auch Werte > 1241 korrekt auf max 1000 gemappt werden
      int mappedValue = map(rawValue, 0, 1241, 0, 1000);
      return constrain(mappedValue, 0, 1000);
    #else
      // Keine Potis verfügbar
      return 500;  // Mittelwert zurückgeben
    #endif
  }

  // ===== DEBUG-FUNKTIONEN =====

  static void printStatus() {
    Serial.println("\n=== CYD_Input Status ===");

    #ifdef gpioSwitch
      Serial.println("Modus: GPIO-Buttons");
    #endif

    #ifdef i2cSwitch
      Serial.println("Modus: I2C-Buttons (PCF8574)");
      Serial.printf("I2C-Adresse: 0x%02X\n", pcfAddress);
    #endif

    Serial.println("\nButtons:");
    Serial.printf("  A: %s\n", readButton(CYD_BTN_A) ? "PRESSED" : "released");
    Serial.printf("  B: %s\n", readButton(CYD_BTN_B) ? "PRESSED" : "released");
    Serial.printf("  C: %s\n", readButton(CYD_BTN_C) ? "PRESSED" : "released");
    Serial.printf("  D: %s\n", readButton(CYD_BTN_D) ? "PRESSED" : "released");

    #ifdef gpioPoti
      Serial.println("\nPotentiometer:");
      Serial.printf("  Left:  %d\n", readPoti(CYD_POTI_LEFT));
      Serial.printf("  Right: %d\n", readPoti(CYD_POTI_RIGHT));
    #else
      Serial.println("\nPotentiometer: Nicht verfügbar");
    #endif

    Serial.println("========================\n");
  }

private:
  #ifdef i2cSwitch
    static PCF8574 pcf;
  #endif
};

// Statische Member initialisieren
#ifdef i2cSwitch
  PCF8574 CYD_Input::pcf(0x20);  // Wird in init() mit richtiger Adresse überschrieben
#endif

#endif // CYD_INPUT_H
