/*
  12_CYD_to_CYD_Communication - ESP-NOW Kommunikation zwischen zwei CYDs

  Dieses Beispiel zeigt ESP-NOW Kommunikation zwischen zwei CYD Displays.

  Funktionen:
  - Touch-Button auf Display (sendet Signal)
  - Empfang und Anzeige von Signalen
  - Automatisches Pairing
  - Visuelles Feedback (LED-Status, Counter)
  - Bi-direktionale Kommunikation

  Lernziele:
  - ESP-NOW Grundlagen verstehen
  - Pairing-Mechanismus implementieren
  - Touch-UI mit ESP-NOW kombinieren
  - Peer-to-Peer Kommunikation zwischen ESPs

  Hardware:
  - 2x CYD Display (2.8" oder 3.5")
  - Keine externe Hardware nötig!

  Anleitung:
  1. Lade diesen Sketch auf BEIDE CYDs
  2. Öffne Serial Monitor (115200) auf beiden
  3. Notiere die MAC-Adressen
  4. Beide CYDs pairen sich automatisch
  5. Drücke Touch-Button auf einem CYD
  6. Der andere CYD zeigt die Nachricht an

  WICHTIG:
  - Dieses Beispiel nutzt NICHT CYD_Input.h (das ist für Game-Input)
  - Stattdessen: Direkter ESP-NOW Code für maximale Lernmöglichkeit
*/

#include <esp_now.h>
#include <WiFi.h>
#include <CYD_Display_Config.h>

// ===== GLOBALE VARIABLEN =====

LGFX lcd;

// ESP-NOW Datenstruktur
struct Message {
  uint8_t type;           // 0=pairing, 1=button_press
  uint8_t senderMAC[6];   // Sender MAC
  uint32_t counter;       // Zähler für gesendete Nachrichten
  uint32_t timestamp;     // millis() vom Sender
};

// State
Message lastMessage;
bool isPaired = false;
uint8_t peerMAC[6] = {0};
uint32_t sendCounter = 0;
uint32_t receiveCounter = 0;
unsigned long lastSendTime = 0;
unsigned long lastReceiveTime = 0;

// UI State
bool buttonPressed = false;
bool lastTouchState = false;
bool needsUIRedraw = false;  // Flag für UI-Update aus Callback

// Connection-Timeout (10 Sekunden ohne Nachricht → Verbindung verloren)
const unsigned long CONNECTION_TIMEOUT = 10000;
unsigned long lastPeerActivity = 0;

// Farben
#define COLOR_BG 0x0000           // Schwarz
#define COLOR_BUTTON 0x001F       // Blau
#define COLOR_BUTTON_PRESSED 0xF800  // Rot
#define COLOR_TEXT 0xFFFF         // Weiß
#define COLOR_UNPAIRED 0xF800     // Rot
#define COLOR_PAIRED 0x07E0       // Grün
#define COLOR_RECEIVE 0xFFE0      // Gelb

// Button Position
#define BUTTON_X 60
#define BUTTON_Y 100
#define BUTTON_W 200
#define BUTTON_H 80

// ===== ESP-NOW CALLBACK =====

