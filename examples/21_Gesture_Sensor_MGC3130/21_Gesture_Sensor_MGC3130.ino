/*
 * CYD-Academy - Beispiel 21: 3D-Gestensensor MGC3130
 *
 * Dieses Beispiel demonstriert berührungslose Gestensteuerung mit dem MGC3130 (GestIC).
 * Der Sensor erkennt 3D-Positionen, Wischgesten, kreisförmige Bewegungen und Touch-Events.
 *
 * Hardware:
 * - ESP32-2432S028R (CYD - Cheap Yellow Display)
 * - MGC3130 Gesture Sensor (z.B. Skywriter HAT, Grove Gesture Sensor)
 *
 * I2C Verbindung:
 * - SDA: GPIO 22 (extSDA)
 * - SCL: GPIO 27 (extSCL)
 * - I2C-Adresse: 0x42 (fest)
 *
 * Features:
 * - 3D-Positionserkennung (X, Y, Z)
 * - Gestenerkennung:
 *   - Swipe Links/Rechts/Oben/Unten
 *   - Kreisbewegungen (Uhrzeigersinn/Gegen)
 * - Touch-Events (5 Elektroden: N, S, E, W, Center)
 * - AirWheel (kreisförmige Drehgesten)
 * - 3D-Visualisierung mit Würfel
 * - Gesten-Animationen mit Icons
 * - Touch-Bereiche mit visueller Rückmeldung
 *
 * Bibliotheken:
 * - SparkFun MGC3130 Library
 *
 * Installation:
 * Arduino IDE -> Tools -> Manage Libraries -> "SparkFun MGC3130" installieren
 *
 * Autor: CYD-Academy
 * Lizenz: MIT
 */

#include <Wire.h>
#include <CYD_Display_Config.h>
#include <MGC3130.h>

// ========== Display Setup ==========
LGFX lcd;

// ========== MGC3130 Sensor ==========
MGC3130 gesture;

// I2C Pins vom CYD
#define extSDA 22
#define extSCL 27

// MGC3130 I2C-Adresse (fest: 0x42)
#define MGC3130_ADDR 0x42

// Optional: Transfer/Reset Pins (meist nicht benötigt bei I2C)
// #define TS_PIN -1
// #define RST_PIN -1

// ========== Display Modi ==========
enum DisplayMode {
  MODE_3D_POSITION = 0,  // 3D-Würfel mit Position
  MODE_GESTURES = 1,     // Gesten-Erkennung
  MODE_TOUCH = 2,        // Touch-Elektroden
  MODE_AIRWHEEL = 3      // AirWheel Rotation
};

DisplayMode currentMode = MODE_3D_POSITION;
const char* modeNames[] = {"3D Position", "Gesten", "Touch", "AirWheel"};

// ========== 3D Position ==========
uint16_t posX = 0;     // 0-65535
uint16_t posY = 0;     // 0-65535
uint16_t posZ = 0;     // 0-65535

// Normalisiert: 0.0 - 1.0
float normX = 0.5;
float normY = 0.5;
float normZ = 0.5;

// ========== Gesten ==========
String lastGesture = "Keine";
unsigned long gestureTime = 0;
#define GESTURE_DISPLAY_TIME 2000  // 2 Sekunden anzeigen

// Gesten-Typen
enum GestureType {
  GESTURE_NONE = 0,
  GESTURE_SWIPE_LEFT,
  GESTURE_SWIPE_RIGHT,
  GESTURE_SWIPE_UP,
  GESTURE_SWIPE_DOWN,
  GESTURE_CIRCLE_CW,
  GESTURE_CIRCLE_CCW
};

GestureType currentGesture = GESTURE_NONE;

// ========== Touch/Tap ==========
bool touchNorth = false;
bool touchSouth = false;
bool touchEast = false;
bool touchWest = false;
bool touchCenter = false;

String lastTouch = "Keine";
unsigned long touchTime = 0;

// ========== AirWheel ==========
int16_t airwheelAngle = 0;     // -1024 bis +1024
int16_t airwheelRotation = 0;  // Akkumuliert
bool airwheelActive = false;

// ========== Sensor Status ==========
bool sensorAvailable = false;
String statusMessage = "Initialisiere...";

