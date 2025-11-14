# 🖼️ Beispiel 15: Image Viewer - Bilder vom Internet laden

## 📝 Was macht dieses Beispiel?

Lädt **JPEG-Bilder von URLs** herunter und zeigt sie auf dem Display an - mit automatischer Skalierung!

**Features:**
- ✅ **WiFi-Verbindung** automatisch herstellen
- ✅ **HTTP GET Requests** für Bild-Downloads
- ✅ **JPEG-Dekodierung** mit TJpg_Decoder
- ✅ **Auto-Skalierung** auf Display-Größe
- ✅ **Progress Bar** während Download
- ✅ **Touch-Navigation** (Links/Rechts/Reload)
- ✅ **Error Handling** (404, Timeout, OOM)
- ✅ **Performance-Metriken** (Download-/Decode-Zeit)

**Steuerung:**
- **Touch Links (<80px):** Vorheriges Bild
- **Touch Rechts (>160px):** Nächstes Bild
- **Touch Mitte:** Aktuelles Bild neu laden

---

## 🎯 Lernziele

1. **WiFi-Programmierung** - ESP32 mit WiFi verbinden
2. **HTTP-Requests** - GET-Requests mit HTTPClient
3. **Streaming** - Download während Anzeige
4. **JPEG-Dekodierung** - TJpg_Decoder verwenden
5. **Auto-Skalierung** - Bilder auf Display anpassen
6. **Asynchrone UI** - Loading States & Progress
7. **Memory Management** - RAM-Limits beachten
8. **Error Handling** - Robuste Netzwerk-Programmierung

---

## ⚙️ Installation & Setup

### 1. Library installieren

**Arduino IDE:**
1. Library Manager öffnen (Strg+Shift+I)
2. "TJpg_Decoder" suchen
3. Von **Bodmer** installieren

**PlatformIO:**
```ini
[env:esp32]
lib_deps =
  lovyan03/LovyanGFX @ ^1.1.0
  bodmer/TJpg_Decoder @ ^1.0.0
```

### 2. WiFi konfigurieren

**Öffne `WiFi_Config.h` und trage deine Daten ein:**

```cpp
const char* WIFI_SSID = "Dein_WiFi_Name";
const char* WIFI_PASSWORD = "Dein_Passwort";
```

**WICHTIG:** Wenn du deinen Code veröffentlichst, füge `WiFi_Config.h` zu `.gitignore` hinzu!

### 3. Kompilieren & Upload

```
Arduino IDE: Upload (Strg+U)
PlatformIO: pio run -t upload
```

### 4. Testen

1. Warte auf WiFi-Verbindung (~5 Sekunden)
2. Erstes Bild wird geladen
3. Touch Links/Rechts zum Blättern

---

## 🔍 Technische Konzepte

### 1. WiFi-Verbindung (Station Mode)

```cpp
void connectWiFi() {
  WiFi.mode(WIFI_STA);  // Station Mode (Client)
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    // Timeout prüfen
    if (millis() - startTime > WIFI_CONNECT_TIMEOUT) {
      return;  // Abbrechen nach 15s
    }
    delay(500);
  }

  // Erfolgreich verbunden!
  Serial.println(WiFi.localIP());
}
```

**Modi:**
- `WIFI_STA`: Station (Client) - verbindet sich mit Router
- `WIFI_AP`: Access Point - CYD wird selbst zum Router
- `WIFI_AP_STA`: Beide gleichzeitig

**Status prüfen:**
```cpp
WiFi.status() == WL_CONNECTED  // Verbunden?
WiFi.RSSI()                    // Signal-Stärke (-30 = perfekt, -90 = schlecht)
WiFi.localIP()                 // IP-Adresse (z.B. 192.168.1.42)
```

---

### 2. HTTP GET Request

