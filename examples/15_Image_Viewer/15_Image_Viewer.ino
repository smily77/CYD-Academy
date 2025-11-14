/*
  15_Image_Viewer - Bilder von URLs laden und anzeigen

  Dieses Beispiel zeigt wie man Bilder vom Internet lädt und auf dem Display anzeigt.

  Funktionen:
  - WiFi-Verbindung herstellen
  - JPEG-Bilder von URLs herunterladen
  - Automatische Skalierung auf Display-Größe
  - Touch-Navigation (Links/Rechts zum Blättern)
  - Loading-Indikator während Download
  - Error Handling (kein WiFi, 404, etc.)

  Lernziele:
  - WiFi-Programmierung (HTTPClient)
  - HTTP GET Requests
  - JPEG-Dekodierung (TJpg_Decoder)
  - Streaming (Download während Anzeige)
  - Asynchrone UI (Loading States)
  - Error Handling

  Hardware:
  - 1x CYD Display (2.8" oder 3.5")
  - WiFi-Verbindung erforderlich!

  Installation:
  1. TJpg_Decoder installieren:
     Arduino IDE: Library Manager → "TJpg_Decoder" → installieren
     PlatformIO: lib_deps = bodmer/TJpg_Decoder @ ^1.0.0

  2. WiFi_Config.h anpassen:
     Trage deine WiFi-Zugangsdaten ein!

  Steuerung:
  - Touch Links: Vorheriges Bild
  - Touch Rechts: Nächstes Bild
  - Touch Mitte: Aktuelles Bild neu laden
*/

#include <LovyanGFX.hpp>
#include <WiFi.h>
#include <HTTPClient.h>
#include <TJpg_Decoder.h>
#include "WiFi_Config.h"

// Display-Konfiguration
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Light_PWM _light_instance;
  lgfx::Touch_XPT2046 _touch_instance;

public:
  LGFX(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = VSPI_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read = 16000000;
      cfg.spi_3wire = true;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = 14;
      cfg.pin_mosi = 13;
      cfg.pin_miso = 12;
      cfg.pin_dc = 2;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = 15;
      cfg.pin_rst = -1;
      cfg.pin_busy = -1;
      cfg.panel_width = 240;
      cfg.panel_height = 320;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = true;
      cfg.invert = false;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;
      _panel_instance.config(cfg);
    }

    {
      auto cfg = _light_instance.config();
      cfg.pin_bl = 21;
      cfg.invert = false;
      cfg.freq = 44100;
      cfg.pwm_channel = 7;
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    {
      auto cfg = _touch_instance.config();
      cfg.x_min = 300;
      cfg.x_max = 3900;
      cfg.y_min = 400;
      cfg.y_max = 3900;
      cfg.pin_int = -1;
      cfg.bus_shared = true;
      cfg.offset_rotation = 0;
      cfg.spi_host = VSPI_HOST;
      cfg.freq = 1000000;
      cfg.pin_sclk = 25;
      cfg.pin_mosi = 32;
      cfg.pin_miso = 39;
      cfg.pin_cs = 33;
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }
};

// ===== KONSTANTEN =====

// Farben
#define COLOR_BG 0x0000          // Schwarz
#define COLOR_TEXT 0xFFFF        // Weiß
#define COLOR_ERROR 0xF800       // Rot
#define COLOR_SUCCESS 0x07E0     // Grün
#define COLOR_LOADING 0x03EF     // Blau
#define COLOR_PROGRESS_BG 0x4208 // Dunkelgrau
#define COLOR_PROGRESS_FG 0x07E0 // Grün

// Touch-Zonen
const int TOUCH_ZONE_LEFT = 80;
const int TOUCH_ZONE_RIGHT = 160;

// ===== BILD-URLS =====

// Liste von Beispiel-Bildern (JPEG, verschiedene Größen)
const char* imageURLs[] = {
  // Kleine Bilder (~50 KB)
  "https://picsum.photos/240/320.jpg",  // Random Bild in exakter Display-Größe

  // Mittlere Bilder (~200 KB)
  "https://picsum.photos/800/600.jpg",  // Landscape

  // Große Bilder (~500 KB)
  "https://picsum.photos/1920/1080.jpg", // Full HD

  // Spezifische Bilder (Beispiele)
  "https://picsum.photos/id/237/800/600.jpg",  // Hund
  "https://picsum.photos/id/1015/800/600.jpg", // Berg
  "https://picsum.photos/id/180/800/600.jpg",  // Natur

  // Square Bilder
  "https://picsum.photos/600/600.jpg",

  // Portrait
  "https://picsum.photos/400/800.jpg",
};