// ========== Display-Farben ==========
#define COLOR_BACKGROUND TFT_BLACK
#define COLOR_TEXT TFT_WHITE
#define COLOR_ACCENT TFT_CYAN
#define COLOR_GESTURE TFT_YELLOW
#define COLOR_TOUCH TFT_GREEN
#define COLOR_AIRWHEEL TFT_MAGENTA
#define COLOR_CUBE TFT_BLUE
#define COLOR_POINT TFT_RED

// Touch-Buttons
struct TouchButton {
  int x, y, w, h;
  String label;
  uint16_t color;
};

TouchButton modeButtons[4] = {
  {5, 430, 75, 35, "3D", TFT_BLUE},
  {85, 430, 75, 35, "GEST", TFT_YELLOW},
  {165, 430, 75, 35, "TOUCH", TFT_GREEN},
  {245, 430, 75, 35, "WHEEL", TFT_MAGENTA}
};

// ========== Setup ==========
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== CYD 3D Gesture Sensor - MGC3130 ===");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(0);  // Portrait
  lcd.fillScreen(COLOR_BACKGROUND);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setTextDatum(TC_DATUM);

  // Willkommensbildschirm
  lcd.setTextSize(2);
  lcd.drawString("3D Gestensensor", 160, 100);
  lcd.setTextSize(1);
  lcd.drawString("MGC3130 GestIC", 160, 130);
  lcd.drawString("Initialisiere I2C...", 160, 200);

  // I2C initialisieren
  Wire.begin(extSDA, extSCL);
  Serial.printf("I2C initialisiert auf SDA: %d, SCL: %d\n", extSDA, extSCL);

  delay(1000);

  // MGC3130 initialisieren
  lcd.drawString("Suche MGC3130...", 160, 230);

  if (!gesture.begin(&Wire, MGC3130_ADDR)) {
    Serial.println("FEHLER: MGC3130 nicht gefunden!");
    lcd.fillScreen(COLOR_BACKGROUND);
    lcd.setTextColor(TFT_RED);
    lcd.drawString("MGC3130 FEHLER!", 160, 200);
    lcd.setTextColor(COLOR_TEXT);
    lcd.drawString("Prüfe Verkabelung:", 160, 240);
    lcd.drawString("SDA -> GPIO 22", 160, 260);
    lcd.drawString("SCL -> GPIO 27", 160, 280);
    lcd.drawString("VCC -> 3.3V", 160, 300);
    lcd.drawString("GND -> GND", 160, 320);

    statusMessage = "SENSOR FEHLT!";
    sensorAvailable = false;

    delay(3000);
  } else {
    Serial.println("MGC3130 gefunden!");
    sensorAvailable = true;

    // Sensor konfigurieren
    gesture.enableGestures(true);      // Gesten aktivieren
    gesture.enableAirWheel(true);      // AirWheel aktivieren
    gesture.enableTouch(true);         // Touch aktivieren
    gesture.enableApproach(true);      // Approach aktivieren

    Serial.println("MGC3130 konfiguriert:");
    Serial.println("- Gesten: Aktiviert");
    Serial.println("- AirWheel: Aktiviert");
    Serial.println("- Touch: Aktiviert");
    Serial.println("- 3D Position: Aktiviert");

    statusMessage = "Sensor bereit";
  }

  // Hauptbildschirm zeichnen
  drawMainScreen();

  Serial.println("Setup abgeschlossen!");
  Serial.println("Bewege deine Hand über dem Sensor (0-15 cm)");
}

// ========== Main Loop ==========
void loop() {
  static unsigned long lastUpdate = 0;

  // Sensor-Daten lesen
  if (sensorAvailable) {
    readSensor();
  }

  // Display aktualisieren (alle 50ms für flüssige Animation)
  if (millis() - lastUpdate >= 50) {
    updateDisplay();
    lastUpdate = millis();
  }

  // Touch-Eingabe verarbeiten
  handleTouch();

  delay(10);
}