```cpp
HTTPClient http;

// URL setzen
http.begin("https://example.com/image.jpg");

// Timeout setzen (wichtig!)
http.setTimeout(10000);  // 10 Sekunden

// Request senden
int httpCode = http.GET();

if (httpCode == HTTP_CODE_OK) {  // 200
  // Erfolg!
  int size = http.getSize();      // Content-Length
  WiFiClient* stream = http.getStreamPtr();  // Stream für Download

  // Daten lesen...
}

http.end();  // Verbindung schließen
```

**HTTP Status Codes:**
- `200 OK`: Erfolg
- `404 Not Found`: Bild existiert nicht
- `500 Internal Server Error`: Server-Fehler
- `< 0`: Verbindungsfehler (Timeout, DNS-Fehler)

---

### 3. Streaming Download

**Problem:** Große Bilder (500 KB) passen nicht in RAM!

**Lösung:** Chunk-weise herunterladen!

```cpp
WiFiClient* stream = http.getStreamPtr();

const int BUFFER_SIZE = 2048;  // 2 KB pro Chunk
uint8_t buffer[BUFFER_SIZE];

int bytesRead;
while ((bytesRead = stream->readBytes(buffer, BUFFER_SIZE)) > 0) {
  // Verarbeite buffer (bytesRead Bytes)
  downloadedBytes += bytesRead;

  // Progress berechnen
  int progress = (downloadedBytes * 100) / totalBytes;
  drawProgressBar(progress);
}
```

**Warum Streaming?**

```
Ohne Streaming:  500 KB Bild → 500 KB RAM benötigt → ESP32 crasht!
Mit Streaming:   500 KB Bild → 2 KB RAM benötigt → Funktioniert!
```

---

### 4. JPEG-Dekodierung

**TJpg_Decoder** dekodiert JPEG → RGB565 Pixel.

#### Setup

```cpp
#include <TJpg_Decoder.h>

// Callback-Funktion (wird für jedes Pixel-Tile aufgerufen)
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  lcd.pushImage(x, y, w, h, bitmap);  // Auf Display zeichnen
  return 1;  // true = weitermachen
}

void setup() {
  TJpgDec.setJpgScale(1);         // 1, 2, 4, 8 (Skalierung)
  TJpgDec.setSwapBytes(true);     // Für ILI9341
  TJpgDec.setCallback(tft_output); // Callback registrieren
}
```

#### Dekodieren

```cpp
uint8_t* jpegData;  // JPEG-Daten im RAM
int jpegSize;       // Größe in Bytes

// JPEG dekodieren (ruft automatisch tft_output() auf)
TJpgDec.drawJpg(0, 0, jpegData, jpegSize);
```

**Wie es funktioniert:**

```
1. TJpgDec liest JPEG-Header (Größe, Format)
2. TJpgDec dekodiert in 16x16 Pixel Tiles
3. Für jedes Tile: tft_output() aufrufen
4. Wir zeichnen Tile direkt auf Display
5. Nächstes Tile...
```

**Vorteil:** Nur 16x16 Pixel = 512 Bytes RAM pro Tile!

---

### 5. Automatische Skalierung

**Problem:** Bild ist 1920x1080, Display ist 240x320!

**Lösung:** TJpg_Decoder kann während Dekodierung skalieren!

```cpp
// JPEG-Größe ermitteln
TJpgDec.getJpgSize(&width, &height, jpegData, jpegSize);

Serial.printf("Original: %d x %d\n", width, height);

// Scale-Faktor berechnen
uint8_t scale = 1;
if (width > lcd.width() * 2 || height > lcd.height() * 2) {
  scale = 2;   // 1/2 Größe
}
if (width > lcd.width() * 4 || height > lcd.height() * 4) {
  scale = 4;   // 1/4 Größe
}
if (width > lcd.width() * 8 || height > lcd.height() * 8) {
  scale = 8;   // 1/8 Größe
}

TJpgDec.setJpgScale(scale);
Serial.printf("Using scale: 1/%d\n", scale);
```