const int IMAGE_COUNT = sizeof(imageURLs) / sizeof(char*);

// ===== GLOBALE VARIABLEN =====

LGFX lcd;
HTTPClient http;

int currentImageIndex = 0;
bool wifiConnected = false;

// Download Progress
int downloadProgress = 0;  // 0-100
int downloadedBytes = 0;
int totalBytes = 0;

// ===== TJPG DECODER CALLBACK =====

// Diese Funktion wird vom TJpg_Decoder aufgerufen um Pixel zu zeichnen
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  // Bitmap direkt auf Display schreiben (sehr schnell!)
  if (y >= lcd.height()) return 0;

  lcd.pushImage(x, y, w, h, bitmap);
  return 1;
}

// ===== SETUP =====

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Image Viewer Demo ===");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(0);  // Portrait
  lcd.setBrightness(255);
  lcd.fillScreen(COLOR_BG);

  // TJpg_Decoder initialisieren
  TJpgDec.setJpgScale(1);  // 1, 2, 4, 8 (1 = volle Auflösung)
  TJpgDec.setSwapBytes(true);  // Für korrekte Farben auf ILI9341
  TJpgDec.setCallback(tft_output);  // Callback-Funktion registrieren

  // WiFi verbinden
  drawMessage("Connecting to WiFi...", COLOR_LOADING);
  connectWiFi();

  if (wifiConnected) {
    drawMessage("WiFi connected!", COLOR_SUCCESS);
    delay(1000);

    // Erstes Bild laden
    loadImage(currentImageIndex);
  } else {
    drawMessage("WiFi failed!\nCheck WiFi_Config.h", COLOR_ERROR);
  }

  Serial.println("Ready! Touch: Left/Right to browse, Center to reload");
}

// ===== LOOP =====

void loop() {
  handleTouch();
  delay(50);
}

// ===== WIFI FUNKTIONEN =====

void connectWiFi() {
  Serial.printf("Connecting to WiFi: %s\n", WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime > WIFI_CONNECT_TIMEOUT) {
      Serial.println("WiFi connection timeout!");
      wifiConnected = false;
      return;
    }

    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("Signal: %d dBm\n", WiFi.RSSI());

  wifiConnected = true;
}

// ===== IMAGE LOADING =====

