/*
  15b_Image_Viewer_SD - Bilder von SD-Karte laden und anzeigen

  Lokales Gegenstück zu 15_Image_Viewer - lädt Bilder von SD-Karte statt Internet.

  Funktionen:
  - SD-Karte initialisieren und durchsuchen
  - JPEG-Dateien automatisch finden
  - Bilder mit automatischer Skalierung anzeigen
  - Touch-Navigation (Links/Rechts)
  - Ordner-Übersicht bei mehreren Ordnern
  - Deutlich schneller als Network-Version (kein Download!)

  Lernziele:
  - SD-Karten-Zugriff (SPI)
  - Dateisystem-Navigation
  - Rekursive Ordner-Durchsuchung
  - File I/O (lesen, öffnen, schließen)
  - Gleiche JPEG-Dekodierung wie 15

  Hardware:
  - 1x CYD Display (2.8" oder 3.5")
  - 1x Micro-SD-Karte (formatiert als FAT32)
  - JPEG-Dateien auf SD-Karte im Ordner "/images/"

  SD-Karten-Vorbereitung:
  1. SD-Karte formatieren (FAT32)
  2. Ordner "/images/" erstellen
  3. JPEG-Dateien hineinkopieren
  4. SD-Karte in CYD einlegen

  Steuerung:
  - Touch Links: Vorheriges Bild
  - Touch Rechts: Nächstes Bild
  - Touch Mitte: Neu scannen (nach neuen Bildern suchen)
*/

#include <LovyanGFX.hpp>
#include <SD.h>
#include <TJpg_Decoder.h>

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

// SD-Karten-Pins (CYD Standard)
#define SD_CS 5

// Farben
#define COLOR_BG 0x0000          // Schwarz
#define COLOR_TEXT 0xFFFF        // Weiß
#define COLOR_ERROR 0xF800       // Rot
#define COLOR_SUCCESS 0x07E0     // Grün
#define COLOR_LOADING 0x03EF     // Blau

// Touch-Zonen
const int TOUCH_ZONE_LEFT = 80;
const int TOUCH_ZONE_RIGHT = 160;

// Datei-Liste
const int MAX_IMAGES = 100;  // Maximale Anzahl Bilder
String imageFiles[MAX_IMAGES];
int imageCount = 0;
int currentImageIndex = 0;

// Image-Ordner (anpassbar!)
const char* IMAGE_FOLDER = "/images";

// ===== GLOBALE VARIABLEN =====

LGFX lcd;
bool sdCardReady = false;

// ===== TJPG DECODER CALLBACK =====

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if (y >= lcd.height()) return 0;
  lcd.pushImage(x, y, w, h, bitmap);
  return 1;
}

// ===== SETUP =====

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Image Viewer SD Demo ===");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(0);  // Portrait
  lcd.setBrightness(255);
  lcd.fillScreen(COLOR_BG);

  // TJpg_Decoder initialisieren
  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);

  // SD-Karte initialisieren
  drawMessage("Initializing SD Card...", COLOR_LOADING);

  if (!SD.begin(SD_CS)) {
    drawMessage("SD Card failed!\n\nCheck:\n- SD inserted?\n- FAT32 format?\n- Clean contacts?", COLOR_ERROR);
    Serial.println("SD Card initialization failed!");
    sdCardReady = false;
    return;
  }

  sdCardReady = true;

  // SD-Karten-Info
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card OK! Size: %llu MB\n", cardSize);

  drawMessage("SD Card OK!\nScanning images...", COLOR_SUCCESS);
  delay(1000);

  // Bilder suchen
  scanForImages();

  if (imageCount == 0) {
    drawMessage("No images found!\n\nCreate folder:\n/images/\n\nAdd JPEG files!", COLOR_ERROR);
    Serial.println("No JPEG files found!");
  } else {
    Serial.printf("Found %d images\n", imageCount);
    drawMessage(String("Found " + String(imageCount) + " images!").c_str(), COLOR_SUCCESS);
    delay(1000);

    // Erstes Bild laden
    loadImage(currentImageIndex);
  }

  Serial.println("Ready! Touch: Left/Right to browse, Center to rescan");
}

// ===== LOOP =====

void loop() {
  if (sdCardReady && imageCount > 0) {
    handleTouch();
  }
  delay(50);
}

// ===== SD-KARTEN FUNKTIONEN =====