// ========== Sensor auslesen ==========
void readSensor() {
  // Prüfen ob neue Daten verfügbar
  if (gesture.dataAvailable()) {
    // Position auslesen (wenn Tracking aktiv)
    if (gesture.positionAvailable()) {
      posX = gesture.getX();
      posY = gesture.getY();
      posZ = gesture.getZ();

      // Normalisieren (0-65535 -> 0.0-1.0)
      normX = posX / 65535.0;
      normY = posY / 65535.0;
      normZ = posZ / 65535.0;

      // Debug-Ausgabe (reduziert)
      static unsigned long lastPosDebug = 0;
      if (millis() - lastPosDebug >= 500) {
        Serial.printf("Position - X: %d, Y: %d, Z: %d\n", posX, posY, posZ);
        lastPosDebug = millis();
      }
    }

    // Gesten auslesen
    if (gesture.gestureAvailable()) {
      uint8_t gest = gesture.getGesture();

      switch (gest) {
        case GESTURE_WEST_EAST:  // Links -> Rechts
          lastGesture = "Swipe Rechts";
          currentGesture = GESTURE_SWIPE_RIGHT;
          gestureTime = millis();
          Serial.println("Geste: Swipe Rechts");
          break;

        case GESTURE_EAST_WEST:  // Rechts -> Links
          lastGesture = "Swipe Links";
          currentGesture = GESTURE_SWIPE_LEFT;
          gestureTime = millis();
          Serial.println("Geste: Swipe Links");
          break;

        case GESTURE_SOUTH_NORTH:  // Unten -> Oben
          lastGesture = "Swipe Hoch";
          currentGesture = GESTURE_SWIPE_UP;
          gestureTime = millis();
          Serial.println("Geste: Swipe Hoch");
          break;

        case GESTURE_NORTH_SOUTH:  // Oben -> Unten
          lastGesture = "Swipe Runter";
          currentGesture = GESTURE_SWIPE_DOWN;
          gestureTime = millis();
          Serial.println("Geste: Swipe Runter");
          break;

        case GESTURE_CLOCKWISE:
          lastGesture = "Kreis (CW)";
          currentGesture = GESTURE_CIRCLE_CW;
          gestureTime = millis();
          Serial.println("Geste: Kreis (Uhrzeigersinn)");
          break;

        case GESTURE_COUNTERCLOCKWISE:
          lastGesture = "Kreis (CCW)";
          currentGesture = GESTURE_CIRCLE_CCW;
          gestureTime = millis();
          Serial.println("Geste: Kreis (Gegen Uhrzeigersinn)");
          break;
      }
    }

    // Touch-Events auslesen
    if (gesture.touchAvailable()) {
      uint16_t touch = gesture.getTouchInfo();

      // Touch-Elektroden (einzeln)
      touchNorth = (touch & TOUCH_NORTH) != 0;
      touchSouth = (touch & TOUCH_SOUTH) != 0;
      touchEast = (touch & TOUCH_EAST) != 0;
      touchWest = (touch & TOUCH_WEST) != 0;
      touchCenter = (touch & TOUCH_CENTER) != 0;

      // Tap-Events
      if (touch & TAP_NORTH) {
        lastTouch = "Tap Nord";
        touchTime = millis();
        Serial.println("Touch: Tap Nord");
      } else if (touch & TAP_SOUTH) {
        lastTouch = "Tap Süd";
        touchTime = millis();
        Serial.println("Touch: Tap Süd");
      } else if (touch & TAP_EAST) {
        lastTouch = "Tap Ost";
        touchTime = millis();
        Serial.println("Touch: Tap Ost");
      } else if (touch & TAP_WEST) {
        lastTouch = "Tap West";
        touchTime = millis();
        Serial.println("Touch: Tap West");
      } else if (touch & TAP_CENTER) {
        lastTouch = "Tap Center";
        touchTime = millis();
        Serial.println("Touch: Tap Center");
      }

      // Double-Tap Events
      if (touch & DOUBLE_TAP_NORTH) {
        lastTouch = "Doppel Nord";
        touchTime = millis();
        Serial.println("Touch: Doppel Nord");
      } else if (touch & DOUBLE_TAP_SOUTH) {
        lastTouch = "Doppel Süd";
        touchTime = millis();
        Serial.println("Touch: Doppel Süd");
      } else if (touch & DOUBLE_TAP_EAST) {
        lastTouch = "Doppel Ost";
        touchTime = millis();
        Serial.println("Touch: Doppel Ost");
      } else if (touch & DOUBLE_TAP_WEST) {
        lastTouch = "Doppel West";
        touchTime = millis();
        Serial.println("Touch: Doppel West");
      } else if (touch & DOUBLE_TAP_CENTER) {
        lastTouch = "Doppel Center";
        touchTime = millis();
        Serial.println("Touch: Doppel Center");
      }
    }

    // AirWheel auslesen
    if (gesture.airWheelAvailable()) {
      int8_t delta = gesture.getAirWheelDelta();
      airwheelAngle += delta;
      airwheelRotation += delta;
      airwheelActive = true;

      // Wrap-around
      if (airwheelAngle > 360) airwheelAngle -= 360;
      if (airwheelAngle < 0) airwheelAngle += 360;

      Serial.printf("AirWheel: Delta=%d, Angle=%d\n", delta, airwheelAngle);

      // Timeout für Inaktivität
      static unsigned long lastAirwheelTime = 0;
      lastAirwheelTime = millis();

      // Nach 1s Inaktivität deaktivieren
      if (millis() - lastAirwheelTime > 1000) {
        airwheelActive = false;
      }
    }
  }

  // Gesten-Timeout (nach 2s ausblenden)
  if (millis() - gestureTime > GESTURE_DISPLAY_TIME) {
    currentGesture = GESTURE_NONE;
  }
}