**Scale-Faktoren:**
- `1`: Volle Auflösung
- `2`: 1/2 Größe (1920x1080 → 960x540)
- `4`: 1/4 Größe (1920x1080 → 480x270)
- `8`: 1/8 Größe (1920x1080 → 240x135)

**Performance:**

```
Scale 1: 1920x1080 = 2,073,600 Pixel → ~500ms
Scale 4: 480x270   = 129,600 Pixel   → ~50ms
```

Skalierung ist **10x schneller**!

---

### 6. Memory Management

**ESP32 hat ~520 KB RAM** (davon ~300 KB frei für Anwendung).

#### RAM-Verbrauch

```cpp
// JPEG im RAM
uint8_t* imageBuffer = (uint8_t*)malloc(150000);  // 150 KB

// Prüfen ob Allocation erfolgreich
if (imageBuffer == nullptr) {
  Serial.println("Out of memory!");
  return;
}

// Nach Verwendung freigeben!
free(imageBuffer);
```

**Limits in diesem Beispiel:**

```
Max Bildgröße: 150 KB
Grund: ESP32 Free Heap (Sicherheitspuffer)

Größere Bilder: Error "Image too large"
```

#### Free Heap prüfen

```cpp
Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());

// Vor Allocation:
int freeBefore = ESP.getFreeHeap();

// Nach Allocation:
int freeAfter = ESP.getFreeHeap();
int used = freeBefore - freeAfter;

Serial.printf("Allocated: %d bytes\n", used);
```

---

### 7. Progress Bar

**Warum?** User soll sehen dass etwas passiert!

```cpp
void drawProgressBar(int progress, const char* label) {
  // progress: 0-100

  int barWidth = lcd.width() - 40;
  int fillWidth = (barWidth * progress) / 100;

  // Background
  lcd.fillRect(x, y, barWidth, barHeight, COLOR_BG);

  // Fortschritt (grün)
  lcd.fillRect(x, y, fillWidth, barHeight, COLOR_FG);

  // Prozent-Text
  lcd.printf("%d%%", progress);
}
```

**Update während Download:**

```cpp
while (downloading) {
  int progress = (downloadedBytes * 100) / totalBytes;

  if (progress != lastProgress) {
    drawProgressBar(progress, "Downloading...");
    lastProgress = progress;
  }
}
```

**Wichtig:** Nur bei Änderung updaten (nicht jeden Loop = Flackern!).

---

## 🚀 Experimente & Erweiterungen

### 1. 🎨 PNG-Unterstützung

**Aktuell:** Nur JPEG.

**Erweitere auf PNG:**

```cpp
#include <PNGdec.h>

PNG png;

void* myOpen(const char* filename, int32_t* size) {
  // Nicht verwendet für RAM-basierte PNGs
  return nullptr;
}

void myClose(void* handle) {
  // Nicht verwendet
}

int32_t myRead(PNGFILE* handle, uint8_t* buffer, int32_t length) {
  // Lese aus RAM
  memcpy(buffer, pngData + readOffset, length);
  readOffset += length;
  return length;
}

int32_t mySeek(PNGFILE* handle, int32_t position) {
  readOffset = position;
  return position;
}

void pngDraw(PNGDRAW* pDraw) {
  // PNG-Pixel auf Display
  lcd.pushImage(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight,
                (uint16_t*)pDraw->pPixels);
}

void loadPNG(uint8_t* pngData, int pngSize) {
  int rc = png.openRAM(pngData, pngSize, myOpen, myClose, myRead, mySeek, pngDraw);

  if (rc == PNG_SUCCESS) {
    png.decode(NULL, 0);
    png.close();
  }
}
```

**Installation:**
```
Arduino: PNGdec by bitbank2
PlatformIO: bitbank2/PNGdec @ ^1.0.0
```

---

### 2. 🔍 Zoom & Pan

**Idee:** Pinch-to-Zoom wie auf Smartphone!