void onDataRecv(const esp_now_recv_info *recv_info, const uint8_t *data, int len) {
  if (len != sizeof(Message)) return;

  Message msg;
  memcpy(&msg, data, sizeof(Message));

  // Pairing-Nachricht (immer akzeptieren für Re-Pairing nach Absturz!)
  if (msg.type == 0) {
    bool isNewPeer = false;

    // Prüfe ob es ein neuer Peer ist
    if (!isPaired) {
      isNewPeer = true;
    } else {
      // Prüfe ob MAC anders ist (Re-Pairing mit neuem Gerät)
      for (int i = 0; i < 6; i++) {
        if (msg.senderMAC[i] != peerMAC[i]) {
          isNewPeer = true;
          break;
        }
      }
    }

    // Aktualisiere Peer-Daten
    memcpy(peerMAC, msg.senderMAC, 6);
    lastPeerActivity = millis();

    if (isNewPeer) {
      bool wasAlreadyPaired = isPaired;
      isPaired = true;

      Serial.print(wasAlreadyPaired ? "Re-" : "");
      Serial.print("Gepaired mit: ");
      for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", peerMAC[i]);
        if (i < 5) Serial.print(":");
      }
      Serial.println();

      // UI-Redraw triggern (nicht direkt aus Callback!)
      needsUIRedraw = true;
    }

    // Immer Pairing-Antwort senden (auch bei Re-Pairing)
    Message response;
    response.type = 0;
    WiFi.macAddress(response.senderMAC);
    response.counter = 0;
    response.timestamp = millis();

    // Peer hinzufügen/aktualisieren
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, peerMAC, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    // Entferne alten Peer falls vorhanden und füge neu hinzu
    if (esp_now_is_peer_exist(peerMAC)) {
      esp_now_del_peer(peerMAC);
    }
    esp_now_add_peer(&peerInfo);

    esp_now_send(peerMAC, (uint8_t*)&response, sizeof(response));

    return;
  }

  // Button-Press Nachricht
  if (msg.type == 1 && isPaired) {
    // Prüfe ob von gepairtem Peer
    bool valid = true;
    for (int i = 0; i < 6; i++) {
      if (msg.senderMAC[i] != peerMAC[i]) {
        valid = false;
        break;
      }
    }

    if (valid) {
      receiveCounter++;
      lastReceiveTime = millis();
      lastPeerActivity = millis();  // Connection-Timeout zurücksetzen
      memcpy(&lastMessage, &msg, sizeof(Message));

      Serial.printf("Empfangen: Counter=%d, Latenz=%dms\n",
                    msg.counter, millis() - msg.timestamp);

      // Visuelles Feedback
      showReceiveAnimation();
    }
  }
}

// ===== SETUP =====

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== CYD-to-CYD Communication ===");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);  // Landscape
  lcd.setBrightness(255);
  lcd.fillScreen(COLOR_BG);

  // Eigene MAC ausgeben
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  uint8_t myMAC[6];
  WiFi.macAddress(myMAC);

  Serial.print("Meine MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", myMAC[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.println();

  // ESP-NOW initialisieren
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init fehlgeschlagen!");
    lcd.setTextColor(COLOR_UNPAIRED);
    lcd.drawString("ESP-NOW FEHLER!", lcd.width()/2, lcd.height()/2);
    while(1);
  }

  // Callback registrieren
  esp_now_register_recv_cb(onDataRecv);

  Serial.println("Warte auf Pairing...");
  Serial.println("(Starte zweites CYD zum Pairen)");

  // Initialen Status anzeigen
  drawUI();

  // Sende initiales Pairing-Paket
  sendPairingBroadcast();
}

// ===== LOOP =====

void loop() {
  // UI-Redraw wenn von Callback getriggert
  if (needsUIRedraw) {
    drawUI();
    needsUIRedraw = false;
  }

  // Connection-Timeout prüfen (10s ohne Nachricht → Verbindung verloren)
  if (isPaired && lastPeerActivity > 0 &&
      millis() - lastPeerActivity > CONNECTION_TIMEOUT) {
    Serial.println("Connection-Timeout! Peer verloren. Re-Pairing möglich.");
    isPaired = false;
    lastPeerActivity = 0;

    // Peer entfernen
    if (esp_now_is_peer_exist(peerMAC)) {
      esp_now_del_peer(peerMAC);
    }

    needsUIRedraw = true;  // UI auf "WAITING" setzen
  }

  // Touch lesen
  uint16_t x, y;
  bool touched = lcd.getTouch(&x, &y);

  // Pairing-Broadcasts senden wenn ungepaired
  if (!isPaired && millis() - lastSendTime > 1000) {
    sendPairingBroadcast();
    lastSendTime = millis();
  }

  // Button-Press erkennen
  if (touched && !lastTouchState) {
    // Neuer Touch
    if (x >= BUTTON_X && x <= BUTTON_X + BUTTON_W &&
        y >= BUTTON_Y && y <= BUTTON_Y + BUTTON_H) {

      // Button gedrückt!
      buttonPressed = true;
      drawButton(true);  // Als gedrückt zeichnen

      if (isPaired) {
        sendButtonPress();
      }
    }
  }

  if (!touched && lastTouchState && buttonPressed) {
    // Button losgelassen
    buttonPressed = false;
    drawButton(false);
  }

  lastTouchState = touched;

  // Status-Update alle 100ms
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 100) {
    updateStatus();
    lastUpdate = millis();
  }

  delay(10);
}

