# 12_CYD_to_CYD_Communication - ESP-NOW zwischen zwei CYDs

## 🎓 Was lernst du hier?

Dieses Beispiel zeigt **Peer-to-Peer Kommunikation** zwischen zwei CYD Displays mit ESP-NOW - ohne Router, ohne WiFi-Netzwerk!

### Lernziele

- ✅ ESP-NOW Grundlagen verstehen
- ✅ Pairing-Mechanismus implementieren
- ✅ Touch-UI mit Netzwerk-Kommunikation kombinieren
- ✅ Bi-direktionale Kommunikation zwischen Geräten
- ✅ Latenz-Messung und Performance-Analyse
- ✅ Event-basierte Programmierung

## 🚀 Was macht das Beispiel?

### Funktionen

1. **Touch-Button auf Display** - Drücke Button um Signal zu senden
2. **Automatisches Pairing** - Zwei CYDs finden sich automatisch
3. **Visuelles Feedback** - Animations bei Empfang
4. **Zähler** - Zeigt gesendete & empfangene Nachrichten
5. **Latenz-Anzeige** - Misst Kommunikations-Geschwindigkeit

### So funktioniert's

```
CYD #1                          CYD #2
  │                               │
  ├─ Pairing-Broadcast ──────────>│
  │<─────────── Pairing-Antwort ──┤
  │         [PAIRED]          [PAIRED]
  │                               │
  ├─ Touch Button gedrückt        │
  ├─ Sende ESP-NOW Paket ─────────>│
  │                               ├─ Empfange Paket
  │                               ├─ Zeige Animation
  │                               └─ Update Counter
  │                               │
```

## 📦 Hardware

**Benötigt:**
- 2x CYD Display (2.8" oder 3.5")
- 2x USB-Kabel
- Keine externe Hardware!

## 🔧 Installation & Setup

### Schritt 1: Sketch hochladen

1. Öffne `12_CYD_to_CYD_Communication.ino` in Arduino IDE
2. Wähle Board: **ESP32 Dev Module**
3. Lade auf **ERSTES CYD** hoch
4. Lade auf **ZWEITES CYD** hoch (selber Sketch!)

### Schritt 2: Pairing

1. Beide CYDs einschalten
2. Warte 2-3 Sekunden
3. Status wechselt von "WAITING..." zu "PAIRED"
4. Fertig!

### Schritt 3: Testen

1. Drücke "SEND" Button auf CYD #1
2. CYD #2 zeigt gelben Flash und erhöht Counter
3. Drücke "SEND" Button auf CYD #2
4. CYD #1 zeigt gelben Flash und erhöht Counter

**Beide CYDs können senden UND empfangen!**

## 🎨 Display-Übersicht

```
╔════════════════════════════════════╗
║  CYD-to-CYD Demo                   ║
║  Status: PAIRED ✓                  ║
║                                    ║
║        ┌──────────────┐            ║
║        │              │            ║
║        │     SEND     │ ← Touch    ║
║        │              │            ║
║        └──────────────┘            ║
║                                    ║
║  Gesendet: 5                       ║
║  Empfangen: 3                      ║
║  Zuletzt: vor 2s                   ║
╚════════════════════════════════════╝
```

## 🔍 Code-Analyse: Was passiert?

### 1. ESP-NOW Initialisierung

```cpp
WiFi.mode(WIFI_STA);      // Station Mode ohne Router
WiFi.disconnect();        // Keine WiFi-Verbindung
esp_now_init();           // ESP-NOW starten
```

**Warum?**
- ESP-NOW funktioniert im WiFi Station Mode
- Braucht KEINE Verbindung zu einem Router
- Direkter Peer-to-Peer Link zwischen ESP32s

### 2. Pairing-Mechanismus

```cpp
// Broadcast an alle in Reichweite
uint8_t broadcastMAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
esp_now_send(broadcastMAC, data, len);
```

**Wie funktioniert's?**
1. Ungepairtes CYD sendet Broadcast alle 1s
2. Andere CYDs empfangen Broadcast
3. Speichern MAC-Adresse des Senders
4. Ab jetzt: Nur noch direkte Kommunikation

**Warum nicht feste MACs?**
- Flexibel: Funktioniert mit beliebigen CYDs
- Keine Konfiguration nötig
- "Plug & Play" Erlebnis

### 3. Datenstruktur

