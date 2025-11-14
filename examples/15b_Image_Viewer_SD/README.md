# 💾 Beispiel 15b: Image Viewer SD - Bilder von SD-Karte

## 📝 Was macht dieses Beispiel?

Lädt **JPEG-Bilder von SD-Karte** statt Internet - schneller und offline!

**Features:**
- ✅ **SD-Karten-Zugriff** über SPI
- ✅ **Automatisches Scannen** nach JPEG-Dateien
- ✅ **Ordner-Navigation** (mit rekursiver Option)
- ✅ **Auto-Skalierung** (identisch zu 15)
- ✅ **Touch-Navigation** (Links/Rechts/Rescan)
- ✅ **Schnell!** Keine Downloads, direkt von SD
- ✅ **Offline** - kein WiFi benötigt
- ✅ **Große Bilder** möglich (bis SD-Kapazität)

**Steuerung:**
- **Touch Links (<80px):** Vorheriges Bild
- **Touch Rechts (>160px):** Nächstes Bild
- **Touch Mitte:** SD-Karte neu scannen

---

## 🎯 Lernziele

1. **SD-Karten-Zugriff** - SPI-Interface & Dateisystem
2. **File I/O** - Dateien öffnen, lesen, schließen
3. **Dateisystem-Navigation** - Ordner durchsuchen
4. **Rekursive Algorithmen** - Unterordner durchsuchen
5. **JPEG von Datei** - Gleiche Dekodierung wie 15
6. **Performance-Vergleich** - SD vs. Network

---

## ⚙️ Installation & Setup

### 1. Library installieren

**Gleiche wie 15_Image_Viewer:**

```
Arduino IDE: TJpg_Decoder von Bodmer
PlatformIO: bodmer/TJpg_Decoder @ ^1.0.0
```

### 2. SD-Karte vorbereiten

**WICHTIG: SD-Karte muss FAT32 formatiert sein!**

#### Schritt-für-Schritt:

1. **SD-Karte formatieren:**
   - Windows: Rechtsklick → Formatieren → FAT32
   - Mac: Festplattendienstprogramm → MS-DOS (FAT)
   - Linux: `sudo mkfs.vfat -F 32 /dev/sdX1`

2. **Ordner erstellen:**
   ```
   /images/
   ```

3. **JPEG-Dateien kopieren:**
   ```
   /images/photo1.jpg
   /images/photo2.jpg
   /images/sunset.jpeg
   ...
   ```

4. **SD-Karte in CYD einlegen**
   - Micro-SD-Slot ist auf der Rückseite
   - Kontakte nach unten
   - Bis zum Klick einschieben

### 3. Kompilieren & Upload

```
Arduino IDE: Upload (Strg+U)
PlatformIO: pio run -t upload
```

### 4. Testen

1. Display zeigt "Initializing SD Card..."
2. "SD Card OK! Scanning images..."
3. "Found X images!"
4. Erstes Bild wird angezeigt
5. Touch Links/Rechts zum Blättern

---

## 🔍 Technische Konzepte

### 1. SD-Karten-Hardware (SPI)

**CYD verwendet SPI für SD-Karte:**

```cpp
// SD-Karten-Pins auf CYD
#define SD_CS 5    // Chip Select

// Standard SPI-Pins (geteilt mit Display!)
#define SD_MOSI 13
#define SD_MISO 12
#define SD_SCK 14
```

**Warum geteilt mit Display?**

```
CYD nutzt SPI-Bus für:
- Display (CS Pin 15)
- Touch (CS Pin 33)
- SD-Karte (CS Pin 5)

Alle teilen sich MOSI/MISO/SCK!
CS (Chip Select) wählt Gerät aus.
```

#### Initialisierung

