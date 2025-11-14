/*
  ESP-NOW Sender - Potentiometer (2x Analog Potis)

  Sendet 2 Potentiometer-Werte über ESP-NOW an das CYD Display.

  Hardware:
  - ESP32 Board
  - 2x Potentiometer (10k Ohm, an ADC-Pins)
  - 1x Status-LED
  - 1x Pairing-Button (optional)

  Konfiguration:
  - Passe CYD_espNow_remote.h an deine Hardware an
  - Setze die MAC-Adresse des CYD in CYD_espNow_remote.h
*/

#include <esp_now.h>
#include <WiFi.h>
#include "../CYD_espNow_remote.h"

// Prüfe ob richtiger Sender-Typ konfiguriert ist
#ifndef SENDER_TYPE_POTI
  #error "FEHLER: SENDER_TYPE_POTI muss in CYD_espNow_remote.h definiert sein!"
#endif

// ===== ESP-NOW DATENSTRUKTUR =====

#define ESPNOW_PACKET_PAIRING 0
#define ESPNOW_PACKET_BUTTON  1
#define ESPNOW_PACKET_ENCODER 2
#define ESPNOW_PACKET_POTI    3

struct ESPNowInputData {
  uint8_t packetType;
  uint8_t senderMAC[6];

  bool buttonA;
  bool buttonB;
  bool buttonC;
  bool buttonD;

  int8_t encoderDelta;
  bool encoderButton;

  uint16_t potiLeft;
  uint16_t potiRight;

  uint32_t timestamp;
};

// ===== GLOBALE VARIABLEN =====

ESPNowInputData inputData;
esp_now_peer_info_t peerInfo;
bool isPaired = false;
unsigned long lastSendTime = 0;
unsigned long lastPairingTime = 0;
unsigned long lastLedBlink = 0;
bool ledState = false;

// ===== SETUP =====

void setup() {
  #if DEBUG_SERIAL
    Serial.begin(SERIAL_BAUD);
    delay(1000);
    Serial.println("\n=== ESP-NOW Sender - Potentiometer ===");
  #endif

  // ADC Pins sind automatisch als Input konfiguriert

  // Pairing-Button initialisieren
  pinMode(PAIRING_BTN_PIN, INPUT_PULLUP);

  // Status-LED initialisieren
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  #if DEBUG_SERIAL
    Serial.println("Potentiometer initialisiert:");
    Serial.printf("  Poti Left:  GPIO %d (ADC)\n", POTI_LEFT_PIN);
    Serial.printf("  Poti Right: GPIO %d (ADC)\n", POTI_RIGHT_PIN);
    Serial.printf("  Pairing:    GPIO %d\n", PAIRING_BTN_PIN);
    Serial.printf("  LED:        GPIO %d\n", STATUS_LED_PIN);
  #endif

  // WiFi in Station-Mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Eigene MAC-Adresse ausgeben
  uint8_t senderMAC[6];
  WiFi.macAddress(senderMAC);

  #if DEBUG_SERIAL
    Serial.print("Sender MAC: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", senderMAC[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.println();

    Serial.print("CYD MAC:    ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", cydMacAddress[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
  #endif

  // ESP-NOW initialisieren
  if (esp_now_init() != ESP_OK) {
    #if DEBUG_SERIAL
      Serial.println("FEHLER: ESP-NOW Init fehlgeschlagen!");
    #endif
    while (1) {
      digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
      delay(100);
    }
  }

  // CYD als Peer hinzufügen
  memcpy(peerInfo.peer_addr, cydMacAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    #if DEBUG_SERIAL
      Serial.println("FEHLER: Peer konnte nicht hinzugefügt werden!");
    #endif
    while (1) {
      digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
      delay(100);
    }
  }

  #if DEBUG_SERIAL
    Serial.println("CYD als Peer hinzugefügt");
    Serial.println("Starte Pairing-Prozess...");
  #endif

  // Datenstruktur initialisieren
  memset(&inputData, 0, sizeof(inputData));
  memcpy(inputData.senderMAC, senderMAC, 6);
}

// ===== LOOP =====

void loop() {
  unsigned long currentTime = millis();

  // Pairing-Button prüfen
  if (digitalRead(PAIRING_BTN_PIN) == LOW) {
    delay(50);
    if (digitalRead(PAIRING_BTN_PIN) == LOW) {
      sendPairingPacket();
      isPaired = true;
      #if DEBUG_SERIAL
        Serial.println("Manuelles Pairing ausgelöst");
      #endif
      while (digitalRead(PAIRING_BTN_PIN) == LOW);
    }
  }

  // Status-LED blinken
  updateStatusLED(currentTime);

  // Wenn ungepaired: Pairing-Pakete senden
  if (!isPaired && currentTime - lastPairingTime >= PAIRING_INTERVAL_MS) {
    sendPairingPacket();
    lastPairingTime = currentTime;
  }

  // Daten senden (wenn gepaired)
  if (isPaired && currentTime - lastSendTime >= SEND_INTERVAL_MS) {
    readPotis();
    sendData();
    lastSendTime = currentTime;
  }

  delay(1);
}

// ===== FUNKTIONEN =====

void readPotis() {
  // ESP32 ADC liefert 0-4095 (12-bit)
  // Wir mappen auf 0-1000 für Kompatibilität mit CYD_Input.h
  int rawLeft = analogRead(POTI_LEFT_PIN);
  int rawRight = analogRead(POTI_RIGHT_PIN);

  inputData.potiLeft = map(rawLeft, 0, 4095, 0, 1000);
  inputData.potiRight = map(rawRight, 0, 4095, 0, 1000);

  // Constrain auf gültigen Bereich
  inputData.potiLeft = constrain(inputData.potiLeft, 0, 1000);
  inputData.potiRight = constrain(inputData.potiRight, 0, 1000);

  inputData.timestamp = millis();
}

void sendPairingPacket() {
  inputData.packetType = ESPNOW_PACKET_PAIRING;

  esp_err_t result = esp_now_send(cydMacAddress, (uint8_t*)&inputData, sizeof(inputData));

  #if DEBUG_SERIAL
    if (result == ESP_OK) {
      Serial.println("Pairing-Paket gesendet");
    } else {
      Serial.printf("Fehler beim Senden: %d\n", result);
    }
  #endif
}

void sendData() {
  inputData.packetType = ESPNOW_PACKET_POTI;

  esp_err_t result = esp_now_send(cydMacAddress, (uint8_t*)&inputData, sizeof(inputData));

  #if DEBUG_SERIAL
    static unsigned long lastDebugPrint = 0;
    if (millis() - lastDebugPrint >= 1000) {  // Debug alle 1s
      Serial.printf("Potis: Left=%4d Right=%4d | Status: %s\n",
                    inputData.potiLeft, inputData.potiRight,
                    (result == ESP_OK) ? "OK" : "FEHLER");
      lastDebugPrint = millis();
    }
  #endif

  if (result == ESP_OK) {
    isPaired = true;
  }
}

void updateStatusLED(unsigned long currentTime) {
  unsigned long blinkInterval = isPaired ? LED_BLINK_PAIRED : LED_BLINK_UNPAIRED;

  if (currentTime - lastLedBlink >= blinkInterval) {
    ledState = !ledState;
    digitalWrite(STATUS_LED_PIN, ledState);
    lastLedBlink = currentTime;
  }
}