// ========== Hauptbildschirm zeichnen ==========
void drawMainScreen() {
  lcd.fillScreen(COLOR_BACKGROUND);

  // Titel
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_ACCENT);
  lcd.setTextDatum(TC_DATUM);
  lcd.drawString("Gestensensor", 160, 5);

  // Modus-Name
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_DARKGREY);
  lcd.drawString(modeNames[currentMode], 160, 30);

  // Buttons zeichnen
  drawButtons();

  // Status-Zeile
  lcd.setTextDatum(TC_DATUM);
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_DARKGREY);
  lcd.drawString("Status:", 160, 470);
}

// ========== Display aktualisieren ==========
void updateDisplay() {
  // Je nach Modus verschiedene Anzeigen
  switch (currentMode) {
    case MODE_3D_POSITION:
      draw3DPosition();
      break;

    case MODE_GESTURES:
      drawGestures();
      break;

    case MODE_TOUCH:
      drawTouchPads();
      break;

    case MODE_AIRWHEEL:
      drawAirWheel();
      break;
  }

  // Status-Text
  lcd.setTextSize(1);
  lcd.setTextDatum(TC_DATUM);
  lcd.setTextColor(sensorAvailable ? TFT_GREEN : TFT_RED);
  lcd.fillRect(10, 470, 300, 10, COLOR_BACKGROUND);
  lcd.drawString(statusMessage, 160, 470);
}

// ========== 3D-Position zeichnen ==========
void draw3DPosition() {
  // Zeichenbereich löschen
  lcd.fillRect(0, 50, 320, 370, COLOR_BACKGROUND);

  // 3D-Würfel zeichnen (Isometrische Projektion)
  int centerX = 160;
  int centerY = 200;
  int cubeSize = 80;

  // Würfel-Kanten (vereinfachte Darstellung)
  lcd.drawRect(centerX - cubeSize, centerY - cubeSize / 2, cubeSize * 2, cubeSize, TFT_DARKGREY);

  // Diagonalen für 3D-Effekt
  lcd.drawLine(centerX - cubeSize, centerY - cubeSize / 2, centerX - cubeSize / 2, centerY - cubeSize, TFT_DARKGREY);
  lcd.drawLine(centerX + cubeSize, centerY - cubeSize / 2, centerX + cubeSize / 2, centerY - cubeSize, TFT_DARKGREY);
  lcd.drawLine(centerX - cubeSize, centerY + cubeSize / 2, centerX - cubeSize / 2, centerY + cubeSize, TFT_DARKGREY);
  lcd.drawLine(centerX + cubeSize, centerY + cubeSize / 2, centerX + cubeSize / 2, centerY + cubeSize, TFT_DARKGREY);

  lcd.drawRect(centerX - cubeSize / 2, centerY - cubeSize, cubeSize, cubeSize, TFT_DARKGREY);

  // Positionspunkt basierend auf 3D-Koordinaten
  // X: Links-Rechts, Y: Vorne-Hinten (Z-Achse auf Display), Z: Höhe
  int pointX = centerX + (normX - 0.5) * cubeSize * 2;
  int pointY = centerY + (normY - 0.5) * cubeSize * 1.5 - (normZ - 0.5) * cubeSize;

  // Punkt zeichnen (größer wenn näher)
  int pointSize = map(posZ, 0, 65535, 3, 15);
  lcd.fillCircle(pointX, pointY, pointSize, COLOR_POINT);

  // Linien vom Zentrum zum Punkt
  lcd.drawLine(centerX, centerY, pointX, pointY, COLOR_ACCENT);

  // Koordinaten anzeigen
  lcd.setTextDatum(TL_DATUM);
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT);

  lcd.drawString("X: " + String((int)(normX * 100)) + "%", 10, 60);
  lcd.drawString("Y: " + String((int)(normY * 100)) + "%", 10, 80);
  lcd.drawString("Z: " + String((int)(normZ * 100)) + "%", 10, 100);

  // Balken für X, Y, Z
  drawBar(10, 130, 100, 15, normX, TFT_RED, "X");
  drawBar(10, 150, 100, 15, normY, TFT_GREEN, "Y");
  drawBar(10, 170, 100, 15, normZ, TFT_BLUE, "Z");

  // Hinweis
  lcd.setTextDatum(TC_DATUM);
  lcd.setTextColor(TFT_DARKGREY);
  lcd.drawString("Bewege Hand über Sensor", 160, 380);
  lcd.drawString("(0-15 cm Abstand)", 160, 400);
}