```cpp
#include <SD.h>

void setup() {
  if (!SD.begin(SD_CS)) {
    Serial.println("SD Card failed!");
    return;
  }

  Serial.println("SD Card OK!");

  // Info abrufen
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("Size: %llu MB\n", cardSize);

  uint8_t cardType = SD.cardType();
  Serial.print("Type: ");
  switch (cardType) {
    case CARD_MMC:  Serial.println("MMC"); break;
    case CARD_SD:   Serial.println("SD"); break;
    case CARD_SDHC: Serial.println("SDHC"); break;
    default:        Serial.println("Unknown"); break;
  }
}
```

---

### 2. Dateisystem-Navigation

#### Ordner öffnen

```cpp
File root = SD.open("/images");

if (!root) {
  Serial.println("Failed to open folder!");
  return;
}

if (!root.isDirectory()) {
  Serial.println("Not a directory!");
  return;
}
```

#### Dateien durchsuchen

```cpp
File file = root.openNextFile();

while (file) {
  if (!file.isDirectory()) {
    // Datei gefunden
    String filename = String(file.name());
    int fileSize = file.size();

    Serial.printf("File: %s (%d bytes)\n", filename.c_str(), fileSize);

    // Prüfe Datei-Endung
    filename.toLowerCase();
    if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) {
      // JPEG gefunden!
    }
  }

  file = root.openNextFile();  // Nächste Datei
}

root.close();
```

#### Rekursive Suche

**Findet Bilder in Unterordnern!**

```cpp
void scanRecursive(const char* dirname, int level = 0) {
  if (level > 5) return;  // Max 5 Ebenen tief

  File root = SD.open(dirname);
  if (!root || !root.isDirectory()) return;

  File file = root.openNextFile();

  while (file) {
    if (file.isDirectory()) {
      // Unterordner → Rekursion
      String subdir = String(dirname) + "/" + String(file.name());
      scanRecursive(subdir.c_str(), level + 1);

    } else {
      // Datei prüfen
      String filename = String(file.name());
      if (filename.endsWith(".jpg")) {
        // Gefunden!
      }
    }

    file = root.openNextFile();
  }

  root.close();
}

// Aufruf:
scanRecursive("/images");
```

**Beispiel-Struktur:**

```
/images/
  ├─ vacation/
  │  ├─ beach.jpg
  │  └─ sunset.jpg
  ├─ family/
  │  └─ portrait.jpg
  └─ nature.jpg

→ Findet alle 4 Bilder!
```

---

### 3. Datei lesen

#### Variante 1: Komplett in RAM (schnell)

```cpp
File file = SD.open("/images/photo.jpg");

int fileSize = file.size();
uint8_t* buffer = (uint8_t*)malloc(fileSize);

if (buffer != nullptr) {
  // Gesamte Datei lesen
  file.read(buffer, fileSize);
  file.close();

  // Verarbeiten
  TJpgDec.drawJpg(0, 0, buffer, fileSize);

  // Buffer freigeben
  free(buffer);
}
```

**Vorteil:** Sehr schnell (1-2ms für 100 KB)

**Nachteil:** Begrenzt durch RAM (~150 KB)

#### Variante 2: Chunk-weise (für große Dateien)

```cpp
File file = SD.open("/images/photo.jpg");

const int CHUNK_SIZE = 2048;
uint8_t chunk[CHUNK_SIZE];

while (file.available()) {
  int bytesRead = file.read(chunk, CHUNK_SIZE);

  // Chunk verarbeiten
  // (Komplex: TJpg_Decoder unterstützt kein Chunk-Feeding direkt)
}

file.close();
```

**Vorteil:** Unbegrenzte Dateigröße

**Nachteil:** Komplex zu implementieren (siehe Experimente)

---

### 4. JPEG-Dekodierung (identisch zu 15)

**Gleicher Code wie Network-Version:**

```cpp
// Auto-Skalierung
TJpgDec.getJpgSize(&width, &height, buffer, fileSize);

uint8_t scale = 1;
if (width > lcd.width() * 2) scale = 2;
if (width > lcd.width() * 4) scale = 4;
if (width > lcd.width() * 8) scale = 8;

TJpgDec.setJpgScale(scale);

// Dekodieren & Anzeigen
TJpgDec.drawJpg(0, 0, buffer, fileSize);
```