```cpp
// Sprite für Zoom (größer als Display)
LGFX_Sprite sprite(&lcd);

void setup() {
  sprite.createSprite(800, 600);  // 3x Display-Größe
}

float zoomLevel = 1.0;  // 1.0 - 3.0
int panX = 0, panY = 0;

void drawZoomedImage() {
  // JPEG in Sprite dekodieren (volle Größe)
  TJpgDec.drawJpg(0, 0, jpegData, jpegSize, &sprite);

  // Ausschnitt auf Display pushen
  int srcX = panX;
  int srcY = panY;
  int srcW = lcd.width() / zoomLevel;
  int srcH = lcd.height() / zoomLevel;

  sprite.pushSprite(0, 0, srcX, srcY, srcW, srcH);
}

void handlePinch() {
  // Zwei Touch-Punkte tracken
  // Distanz berechnen
  // Zoom anpassen
}
```

---

### 3. 📁 Lokale Bilder (SD-Karte)

**Aktuell:** Nur von URLs.

**Erweitere auf SD:**

```cpp
#include <SD.h>

// SD-Karte initialisieren
void setup() {
  if (!SD.begin(5)) {  // CS Pin 5 für CYD
    Serial.println("SD failed!");
    return;
  }
}

void loadImageFromSD(const char* filename) {
  File file = SD.open(filename, FILE_READ);

  if (!file) {
    Serial.println("File not found!");
    return;
  }

  int fileSize = file.size();
  uint8_t* buffer = (uint8_t*)malloc(fileSize);

  if (buffer) {
    file.read(buffer, fileSize);
    file.close();

    // JPEG dekodieren
    TJpgDec.drawJpg(0, 0, buffer, fileSize);

    free(buffer);
  }
}

// Alle JPEG-Dateien im Ordner listen
void listImages() {
  File root = SD.open("/images");
  File file;

  while (file = root.openNextFile()) {
    if (!file.isDirectory()) {
      String name = file.name();
      if (name.endsWith(".jpg") || name.endsWith(".jpeg")) {
        Serial.println(name);
      }
    }
    file.close();
  }
}
```

---

### 4. 🎬 Slideshow-Modus

**Idee:** Automatisches Blättern alle X Sekunden.

```cpp
bool slideshowActive = false;
unsigned long lastSlideChange = 0;
const int SLIDE_INTERVAL = 5000;  // 5 Sekunden

void loop() {
  if (slideshowActive) {
    if (millis() - lastSlideChange > SLIDE_INTERVAL) {
      currentImageIndex++;
      if (currentImageIndex >= IMAGE_COUNT) {
        currentImageIndex = 0;
      }

      loadImage(currentImageIndex);
      lastSlideChange = millis();
    }
  }

  handleTouch();
}

void handleTouch() {
  // Touch irgendwo: Slideshow pausieren/starten
  if (touched) {
    slideshowActive = !slideshowActive;

    if (slideshowActive) {
      drawMessage("Slideshow ON", COLOR_SUCCESS);
      delay(500);
    } else {
      drawMessage("Slideshow OFF", COLOR_ERROR);
      delay(500);
    }
  }
}
```

---

### 5. 🌐 URL-Eingabe mit virtuellem Keyboard

**Idee:** Nutzer kann eigene URLs eingeben!

```cpp
#include <lvgl.h>  // Für virtuelles Keyboard

lv_obj_t* textarea;
lv_obj_t* keyboard;
char customURL[256] = "";

void createURLInput() {
  // Text-Feld
  textarea = lv_textarea_create(lv_scr_act());
  lv_textarea_set_one_line(textarea, true);
  lv_textarea_set_placeholder_text(textarea, "Enter image URL...");
  lv_obj_set_width(textarea, 220);
  lv_obj_align(textarea, LV_ALIGN_TOP_MID, 0, 10);

  // Virtuelles Keyboard
  keyboard = lv_keyboard_create(lv_scr_act());
  lv_keyboard_set_textarea(keyboard, textarea);

  // Button: Load
  lv_obj_t* btn = lv_btn_create(lv_scr_act());
  lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -10);

  lv_obj_t* label = lv_label_create(btn);
  lv_label_set_text(label, "Load Image");

  lv_obj_add_event_cb(btn, [](lv_event_t* e) {
    strcpy(customURL, lv_textarea_get_text(textarea));
    Serial.printf("Loading: %s\n", customURL);

    // Zurück zur Image-View
    lv_obj_clean(lv_scr_act());

    // Bild laden (ohne LVGL)
    loadImageFromURL(customURL);
  }, LV_EVENT_CLICKED, NULL);
}
```