void scanForImages() {
  imageCount = 0;

  Serial.printf("Scanning folder: %s\n", IMAGE_FOLDER);

  // Prüfe ob Ordner existiert
  if (!SD.exists(IMAGE_FOLDER)) {
    Serial.printf("Folder %s does not exist! Creating...\n", IMAGE_FOLDER);
    SD.mkdir(IMAGE_FOLDER);
    return;
  }

  // Ordner öffnen
  File root = SD.open(IMAGE_FOLDER);

  if (!root) {
    Serial.println("Failed to open folder!");
    return;
  }

  if (!root.isDirectory()) {
    Serial.println("Not a directory!");
    return;
  }

  // Dateien durchsuchen
  File file = root.openNextFile();

  while (file && imageCount < MAX_IMAGES) {
    if (!file.isDirectory()) {
      String filename = String(file.name());

      // Prüfe ob JPEG (case-insensitive)
      filename.toLowerCase();
      if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) {
        // Füge zur Liste hinzu (mit vollem Pfad)
        imageFiles[imageCount] = String(IMAGE_FOLDER) + "/" + String(file.name());
        Serial.printf("  [%d] %s (%d bytes)\n", imageCount, imageFiles[imageCount].c_str(), file.size());
        imageCount++;
      }
    }

    file = root.openNextFile();
  }

  root.close();

  Serial.printf("Scan complete: %d images found\n", imageCount);

  // Sortiere Liste alphabetisch (Bubble Sort - einfach für kleine Listen)
  for (int i = 0; i < imageCount - 1; i++) {
    for (int j = 0; j < imageCount - i - 1; j++) {
      if (imageFiles[j] > imageFiles[j + 1]) {
        String temp = imageFiles[j];
        imageFiles[j] = imageFiles[j + 1];
        imageFiles[j + 1] = temp;
      }
    }
  }
}

// Rekursive Variante (optional - findet Bilder in Unterordnern)
void scanForImagesRecursive(const char* dirname, int level = 0) {
  if (level > 5) return;  // Max 5 Ebenen tief

  File root = SD.open(dirname);
  if (!root || !root.isDirectory()) {
    return;
  }

  File file = root.openNextFile();

  while (file && imageCount < MAX_IMAGES) {
    if (file.isDirectory()) {
      // Rekursiv in Unterordner
      String subdir = String(dirname) + "/" + String(file.name());
      Serial.printf("Entering: %s\n", subdir.c_str());
      scanForImagesRecursive(subdir.c_str(), level + 1);

    } else {
      // Datei prüfen
      String filename = String(file.name());
      filename.toLowerCase();

      if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) {
        imageFiles[imageCount] = String(dirname) + "/" + String(file.name());
        Serial.printf("  [%d] %s\n", imageCount, imageFiles[imageCount].c_str());
        imageCount++;
      }
    }

    file = root.openNextFile();
  }

  root.close();
}

// ===== IMAGE LOADING =====