Siehe [15_Image_Viewer README](../15_Image_Viewer/README.md) für Details!

---

### 5. Performance-Vergleich: SD vs. Network

**Gleiche 125 KB JPEG-Datei:**

| Operation | Network (15) | SD (15b) | Speedup |
|-----------|--------------|----------|---------|
| Verbindung | ~500 ms | ~2 ms | **250x** |
| Download | ~400 ms | ~3 ms | **133x** |
| Decode | ~50 ms | ~50 ms | 1x |
| **Total** | **~950 ms** | **~55 ms** | **17x** |

**SD ist ~17x schneller!** 🚀

**Warum?**

```
Network:
  1. WiFi-Handshake
  2. HTTP-Request senden
  3. Server-Antwort warten
  4. Download über Internet
  → Viele Latenz-Punkte!

SD:
  1. Datei öffnen (SPI)
  2. Direkt lesen (SPI @ 25 MHz)
  → Nur lokale Hardware!
```

---

## 🚀 Experimente & Erweiterungen

### 1. 📁 Ordner-Ansicht

**Idee:** Zeige Liste von Ordnern, dann Bilder im gewählten Ordner.

```cpp
String folders[50];
int folderCount = 0;
String currentFolder = "/images";

void scanFolders() {
  File root = SD.open("/");
  File file = root.openNextFile();

  while (file) {
    if (file.isDirectory()) {
      folders[folderCount++] = String(file.name());
    }
    file = root.openNextFile();
  }
}

void drawFolderList() {
  lcd.fillScreen(COLOR_BG);

  lcd.setTextSize(2);
  lcd.setCursor(10, 10);
  lcd.print("Select Folder:");

  for (int i = 0; i < folderCount; i++) {
    lcd.setCursor(10, 40 + i * 30);
    lcd.print(folders[i]);
  }
}

void handleFolderTouch(int y) {
  int index = (y - 40) / 30;

  if (index >= 0 && index < folderCount) {
    currentFolder = "/" + folders[index];
    scanForImages();  // In diesem Ordner
    loadImage(0);
  }
}
```

---

### 2. 🖼️ Thumbnail-Grid

**Idee:** Zeige 3x3 Grid von Thumbnails, Touch zum Vergrößern.

```cpp
const int THUMB_SIZE = 70;
const int THUMB_SPACING = 10;
const int THUMBS_PER_ROW = 3;

LGFX_Sprite thumbnails[9];  // Max 9 sichtbar

void createThumbnailGrid(int startIndex) {
  lcd.fillScreen(COLOR_BG);

  for (int i = 0; i < 9 && (startIndex + i) < imageCount; i++) {
    // Sprite erstellen
    thumbnails[i].createSprite(THUMB_SIZE, THUMB_SIZE);
    thumbnails[i].fillSprite(0x0000);

    // Bild laden (kleine Version)
    File file = SD.open(imageFiles[startIndex + i]);
    int size = file.size();

    if (size < 150000) {
      uint8_t* buffer = (uint8_t*)malloc(size);
      file.read(buffer, size);
      file.close();

      // In Sprite dekodieren (skaliert)
      TJpgDec.setJpgScale(8);  // Sehr klein
      // ... decode in sprite

      free(buffer);
    }

    // Position berechnen
    int col = i % THUMBS_PER_ROW;
    int row = i / THUMBS_PER_ROW;

    int x = col * (THUMB_SIZE + THUMB_SPACING) + 10;
    int y = row * (THUMB_SIZE + THUMB_SPACING) + 40;

    // Auf Display zeichnen
    thumbnails[i].pushSprite(x, y);
  }
}

void handleThumbnailTouch(int touchX, int touchY) {
  int col = (touchX - 10) / (THUMB_SIZE + THUMB_SPACING);
  int row = (touchY - 40) / (THUMB_SIZE + THUMB_SPACING);

  int index = row * THUMBS_PER_ROW + col;

  if (index >= 0 && index < 9) {
    // Vergrößere dieses Bild
    currentImageIndex = index;
    loadImage(index);
  }
}
```

