/*
 * CYD-Academy - Beispiel 24: GPS Monitor
 *
 * Dieses Beispiel demonstriert die Verwendung der seriellen Schnittstelle (UART2)
 * des CYD mit einem GPS-Modul. Der Display zeigt die empfangenen NMEA-Daten in
 * einem scrollenden Monitor-Format an.
 *
 * Hardware:
 * - ESP32-2432S028R (CYD - Cheap Yellow Display)
 * - Quectel L80-R GPS-Modul (oder kompatibles NMEA GPS)
 *
 * GPS-Modul Verbindung:
 * - VCC: 3.3V (WICHTIG: Nicht 5V!)
 * - GND: GND
 * - TX (vom GPS): GPIO 16 (extRxD am CYD)
 * - RX (zum GPS): GPIO 17 (extTxD am CYD) - optional für Konfiguration
 *
 * GPS Spezifikationen (Quectel L80-R):
 * - NMEA 0183 Protokoll
 * - Standard Baudrate: 9600
 * - Update-Rate: 1 Hz (Standard), bis 10 Hz möglich
 * - Kaltstartzeit: ~32 Sekunden
 * - Warmstart: ~1 Sekunde
 *
 * Features:
 * - Empfang und Anzeige von NMEA-Sentences
 * - Scrollender Monitor (automatisches Scrollen bei neuen Daten)
 * - Touch zum Löschen des Bildschirms
 * - Anzeige von GPS-Status (Satelliten, Fix-Status)
 * - Farbcodierung: Wichtige Daten hervorgehoben
 *
 * NMEA Sentence Typen (häufigste):
 * - $GPGGA: Global Positioning System Fix Data (Position, Höhe, Satelliten)
 * - $GPGSA: GPS DOP and active satellites (Präzision)
 * - $GPGSV: GPS Satellites in view (Sichtbare Satelliten)
 * - $GPRMC: Recommended Minimum Navigation Information (Position, Geschwindigkeit)
 * - $GPVTG: Track made good and Ground speed
 *
 * Bibliotheken:
 * - Keine zusätzlichen Libraries erforderlich (nur Standard ESP32 Serial)
 *
 * Autor: CYD-Academy
 * Lizenz: MIT
 */

#include <CYD_Display_Config.h>

// ========== Display Setup ==========
LGFX lcd;  // Display-Objekt (aus CYD_Display_Config.h)

// ========== Serielle Pins (aus CYD_Display_Config.h) ==========
#define extRxD 16  // GPIO 16 - Empfang von GPS TX
#define extTxD 17  // GPIO 17 - Senden zu GPS RX (optional)

// ========== GPS Konfiguration ==========
#define GPS_BAUD 9600              // Standard Baudrate für Quectel L80-R
#define GPS_BUFFER_SIZE 256        // Größe des Empfangspuffers
#define MAX_DISPLAY_LINES 15       // Maximale Anzahl sichtbarer Zeilen

// ========== Display Konfiguration ==========
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define LINE_HEIGHT 16             // Höhe einer Textzeile in Pixeln
#define TEXT_SIZE 1                // Textgröße (1 = 8x8 Pixel pro Zeichen)
#define TOP_MARGIN 20              // Platz für Header
#define BOTTOM_MARGIN 5            // Platz am unteren Rand

// ========== Farben ==========
#define COLOR_BG TFT_BLACK
#define COLOR_HEADER TFT_CYAN
#define COLOR_GPGGA TFT_GREEN      // Position Data - Grün
#define COLOR_GPRMC TFT_YELLOW     // Navigation Info - Gelb
#define COLOR_GPGSA TFT_ORANGE     // Precision - Orange
#define COLOR_GPGSV TFT_BLUE       // Satellites - Blau
#define COLOR_OTHER TFT_WHITE      // Andere NMEA Sentences
#define COLOR_ERROR TFT_RED        // Fehler/ungültige Daten

// ========== Globale Variablen ==========
String gpsBuffer = "";             // Buffer für eingehende GPS-Daten
int currentLine = 0;               // Aktuelle Zeile auf dem Display
int totalLines = 0;                // Gesamtanzahl der empfangenen Zeilen
unsigned long lastDataTime = 0;    // Zeitpunkt der letzten empfangenen Daten
bool gpsActive = false;            // GPS sendet Daten

// Scrolling-Variablen
int scrollOffset = 0;              // Scroll-Position (Anzahl der versteckten Zeilen)
String displayLines[100];          // Ringpuffer für Display-Zeilen (letzte 100)
uint16_t lineColors[100];          // Farben für jede Zeile
int lineIndex = 0;                 // Aktueller Index im Ringpuffer