// ========== Balken zeichnen ==========
void drawBar(int x, int y, int w, int h, float value, uint16_t color, String label) {
  // Rahmen
  lcd.drawRect(x, y, w, h, TFT_DARKGREY);

  // Balken
  int barWidth = value * (w - 2);
  lcd.fillRect(x + 1, y + 1, barWidth, h - 2, color);

  // Hintergrund
  if (barWidth < w - 2) {
    lcd.fillRect(x + 1 + barWidth, y + 1, w - 2 - barWidth, h - 2, COLOR_BACKGROUND);
  }

  // Label
  lcd.setTextDatum(ML_DATUM);
  lcd.setTextColor(TFT_DARKGREY);
  lcd.drawString(label, x + w + 5, y + h / 2);
}

// ========== Gesten zeichnen ==========
void drawGestures() {
  // Zeichenbereich löschen
  lcd.fillRect(0, 50, 320, 370, COLOR_BACKGROUND);

  // Aktuelle Geste anzeigen
  if (currentGesture != GESTURE_NONE && millis() - gestureTime < GESTURE_DISPLAY_TIME) {
    drawGestureIcon(currentGesture);
  } else {
    // Wartezustand
    lcd.setTextDatum(TC_DATUM);
    lcd.setTextSize(2);
    lcd.setTextColor(TFT_DARKGREY);
    lcd.drawString("Wische mit der Hand", 160, 180);
    lcd.setTextSize(1);
    lcd.drawString("Links / Rechts / Hoch / Runter", 160, 210);
    lcd.drawString("oder mache Kreisbewegungen", 160, 230);
  }

  // Letzte Geste anzeigen
  lcd.setTextDatum(TC_DATUM);
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT);
  lcd.drawString("Letzte Geste:", 160, 360);
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_GESTURE);
  lcd.drawString(lastGesture, 160, 380);
}