void loadImage(int index) {
  if (!wifiConnected) {
    drawMessage("No WiFi!", COLOR_ERROR);
    return;
  }

  if (index < 0 || index >= IMAGE_COUNT) {
    Serial.println("Invalid image index!");
    return;
  }

  const char* url = imageURLs[index];
  Serial.printf("\n=== Loading Image %d/%d ===\n", index + 1, IMAGE_COUNT);
  Serial.printf("URL: %s\n", url);

  // Bildschirm löschen
  lcd.fillScreen(COLOR_BG);

  // Loading-Nachricht
  drawProgressBar(0, "Loading...");

  // HTTP GET Request
  http.begin(url);
  http.setTimeout(HTTP_TIMEOUT);

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    // Content-Length holen (für Progress Bar)
    totalBytes = http.getSize();
    downloadedBytes = 0;

    Serial.printf("Image size: %d bytes\n", totalBytes);

    // Stream vom Server holen
    WiFiClient* stream = http.getStreamPtr();

    // Buffer für Download
    const int BUFFER_SIZE = 2048;
    uint8_t buffer[BUFFER_SIZE];
    int bytesRead = 0;

    // Download-Start-Zeit
    unsigned long startTime = millis();

    // Temporärer Buffer für gesamtes Bild (für TJpg_Decoder)
    // ACHTUNG: Große Bilder können RAM sprengen! Max ~100 KB sicher.
    uint8_t* imageBuffer = nullptr;

    if (totalBytes > 0 && totalBytes < 150000) {  // Max 150 KB
      imageBuffer = (uint8_t*)malloc(totalBytes);

      if (imageBuffer != nullptr) {
        int offset = 0;

        // Download in Buffer
        while (http.connected() && (bytesRead = stream->readBytes(buffer, BUFFER_SIZE)) > 0) {
          memcpy(imageBuffer + offset, buffer, bytesRead);
          offset += bytesRead;
          downloadedBytes += bytesRead;

          // Progress Bar updaten
          int progress = (downloadedBytes * 100) / totalBytes;
          if (progress != downloadProgress) {
            downloadProgress = progress;
            drawProgressBar(downloadProgress, "Downloading...");
          }
        }

        unsigned long downloadTime = millis() - startTime;
        Serial.printf("Download complete: %d bytes in %lu ms\n", downloadedBytes, downloadTime);
        Serial.printf("Speed: %.2f KB/s\n", (downloadedBytes / 1024.0) / (downloadTime / 1000.0));

        // Bildschirm löschen für Anzeige
        lcd.fillScreen(COLOR_BG);

        // JPEG dekodieren und anzeigen
        drawProgressBar(0, "Decoding...");

        uint32_t decodeStart = millis();

        // Automatische Skalierung berechnen
        TJpgDec.getJpgSize(nullptr, nullptr, imageBuffer, totalBytes);
        uint16_t jpgWidth = TJpgDec.getWidth();
        uint16_t jpgHeight = TJpgDec.getHeight();

        Serial.printf("JPEG size: %d x %d\n", jpgWidth, jpgHeight);

        // Scale-Faktor berechnen (1, 2, 4, 8)
        uint8_t scale = 1;
        if (jpgWidth > lcd.width() * 2 || jpgHeight > lcd.height() * 2) {
          scale = 2;
        }
        if (jpgWidth > lcd.width() * 4 || jpgHeight > lcd.height() * 4) {
          scale = 4;
        }
        if (jpgWidth > lcd.width() * 8 || jpgHeight > lcd.height() * 8) {
          scale = 8;
        }

        TJpgDec.setJpgScale(scale);
        Serial.printf("Using scale: 1/%d\n", scale);

        // Dekodieren (TJpg_Decoder ruft automatisch tft_output() auf)
        TJpgDec.drawJpg(0, 0, imageBuffer, totalBytes);

        uint32_t decodeTime = millis() - decodeStart;
        Serial.printf("Decode time: %lu ms\n", decodeTime);

        // Info anzeigen
        drawImageInfo(index, totalBytes, downloadTime, decodeTime);

        // Buffer freigeben
        free(imageBuffer);

      } else {
        drawMessage("Out of memory!\nImage too large", COLOR_ERROR);
        Serial.printf("Failed to allocate %d bytes\n", totalBytes);
      }
    } else {
      drawMessage("Image too large!\nMax 150 KB", COLOR_ERROR);
      Serial.printf("Image too large: %d bytes\n", totalBytes);
    }

  } else {
    // HTTP Fehler
    Serial.printf("HTTP Error: %d\n", httpCode);

    char errorMsg[64];
    if (httpCode == HTTP_CODE_NOT_FOUND) {
      sprintf(errorMsg, "Error 404\nImage not found");
    } else if (httpCode < 0) {
      sprintf(errorMsg, "Connection failed\nCheck WiFi");
    } else {
      sprintf(errorMsg, "HTTP Error: %d", httpCode);
    }

    drawMessage(errorMsg, COLOR_ERROR);
  }

  http.end();
}

// ===== TOUCH HANDLING =====

void handleTouch() {
  static bool wasTouched = false;
  uint16_t x, y;
  bool touched = lcd.getTouch(&x, &y);

  if (touched && !wasTouched) {
    wasTouched = true;

    Serial.printf("Touch at x=%d, y=%d\n", x, y);

    if (x < TOUCH_ZONE_LEFT) {
      // Linke Zone: Vorheriges Bild
      currentImageIndex--;
      if (currentImageIndex < 0) {
        currentImageIndex = IMAGE_COUNT - 1;  // Wrap around
      }
      loadImage(currentImageIndex);

    } else if (x > TOUCH_ZONE_RIGHT) {
      // Rechte Zone: Nächstes Bild
      currentImageIndex++;
      if (currentImageIndex >= IMAGE_COUNT) {
        currentImageIndex = 0;  // Wrap around
      }
      loadImage(currentImageIndex);

    } else {
      // Mitte: Neu laden
      Serial.println("Reloading current image");
      loadImage(currentImageIndex);
    }
  }

  if (!touched) {
    wasTouched = false;
  }
}

