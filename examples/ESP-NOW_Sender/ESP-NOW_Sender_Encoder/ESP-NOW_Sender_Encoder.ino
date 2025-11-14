/*
  ESP-NOW Sender - Encoder (Rotary Encoder mit Button)

  Sendet Rotary Encoder Daten über ESP-NOW an das CYD Display.

  Hardware:
  - ESP32 Board
  - 1x Rotary Encoder (mit A, B, SW Pins)
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
#ifndef SENDER_TYPE_ENCODER
  #error "FEHLER: SENDER_TYPE_ENCODER muss in CYD_espNow_remote.h definiert sein!"
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

// Encoder State
int lastEncA = HIGH;
int lastEncB = HIGH;
int8_t encoderDelta = 0;

// ===== SETUP =====

void setup() {
  #if DEBUG_SERIAL
    Serial.begin(SERIAL_BAUD);
    delay(1000);
    Serial.println("\n=== ESP-NOW Sender - Encoder ===");
  #endif

  // Encoder-Pins initialisieren
  pinMode(ENC_A_PIN, INPUT_PULLUP);
  pinMode(ENC_B_PIN, INPUT_PULLUP);
  pinMode(ENC_SW_PIN, INPUT_PULLUP);

  // Initialen Encoder-State lesen
  lastEncA = digitalRead(ENC_A_PIN);
  lastEncB = digitalRead(ENC_B_PIN);

  // Pairing-Button initialisieren
  pinMode(PAIRING_BTN_PIN, INPUT_PULLUP);

  // Status-LED initialisieren
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  #if DEBUG_SERIAL
    Serial.println("Encoder initialisiert:");
    Serial.printf("  Enc A:    GPIO %d\n", ENC_A_PIN);
    Serial.printf("  Enc B:    GPIO %d\n", ENC_B_PIN);
    Serial.printf("  Enc SW:   GPIO %d\n", ENC_SW_PIN);
    Serial.printf("  Pairing:  GPIO %d\n", PAIRING_BTN_PIN);
    Serial.printf("  LED:      GPIO %d\n", STATUS_LED_PIN);
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

  // Encoder lesen
  readEncoder();

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
    readEncoderButton();
    sendData();
    lastSendTime = currentTime;
  }

  delay(1);
}

// ===== FUNKTIONEN =====

void readEncoder() {
  int currentA = digitalRead(ENC_A_PIN);
  int currentB = digitalRead(ENC_B_PIN);

  encoderDelta = 0;

  // Prüfe auf Flanke von A
  if (currentA != lastEncA) {
    // Bei fallender Flanke von A
    if (currentA == LOW) {
      // Wenn B auch LOW ist, dann Drehung im Uhrzeigersinn (rechts)
      if (currentB == LOW) {
        encoderDelta = 1;
      } else {
        // Sonst gegen Uhrzeigersinn (links)
        encoderDelta = -1;
      }
    }
  }

  lastEncA = currentA;
  lastEncB = currentB;
}

void readEncoderButton() {
  inputData.encoderButton = (digitalRead(ENC_SW_PIN) == LOW);
  inputData.encoderDelta = encoderDelta;
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
  inputData.packetType = ESPNOW_PACKET_ENCODER;

  esp_err_t result = esp_now_send(cydMacAddress, (uint8_t*)&inputData, sizeof(inputData));

  #if DEBUG_SERIAL
    static unsigned long lastDebugPrint = 0;
    // Debug nur bei Änderung oder alle 5s
    if (encoderDelta != 0 || inputData.encoderButton || (millis() - lastDebugPrint >= 5000)) {
      Serial.printf("Encoder: Delta=%d Button=%d | Status: %s\n",
                    encoderDelta, inputData.encoderButton,
                    (result == ESP_OK) ? "OK" : "FEHLER");
      lastDebugPrint = millis();
    }
  #endif

  if (result == ESP_OK) {
    isPaired = true;
    // Delta zurücksetzen nach erfolgreichem Senden
    encoderDelta = 0;
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