```cpp
struct Message {
  uint8_t type;         // 0=pairing, 1=button_press
  uint8_t senderMAC[6]; // Wer sendet?
  uint32_t counter;     // Nachricht #N
  uint32_t timestamp;   // Wann gesendet?
};
```

**Warum diese Felder?**
- `type`: Unterscheide Pairing vs. Daten
- `senderMAC`: Validiere Absender
- `counter`: Verlorene Pakete erkennen
- `timestamp`: Latenz messen

### 4. Callback-Funktion

```cpp
void onDataRecv(const esp_now_recv_info *recv_info, const uint8_t *data, int len) {
  // Wird automatisch aufgerufen bei Empfang!
  // recv_info enthält Absender-MAC: recv_info->src_addr
  Message msg;
  memcpy(&msg, data, sizeof(Message));

  if (msg.type == 0) {
    // Pairing-Anfrage
  } else if (msg.type == 1) {
    // Button-Press
    showReceiveAnimation();
  }
}
```

**Event-basiert:**
- Callback wird von ESP-NOW automatisch aufgerufen
- Kein Polling nötig
- Sehr effizient!

### 5. Touch-Handling

```cpp
bool touched = lcd.getTouch(&x, &y);

if (touched && !lastTouchState) {
  // Neuer Touch (Flanke)
  if (in_button_area(x, y)) {
    sendButtonPress();
  }
}
```

**Flanken-Erkennung:**
- Nur bei NEUEM Touch senden
- Verhindert mehrfaches Senden
- Bessere User Experience

## 📊 Performance

### Typische Werte

| Metrik | Wert |
|--------|------|
| **Latenz** | 10-20ms |
| **Reichweite** | ~20m Indoor, ~100m Outdoor |
| **Paket-Größe** | 19 Bytes |
| **Erfolgsrate** | >99% |

### Latenz messen

```cpp
// Sender
msg.timestamp = millis();

// Empfänger (in Callback)
uint32_t latenz = millis() - msg.timestamp;
Serial.printf("Latenz: %dms\n", latenz);
```

**Beobachtung:**
- Erste Nachricht: ~20ms (Peer-Discovery)
- Folgende: ~10ms (Direct Link)
- Sehr konsistent!

## 🎯 Experimente & Erweiterungen

### Experiment 1: Reichweite testen

**Frage:** Wie weit können die CYDs auseinander sein?

**Vorgehen:**
1. Starte mit CYDs nebeneinander
2. Bewege eins langsam weg
3. Drücke Button alle paar Meter
4. Notiere: Wann bricht Verbindung ab?

**Erwartung:**
- Indoor (Wohnung): 15-25m
- Outdoor (Freifeld): 80-120m
- Durch Wände: Reduziert um ~50%

### Experiment 2: Mehrere CYDs

**Challenge:** Füge ein drittes CYD hinzu!

**Hints:**
- Pairing-Logic erweitern (Array von Peers)
- Broadcasts an alle gepairten senden
- Group-Chat Logik

### Experiment 3: Daten-Typen

**Idee:** Sende verschiedene Nachrichten!

```cpp
// Erweitere Message struct
struct Message {
  uint8_t type;  // 0=pair, 1=button, 2=slider, 3=color
  uint32_t value;  // Wert je nach Type
  // ...
};
```

**Möglichkeiten:**
- Slider-Wert (0-100)
- Farb-Auswahl (RGB)
- Text-Nachricht
- Sensor-Daten

### Experiment 4: Visualisierung

**Idee:** Zeige Empfang visuell schöner!

**Umsetzung:**
- Wellenmuster vom Button zum Rand
- Partikel-Effekt bei Empfang
- Farbverlauf basierend auf Latenz
- Trail-Effekt für letzte N Nachrichten

### Experiment 5: IoT Dashboard

**Project:** Ein CYD als "Hub", mehrere als Sensoren!

**Architektur:**
```
Sensor-CYD #1 ─────┐
Sensor-CYD #2 ─────┼──> Hub-CYD (zeigt alle Werte)
Sensor-CYD #3 ─────┘
```

**Anwendungen:**
- Home Automation (Temp, Humidity, etc.)
- Multi-Room Monitoring
- Verteilte Sensor-Netzwerke

## 🐛 Troubleshooting

### Problem: Kein Pairing

**Symptom:** Status bleibt "WAITING..."