---

### 3. 📊 EXIF-Daten auslesen

**Idee:** Zeige Foto-Metadaten (Datum, Kamera, etc.)

```cpp
#include <TinyEXIF.h>

void showEXIF(const char* filepath) {
  File file = SD.open(filepath);
  int size = file.size();
  uint8_t* buffer = (uint8_t*)malloc(size);
  file.read(buffer, size);
  file.close();

  TinyEXIF::EXIFInfo exif(buffer, size);

  if (exif.Fields) {
    Serial.printf("Camera: %s %s\n", exif.Make.c_str(), exif.Model.c_str());
    Serial.printf("Date: %s\n", exif.DateTime.c_str());
    Serial.printf("Focal Length: %.1f mm\n", exif.FocalLength);
    Serial.printf("ISO: %d\n", exif.ISOSpeedRatings);
    Serial.printf("Shutter: 1/%.0f s\n", 1.0 / exif.ExposureTime);
    Serial.printf("Aperture: f/%.1f\n", exif.FNumber);

    // Auf Display anzeigen
    lcd.setCursor(5, 100);
    lcd.printf("%s", exif.DateTime.c_str());
    lcd.setCursor(5, 115);
    lcd.printf("%s %s", exif.Make.c_str(), exif.Model.c_str());
  }

  free(buffer);
}
```

**Installation:**
```
Arduino: TinyEXIF
PlatformIO: mobiledevices/TinyEXIF
```

---

### 4. 🎬 Video-Frames (MJPEG)

**Idee:** MJPEG-Video abspielen (Folge von JPEGs)

```cpp
// MJPEG = Motion JPEG (Video als JPEG-Sequenz)
// Format: video.mjpeg ist eigentlich viele JPEGs hintereinander

void playMJPEG(const char* filepath) {
  File file = SD.open(filepath);

  const int JPEG_START = 0xFFD8;
  const int JPEG_END = 0xFFD9;

  while (file.available()) {
    // Suche JPEG-Start (FF D8)
    while (file.read() != 0xFF) {}
    if (file.read() != 0xD8) continue;

    // JPEG-Anfang gefunden - suche Ende
    int startPos = file.position() - 2;

    while (file.available()) {
      if (file.read() == 0xFF && file.read() == 0xD9) {
        // JPEG-Ende gefunden
        int endPos = file.position();
        int jpegSize = endPos - startPos;

        // JPEG laden
        uint8_t* buffer = (uint8_t*)malloc(jpegSize);
        file.seek(startPos);
        file.read(buffer, jpegSize);

        // Anzeigen
        TJpgDec.drawJpg(0, 0, buffer, jpegSize);

        free(buffer);
        break;
      }
    }

    delay(33);  // ~30 FPS
  }

  file.close();
}
```

**MJPEG erstellen:**
```bash
# Mit ffmpeg aus Video
ffmpeg -i input.mp4 -vf scale=240:320 -q:v 5 output.mjpeg
```

---

### 5. 💾 Slideshow mit zufälliger Reihenfolge

**Idee:** Shuffle-Modus für Slideshow.

```cpp
void shuffleImages() {
  // Fisher-Yates Shuffle
  for (int i = imageCount - 1; i > 0; i--) {
    int j = random(0, i + 1);

    // Swap
    String temp = imageFiles[i];
    imageFiles[i] = imageFiles[j];
    imageFiles[j] = temp;
  }

  Serial.println("Images shuffled!");
}

bool slideshowActive = false;
unsigned long lastSlideChange = 0;
const int SLIDE_INTERVAL = 3000;  // 3 Sekunden

void loop() {
  if (slideshowActive) {
    if (millis() - lastSlideChange > SLIDE_INTERVAL) {
      currentImageIndex++;
      if (currentImageIndex >= imageCount) {
        shuffleImages();  // Neu mischen am Ende
        currentImageIndex = 0;
      }

      loadImage(currentImageIndex);
      lastSlideChange = millis();
    }
  }

  handleTouch();
}
```