// ========== Gesten-Icon zeichnen ==========
void drawGestureIcon(GestureType gest) {
  int centerX = 160;
  int centerY = 180;
  int arrowSize = 60;

  lcd.setTextDatum(TC_DATUM);
  lcd.setTextSize(3);
  lcd.setTextColor(COLOR_GESTURE);

  switch (gest) {
    case GESTURE_SWIPE_LEFT:
      // Pfeil nach links
      lcd.fillTriangle(centerX - arrowSize, centerY,
                       centerX + arrowSize / 2, centerY - arrowSize,
                       centerX + arrowSize / 2, centerY + arrowSize,
                       COLOR_GESTURE);
      lcd.drawString("<", centerX, centerY - 20);
      break;

    case GESTURE_SWIPE_RIGHT:
      // Pfeil nach rechts
      lcd.fillTriangle(centerX + arrowSize, centerY,
                       centerX - arrowSize / 2, centerY - arrowSize,
                       centerX - arrowSize / 2, centerY + arrowSize,
                       COLOR_GESTURE);
      lcd.drawString(">", centerX, centerY - 20);
      break;

    case GESTURE_SWIPE_UP:
      // Pfeil nach oben
      lcd.fillTriangle(centerX, centerY - arrowSize,
                       centerX - arrowSize, centerY + arrowSize / 2,
                       centerX + arrowSize, centerY + arrowSize / 2,
                       COLOR_GESTURE);
      lcd.drawString("^", centerX - 10, centerY - 20);
      break;

    case GESTURE_SWIPE_DOWN:
      // Pfeil nach unten
      lcd.fillTriangle(centerX, centerY + arrowSize,
                       centerX - arrowSize, centerY - arrowSize / 2,
                       centerX + arrowSize, centerY - arrowSize / 2,
                       COLOR_GESTURE);
      lcd.drawString("v", centerX - 10, centerY);
      break;

    case GESTURE_CIRCLE_CW:
      // Kreis (Uhrzeigersinn)
      lcd.drawCircle(centerX, centerY, arrowSize, COLOR_GESTURE);
      lcd.drawString("CW", centerX - 20, centerY - 15);
      // Pfeilspitze
      lcd.fillTriangle(centerX + arrowSize, centerY,
                       centerX + arrowSize - 15, centerY - 15,
                       centerX + arrowSize - 15, centerY + 15,
                       COLOR_GESTURE);
      break;

    case GESTURE_CIRCLE_CCW:
      // Kreis (Gegen Uhrzeigersinn)
      lcd.drawCircle(centerX, centerY, arrowSize, COLOR_GESTURE);
      lcd.drawString("CCW", centerX - 30, centerY - 15);
      // Pfeilspitze
      lcd.fillTriangle(centerX - arrowSize, centerY,
                       centerX - arrowSize + 15, centerY - 15,
                       centerX - arrowSize + 15, centerY + 15,
                       COLOR_GESTURE);
      break;

    default:
      break;
  }
}

// ========== Touch-Pads zeichnen ==========
void drawTouchPads() {
  // Zeichenbereich löschen
  lcd.fillRect(0, 50, 320, 370, COLOR_BACKGROUND);

  // Touch-Layout (wie auf MGC3130 Sensor)
  //       Nord
  //   West  Center  East
  //       Süd

  int centerX = 160;
  int centerY = 200;
  int padSize = 60;
  int padSpacing = 80;

  // Center
  uint16_t colorCenter = touchCenter ? TFT_GREEN : TFT_DARKGREY;
  lcd.fillCircle(centerX, centerY, padSize / 2, colorCenter);
  lcd.drawCircle(centerX, centerY, padSize / 2, COLOR_TEXT);
  lcd.setTextDatum(MC_DATUM);
  lcd.setTextColor(COLOR_BACKGROUND);
  lcd.drawString("C", centerX, centerY);

  // Nord (oben)
  uint16_t colorNorth = touchNorth ? TFT_GREEN : TFT_DARKGREY;
  lcd.fillCircle(centerX, centerY - padSpacing, padSize / 2, colorNorth);
  lcd.drawCircle(centerX, centerY - padSpacing, padSize / 2, COLOR_TEXT);
  lcd.setTextColor(COLOR_BACKGROUND);
  lcd.drawString("N", centerX, centerY - padSpacing);

  // Süd (unten)
  uint16_t colorSouth = touchSouth ? TFT_GREEN : TFT_DARKGREY;
  lcd.fillCircle(centerX, centerY + padSpacing, padSize / 2, colorSouth);
  lcd.drawCircle(centerX, centerY + padSpacing, padSize / 2, COLOR_TEXT);
  lcd.setTextColor(COLOR_BACKGROUND);
  lcd.drawString("S", centerX, centerY + padSpacing);

  // West (links)
  uint16_t colorWest = touchWest ? TFT_GREEN : TFT_DARKGREY;
  lcd.fillCircle(centerX - padSpacing, centerY, padSize / 2, colorWest);
  lcd.drawCircle(centerX - padSpacing, centerY, padSize / 2, COLOR_TEXT);
  lcd.setTextColor(COLOR_BACKGROUND);
  lcd.drawString("W", centerX - padSpacing, centerY);

  // East (rechts)
  uint16_t colorEast = touchEast ? TFT_GREEN : TFT_DARKGREY;
  lcd.fillCircle(centerX + padSpacing, centerY, padSize / 2, colorEast);
  lcd.drawCircle(centerX + padSpacing, centerY, padSize / 2, COLOR_TEXT);
  lcd.setTextColor(COLOR_BACKGROUND);
  lcd.drawString("E", centerX + padSpacing, centerY);

  // Letzter Touch
  lcd.setTextDatum(TC_DATUM);
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT);
  lcd.drawString("Letzter Touch:", 160, 340);
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TOUCH);
  lcd.drawString(lastTouch, 160, 360);

  // Hinweis
  lcd.setTextSize(1);
  lcd.setTextDatum(TC_DATUM);
  lcd.setTextColor(TFT_DARKGREY);
  lcd.drawString("Berühre Sensor oder Tap/Doppel-Tap", 160, 390);
}