// ===== FUNKTIONEN =====

void sendPairingBroadcast() {
  Message msg;
  msg.type = 0;  // Pairing
  WiFi.macAddress(msg.senderMAC);
  msg.counter = 0;
  msg.timestamp = millis();

  // Broadcast an alle
  uint8_t broadcastMAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Prüfe ob Peer bereits existiert
  if (!esp_now_is_peer_exist(broadcastMAC)) {
    esp_now_add_peer(&peerInfo);
  }

  esp_now_send(broadcastMAC, (uint8_t*)&msg, sizeof(msg));

  Serial.println("Pairing-Broadcast gesendet");
}

void sendButtonPress() {
  if (!isPaired) return;

  Message msg;
  msg.type = 1;  // Button Press
  WiFi.macAddress(msg.senderMAC);
  msg.counter = ++sendCounter;
  msg.timestamp = millis();

  // Füge Peer hinzu falls noch nicht vorhanden
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (!esp_now_is_peer_exist(peerMAC)) {
    esp_now_add_peer(&peerInfo);
  }

  esp_err_t result = esp_now_send(peerMAC, (uint8_t*)&msg, sizeof(msg));

  Serial.printf("Button gesendet: Counter=%d, Status=%s\n",
                sendCounter, (result == ESP_OK) ? "OK" : "FEHLER");

  lastSendTime = millis();
}

void drawUI() {
  lcd.fillScreen(COLOR_BG);

  // Titel
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(10, 10);
  lcd.println("CYD-to-CYD Demo");

  // Pairing-Status
  lcd.setTextSize(1);
  lcd.setCursor(10, 35);
  if (isPaired) {
    lcd.setTextColor(COLOR_PAIRED);
    lcd.print("Status: PAIRED");
  } else {
    lcd.setTextColor(COLOR_UNPAIRED);
    lcd.print("Status: WAITING...");
  }

  // Button zeichnen
  drawButton(false);

  // Zähler
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(10, 200);
  lcd.printf("Gesendet: %d", sendCounter);
  lcd.setCursor(10, 220);
  lcd.printf("Empfangen: %d", receiveCounter);
}

void drawButton(bool pressed) {
  uint32_t color = pressed ? COLOR_BUTTON_PRESSED : COLOR_BUTTON;

  lcd.fillRoundRect(BUTTON_X, BUTTON_Y, BUTTON_W, BUTTON_H, 10, color);
  lcd.drawRoundRect(BUTTON_X, BUTTON_Y, BUTTON_W, BUTTON_H, 10, COLOR_TEXT);

  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(BUTTON_X + 30, BUTTON_Y + 30);
  lcd.println("SEND");
}

void updateStatus() {
  // Zähler aktualisieren
  lcd.fillRect(10, 200, 200, 40, COLOR_BG);
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(10, 200);
  lcd.printf("Gesendet: %d", sendCounter);
  lcd.setCursor(10, 220);
  lcd.printf("Empfangen: %d", receiveCounter);

  // Letzter Empfang
  if (receiveCounter > 0 && millis() - lastReceiveTime < 5000) {
    lcd.setCursor(10, 240);
    lcd.setTextColor(COLOR_RECEIVE);
    lcd.printf("Zuletzt: vor %ds", (millis() - lastReceiveTime) / 1000);
  }
}

void showReceiveAnimation() {
  // Kurzer Flash-Effekt
  lcd.fillRect(BUTTON_X - 10, BUTTON_Y - 10,
               BUTTON_W + 20, BUTTON_H + 20, COLOR_RECEIVE);
  delay(100);
  drawButton(false);
  updateStatus();
}