---

### 6. 🖼️ Thumbnail-Gallery

**Idee:** Grid von Thumbnails, Touch zum Vergrößern.

```cpp
const int THUMB_SIZE = 60;
const int THUMBS_PER_ROW = 3;

LGFX_Sprite thumbnails[IMAGE_COUNT];

void createThumbnails() {
  for (int i = 0; i < IMAGE_COUNT; i++) {
    // Thumbnail-Sprite erstellen
    thumbnails[i].createSprite(THUMB_SIZE, THUMB_SIZE);

    // Bild herunterladen & in Sprite dekodieren (klein)
    // ...

    // Auf Display zeichnen
    int col = i % THUMBS_PER_ROW;
    int row = i / THUMBS_PER_ROW;

    int x = col * (THUMB_SIZE + 5) + 5;
    int y = row * (THUMB_SIZE + 5) + 40;

    thumbnails[i].pushSprite(x, y);
  }
}

void handleTouch() {
  // Touch auf Thumbnail: Bild in Vollbild anzeigen
  int col = x / (THUMB_SIZE + 5);
  int row = y / (THUMB_SIZE + 5);
  int index = row * THUMBS_PER_ROW + col;

  if (index < IMAGE_COUNT) {
    loadImage(index);
  }
}
```

---

### 7. 💾 Caching (Bilder speichern)

**Idee:** Einmal geladene Bilder in SPIFFS speichern.

```cpp
#include <SPIFFS.h>

void setup() {
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS failed!");
  }
}

void loadImageWithCache(const char* url) {
  // Hash der URL als Dateiname
  uint32_t hash = hashString(url);
  char filename[32];
  sprintf(filename, "/cache/%08x.jpg", hash);

  // Prüfen ob im Cache
  if (SPIFFS.exists(filename)) {
    Serial.println("Loading from cache!");

    File file = SPIFFS.open(filename, FILE_READ);
    int size = file.size();
    uint8_t* buffer = (uint8_t*)malloc(size);
    file.read(buffer, size);
    file.close();

    TJpgDec.drawJpg(0, 0, buffer, size);
    free(buffer);

  } else {
    Serial.println("Downloading...");

    // HTTP-Download
    // ...

    // In Cache speichern
    File file = SPIFFS.open(filename, FILE_WRITE);
    file.write(imageBuffer, imageSize);
    file.close();

    // Anzeigen
    TJpgDec.drawJpg(0, 0, imageBuffer, imageSize);
  }
}

uint32_t hashString(const char* str) {
  // Einfacher String-Hash
  uint32_t hash = 5381;
  int c;

  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + c;
  }

  return hash;
}
```

---

## 🐛 Troubleshooting

### Problem: "WiFi verbindet nicht"

**Symptom:** "WiFi connection timeout!" auf Display.

**Ursachen:**
1. Falsche SSID/Passwort in `WiFi_Config.h`
2. WiFi-Netz zu weit weg (schwaches Signal)
3. ESP32 nicht richtig gebootet

**Debug:**

```cpp
void connectWiFi() {
  Serial.printf("SSID: %s\n", WIFI_SSID);
  Serial.printf("Password: %s\n", WIFI_PASSWORD);  // Temporär zum Debuggen

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf("Status: %d\n", WiFi.status());
    // 0 = WL_IDLE_STATUS
    // 3 = WL_CONNECTED
    // 4 = WL_CONNECT_FAILED
    // 6 = WL_DISCONNECTED

    delay(500);
  }
}
```

