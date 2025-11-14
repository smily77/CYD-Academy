# ESP-NOW Sender für CYD Arcade Games

Diese Beispiele zeigen, wie du einen drahtlosen Remote-Controller für das CYD Display mit ESP-NOW baust.

## 📡 Was ist ESP-NOW?

ESP-NOW ist ein drahtloses Kommunikationsprotokoll von Espressif, das:
- ✓ Schnelle, latenzarme Kommunikation ermöglicht
- ✓ Ohne WiFi-Router funktioniert (Peer-to-Peer)
- ✓ Geringe Stromaufnahme hat (ideal für batteriebetriebene Remotes)
- ✓ Einfach zu implementieren ist

## 🎮 Verfügbare Sender

| Sender | Beschreibung | Hardware |
|--------|--------------|----------|
| **ESP-NOW_Sender_Switch** | 4 digitale Buttons | 4x Buttons (Active-LOW) |
| **ESP-NOW_Sender_Encoder** | Rotary Encoder | 1x Rotary Encoder mit Button |
| **ESP-NOW_Sender_Poti** | 2 Potentiometer | 2x 10k Potis (analog) |

## 🚀 Quick Start

### 1. Hardware vorbereiten

**Minimum:**
- 1x ESP32 Development Board (für Sender)
- 1x CYD Display (Empfänger)
- Eingabe-Hardware (Buttons/Encoder/Potis je nach Sender)
- 1x LED (optional, für Status-Feedback)
- 1x Button (optional, für manuelles Pairing)

**Verkabelung Beispiel (ESP-NOW_Sender_Switch):**
```
Button A → GPIO 17 → GND
Button B → GPIO 5  → GND
Button C → GPIO 16 → GND
Button D → GPIO 18 → GND
Status LED → GPIO 2 (mit Vorwiderstand 220Ω)
Pairing Button → GPIO 0 (BOOT) → GND
```

### 2. CYD MAC-Adresse herausfinden

1. Lade ein Arcade-Spiel auf das CYD (z.B. `00_RetroGames`)
2. Öffne den Serial Monitor (115200 Baud)
3. Suche nach der Zeile: `CYD MAC-Adresse: XX:XX:XX:XX:XX:XX`
4. Notiere diese MAC-Adresse

### 3. Sender konfigurieren

1. Öffne `CYD_espNow_remote.h` im Editor
2. Wähle den **Sender-Typ** (auskommentiere die anderen):
   ```cpp
   #define SENDER_TYPE_SWITCH    // ← Für Buttons
   // #define SENDER_TYPE_ENCODER  // Für Encoder
   // #define SENDER_TYPE_POTI     // Für Potis
   ```

3. **Trage die CYD MAC-Adresse** ein:
   ```cpp
   uint8_t cydMacAddress[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};
   ```

4. **Passe Pin-Definitionen** an (falls nötig):
   ```cpp
   #define BTN_A_PIN 17    // Button A GPIO
   #define BTN_B_PIN 5     // Button B GPIO
   // ... etc
   ```

### 4. Sender hochladen

1. Öffne den gewünschten Sender-Sketch (z.B. `ESP-NOW_Sender_Switch.ino`)
2. Wähle das richtige Board: **ESP32 Dev Module**
3. Wähle den richtigen COM-Port
4. Klicke auf **Upload**

### 5. Pairing durchführen

**Automatisches Pairing:**
1. Schalte das CYD ein (Arcade-Spiel läuft)
2. Schalte den Sender ein
3. Warte 2-3 Sekunden
4. Die Status-LED blinkt langsamer = Verbindung hergestellt!

**Manuelles Pairing:**
1. Drücke den Pairing-Button (GPIO 0 / BOOT) am Sender
2. Die Status-LED blinkt langsamer = Verbindung hergestellt!

### 6. Spielen!

- Öffne ein Arcade-Spiel auf dem CYD
- Steuere mit dem Remote-Controller
- Bei Verbindungsverlust: Sender neu starten oder Pairing-Button drücken

## 🔧 Hardware-Details

### ESP-NOW_Sender_Switch

**Schaltplan:**
```
ESP32
GPIO 17 ──┤ Button A ├── GND
GPIO 5  ──┤ Button B ├── GND
GPIO 16 ──┤ Button C ├── GND
GPIO 18 ──┤ Button D ├── GND
GPIO 0  ──┤ Pairing  ├── GND
GPIO 2  ──┤ LED ├──[220Ω]── GND
```

**Features:**
- 4 unabhängige Buttons
- Pull-Up aktiviert (Active-LOW)
- Debouncing im Code
- Status-LED Feedback

### ESP-NOW_Sender_Encoder

**Schaltplan:**
```
Rotary Encoder
CLK → GPIO 17
DT  → GPIO 5
SW  → GPIO 16
GND → GND
VCC → 3.3V (falls benötigt)
```

**Features:**
- Rotations-Erkennung (links/rechts)
- Encoder-Button
- Edge-Detection im Code
- Delta-Reset nach erfolgreichem Senden

### ESP-NOW_Sender_Poti

**Schaltplan:**
```
Potentiometer Links (10k):
Pin 1 → 3.3V
Pin 2 → GPIO 34 (ADC)
Pin 3 → GND

Potentiometer Rechts (10k):
Pin 1 → 3.3V
Pin 2 → GPIO 35 (ADC)
Pin 3 → GND
```

**Features:**
- 2x Analog-Input (12-bit ADC)
- Automatisches Mapping auf 0-1000
- Smooth-Steuerung
- Low-Pass-Filter möglich (im Code anpassbar)

## 📊 Status-LED Verhalten