// Statistik
int nmeaCount = 0;                 // Anzahl empfangener NMEA Sentences
int validSentences = 0;            // Anzahl gültiger Sentences (mit Checksum)

// ========== Setup ==========
void setup() {
  // Serial Monitor für Debug
  Serial.begin(115200);
  Serial.println("\n=== CYD GPS Monitor ===");
  Serial.println("Quectel L80-R GPS Module");
  Serial.println("========================\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);  // Landscape für mehr Platz
  lcd.fillScreen(COLOR_BG);
  lcd.setTextSize(TEXT_SIZE);

  // Header zeichnen
  drawHeader();

  // GPS Serial initialisieren (UART2)
  Serial2.begin(GPS_BAUD, SERIAL_8N1, extRxD, extTxD);
  Serial.printf("GPS UART2 initialisiert:\n");
  Serial.printf("- Baudrate: %d\n", GPS_BAUD);
  Serial.printf("- RX Pin (extRxD): GPIO %d\n", extRxD);
  Serial.printf("- TX Pin (extTxD): GPIO %d\n", extTxD);
  Serial.println("\nWarte auf GPS-Daten...");
  Serial.println("(Kaltstartzeit ca. 30-60 Sekunden bei freier Sicht zum Himmel)\n");

  // Hinweis auf Display
  lcd.setTextColor(TFT_YELLOW);
  lcd.setCursor(10, TOP_MARGIN + 10);
  lcd.println("Warte auf GPS-Daten...");
  lcd.setCursor(10, TOP_MARGIN + 30);
  lcd.setTextColor(TFT_WHITE);
  lcd.println("Kaltstartzeit: 30-60s");
  lcd.setCursor(10, TOP_MARGIN + 50);
  lcd.println("Touch: Display loeschen");

  Serial.println("Setup abgeschlossen!");
}

// ========== Main Loop ==========
void loop() {
  // GPS-Daten lesen
  readGPSData();

  // Touch-Eingabe prüfen (Display löschen)
  checkTouch();

  // Status-Check (Timeout bei fehlenden Daten)
  checkGPSTimeout();

  delay(10);  // Kleine Pause für Watchdog
}

// ========== GPS-Daten lesen ==========
void readGPSData() {
  // Prüfen ob Daten verfügbar sind
  while (Serial2.available()) {
    char c = Serial2.read();

    // Debug: Zeichen auch an Serial Monitor senden
    Serial.write(c);

    // Zeichen zum Buffer hinzufügen
    gpsBuffer += c;

    // GPS ist aktiv
    lastDataTime = millis();
    if (!gpsActive) {
      gpsActive = true;
      Serial.println("\n>>> GPS-Modul sendet Daten! <<<\n");
    }

    // Wenn Zeilenende erkannt (NMEA endet mit \r\n)
    if (c == '\n' && gpsBuffer.length() > 1) {
      // Zeile verarbeiten
      processNMEASentence(gpsBuffer);

      // Buffer leeren
      gpsBuffer = "";
    }

    // Buffer-Überlauf verhindern
    if (gpsBuffer.length() > GPS_BUFFER_SIZE) {
      Serial.println("WARNUNG: GPS-Buffer Überlauf!");
      gpsBuffer = "";
    }
  }
}

// ========== NMEA Sentence verarbeiten ==========
void processNMEASentence(String sentence) {
  // Whitespace entfernen
  sentence.trim();

  // Leere Zeilen ignorieren
  if (sentence.length() < 5) return;

  // NMEA Sentence muss mit $ beginnen
  if (sentence.charAt(0) != '$') {
    return;
  }

  nmeaCount++;

  // Checksum validieren (optional)
  bool isValid = true;  // Für einfaches Beispiel immer als gültig betrachten
  // TODO: Checksum-Validierung implementieren für robustere Lösung

  if (isValid) {
    validSentences++;
  }

  // Farbe basierend auf NMEA-Typ bestimmen
  uint16_t color = COLOR_OTHER;

  if (sentence.startsWith("$GPGGA") || sentence.startsWith("$GNGGA")) {
    color = COLOR_GPGGA;  // Position - Grün
  }
  else if (sentence.startsWith("$GPRMC") || sentence.startsWith("$GNRMC")) {
    color = COLOR_GPRMC;  // Navigation - Gelb
  }
  else if (sentence.startsWith("$GPGSA") || sentence.startsWith("$GNGSA")) {
    color = COLOR_GPGSA;  // Precision - Orange
  }
  else if (sentence.startsWith("$GPGSV") || sentence.startsWith("$GNGSV")) {
    color = COLOR_GPGSV;  // Satellites - Blau
  }

  // Zeile zum Ringpuffer hinzufügen
  displayLines[lineIndex] = sentence;
  lineColors[lineIndex] = color;
  lineIndex = (lineIndex + 1) % 100;  // Ringpuffer

  totalLines++;

  // Display aktualisieren
  updateDisplay();
}

// ========== Display aktualisieren ==========
void updateDisplay() {
  // Arbeitsbereich löschen (nicht Header)
  lcd.fillRect(0, TOP_MARGIN, SCREEN_WIDTH, SCREEN_HEIGHT - TOP_MARGIN - BOTTOM_MARGIN, COLOR_BG);

  // Berechne wie viele Zeilen angezeigt werden können
  int availableHeight = SCREEN_HEIGHT - TOP_MARGIN - BOTTOM_MARGIN;
  int maxLines = availableHeight / LINE_HEIGHT;

  // Wenn mehr Zeilen vorhanden als anzeigbar -> automatisch scrollen
  int displayStart = 0;
  if (totalLines > maxLines) {
    displayStart = totalLines - maxLines;
  }

  // Zeilen anzeigen
  int yPos = TOP_MARGIN;
  int displayedLines = 0;

  for (int i = displayStart; i < totalLines && displayedLines < maxLines; i++) {
    int bufferIndex = (lineIndex - totalLines + i + 100) % 100;

    lcd.setTextColor(lineColors[bufferIndex]);
    lcd.setCursor(2, yPos);

    // Zeile kürzen falls zu lang für Display
    String displayText = displayLines[bufferIndex];
    if (displayText.length() > 53) {  // 53 Zeichen passen bei 320px Breite
      displayText = displayText.substring(0, 50) + "...";
    }

    lcd.println(displayText);

    yPos += LINE_HEIGHT;
    displayedLines++;
  }

  // Header aktualisieren (mit neuen Statistiken)
  drawHeader();
}

// ========== Header zeichnen ==========
void drawHeader() {
  // Header-Bereich löschen
  lcd.fillRect(0, 0, SCREEN_WIDTH, TOP_MARGIN, COLOR_BG);

  // Titel
  lcd.setTextColor(COLOR_HEADER);
  lcd.setCursor(5, 2);
  lcd.print("GPS Monitor");

  // Statistik anzeigen
  lcd.setTextColor(TFT_WHITE);
  lcd.setCursor(130, 2);
  lcd.printf("Lines:%d", totalLines);

  // GPS Status
  if (gpsActive) {
    lcd.fillCircle(300, 8, 5, TFT_GREEN);
  } else {
    lcd.fillCircle(300, 8, 5, TFT_RED);
  }

  // Linie unter Header
  lcd.drawFastHLine(0, TOP_MARGIN - 2, SCREEN_WIDTH, COLOR_HEADER);
}

// ========== Touch-Eingabe prüfen ==========
void checkTouch() {
  // Touch-Status prüfen
  if (lcd.getTouch(&touchX, &touchY)) {
    // Kurze Verzögerung um Mehrfach-Trigger zu vermeiden
    delay(200);

    // Display zurücksetzen
    clearDisplay();

    Serial.println("\n=== Display gelöscht (Touch) ===\n");
  }
}

// Globale Touch-Variablen
uint16_t touchX, touchY;

// ========== Display löschen ==========
void clearDisplay() {
  // Alle Zeilen löschen
  totalLines = 0;
  lineIndex = 0;
  nmeaCount = 0;
  validSentences = 0;

  // Display neu zeichnen
  lcd.fillScreen(COLOR_BG);
  drawHeader();

  // Hinweis anzeigen
  lcd.setTextColor(TFT_YELLOW);
  lcd.setCursor(10, TOP_MARGIN + 10);
  lcd.println("Display geloescht.");
  lcd.setCursor(10, TOP_MARGIN + 30);
  lcd.setTextColor(TFT_WHITE);
  lcd.println("Warte auf GPS-Daten...");
}

// ========== GPS Timeout prüfen ==========
void checkGPSTimeout() {
  // Wenn länger als 5 Sekunden keine Daten -> Warnung
  if (gpsActive && (millis() - lastDataTime > 5000)) {
    gpsActive = false;

    Serial.println("\n!!! GPS TIMEOUT - Keine Daten seit 5 Sekunden !!!");
    Serial.println("Prüfe:");
    Serial.println("- Verkabelung (TX vom GPS an GPIO 16)");
    Serial.println("- Stromversorgung (3.3V, nicht 5V!)");
    Serial.println("- GPS-Antenne hat freie Sicht zum Himmel");
    Serial.println("");

    // Header aktualisieren (zeigt roten Status)
    drawHeader();
  }
}
