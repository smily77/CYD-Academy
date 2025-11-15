/*
  13_Racing_Track - Auto auf Rennstrecke mit Pfad-Animation

  Dieses Beispiel zeigt ein Auto, das einer Rennstrecke folgt - von oben betrachtet.

  Funktionen:
  - Geschwungene Rennstrecke (ähnlich einer 8)
  - Auto folgt dem Pfad automatisch
  - Touch für Beschleunigung/Bremsen
  - Rundenzeit-Messung
  - Sprite-Rotation basierend auf Fahrtrichtung

  Lernziele:
  - Komplexe Pfade zeichnen (Bézierkurven)
  - Pfad-basierte Animation (Interpolation)
  - Sprite-Rotation berechnen
  - Geschwindigkeits-Steuerung
  - Frame-Rate unabhängige Bewegung

  Hardware:
  - 1x CYD Display (2.8" oder 3.5")
  - Nur Touch-Input!

  Steuerung:
  - Touch oben: Beschleunigen
  - Touch unten: Bremsen
  - Kein Touch: Langsam abbremsen
*/

#include <CYD_Display_Config.h>

// ===== KONSTANTEN =====

// Farben
#define COLOR_GRASS 0x8BC34A     // Grünes Gras (angepasst von #8bc34a)
#define COLOR_ROAD_OUTER 0x4208  // Dunkler Asphalt (#424242)
#define COLOR_ROAD_INNER 0x630C  // Hellerer Asphalt (#616161)
#define COLOR_LINE 0xFFFF        // Weiße Mittellinie
#define COLOR_TREE 0x4CAF50      // Grüner Baum (angepasst)
#define COLOR_CAR 0xF800         // Rotes Auto
#define COLOR_TEXT 0xFFFF        // Weißer Text

// Pfad-Punkte (vorberechnete Samples entlang der Bézierkurve)
// Diese Punkte beschreiben die Rennstrecken-Mittellinie
const int PATH_POINTS = 64;  // Anzahl der Pfadpunkte
struct Point {
  float x, y;
};

Point trackPath[PATH_POINTS];

// Auto-Parameter
float carPosition = 0.0;     // Position auf Pfad (0.0 - 1.0)
float carSpeed = 0.0;        // Aktuelle Geschwindigkeit
const float MAX_SPEED = 0.015;
const float ACCELERATION = 0.0005;
const float BRAKE = 0.001;
const float FRICTION = 0.0002;

// Rundenzeit
unsigned long lapStartTime = 0;
unsigned long bestLapTime = 0;
int lapCount = 0;

// Anti-Flacker System
Point lastCarPos = {0, 0};
float lastCarAngle = 0;
int lastLapCount = -1;
float lastDisplayedSpeed = -1;
unsigned long lastUIUpdate = 0;
const int UI_UPDATE_INTERVAL = 100;  // UI nur alle 100ms aktualisieren

// Display
LGFX lcd;

// ===== SETUP =====

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Racing Track Demo ===");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(0);  // Portrait
  lcd.setBrightness(255);

  // Pfad generieren
  generateTrackPath();

  // Initiales Zeichnen (nur 1x!)
  drawTrack();

  // Erste Auto-Position merken
  lastCarPos = getPointOnPath(carPosition);
  lastCarAngle = getAngle(carPosition);

  lapStartTime = millis();

  Serial.println("Ready! Touch oben = Gas, unten = Bremse");
}

// ===== LOOP =====

void loop() {
  static unsigned long lastFrame = 0;
  unsigned long currentTime = millis();
  float deltaTime = (currentTime - lastFrame) / 1000.0;  // in Sekunden
  lastFrame = currentTime;

  // Touch lesen
  handleInput();

  // Geschwindigkeit aktualisieren (mit Reibung)
  if (carSpeed > 0) {
    carSpeed -= FRICTION;
    if (carSpeed < 0) carSpeed = 0;
  }

  // Position aktualisieren
  carPosition += carSpeed;

  // Runde erkennen
  if (carPosition >= 1.0) {
    carPosition -= 1.0;

    // Rundenzeit
    unsigned long lapTime = currentTime - lapStartTime;
    lapCount++;

    if (bestLapTime == 0 || lapTime < bestLapTime) {
      bestLapTime = lapTime;
    }

    Serial.printf("Runde %d: %.2f s (Best: %.2f s)\n",
                  lapCount, lapTime / 1000.0, bestLapTime / 1000.0);

    lapStartTime = currentTime;
  }

  // Nur Auto und UI neu zeichnen (Track bleibt!)
  drawCar();
  drawUI();

  delay(16);  // ~60 FPS
}