// ========== AirWheel zeichnen ==========
void drawAirWheel() {
  // Zeichenbereich löschen
  lcd.fillRect(0, 50, 320, 370, COLOR_BACKGROUND);

  // Großes Rad zeichnen
  int centerX = 160;
  int centerY = 200;
  int wheelRadius = 80;

  // Äußerer Kreis
  lcd.drawCircle(centerX, centerY, wheelRadius, COLOR_AIRWHEEL);
  lcd.drawCircle(centerX, centerY, wheelRadius - 5, COLOR_AIRWHEEL);

  // Innerer Kreis
  lcd.drawCircle(centerX, centerY, 20, COLOR_AIRWHEEL);

  // Speichen (basierend auf Winkel)
  for (int i = 0; i < 8; i++) {
    float angle = (i * 45 + airwheelAngle) * PI / 180.0;
    int x1 = centerX + cos(angle) * 25;
    int y1 = centerY + sin(angle) * 25;
    int x2 = centerX + cos(angle) * (wheelRadius - 10);
    int y2 = centerY + sin(angle) * (wheelRadius - 10);
    lcd.drawLine(x1, y1, x2, y2, COLOR_AIRWHEEL);
  }

  // Pfeil für Rotationsrichtung
  float arrowAngle = airwheelAngle * PI / 180.0;
  int arrowX = centerX + cos(arrowAngle) * wheelRadius;
  int arrowY = centerY + sin(arrowAngle) * wheelRadius;
  lcd.fillCircle(arrowX, arrowY, 8, TFT_RED);

  // Winkel anzeigen
  lcd.setTextDatum(TC_DATUM);
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);
  lcd.drawString(String(airwheelAngle) + "°", centerX, centerY - 10);

  // Rotation total
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_DARKGREY);
  lcd.drawString("Gesamt: " + String(airwheelRotation), centerX, centerY + 15);

  // Status
  lcd.setTextDatum(TC_DATUM);
  lcd.setTextColor(airwheelActive ? TFT_GREEN : TFT_DARKGREY);
  lcd.drawString(airwheelActive ? "AKTIV" : "INAKTIV", 160, 340);

  // Hinweis
  lcd.setTextColor(TFT_DARKGREY);
  lcd.drawString("Mache kreisförmige Bewegungen", 160, 370);
  lcd.drawString("über dem Sensor", 160, 390);
}

// ========== Buttons zeichnen ==========
void drawButtons() {
  for (int i = 0; i < 4; i++) {
    TouchButton btn = modeButtons[i];
    uint16_t color = (i == currentMode) ? TFT_WHITE : btn.color;
    uint16_t textColor = (i == currentMode) ? TFT_BLACK : TFT_WHITE;

    lcd.fillRoundRect(btn.x, btn.y, btn.w, btn.h, 5, color);
    lcd.drawRoundRect(btn.x, btn.y, btn.w, btn.h, 5, TFT_WHITE);

    lcd.setTextDatum(MC_DATUM);
    lcd.setTextSize(1);
    lcd.setTextColor(textColor);
    lcd.drawString(btn.label, btn.x + btn.w / 2, btn.y + btn.h / 2);
  }
}

// ========== Touch-Handler (Display) ==========
void handleTouch() {
  uint16_t x, y;
  if (!lcd.getTouch(&x, &y)) return;

  // Mode-Buttons
  for (int i = 0; i < 4; i++) {
    TouchButton btn = modeButtons[i];
    if (x >= btn.x && x <= btn.x + btn.w && y >= btn.y && y <= btn.y + btn.h) {
      if (currentMode != (DisplayMode)i) {
        currentMode = (DisplayMode)i;
        drawMainScreen();
        Serial.printf("Modus gewechselt: %s\n", modeNames[currentMode]);
      }
      delay(200);
      return;
    }
  }
}