**Lösung:**
- Überprüfe Credentials
- Stelle CYD näher an Router
- Reset ESP32 (EN-Button drücken)

---

### Problem: "HTTP Error: -1" oder "Connection failed"

**Symptom:** Bild lädt nicht, HTTP Error negativ.

**Ursachen:**
1. WiFi verbunden, aber kein Internet
2. URL falsch (Tippfehler)
3. Server antwortet nicht
4. Timeout zu kurz

**Debug:**

```cpp
http.begin(url);
int httpCode = http.GET();

Serial.printf("HTTP Code: %d\n", httpCode);

if (httpCode < 0) {
  Serial.printf("Error: %s\n", http.errorToString(httpCode).c_str());
}
```

**Error Codes:**
- `-1 HTTPC_ERROR_CONNECTION_FAILED`: Server nicht erreichbar
- `-2 HTTPC_ERROR_SEND_HEADER_FAILED`: Header senden fehlgeschlagen
- `-11 HTTPC_ERROR_READ_TIMEOUT`: Timeout beim Lesen

**Lösung:**
- Teste URL im Browser
- Erhöhe Timeout: `http.setTimeout(20000);`
- Prüfe DNS: `ping example.com` auf PC

---

### Problem: "Out of memory!" oder ESP32 crasht

**Symptom:** Display zeigt "Out of memory!" oder ESP32 startet neu.

**Ursache:** Bild zu groß für RAM (>150 KB).

**Debug:**

```cpp
void loadImage(int index) {
  Serial.printf("Free heap before: %d\n", ESP.getFreeHeap());

  uint8_t* buffer = (uint8_t*)malloc(totalBytes);

  if (buffer == nullptr) {
    Serial.printf("Allocation failed! Requested: %d\n", totalBytes);
    Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
    return;
  }

  // ...

  free(buffer);
  Serial.printf("Free heap after: %d\n", ESP.getFreeHeap());
}
```

**Lösung:**
- Verwende kleinere Bilder (<150 KB)
- Oder: Implementiere Streaming-Decoder (fortgeschritten!)
- Oder: Komprimiere Bilder vor Upload

---

### Problem: "Bild ist verzerrt / falsche Farben"

**Symptom:** Bild wird angezeigt, aber Farben sind falsch oder Streifen.

**Ursache:** `setSwapBytes()` fehlt oder falsch.

**Lösung:**

```cpp
TJpgDec.setSwapBytes(true);  // Für ILI9341

// Wenn immer noch falsch:
TJpgDec.setSwapBytes(false);  // Versuche ohne
```

**Andere Displays:**
- ILI9341: `setSwapBytes(true)`
- ST7789: `setSwapBytes(false)`  (meist)

---

### Problem: "Download ist sehr langsam (>10 Sekunden)"

**Symptom:** Progress Bar bewegt sich kaum.

**Ursachen:**
1. Schwaches WiFi-Signal
2. Langsamer Server
3. Großes Bild

**Debug:**

```cpp
Serial.printf("WiFi RSSI: %d dBm\n", WiFi.RSSI());
// -30 = Perfekt
// -50 = Gut
// -70 = OK
// -90 = Schlecht

Serial.printf("Download speed: %.2f KB/s\n",
              (totalBytes / 1024.0) / (downloadTime / 1000.0));
```

**Lösung:**
- Verbessere WiFi-Signal (näher an Router)
- Verwende kleinere Bilder
- Verwende schnelleren Server (CDN)

---

### Problem: "Image too large! Max 150 KB"

**Symptom:** Meldung auf Display.

**Ursache:** Bild ist größer als 150 KB (RAM-Limit).

**Lösung 1:** Verwende kleinere Bilder

```
Komprimiere JPEG: Qualität 80% statt 100%
Reduziere Auflösung: 800x600 statt 1920x1080
```

**Lösung 2:** Erhöhe Limit (riskant!)