**Lösungen:**
1. ✓ Beide CYDs eingeschaltet?
2. ✓ Zu weit auseinander? (Maximal 10m für Pairing)
3. ✓ Serial Monitor öffnen - siehst du "Pairing-Broadcast"?
4. ✓ Reset beide CYDs gleichzeitig

### Problem: Nachrichten kommen nicht an

**Symptom:** Counter erhöht sich nicht

**Lösungen:**
1. ✓ Pairing erfolgreich? (Status = PAIRED?)
2. ✓ Zu viele 2.4GHz WiFi-Netzwerke in der Nähe?
3. ✓ Serial Monitor: Siehst du "Button gesendet: ... Status=OK"?
4. ✓ Metallische Hindernisse zwischen CYDs?

### Problem: Hohe Latenz (>50ms)

**Ursachen:**
- WiFi-Interferenz
- Zu viele ESP-NOW Pakete gleichzeitig
- Physikalische Hindernisse

**Lösungen:**
- Sende-Rate reduzieren
- Näher zusammen bringen
- Andere WiFi-Kanal nutzen

## 💡 Code-Verbesserungen

### Von "gut" zu "besser"

**Aktuell:**
```cpp
// Einfach aber funktional
if (msg.type == 1) {
  receiveCounter++;
}
```

**Besser:**
```cpp
// Mit Fehlerbehandlung
if (msg.type == 1) {
  // Prüfe auf verlorene Pakete
  if (msg.counter != lastCounter + 1) {
    Serial.printf("Paket verloren! Erwartet: %d, Bekommen: %d\n",
                  lastCounter + 1, msg.counter);
  }
  lastCounter = msg.counter;
  receiveCounter++;
}
```

**Warum besser?**
- Erkennt Packet-Loss
- Hilft bei Debugging
- Professionellerer Ansatz

### Von "blocking" zu "non-blocking"

**Aktuell:**
```cpp
// Blockiert für 100ms!
delay(100);
```

**Besser:**
```cpp
// Nicht-blockierend
static unsigned long lastUpdate = 0;
if (millis() - lastUpdate > 100) {
  updateStatus();
  lastUpdate = millis();
}
```

**Warum besser?**
- Touch-Input bleibt responsive
- Keine "Freezes"
- Besser für Real-Time Anwendungen

## 📚 Weiterführendes

### ESP-NOW vs. WiFi

| Feature | ESP-NOW | WiFi |
|---------|---------|------|
| **Setup** | Einfach | Komplex |
| **Router nötig?** | Nein | Ja |
| **Latenz** | ~10ms | ~50-100ms |
| **Reichweite** | ~100m | ~50m |
| **Stromverbrauch** | Niedrig | Höher |
| **Internet?** | Nein | Ja |

**Wann ESP-NOW nutzen?**
- Schnelle lokale Kommunikation
- Batterie-betriebene Geräte
- Keine Internet-Verbindung verfügbar
- Einfaches Setup gewünscht

**Wann WiFi nutzen?**
- Internet-Zugriff nötig
- Cloud-Services
- Große Datenmengen
- Lange Distanzen (mit Repeatern)

### Nächste Schritte

1. **Studiere CYD_Input.h** - Siehe wie ESP-NOW dort verwendet wird
2. **Arcade-Spiele** - ESP-NOW Remote für Multiplayer?
3. **ESP-NOW + WiFi** - Kombiniere beides für IoT-Gateway
4. **Mesh-Netzwerk** - ESP-NOW für Multi-Hop Kommunikation

## 🔗 Relevante Ressourcen

- [ESP-NOW Dokumentation (Espressif)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html)
- [CYD_Input.h](../../libraries/CYD_Games/src/CYD_Input.h) - Professionelle ESP-NOW Implementierung
- [ESP-NOW_Sender/](../ESP-NOW_Sender/) - Remote Controller als Referenz
- [Arcade Games](../../arcade-games/) - Sehe wie Input-Handling in echten Projekten funktioniert

## ✨ Fazit

Du hast jetzt die Grundlagen von ESP-NOW verstanden und kannst:
- ✅ Peer-to-Peer Verbindungen aufbauen
- ✅ Daten zwischen ESPs austauschen
- ✅ Touch-UI mit Netzwerk kombinieren
- ✅ Pairing-Mechanismen implementieren

**Nächster Schritt:** Baue dein eigenes Multi-CYD Projekt! 🚀

---

**Viel Spaß beim Experimentieren!**
