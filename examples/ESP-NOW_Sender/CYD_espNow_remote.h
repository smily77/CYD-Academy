/*
  CYD_espNow_remote.h - Konfiguration für ESP-NOW Sender (Remote)

  Diese Datei definiert die Hardware-Konfiguration für den ESP-NOW Sender.
  Sie wird von allen Sender-Sketches verwendet.

  WICHTIG: Passe diese Datei an deine Hardware an!
*/

#ifndef CYD_ESPNOW_REMOTE_H
#define CYD_ESPNOW_REMOTE_H

#include <Arduino.h>

// ===== SENDER-KONFIGURATION =====

// SENDER-TYP: Wähle EINEN der folgenden Modi
// Kommentiere die anderen aus!
#define SENDER_TYPE_SWITCH    // Für 4 Buttons
// #define SENDER_TYPE_ENCODER  // Für Rotary Encoder
// #define SENDER_TYPE_POTI     // Für 2 Potentiometer

// ===== PIN-KONFIGURATION =====

// --- Buttons (für SENDER_TYPE_SWITCH) ---
#ifdef SENDER_TYPE_SWITCH
  #define BTN_A_PIN 17    // Button A GPIO Pin
  #define BTN_B_PIN 5     // Button B GPIO Pin
  #define BTN_C_PIN 16    // Button C GPIO Pin
  #define BTN_D_PIN 18    // Button D GPIO Pin
#endif

// --- Rotary Encoder (für SENDER_TYPE_ENCODER) ---
#ifdef SENDER_TYPE_ENCODER
  #define ENC_A_PIN  17   // Encoder Pin A (CLK)
  #define ENC_B_PIN  5    // Encoder Pin B (DT)
  #define ENC_SW_PIN 16   // Encoder Button (SW)
#endif

// --- Potentiometer (für SENDER_TYPE_POTI) ---
#ifdef SENDER_TYPE_POTI
  #define POTI_LEFT_PIN  34   // Linkes Potentiometer (ADC Pin)
  #define POTI_RIGHT_PIN 35   // Rechtes Potentiometer (ADC Pin)
#endif

// ===== STATUS-LED KONFIGURATION =====

#define STATUS_LED_PIN 2      // GPIO Pin für Status-LED (oft onboard LED)
#define LED_BLINK_UNPAIRED 500  // Blink-Intervall wenn ungepaired (ms)
#define LED_BLINK_PAIRED 2000   // Blink-Intervall wenn gepaired (ms)

// ===== PAIRING-BUTTON =====

#define PAIRING_BTN_PIN 0     // GPIO Pin für Pairing-Button (oft BOOT button)

// ===== ESP-NOW KONFIGURATION =====

// MAC-Adresse des CYD (Empfänger)
// WICHTIG: Diese MAC-Adresse wird beim ersten Start des CYD angezeigt!
// Format: {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}
//
// So findest du die MAC-Adresse des CYD:
// 1. Lade ein Spiel-Sketch auf das CYD
// 2. Öffne den Serial Monitor (115200 Baud)
// 3. Suche nach "CYD MAC-Adresse:" in der Ausgabe
// 4. Trage die MAC-Adresse hier ein

uint8_t cydMacAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  // ← HIER ANPASSEN!

// ===== TIMING-KONFIGURATION =====

#define SEND_INTERVAL_MS 20     // Sende-Intervall (20ms = 50 Hz)
#define PAIRING_INTERVAL_MS 500 // Pairing-Paket alle 500ms wenn ungepaired

// ===== DEBUG-KONFIGURATION =====

#define DEBUG_SERIAL true       // true = Serial-Debug-Ausgabe aktivieren
#define SERIAL_BAUD 115200      // Serial Baudrate

#endif // CYD_ESPNOW_REMOTE_H