```cpp
if (totalBytes > 0 && totalBytes < 200000) {  // 200 KB statt 150
  // ...
}
```

**Aber:** ESP32 kann crashen bei >200 KB!

---

## 📚 Weiterführende Konzepte

### 1. Progressive JPEG

**Aktuell:** Baseline JPEG (gesamtes Bild auf einmal).

**Progressive JPEG:** Bild wird in mehreren "Scans" schärfer.

```
Scan 1: Grobes Bild (10%)
Scan 2: Detaillierter (30%)
Scan 3: Noch detaillierter (60%)
Scan 4: Vollständig (100%)
```

**Vorteil:** User sieht schnell ein grobes Bild!

**TJpg_Decoder:** Unterstützt Progressive JPEG automatisch.

---

### 2. HTTPS (Verschlüsselte Verbindung)

**Aktuell:** HTTP (unverschlüsselt).

**HTTPS:** Verschlüsselt, sicherer.

```cpp
#include <WiFiClientSecure.h>

WiFiClientSecure client;
client.setInsecure();  // WARNUNG: Keine Zertifikatsprüfung!

HTTPClient https;
https.begin(client, "https://example.com/image.jpg");
```

**Mit Zertifikat-Prüfung:**

```cpp
const char* rootCACertificate = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ
...
-----END CERTIFICATE-----
)EOF";

client.setCACert(rootCACertificate);
```

---

### 3. WebP-Format

**Aktuell:** JPEG.

**WebP:** Moderneres Format, ~30% kleinere Dateien!

**Leider:** Keine gute ESP32-Library verfügbar.

**Alternative:** Konvertiere WebP → JPEG auf Server.

---

### 4. Streaming-Decoder (kein RAM-Buffer)

**Aktuell:** Gesamtes JPEG im RAM.

**Streaming:** Dekodiere während Download!

```cpp
// Fortgeschritten: Custom Stream-Adapter für TJpg_Decoder
class HTTPStream {
  WiFiClient* stream;

  uint8_t read() {
    return stream->read();
  }

  int available() {
    return stream->available();
  }
};

// TJpg_Decoder mit Stream füttern (sehr komplex!)
```

**Vorteil:** Unendlich große Bilder möglich!

**Nachteil:** Sehr komplex zu implementieren.

---

## 🎓 Was hast du gelernt?

Nach diesem Beispiel verstehst du:

- ✅ **WiFi-Programmierung** - ESP32 als Client verbinden
- ✅ **HTTP-Protokoll** - GET-Requests, Status Codes, Headers
- ✅ **Streaming** - Chunk-weise Downloads
- ✅ **JPEG-Dekodierung** - Bild-Formate verstehen
- ✅ **Memory Management** - RAM-Limits beachten, malloc/free
- ✅ **Asynchrone UI** - Loading States für bessere UX
- ✅ **Error Handling** - Robuste Netzwerk-Anwendungen
- ✅ **Performance** - Download-/Decode-Zeit messen

**Nächste Schritte:**
1. Implementiere PNG-Unterstützung (PNGdec)
2. Füge SD-Karten-Support hinzu
3. Baue Thumbnail-Gallery
4. Implementiere Caching

---

## 🔗 Siehe auch

- [03_WiFi_Connection](../03_WiFi_Connection/) - WiFi-Grundlagen (falls vorhanden)
- [14_Selection_Menu_Native](../14_Selection_Menu_Native/) - Touch-UI
- [TJpg_Decoder GitHub](https://github.com/Bodmer/TJpg_Decoder)
- [PNGdec GitHub](https://github.com/bitbank2/PNGdec)

**Externe Ressourcen:**
- [ESP32 WiFi Docs](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html)
- [HTTP Status Codes](https://developer.mozilla.org/en-US/docs/Web/HTTP/Status)
- [JPEG Format](https://en.wikipedia.org/wiki/JPEG)

---

**Viel Spaß beim Experimentieren mit Netzwerk & Bildern! 🖼️📡**