// ===== UI FUNKTIONEN =====

void drawMessage(const char* message, uint16_t color) {
  lcd.fillScreen(COLOR_BG);

  lcd.setTextSize(2);
  lcd.setTextColor(color);

  // Text zentrieren (multi-line)
  int lineHeight = 20;
  String msg = String(message);
  int lineCount = 1;

  // Zeilen zählen
  for (int i = 0; i < msg.length(); i++) {
    if (msg[i] == '\n') lineCount++;
  }

  int startY = (lcd.height() - (lineCount * lineHeight)) / 2;

  // Zeilen zeichnen
  int currentLine = 0;
  int lastPos = 0;

  for (int i = 0; i <= msg.length(); i++) {
    if (i == msg.length() || msg[i] == '\n') {
      String line = msg.substring(lastPos, i);

      int textWidth = line.length() * 12;  // Ungefähre Breite (Textsize 2)
      int x = (lcd.width() - textWidth) / 2;
      int y = startY + (currentLine * lineHeight);

      lcd.setCursor(x, y);
      lcd.print(line);

      currentLine++;
      lastPos = i + 1;
    }
  }
}

void drawProgressBar(int progress, const char* label) {
  // Progress Bar unten
  int barHeight = 30;
  int barY = lcd.height() - barHeight - 10;
  int barWidth = lcd.width() - 40;
  int barX = 20;

  // Background
  lcd.fillRect(barX, barY, barWidth, barHeight, COLOR_PROGRESS_BG);

  // Fortschritt
  int fillWidth = (barWidth * progress) / 100;
  lcd.fillRect(barX, barY, fillWidth, barHeight, COLOR_PROGRESS_FG);

  // Border
  lcd.drawRect(barX, barY, barWidth, barHeight, COLOR_TEXT);

  // Prozent-Text
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);

  char buf[32];
  sprintf(buf, "%d%%", progress);
  int textWidth = strlen(buf) * 12;
  lcd.setCursor((lcd.width() - textWidth) / 2, barY + 7);
  lcd.print(buf);

  // Label oben
  if (label != nullptr) {
    lcd.setTextSize(1);
    int labelWidth = strlen(label) * 6;
    lcd.setCursor((lcd.width() - labelWidth) / 2, barY - 20);
    lcd.print(label);
  }
}

void drawImageInfo(int index, int fileSize, unsigned long downloadTime, unsigned long decodeTime) {
  // Info-Panel unten (halbtransparent simuliert durch dunklen Hintergrund)
  int panelHeight = 60;
  lcd.fillRect(0, lcd.height() - panelHeight, lcd.width(), panelHeight, 0x18E3);  // Dunkelgrau

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT);

  // Zeile 1: Bild-Index
  lcd.setCursor(5, lcd.height() - panelHeight + 5);
  lcd.printf("Image %d/%d", index + 1, IMAGE_COUNT);

  // Zeile 2: Dateigröße
  lcd.setCursor(5, lcd.height() - panelHeight + 20);
  if (fileSize < 1024) {
    lcd.printf("Size: %d B", fileSize);
  } else {
    lcd.printf("Size: %.1f KB", fileSize / 1024.0);
  }

  // Zeile 3: Download-Zeit
  lcd.setCursor(5, lcd.height() - panelHeight + 35);
  lcd.printf("DL: %lu ms", downloadTime);

  // Zeile 4: Decode-Zeit
  lcd.setCursor(120, lcd.height() - panelHeight + 35);
  lcd.printf("Dec: %lu ms", decodeTime);

  // Navigation-Hilfe
  lcd.setTextColor(0x7BEF);  // Grau
  lcd.setCursor(140, lcd.height() - panelHeight + 5);
  lcd.print("<   >");
}