| Zustand | LED-Verhalten | Bedeutung |
|---------|---------------|-----------|
| **Ungepaired** | Schnelles Blinken (500ms) | Sucht CYD, sendet Pairing-Pakete |
| **Gepaired** | Langsames Blinken (2s) | Verbindung OK, Daten werden gesendet |
| **Fehler** | Sehr schnelles Blinken (100ms) | ESP-NOW Init fehlgeschlagen |

## 🔋 Batteriebetrieb

Für batteriebetriebene Remotes:

**Stromverbrauch (ca.):**
- ESP32 aktiv: ~80-120mA
- ESP32 mit DeepSleep: ~10µA (nicht implementiert)
- LED: ~20mA (kann deaktiviert werden)

**Batterie-Empfehlungen:**
- **LiPo 3.7V 500mAh**: ~4-6 Stunden Dauerbetrieb
- **LiPo 3.7V 1000mAh**: ~8-12 Stunden Dauerbetrieb
- **2x AA (3V)**: ~6-10 Stunden (mit LDO Regler)

**Power-Saving Tipps:**
1. LED deaktivieren (Zeile auskommentieren: `digitalWrite(STATUS_LED_PIN, ledState);`)
2. Send-Intervall erhöhen (z.B. 50ms statt 20ms)
3. DeepSleep zwischen Paketen (für Fortgeschrittene)

## 🐛 Troubleshooting

### Problem: Keine Verbindung

**Lösung:**
1. ✓ MAC-Adresse korrekt in `CYD_espNow_remote.h` eingetragen?
2. ✓ Sender-Typ richtig gewählt (`SENDER_TYPE_*`)?
3. ✓ CYD und Sender beide eingeschaltet?
4. ✓ Serial Monitor öffnen (115200 Baud) für Debug-Ausgabe
5. ✓ Pairing-Button manuell drücken

### Problem: Verbindung bricht ab

**Lösung:**
1. ✓ Zu große Entfernung? (ESP-NOW: max ~100m Freifeld, ~20m Indoor)
2. ✓ Metallische Hindernisse zwischen Sender und CYD?
3. ✓ Zu viele 2.4GHz WiFi-Netzwerke in der Nähe?
4. ✓ Sender-Batterie leer?

### Problem: Eingaben verzögert

**Lösung:**
1. ✓ `SEND_INTERVAL_MS` in `CYD_espNow_remote.h` verringern (z.B. von 20ms auf 10ms)
2. ✓ Timeout in `CYD_Input.h` erhöhen (von 500ms auf 1000ms)

### Problem: LED blinkt sehr schnell

**Lösung:**
- ESP-NOW Initialisierung fehlgeschlagen
- Prüfe Serial Monitor für Fehler-Meldung
- ESP32 Board korrekt gewählt in Arduino IDE?

## 🎯 Spiel-Kompatibilität

| Spiel | Switch | Encoder | Poti | Notizen |
|-------|--------|---------|------|---------|
| **Pong** | ✓ | ✓ | ✓ | Mit Encoder nur AI-Modus |
| **Snake** | ✓ | ✗ | ✗ | Braucht 4-Wege-Steuerung |
| **Breakout** | ✓ | ✓ | ✓ | - |
| **Space Invaders** | ✓ | ✓ | ✗ | - |
| **Asteroids** | ✓ | ✗ | ✗ | Braucht mehrere Tasten |
| **Frogger** | ✓ | ✓ | ✗ | - |
| **Tetris** | ✓ | ✓ | ✗ | Encoder ohne Schnellabsenkung |

## 📝 Eigene Sender erstellen

### Datenstruktur

Die `ESPNowInputData` Struktur ist in `CYD_Input.h` und allen Sendern identisch:

```cpp
struct ESPNowInputData {
  uint8_t packetType;     // 0=pairing, 1=button, 2=encoder, 3=poti
  uint8_t senderMAC[6];   // Sender MAC für Identifikation

  // Button-Daten
  bool buttonA, buttonB, buttonC, buttonD;

  // Encoder-Daten
  int8_t encoderDelta;    // -1, 0, +1
  bool encoderButton;

  // Poti-Daten
  uint16_t potiLeft;      // 0-1000
  uint16_t potiRight;     // 0-1000

  uint32_t timestamp;     // millis() vom Sender
};
```

### Beispiel: Hybrid-Sender (Buttons + Encoder)

Du kannst mehrere Eingabe-Typen kombinieren:

```cpp
// Lese Encoder
readEncoder();
inputData.encoderDelta = encoderDelta;
inputData.encoderButton = (digitalRead(ENC_SW_PIN) == LOW);

// Lese zusätzliche Buttons
inputData.buttonA = (digitalRead(BTN_EXTRA_PIN) == LOW);

// Sende als Encoder-Paket
inputData.packetType = ESPNOW_PACKET_ENCODER;
esp_now_send(cydMacAddress, (uint8_t*)&inputData, sizeof(inputData));
```

## 🔗 Weitere Informationen

- [ESP-NOW Dokumentation (Espressif)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html)
- [CYD_Games Library README](../../libraries/CYD_Games/README.md)
- [Arcade Games Übersicht](../../arcade-games/README.md)

## 💡 Projekt-Ideen

- **Wireless Arcade Controller**: 2-Spieler Setup mit je einem Sender
- **Portable Gaming Remote**: Mit Akku und 3D-gedrucktem Gehäuse
- **Joystick-Adapter**: Klassischen Arcade-Joystick drahtlos machen
- **Multi-Remote**: Sender mit Buttons + Encoder für maximale Kompatibilität

---

**Viel Spaß beim Bauen deines drahtlosen Controllers! 🎮**