---

### 6. 🔄 RAW-Format Unterstützung

**Idee:** Zeige RAW-Bilder (von Kamera)

**Problem:** RAW ist sehr komplex (proprietär)!

**Lösung:** Zeige Embedded JPEG-Preview aus RAW.

```cpp
// Die meisten RAW-Formate enthalten ein JPEG-Preview
// Dieses ist bei Byte-Offset X im File

// Beispiel für Canon CR2:
void showRAWPreview(const char* filepath) {
  File file = SD.open(filepath);

  // CR2: JPEG-Preview bei Offset ~20000 (variiert!)
  // Suche FF D8 (JPEG Start Marker)

  while (file.available()) {
    if (file.read() == 0xFF && file.peek() == 0xD8) {
      int jpegStart = file.position() - 1;

      // Suche Ende (FF D9)
      while (file.available()) {
        if (file.read() == 0xFF && file.peek() == 0xD9) {
          int jpegEnd = file.position() + 1;
          int jpegSize = jpegEnd - jpegStart;

          // JPEG extrahieren
          uint8_t* buffer = (uint8_t*)malloc(jpegSize);
          file.seek(jpegStart);
          file.read(buffer, jpegSize);
          file.close();

          // Anzeigen
          TJpgDec.drawJpg(0, 0, buffer, jpegSize);

          free(buffer);
          return;
        }
      }
    }
  }

  file.close();
  drawMessage("No preview found!", COLOR_ERROR);
}
```

---

### 7. 🗑️ Bild löschen (Langdruck)

**Idee:** Long-Press auf Bild = Löschen mit Bestätigung.

```cpp
unsigned long touchStartTime = 0;
const int LONG_PRESS_DURATION = 2000;  // 2 Sekunden

void handleTouch() {
  uint16_t x, y;
  bool touched = lcd.getTouch(&x, &y);

  static bool wasTouched = false;

  if (touched && !wasTouched) {
    // Touch Start
    touchStartTime = millis();
    wasTouched = true;

  } else if (touched && wasTouched) {
    // Touch Hold
    unsigned long holdDuration = millis() - touchStartTime;

    if (holdDuration > LONG_PRESS_DURATION) {
      // Long Press erkannt!
      showDeleteConfirmation();
      wasTouched = false;
    }

  } else if (!touched && wasTouched) {
    // Touch End (kurz)
    unsigned long holdDuration = millis() - touchStartTime;

    if (holdDuration < 500) {
      // Normal Tap - navigate
      // ...
    }

    wasTouched = false;
  }
}

void showDeleteConfirmation() {
  lcd.fillRect(0, 100, 240, 120, 0x0000);
  lcd.drawRect(10, 110, 220, 100, COLOR_ERROR);

  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_ERROR);
  lcd.setCursor(40, 125);
  lcd.print("Delete image?");

  lcd.setTextSize(1);
  lcd.setCursor(20, 155);
  lcd.printf("%s", imageFiles[currentImageIndex].c_str());

  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(30, 180);
  lcd.print("Touch LEFT: Yes");
  lcd.setCursor(30, 195);
  lcd.print("Touch RIGHT: No");

  // Warte auf Antwort
  delay(500);  // Debounce

  while (true) {
    uint16_t x, y;
    if (lcd.getTouch(&x, &y)) {
      if (x < TOUCH_ZONE_LEFT) {
        // YES - Löschen
        deleteCurrentImage();
        break;
      } else if (x > TOUCH_ZONE_RIGHT) {
        // NO - Abbrechen
        loadImage(currentImageIndex);
        break;
      }
    }
    delay(50);
  }
}

void deleteCurrentImage() {
  String filepath = imageFiles[currentImageIndex];

  if (SD.remove(filepath.c_str())) {
    Serial.printf("Deleted: %s\n", filepath.c_str());
    drawMessage("Image deleted!", COLOR_SUCCESS);
    delay(1000);

    // Neu scannen
    scanForImages();

    if (imageCount > 0) {
      if (currentImageIndex >= imageCount) {
        currentImageIndex = imageCount - 1;
      }
      loadImage(currentImageIndex);
    } else {
      drawMessage("No images left!", COLOR_ERROR);
    }

  } else {
    drawMessage("Delete failed!", COLOR_ERROR);
    delay(2000);
    loadImage(currentImageIndex);
  }
}
```