// ===== FUNKTIONEN =====

void generateTrackPath() {
  // Generiere Punkte entlang der Rennstrecke
  // Basierend auf der SVG-Pfad Beschreibung
  // Die Strecke ist eine geschwungene 8-förmige Kurve

  for (int i = 0; i < PATH_POINTS; i++) {
    float t = (float)i / PATH_POINTS;  // 0.0 bis 1.0

    // Parametrische Bézierkurven-Approximation
    // Vereinfachte 8-Form
    float angle = t * 2 * PI;

    // X: Sinuskurve für horizontale Bewegung
    float x = 120 + sin(angle) * 80;

    // Y: Doppelte Frequenz für 8-Form
    float y = 160 + sin(angle * 2) * 120;

    trackPath[i].x = x;
    trackPath[i].y = y;
  }
}

Point getPointOnPath(float t) {
  // Interpoliere zwischen Pfadpunkten
  t = fmod(t, 1.0);  // Wrap auf 0-1
  if (t < 0) t += 1.0;

  float index = t * PATH_POINTS;
  int i0 = (int)index;
  int i1 = (i0 + 1) % PATH_POINTS;
  float frac = index - i0;

  Point p;
  p.x = trackPath[i0].x + (trackPath[i1].x - trackPath[i0].x) * frac;
  p.y = trackPath[i0].y + (trackPath[i1].y - trackPath[i0].y) * frac;

  return p;
}

float getAngle(float t) {
  // Berechne Richtung (Tangente) am Punkt t
  Point p1 = getPointOnPath(t);
  Point p2 = getPointOnPath(t + 0.01);

  return atan2(p2.y - p1.y, p2.x - p1.x);
}

void handleInput() {
  uint16_t x, y;
  bool touched = lcd.getTouch(&x, &y);

  if (touched) {
    if (y < lcd.height() / 2) {
      // Obere Hälfte: Beschleunigen
      carSpeed += ACCELERATION;
      if (carSpeed > MAX_SPEED) carSpeed = MAX_SPEED;
    } else {
      // Untere Hälfte: Bremsen
      carSpeed -= BRAKE;
      if (carSpeed < 0) carSpeed = 0;
    }
  }
}

void drawTrack() {
  // Hintergrund (Gras)
  lcd.fillScreen(COLOR_GRASS);

  // Äußere Straße (dunkler Asphalt)
  for (int i = 0; i < PATH_POINTS; i++) {
    int next = (i + 1) % PATH_POINTS;
    lcd.fillCircle(trackPath[i].x, trackPath[i].y, 40, COLOR_ROAD_OUTER);
  }

  // Innere Straße (heller Asphalt)
  for (int i = 0; i < PATH_POINTS; i++) {
    lcd.fillCircle(trackPath[i].x, trackPath[i].y, 25, COLOR_ROAD_INNER);
  }

  // Mittellinie (gestrichelt)
  for (int i = 0; i < PATH_POINTS; i += 2) {
    int next = (i + 1) % PATH_POINTS;
    lcd.drawLine(trackPath[i].x, trackPath[i].y,
                 trackPath[next].x, trackPath[next].y, COLOR_LINE);
  }

  // Bäume/Dekoration (Kreise um die Strecke)
  lcd.fillCircle(30, 80, 8, 0x2D5A);   // Dunkleres Grün
  lcd.fillCircle(210, 80, 8, 0x2D5A);
  lcd.fillCircle(30, 240, 8, 0x2D5A);
  lcd.fillCircle(210, 240, 8, 0x2D5A);
  lcd.fillCircle(120, 20, 8, 0x2D5A);
  lcd.fillCircle(120, 300, 8, 0x2D5A);
}