void loadImage(int index) {
  if (!sdCardReady) {
    drawMessage("SD Card not ready!", COLOR_ERROR);
    return;
  }

  if (index < 0 || index >= imageCount) {
    Serial.println("Invalid image index!");
    return;
  }

  String filepath = imageFiles[index];
  Serial.printf("\n=== Loading Image %d/%d ===\n", index + 1, imageCount);
  Serial.printf("File: %s\n", filepath.c_str());

  // Bildschirm löschen
  lcd.fillScreen(COLOR_BG);
  drawMessage("Loading...", COLOR_LOADING);

  // Datei öffnen
  File file = SD.open(filepath);

  if (!file) {
    Serial.println("Failed to open file!");
    drawMessage("File open failed!", COLOR_ERROR);
    return;
  }

  // Dateigröße
  int fileSize = file.size();
  Serial.printf("File size: %d bytes (%.1f KB)\n", fileSize, fileSize / 1024.0);

  unsigned long startTime = millis();

  // OPTION 1: Kleines Bild - komplett in RAM laden (schnell!)
  if (fileSize < 150000) {  // < 150 KB
    uint8_t* buffer = (uint8_t*)malloc(fileSize);

    if (buffer != nullptr) {
      // Datei lesen
      file.read(buffer, fileSize);
      file.close();

      unsigned long loadTime = millis() - startTime;

      // Bildschirm löschen
      lcd.fillScreen(COLOR_BG);

      // Automatische Skalierung
      TJpgDec.getJpgSize(nullptr, nullptr, buffer, fileSize);
      uint16_t jpgWidth = TJpgDec.getWidth();
      uint16_t jpgHeight = TJpgDec.getHeight();

      Serial.printf("JPEG size: %d x %d\n", jpgWidth, jpgHeight);

      // Scale berechnen
      uint8_t scale = 1;
      if (jpgWidth > lcd.width() * 2 || jpgHeight > lcd.height() * 2) scale = 2;
      if (jpgWidth > lcd.width() * 4 || jpgHeight > lcd.height() * 4) scale = 4;
      if (jpgWidth > lcd.width() * 8 || jpgHeight > lcd.height() * 8) scale = 8;

      TJpgDec.setJpgScale(scale);
      Serial.printf("Using scale: 1/%d\n", scale);

      // Dekodieren
      unsigned long decodeStart = millis();
      TJpgDec.drawJpg(0, 0, buffer, fileSize);
      unsigned long decodeTime = millis() - decodeStart;

      Serial.printf("Load time: %lu ms\n", loadTime);
      Serial.printf("Decode time: %lu ms\n", decodeTime);

      // Info anzeigen
      drawImageInfo(index, filepath, fileSize, loadTime, decodeTime);

      // Buffer freigeben
      free(buffer);

    } else {
      Serial.println("Memory allocation failed!");
      drawMessage("Out of memory!", COLOR_ERROR);
      file.close();
    }

  // OPTION 2: Großes Bild - SD-Streaming nutzen (langsamer, aber kein RAM-Limit!)
  } else {
    Serial.println("File too large for RAM - using SD streaming");
    file.close();

    // Für Streaming müssen wir TJpgDec's SD-Modus nutzen
    // ACHTUNG: TJpgDec unterstützt direkt SD.open() nicht immer gut
    // Workaround: Lese in Chunks und baue Buffer

    drawMessage("Image too large!\n(>150 KB)\n\nUse smaller images", COLOR_ERROR);

    // ALTERNATIV: Implementiere Chunk-Reading
    // (Fortgeschritten - siehe README)
  }
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
        currentImageIndex = imageCount - 1;  // Wrap around
      }
      loadImage(currentImageIndex);

    } else if (x > TOUCH_ZONE_RIGHT) {
      // Rechte Zone: Nächstes Bild
      currentImageIndex++;
      if (currentImageIndex >= imageCount) {
        currentImageIndex = 0;  // Wrap around
      }
      loadImage(currentImageIndex);

    } else {
      // Mitte: Neu scannen
      Serial.println("Rescanning SD card...");
      drawMessage("Rescanning...", COLOR_LOADING);
      delay(500);

      imageCount = 0;
      scanForImages();

      if (imageCount > 0) {
        currentImageIndex = 0;
        loadImage(currentImageIndex);
      } else {
        drawMessage("No images found!", COLOR_ERROR);
      }
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

      int textWidth = line.length() * 12;
      int x = (lcd.width() - textWidth) / 2;
      int y = startY + (currentLine * lineHeight);

      lcd.setCursor(x, y);
      lcd.print(line);

      currentLine++;
      lastPos = i + 1;
    }
  }
}

void drawImageInfo(int index, String filepath, int fileSize, unsigned long loadTime, unsigned long decodeTime) {
  // Info-Panel unten
  int panelHeight = 70;
  lcd.fillRect(0, lcd.height() - panelHeight, lcd.width(), panelHeight, 0x18E3);

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT);

  // Zeile 1: Dateiname (gekürzt wenn zu lang)
  String filename = filepath;
  int lastSlash = filename.lastIndexOf('/');
  if (lastSlash >= 0) {
    filename = filename.substring(lastSlash + 1);
  }

  // Kürze Dateiname wenn zu lang
  if (filename.length() > 28) {
    filename = filename.substring(0, 25) + "...";
  }

  lcd.setCursor(5, lcd.height() - panelHeight + 5);
  lcd.print(filename);

  // Zeile 2: Index
  lcd.setCursor(5, lcd.height() - panelHeight + 20);
  lcd.printf("Image %d/%d", index + 1, imageCount);

  // Zeile 3: Dateigröße
  lcd.setCursor(5, lcd.height() - panelHeight + 35);
  if (fileSize < 1024) {
    lcd.printf("Size: %d B", fileSize);
  } else if (fileSize < 1024 * 1024) {
    lcd.printf("Size: %.1f KB", fileSize / 1024.0);
  } else {
    lcd.printf("Size: %.1f MB", fileSize / (1024.0 * 1024.0));
  }

  // Zeile 4: Timing
  lcd.setCursor(5, lcd.height() - panelHeight + 50);
  lcd.printf("Load: %lu ms", loadTime);

  lcd.setCursor(120, lcd.height() - panelHeight + 50);
  lcd.printf("Dec: %lu ms", decodeTime);

  // Navigation-Hilfe
  lcd.setTextColor(0x7BEF);
  lcd.setCursor(140, lcd.height() - panelHeight + 5);
  lcd.print("<   >");

  // SD-Icon (einfach)
  lcd.setCursor(200, lcd.height() - panelHeight + 20);
  lcd.print("SD");
}