---

## 🐛 Troubleshooting

### Problem: "SD Card failed!"

**Symptom:** Display zeigt "SD Card failed! Check: SD inserted?..."

**Ursachen:**
1. SD-Karte nicht eingelegt
2. Falsche Formatierung (nicht FAT32)
3. Defekte SD-Karte
4. Schmutzige Kontakte
5. Falscher CS Pin

**Debug:**

```cpp
void setup() {
  Serial.begin(115200);

  Serial.println("Initializing SD...");
  Serial.printf("CS Pin: %d\n", SD_CS);

  if (!SD.begin(SD_CS)) {
    Serial.println("SD.begin() failed!");

    // Teste manuell
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, LOW);
    delay(10);
    digitalWrite(SD_CS, HIGH);

    Serial.println("Manual CS test done");
    return;
  }

  Serial.println("SD OK!");
}
```

**Lösung:**
- Überprüfe SD-Karte ist eingelegt
- Formatiere als FAT32
- Reinige Kontakte mit Alkohol
- Teste mit anderer SD-Karte
- Überprüfe CS Pin (sollte 5 sein)

---

### Problem: "No images found!"

**Symptom:** "Found 0 images"

**Ursachen:**
1. Falscher Ordner (nicht `/images`)
2. Keine JPEG-Dateien
3. Falsche Datei-Endung (z.B. `.JPG` statt `.jpg`)
4. Dateien im Root statt in Ordner

**Debug:**

```cpp
void debugSDContents() {
  File root = SD.open("/");
  File file = root.openNextFile();

  Serial.println("=== SD Card Contents ===");

  while (file) {
    if (file.isDirectory()) {
      Serial.printf("[DIR]  %s\n", file.name());
    } else {
      Serial.printf("[FILE] %s (%d bytes)\n", file.name(), file.size());
    }

    file = root.openNextFile();
  }

  Serial.println("========================");
}
```

**Lösung:**
- Erstelle Ordner `/images` auf SD
- Kopiere JPEG-Dateien in `/images/`
- Verwende kleine Dateinamen (8.3 Format wenn möglich)
- Überprüfe Datei-Endung (.jpg oder .jpeg)

---

### Problem: "Image too large! (>150 KB)"

**Symptom:** Meldung auf Display.

**Ursache:** Bild ist größer als RAM-Limit.

**Lösung 1:** Komprimiere Bilder vor Kopieren

```bash
# Mit ImageMagick
convert input.jpg -quality 80 -resize 1024x768 output.jpg

# Batch-Convert (alle Bilder im Ordner)
for f in *.jpg; do
  convert "$f" -quality 80 -resize 1024x768 "small_$f"
done
```

**Lösung 2:** Erhöhe RAM-Limit (riskant!)

```cpp
if (fileSize < 200000) {  // 200 KB statt 150
  // ...
}
```

**Aber:** ESP32 kann crashen!

**Lösung 3:** Implementiere Streaming (siehe nächstes Experiment)

---

### Problem: "Decode failed" oder verzerrtes Bild

**Symptom:** Bild wird nicht angezeigt oder ist verfärbt.

**Ursachen:**
1. Korrupte JPEG-Datei
2. Progressive JPEG (manchmal Probleme)
3. CMYK-Farbprofil (nicht unterstützt)

**Debug:**

```cpp
TJpgDec.getJpgSize(&w, &h, buffer, size);

Serial.printf("JPEG: %d x %d\n", w, h);

if (w == 0 || h == 0) {
  Serial.println("Invalid JPEG!");
  return;
}

int result = TJpgDec.drawJpg(0, 0, buffer, size);
Serial.printf("Draw result: %d\n", result);
// 0 = JDR_OK
// -1 = JDR_INP (Input error)
```