void drawCar() {
  Point pos = getPointOnPath(carPosition);
  float angle = getAngle(carPosition);

  float carWidth = 8;
  float carLength = 14;

  // ===== ALTE POSITION LÖSCHEN =====
  // Prüfe ob Auto auf äußerer oder innerer Straße ist
  uint16_t eraseColor = COLOR_ROAD_INNER;  // Standardmäßig innere Straße

  // Lösche altes Auto (mit größerem Radius für sauberes Löschen)
  if (lastCarPos.x != 0 || lastCarPos.y != 0) {
    lcd.fillCircle(lastCarPos.x, lastCarPos.y, 12, eraseColor);
  }

  // ===== NEUE POSITION ZEICHNEN =====
  // Lokale Koordinaten (Auto zeigt nach rechts)
  float corners[4][2] = {
    {-carLength/2, -carWidth/2},  // Hinten Links
    {carLength/2, -carWidth/2},   // Vorne Links
    {carLength/2, carWidth/2},    // Vorne Rechts
    {-carLength/2, carWidth/2}    // Hinten Rechts
  };

  // Rotieren und transformieren
  int16_t rotated[4][2];
  for (int i = 0; i < 4; i++) {
    float rx = corners[i][0] * cos(angle) - corners[i][1] * sin(angle);
    float ry = corners[i][0] * sin(angle) + corners[i][1] * cos(angle);
    rotated[i][0] = pos.x + rx;
    rotated[i][1] = pos.y + ry;
  }

  // Auto zeichnen
  lcd.fillTriangle(rotated[0][0], rotated[0][1],
                   rotated[1][0], rotated[1][1],
                   rotated[2][0], rotated[2][1], COLOR_CAR);
  lcd.fillTriangle(rotated[0][0], rotated[0][1],
                   rotated[2][0], rotated[2][1],
                   rotated[3][0], rotated[3][1], COLOR_CAR);

  // Richtungs-Indikator (vordere Spitze)
  float frontX = pos.x + cos(angle) * (carLength/2 + 2);
  float frontY = pos.y + sin(angle) * (carLength/2 + 2);
  lcd.fillCircle(frontX, frontY, 2, 0xFFE0);  // Gelb

  // Aktuelle Position für nächstes Frame speichern
  lastCarPos = pos;
  lastCarAngle = angle;
}

void drawUI() {
  unsigned long currentTime = millis();

  // UI nur alle 100ms aktualisieren (weniger Flackern)
  if (currentTime - lastUIUpdate < UI_UPDATE_INTERVAL) {
    return;
  }
  lastUIUpdate = currentTime;

  // Geschwindigkeitsanzeige (nur bei Änderung)
  float currentSpeed = (carSpeed / MAX_SPEED) * 100;
  if (abs(currentSpeed - lastDisplayedSpeed) > 1.0) {  // Nur bei >1% Änderung
    lcd.setTextSize(1);
    lcd.setTextColor(COLOR_TEXT, COLOR_GRASS);
    lcd.setCursor(5, 5);
    lcd.printf("Speed: %3.0f%%", currentSpeed);
    lastDisplayedSpeed = currentSpeed;
  }

  // Aktuelle Rundenzeit (immer aktualisieren)
  unsigned long currentLapTime = millis() - lapStartTime;
  lcd.setTextColor(COLOR_TEXT, COLOR_GRASS);
  lcd.setCursor(5, 18);
  lcd.printf("Lap: %5.2f s", currentLapTime / 1000.0);

  // Runden-Counter (nur bei Änderung)
  if (lastLapCount != lapCount) {
    lcd.setCursor(5, 31);
    lcd.printf("Count: %d  ", lapCount);  // Extra Leerzeichen zum Löschen alter Ziffern
    lastLapCount = lapCount;

    // Beste Zeit (bei neuer Runde aktualisieren)
    if (bestLapTime > 0) {
      lcd.setCursor(5, 44);
      lcd.printf("Best: %.2f s", bestLapTime / 1000.0);
    }
  }

  // Touch-Hilfe (nur einmal zeichnen)
  static bool helpDrawn = false;
  if (!helpDrawn) {
    lcd.setTextColor(0xFFE0, COLOR_GRASS);  // Gelb
    lcd.setCursor(140, 5);
    lcd.print("Touch:");
    lcd.setCursor(140, 18);
    lcd.print("Top=Gas");
    lcd.setCursor(140, 31);
    lcd.print("Bot=Brake");
    helpDrawn = true;
  }
}
