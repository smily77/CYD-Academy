/*
  CYD_Input.h - Zentrale Input-Abstraktion für CYD Games

  Unterstützt:
  - GPIO-Buttons (direkt am ESP32)
  - I2C-Buttons (über PCF8574 Portextender)
  - GPIO-Encoder (Rotary Encoder direkt am ESP32)
  - I2C-Encoder (Rotary Encoder über PCF8574)
  - GPIO-Potentiometer (optional, nur mit Buttons)

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

  Option 3: Encoder direkt an GPIO
    #define gpioEnc
    #define encA 17
    #define encB 5
    #define encSW 16

  Option 4: Encoder über I2C Portextender PCF8574
    #define i2cEnc
    #define encA 0      // PCF8574 Pin 0
    #define encB 1      // PCF8574 Pin 1
    #define encSW 2     // PCF8574 Pin 2
    #define pcfAddress 0x20

  Potentiometer (optional, nur mit gpioSwitch/i2cSwitch):
    #define gpioPoti
    #define potiLeft 34
    #define potiRight 35

  Verwendung in Spielen:
    CYD_Input::init();
    if (CYD_Input::readButton(CYD_BTN_A)) { ... }
    int value = CYD_Input::readPoti(CYD_POTI_LEFT);
    int delta = CYD_Input::readEncoderDelta();
*/

#ifndef CYD_INPUT_H
#define CYD_INPUT_H

#include <Arduino.h>

// Konfigurationsprüfung - nur EINE Methode darf definiert sein
#if (defined(gpioSwitch) + defined(i2cSwitch) + defined(gpioEnc) + defined(i2cEnc)) > 1
  #error "FEHLER: Nur EINE von gpioSwitch, i2cSwitch, gpioEnc, i2cEnc darf definiert sein!"
#endif

#if !defined(gpioSwitch) && !defined(i2cSwitch) && !defined(gpioEnc) && !defined(i2cEnc)
  #error "FEHLER: Genau EINE von gpioSwitch, i2cSwitch, gpioEnc, i2cEnc muss definiert sein!"
#endif

// PCF8574 Library nur laden wenn i2cSwitch oder i2cEnc aktiv
#if defined(i2cSwitch) || defined(i2cEnc)
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

    #ifdef gpioEnc
      // GPIO-Encoder initialisieren
      pinMode(encA, INPUT_PULLUP);
      pinMode(encB, INPUT_PULLUP);
      pinMode(encSW, INPUT_PULLUP);

      // Initialen Status lesen
      lastEncA = digitalRead(encA);
      lastEncB = digitalRead(encB);
      encoderPos = 0;

      Serial.println("CYD_Input: GPIO-Encoder initialisiert");
      Serial.printf("  encA:  GPIO %d\n", encA);
      Serial.printf("  encB:  GPIO %d\n", encB);
      Serial.printf("  encSW: GPIO %d\n", encSW);
    #endif

    #ifdef i2cEnc
      // PCF8574 initialisieren
      #ifndef pcfAddress
        #error "FEHLER: pcfAddress muss definiert sein wenn i2cEnc aktiv ist!"
      #endif

      pcf = PCF8574(pcfAddress);

      // Encoder-Pins als Input
      pcf.pinMode(encA, INPUT);
      pcf.pinMode(encB, INPUT);
      pcf.pinMode(encSW, INPUT);

      // PCF8574 starten
      if (!pcf.begin()) {
        Serial.println("FEHLER: PCF8574 nicht gefunden!");
        return false;
      }

      // Initialen Status lesen
      lastEncA = pcf.digitalRead(encA);
      lastEncB = pcf.digitalRead(encB);
      encoderPos = 0;

      Serial.println("CYD_Input: I2C-Encoder (PCF8574) initialisiert");
      Serial.printf("  I2C-Adresse: 0x%02X\n", pcfAddress);
      Serial.printf("  encA:  PCF Pin %d\n", encA);
      Serial.printf("  encB:  PCF Pin %d\n", encB);
      Serial.printf("  encSW: PCF Pin %d\n", encSW);
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

  // ===== ENCODER-INPUT =====

  // Prüft ob Encoder verfügbar ist
  static bool hasEncoder() {
    #if defined(gpioEnc) || defined(i2cEnc)
      return true;
    #else
      return false;
    #endif
  }

  // Liest Encoder-Drehung und gibt Delta zurück (-1 = links, 0 = keine Änderung, +1 = rechts)
  static int readEncoderDelta() {
    #if defined(gpioEnc) || defined(i2cEnc)
      // Aktuelle Werte lesen
      #ifdef gpioEnc
        int currentA = digitalRead(encA);
        int currentB = digitalRead(encB);
      #else
        int currentA = pcf.digitalRead(encA);
        int currentB = pcf.digitalRead(encB);
      #endif

      int delta = 0;

      // Prüfe auf Flanke von A
      if (currentA != lastEncA) {
        // Bei fallender Flanke von A
        if (currentA == LOW) {
          // Wenn B auch LOW ist, dann Drehung im Uhrzeigersinn (rechts)
          if (currentB == LOW) {
            encoderPos++;
            delta = 1;
          } else {
            // Sonst gegen Uhrzeigersinn (links)
            encoderPos--;
            delta = -1;
          }
        }
      }

      lastEncA = currentA;
      lastEncB = currentB;

      return delta;
    #else
      return 0;
    #endif
  }

  // Liest Encoder-Button (LOW = gedrückt)
  static bool readEncoderButton() {
    #ifdef gpioEnc
      return digitalRead(encSW) == LOW;
    #endif

    #ifdef i2cEnc
      return pcf.digitalRead(encSW) == LOW;
    #endif

    return false;
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
  #if defined(i2cSwitch) || defined(i2cEnc)
    static PCF8574 pcf;
  #endif

  #if defined(gpioEnc) || defined(i2cEnc)
    static int lastEncA;
    static int lastEncB;
    static int encoderPos;
  #endif
};

// Statische Member initialisieren
#if defined(i2cSwitch) || defined(i2cEnc)
  PCF8574 CYD_Input::pcf(0x20);  // Wird in init() mit richtiger Adresse überschrieben
#endif

#if defined(gpioEnc) || defined(i2cEnc)
  int CYD_Input::lastEncA = 0;
  int CYD_Input::lastEncB = 0;
  int CYD_Input::encoderPos = 0;
#endif

#endif // CYD_INPUT_H