**Lösung:**
- Öffne JPEG auf PC (prüfe ob valide)
- Konvertiere neu: `convert input.jpg output.jpg`
- Verwende nur Baseline JPEG (nicht Progressive)
- Nur RGB-Farbprofil (nicht CMYK)

---

### Problem: "Sehr langsam beim Laden"

**Symptom:** 5+ Sekunden pro Bild.

**Ursachen:**
1. SD-Karte sehr langsam (Class 2/4)
2. Fragmentierte SD-Karte
3. Große Dateien

**Debug:**

```cpp
unsigned long start = millis();

File file = SD.open(filepath);
unsigned long openTime = millis() - start;

start = millis();
file.read(buffer, fileSize);
unsigned long readTime = millis() - start;

Serial.printf("Open: %lu ms, Read: %lu ms\n", openTime, readTime);
```

**Lösung:**
- Verwende schnelle SD-Karte (Class 10, UHS-I)
- Formatiere SD neu (defragmentiert)
- Komprimiere Bilder kleiner

**Erwartete Performance:**
```
Class 10 SD-Karte, 100 KB JPEG:
- Open: ~2 ms
- Read: ~3 ms
- Decode: ~50 ms
Total: ~55 ms
```

---

## 📚 Weiterführende Konzepte

### 1. File Streaming für große Bilder

**Problem:** Bilder >150 KB passen nicht in RAM.

**Lösung:** Streaming-Decoder (komplex!)

```cpp
// Custom Stream-Klasse für TJpg_Decoder
class SDFileStream {
public:
  File file;

  bool open(const char* path) {
    file = SD.open(path);
    return file;
  }

  void close() {
    file.close();
  }

  int read(uint8_t* buffer, int length) {
    return file.read(buffer, length);
  }

  int seek(int position) {
    return file.seek(position);
  }

  int available() {
    return file.available();
  }
};

// TJpg_Decoder anpassen (benötigt Library-Modifikation)
// ... komplex, siehe TJpg_Decoder Dokumentation
```

**Alternativ:** Verwende andere JPEG-Library die Streaming unterstützt.

---

### 2. SD-Karte über SPI vs. SDMMC

**Aktuell:** SPI-Modus (kompatibel, aber langsam)

**SDMMC-Modus:** Schneller (4-bit parallel)!

```cpp
#include <SD_MMC.h>

void setup() {
  // 1-bit Mode (schneller als SPI)
  if (!SD_MMC.begin("/sdcard", true)) {  // true = 1-bit mode
    Serial.println("SD_MMC failed!");
    return;
  }

  // 4-bit Mode (noch schneller, benötigt mehr Pins!)
  // if (!SD_MMC.begin()) {
  //   Serial.println("SD_MMC failed!");
  //   return;
  // }

  Serial.printf("Size: %llu MB\n", SD_MMC.cardSize() / (1024*1024));
}

// Rest identisch zu SD.h
File file = SD_MMC.open("/images/photo.jpg");
```

**Performance:**

| Modus | Speed | Pins |
|-------|-------|------|
| SPI | ~3 MB/s | 4 (MOSI, MISO, SCK, CS) |
| SDMMC 1-bit | ~10 MB/s | 3 (CMD, DAT0, CLK) |
| SDMMC 4-bit | ~40 MB/s | 6 (CMD, DAT0-3, CLK) |

**Aber:** CYD nutzt SPI-Pins bereits für Display → SDMMC schwierig!

---

### 3. Image Datenbank (SQLite)

**Idee:** Metadaten in Datenbank speichern.

```cpp
#include <sqlite3.h>

sqlite3* db;

void createDatabase() {
  sqlite3_open("/sd/images.db", &db);

  const char* sql = "CREATE TABLE IF NOT EXISTS images ("
                    "id INTEGER PRIMARY KEY,"
                    "filename TEXT,"
                    "size INTEGER,"
                    "width INTEGER,"
                    "height INTEGER,"
                    "date TEXT"
                    ");";

  sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
}

void addImage(const char* filename, int size, int width, int height) {
  char sql[256];
  sprintf(sql, "INSERT INTO images (filename, size, width, height, date) "
               "VALUES ('%s', %d, %d, %d, datetime('now'));",
               filename, size, width, height);

  sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
}

void searchImages(const char* keyword) {
  char sql[256];
  sprintf(sql, "SELECT filename FROM images WHERE filename LIKE '%%%s%%';", keyword);

  sqlite3_exec(db, sql, [](void* data, int argc, char** argv, char** azColName) {
    Serial.printf("Found: %s\n", argv[0]);
    return 0;
  }, nullptr, nullptr);
}
```

**Installation:**
```
PlatformIO: siara-cc/esp32_arduino_sqlite3_lib
```

---

### 4. Foto-Tagging & Kategorien

**Idee:** Manuelles Tagging von Bildern.

```cpp
// Tags in separater Datei speichern: /images/tags.txt
// Format: filename.jpg|tag1,tag2,tag3

void saveTag(const char* filename, const char* tag) {
  File file = SD.open("/images/tags.txt", FILE_APPEND);
  file.printf("%s|%s\n", filename, tag);
  file.close();
}

void loadTags(const char* filename, String& tags) {
  File file = SD.open("/images/tags.txt");

  while (file.available()) {
    String line = file.readStringUntil('\n');
    int pipePos = line.indexOf('|');

    String fn = line.substring(0, pipePos);
    String tg = line.substring(pipePos + 1);

    if (fn == filename) {
      tags = tg;
      break;
    }
  }

  file.close();
}

void filterByTag(const char* tag) {
  imageCount = 0;

  File file = SD.open("/images/tags.txt");

  while (file.available()) {
    String line = file.readStringUntil('\n');

    if (line.indexOf(tag) >= 0) {
      // Dieses Bild hat den Tag
      int pipePos = line.indexOf('|');
      String filename = line.substring(0, pipePos);

      imageFiles[imageCount++] = "/images/" + filename;
    }
  }

  file.close();
}
```

---

## 🎓 Was hast du gelernt?

Nach diesem Beispiel verstehst du:

- ✅ **SD-Karten-Zugriff** - SPI-Interface & Hardware
- ✅ **Dateisystem-Navigation** - Ordner durchsuchen, Dateien finden
- ✅ **File I/O** - Öffnen, Lesen, Schließen
- ✅ **Rekursive Algorithmen** - Unterordner durchsuchen
- ✅ **Performance-Optimierung** - SD vs. Network (~17x schneller!)
- ✅ **JPEG-Dekodierung** - Von Datei statt Stream
- ✅ **Memory-Management** - RAM-Limits & Workarounds

**Vergleich zu 15_Image_Viewer:**

| Aspekt | Network (15) | SD (15b) |
|--------|--------------|----------|
| **Speed** | ~950 ms | ~55 ms (**17x**) |
| **WiFi** | Erforderlich | Nicht nötig |
| **Offline** | Nein | Ja |
| **Max Größe** | ~150 KB (RAM) | ~150 KB (aktuell) |
| **Setup** | WiFi Config | SD vorbereiten |
| **Komplexität** | HTTP + WiFi | File I/O |

---

## 🔗 Siehe auch

- [15_Image_Viewer](../15_Image_Viewer/) - Network-Version (Vergleich!)
- [SD Library Docs](https://github.com/espressif/arduino-esp32/tree/master/libraries/SD)
- [TJpg_Decoder](https://github.com/Bodmer/TJpg_Decoder)

**Externe Ressourcen:**
- [FAT32 Format Guide](https://www.howtogeek.com/316977/how-to-format-usb-drives-larger-than-32gb-with-fat32-on-windows/)
- [EXIF Standard](https://en.wikipedia.org/wiki/Exif)

---

**Offline Bilder in Sekundenschnelle! 💾⚡**
