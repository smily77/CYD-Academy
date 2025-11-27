/*
  23_3D_Dice_BNO055 - Interaktiver 3D-Würfel mit Orientierungssensor

  Dieses Beispiel zeigt einen 3D-Würfel mit Würfelaugen, der virtuell im Raum
  "schwebt". Das CYD-Board funktioniert wie eine Kamera, die du um den Würfel
  herum bewegen kannst. Drehe das Board nach rechts → siehst du den Würfel von
  rechts. Kippe es nach vorne → siehst du den Würfel von oben. Genau wie bei
  einem echten Objekt, das du von allen Seiten betrachten kannst!

  Funktionen:
  - BNO055 9-DoF Sensor für absolute Orientierung
  - 3D-Würfel-Rendering mit Perspektive
  - 6 verschiedenfarbige Seiten mit Würfelaugen (1-6)
  - Smooth Animation mit Exponential Smoothing
  - Back-face Culling für realistische Darstellung
  - Kalibrierungsstatus-Anzeige
  - Touch-Interaktion für Einstellungen

  Lernziele:
  - 3D-Grafik-Programmierung (Rotation, Projektion)
  - Matrix-Mathematik für 3D-Transformationen
  - Quaternionen & Euler-Winkel verstehen
  - Sensor-Fusion für Orientierung
  - Back-face Culling Algorithmus
  - Perspektiv-Projektion
  - Painter's Algorithm für Z-Sortierung

  Hardware:
  - 1x CYD Display (2.8" oder 3.5")
  - 1x BNO055 Absolute Orientation Sensor
  - 4x Dupont-Kabel (VCC, GND, SDA, SCL)

  BNO055 Verkabelung:
  - VCC → 3.3V (oder 5V, Sensor akzeptiert beides)
  - GND → GND
  - SDA → GPIO 22 (extSDA)
  - SCL → GPIO 27 (extSCL)
  - I2C-Adresse: 0x29 (Standard in diesem Code, alternativ 0x28)

  Installation:
  1. Adafruit BNO055 Library installieren:
     Arduino IDE: Library Manager → "Adafruit BNO055" → installieren
     (Installiert auch Adafruit Sensor & BusIO automatisch)

     PlatformIO: lib_deps = adafruit/Adafruit BNO055 @ ^1.6.0

  2. In CYD_Display_Config.h definieren (falls nicht vorhanden):
     #define extSDA 22
     #define extSCL 27

  Steuerung:
  - Bewege das CYD um den virtuellen Würfel herum (Board = Kamera)
    • Nach rechts neigen → Würfel von rechts betrachten
    • Nach vorne kippen → Würfel von oben betrachten
    • Drehen → Würfel von allen Seiten sehen
  - Touch Links: Demo-Rotation aktivieren/deaktivieren
  - Touch Rechts: Wireframe-Modus (nur Kanten)
  - Touch Mitte: Kalibrierungshilfe anzeigen
*/

#include <CYD_Display_Config.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

// I2C Pins (CYD Standard - external I2C)
#ifndef extSDA
#define extSDA 22
#endif

#ifndef extSCL
#define extSCL 27
#endif

// ===== SENSOR-ORIENTIERUNG KONFIGURATION =====
// WICHTIG: Passe diese Werte an deine BNO055-Montage an!
//
// Problem: Z- und X-Achse vertauscht? → Ändere Achsen-Mapping unten
// Problem: Falsche Rotations-Richtung? → Aktiviere entsprechende INVERT-Flags

// ACHSEN-MAPPING: Welche Sensor-Achse → Würfel-Achse?
// Standard-Konfiguration (AKTIVIERT):
#define CUBE_X_FROM_PITCH    // X-Achse des Würfels ← Pitch des Sensors
#define CUBE_Y_FROM_HEADING  // Y-Achse des Würfels ← Heading des Sensors
#define CUBE_Z_FROM_ROLL     // Z-Achse des Würfels ← Roll des Sensors

// Alternative Konfiguration (Z und X vertauscht):
// Falls benötigt, kommentiere oben aus und aktiviere diese:
// #define CUBE_X_FROM_ROLL     // X-Achse des Würfels ← Roll des Sensors
// #define CUBE_Y_FROM_HEADING  // Y-Achse des Würfels ← Heading des Sensors
// #define CUBE_Z_FROM_PITCH    // Z-Achse des Würfels ← Pitch des Sensors

// VORZEICHEN-INVERSIONEN: Dreht Achse falsch herum?
// WICHTIG: Diese Flags sind mathematisch problematisch mit Quaternionen!
// Verwende nur das Konjugat für die inverse Kamera-Rotation.
// #define INVERT_CUBE_X      // X-Achse umkehren
// #define INVERT_CUBE_Y      // Y-Achse umkehren (DEAKTIVIERT - stört Quaternion-Rotation)
// #define INVERT_CUBE_Z      // Z-Achse umkehren

// ===== KONSTANTEN =====

// Farben (Dark Theme)
#define COLOR_BG 0x0841          // Dunkles Blau-Grau
#define COLOR_TEXT 0xFFFF        // Weiß
#define COLOR_TEXT_DIM 0x7BEF    // Grau
#define COLOR_CAL_NONE 0xF800    // Rot (nicht kalibriert)
#define COLOR_CAL_LOW 0xFD20     // Orange (teilweise)
#define COLOR_CAL_MED 0xFFE0     // Gelb (mittel)
#define COLOR_CAL_FULL 0x07E0    // Grün (voll kalibriert)

// Würfel-Farben (traditionelle Würfelfarben: weiß mit schwarzen Augen)
#define DICE_FACE_1 0xF800       // Rot (1)
#define DICE_FACE_2 0x07E0       // Grün (2)
#define DICE_FACE_3 0x001F       // Blau (3)
#define DICE_FACE_4 0xFFE0       // Gelb (4)
#define DICE_FACE_5 0xF81F       // Magenta (5)
#define DICE_FACE_6 0x07FF       // Cyan (6)
#define DICE_EDGE 0x0000         // Schwarz (Kanten)
#define DICE_DOT 0x0000          // Schwarz (Punkte)

// 3D-Parameter
const float CUBE_SIZE = 60.0;           // Würfel-Größe (halbe Kantenlänge)
const float PERSPECTIVE_DISTANCE = 250.0; // Kamera-Distanz (größer = weniger Perspektive)
const int SCREEN_CENTER_X = 120;        // Bildschirm-Mitte X (240/2)
const int SCREEN_CENTER_Y = 140;        // Bildschirm-Mitte Y (etwas unten)

// Animation
const float SMOOTH_FACTOR = 0.15;       // Smoothing-Faktor (0.0-1.0)
const float DEMO_ROTATION_SPEED = 0.5;  // Grad pro Frame für Demo

// ===== STRUKTUREN =====

// 3D-Vektor
struct Vec3 {
  float x, y, z;

  Vec3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}

  // Vektor-Addition
  Vec3 operator+(const Vec3& v) const {
    return Vec3(x + v.x, y + v.y, z + v.z);
  }

  // Vektor-Subtraktion
  Vec3 operator-(const Vec3& v) const {
    return Vec3(x - v.x, y - v.y, z - v.z);
  }

  // Skalar-Multiplikation
  Vec3 operator*(float s) const {
    return Vec3(x * s, y * s, z * s);
  }

  // Länge
  float length() const {
    return sqrt(x*x + y*y + z*z);
  }

  // Normalisieren
  Vec3 normalize() const {
    float len = length();
    if (len > 0.0001) return Vec3(x/len, y/len, z/len);
    return Vec3(0, 0, 0);
  }

  // Kreuzprodukt
  Vec3 cross(const Vec3& v) const {
    return Vec3(
      y * v.z - z * v.y,
      z * v.x - x * v.z,
      x * v.y - y * v.x
    );
  }

  // Skalarprodukt
  float dot(const Vec3& v) const {
    return x * v.x + y * v.y + z * v.z;
  }
};

// 2D-Punkt (nach Projektion)
struct Point2D {
  int x, y;
  Point2D(int _x = 0, int _y = 0) : x(_x), y(_y) {}
};

// Würfel-Fläche
struct Face {
  int vertices[4];  // Indices der 4 Eckpunkte
  uint16_t color;   // Farbe der Fläche
  int diceValue;    // Würfel-Augenzahl (1-6)
  float avgZ;       // Durchschnittliche Z-Koordinate (für Sortierung)

  Face(int v0, int v1, int v2, int v3, uint16_t col, int dice) {
    vertices[0] = v0;
    vertices[1] = v1;
    vertices[2] = v2;
    vertices[3] = v3;
    color = col;
    diceValue = dice;
    avgZ = 0;
  }
};

// ===== GLOBALE VARIABLEN =====

LGFX lcd;
LGFX_Sprite cubeSprite(&lcd);  // Sprite für Double-Buffering (reduziert Flackern)
bool renderToSprite = false;   // Flag: zeichne auf Sprite statt lcd
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x29);

// Sensor-Status
bool sensorConnected = false;

// Würfel-Eckpunkte (8 Vertices)
Vec3 cubeVertices[8] = {
  Vec3(-1, -1, -1), Vec3( 1, -1, -1), Vec3( 1,  1, -1), Vec3(-1,  1, -1), // Hinten
  Vec3(-1, -1,  1), Vec3( 1, -1,  1), Vec3( 1,  1,  1), Vec3(-1,  1,  1)  // Vorne
};

// Transformierte Vertices (nach Rotation)
Vec3 transformedVertices[8];

// Projizierte 2D-Punkte
Point2D projectedPoints[8];

// Würfel-Flächen (6 Seiten, CCW von außen gesehen)
Face cubeFaces[6] = {
  Face(4, 5, 6, 7, DICE_FACE_1, 1), // Vorne (Z+) - 1
  Face(1, 0, 3, 2, DICE_FACE_6, 6), // Hinten (Z-) - 6
  Face(0, 4, 7, 3, DICE_FACE_2, 2), // Links (X-) - 2
  Face(5, 1, 2, 6, DICE_FACE_5, 5), // Rechts (X+) - 5
  Face(3, 7, 6, 2, DICE_FACE_3, 3), // Oben (Y+) - 3
  Face(0, 1, 5, 4, DICE_FACE_4, 4)  // Unten (Y-) - 4
};

// Orientierungs-Daten
float heading = 0.0;   // Yaw (0-360°)
float roll = 0.0;      // Roll (-180 bis +180°)
float pitch = 0.0;     // Pitch (-90 bis +90°)

// Letzte Werte für Delta-Berechnung (Debug)
float lastHeading = 0.0;
float lastPitch = 0.0;
float lastRoll = 0.0;

// Geglättete Orientierung (für Demo-Modus)
float smoothHeading = 0.0;
float smoothRoll = 0.0;
float smoothPitch = 0.0;

// Quaternionen (vermeidet Gimbal Lock - DAS IST DER FIX!)
imu::Quaternion currentQuat(1, 0, 0, 0);  // Identity Quaternion
imu::Quaternion smoothQuat(1, 0, 0, 0);

// Kalibrierungsstatus
uint8_t calSystem = 0;

// Demo-Modus
bool demoMode = false;
float demoAngle = 0.0;

// Touch-Handling
unsigned long lastTouchTime = 0;

// Display-Modus
bool showWireframe = false;

// ===== SETUP =====

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== 3D Dice BNO055 Demo ===");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(0);  // Portrait
  lcd.setBrightness(255);
  lcd.fillScreen(COLOR_BG);

  // Splash Screen
  drawSplashScreen();
  delay(2000);

  // I2C initialisieren
  Serial.printf("Initializing I2C (SDA=%d, SCL=%d)...\n", extSDA, extSCL);
  Wire.begin(extSDA, extSCL);

  // BNO055 initialisieren
  lcd.fillScreen(COLOR_BG);
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setTextDatum(MC_DATUM);
  lcd.drawString("Initializing", 120, 140);
  lcd.drawString("BNO055...", 120, 165);

  if (!bno.begin()) {
    Serial.println("BNO055 not found!");
    drawErrorScreen();
    sensorConnected = false;
    // Im Demo-Modus weiterlaufen
    demoMode = true;
  } else {
    sensorConnected = true;
    Serial.println("BNO055 found!");

    // Warte auf Sensor-Initialisierung
    delay(1000);

    // Setze zu externem Crystal (genauer)
    bno.setExtCrystalUse(true);

    Serial.println("BNO055 initialized!");
  }

  // Initiale Anzeige
  lcd.fillScreen(COLOR_BG);

  if (sensorConnected) {
    drawCalibrationHint();
    delay(3000);
  }

  // Sprite für Double-Buffering erstellen (reduziert Flackern bei gefüllten Flächen)
  Serial.println("Creating sprite buffer for smooth rendering...");
  cubeSprite.createSprite(240, 260);  // 240x260px Sprite (Würfel-Bereich + UI oben)
  cubeSprite.fillSprite(COLOR_BG);
  Serial.printf("Sprite created: 240x260px (%.1f KB RAM)\n", (240.0 * 260.0 * 2.0) / 1024.0);

  Serial.println("Setup complete! Move the board around the virtual dice (board = camera).");
  lcd.fillScreen(COLOR_BG);
}

// ===== LOOP =====

void loop() {
  // Sensor auslesen (oder Demo-Rotation)
  if (sensorConnected && !demoMode) {
    readSensor();
    smoothValues();
  } else {
    // Demo-Rotation
    demoAngle += DEMO_ROTATION_SPEED;
    if (demoAngle >= 360) demoAngle -= 360;
    smoothHeading = demoAngle;
    smoothRoll = sin(demoAngle * PI / 180.0) * 30;
    smoothPitch = cos(demoAngle * PI / 180.0) * 30;
  }

  // Touch-Handling
  handleTouch();

  // RENDERING: Sprite-basiert (gefüllt) oder direkt (wireframe)
  if (showWireframe) {
    // Wireframe-Modus: Direkt auf Display zeichnen (kein Flackern)
    int clearMargin = 115;
    lcd.fillRect(SCREEN_CENTER_X - clearMargin, SCREEN_CENTER_Y - clearMargin,
                 clearMargin * 2, clearMargin * 2, COLOR_BG);
    renderCube();  // Zeichnet direkt auf lcd
    drawUI();      // UI direkt auf lcd
  } else {
    // Filled-Modus: Double-Buffering mit Sprite (eliminiert Flackern!)
    cubeSprite.fillSprite(COLOR_BG);  // Clear sprite
    renderCubeToSprite();              // Zeichnet in Sprite
    cubeSprite.pushSprite(0, 0);       // Sprite → Display (atomare Operation)
    drawUI();                          // UI direkt auf lcd (über Sprite)
  }

  delay(20);  // ~50 FPS
}

// ===== SENSOR FUNKTIONEN =====

void readSensor() {
  // Quaternionen auslesen (vermeidet Gimbal Lock!)
  currentQuat = bno.getQuat();

  // Euler-Winkel nur für Debug ausgeben
  sensors_event_t event;
  bno.getEvent(&event);
  heading = event.orientation.x;  // 0-360°
  pitch = event.orientation.y;    // -180 bis +180°
  roll = event.orientation.z;     // -90 bis +90°

  // Kalibrierungsstatus
  uint8_t calGyro, calAccel, calMag;
  bno.getCalibration(&calSystem, &calGyro, &calAccel, &calMag);

  // Debug: Nur bewegte Achse ausgeben (für gezieltes Feedback)
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 200) {  // Schnellere Updates für besseres Tracking
    lastDebug = millis();

    // Berechne Deltas (Bewegung seit letztem Frame)
    float deltaH = abs(heading - lastHeading);
    float deltaP = abs(pitch - lastPitch);
    float deltaR = abs(roll - lastRoll);

    // Schwellwert für "signifikante Bewegung"
    const float threshold = 1.0;  // 1 Grad Änderung

    // Finde die Achse mit der größten Bewegung
    if (deltaH > threshold && deltaH > deltaP && deltaH > deltaR) {
      // Heading (Y-Achse im Würfel-Space) bewegt sich
      String direction = (heading > lastHeading) ? "POSITIV" : "NEGATIV";
      Serial.printf(">>> Y-ACHSE (Heading/Yaw) %s  [%.1f°]  Cal:%d\n",
                    direction.c_str(), heading, calSystem);
    }
    else if (deltaP > threshold && deltaP > deltaH && deltaP > deltaR) {
      // Pitch (X-Achse im Würfel-Space, da CUBE_X_FROM_PITCH) bewegt sich
      String direction = (pitch > lastPitch) ? "POSITIV" : "NEGATIV";
      Serial.printf(">>> X-ACHSE (Pitch) %s  [%.1f°]  Cal:%d\n",
                    direction.c_str(), pitch, calSystem);
    }
    else if (deltaR > threshold && deltaR > deltaP && deltaR > deltaH) {
      // Roll (Z-Achse im Würfel-Space, da CUBE_Z_FROM_ROLL) bewegt sich
      String direction = (roll > lastRoll) ? "POSITIV" : "NEGATIV";
      Serial.printf(">>> Z-ACHSE (Roll) %s  [%.1f°]  Cal:%d\n",
                    direction.c_str(), roll, calSystem);
    }

    // Speichere aktuelle Werte für nächsten Vergleich
    lastHeading = heading;
    lastPitch = pitch;
    lastRoll = roll;
  }
}

void smoothValues() {
  // Quaternion-Smoothing (SLERP - Spherical Linear Interpolation)
  // Einfache Version: Lineares Interpolieren und dann normalisieren

  smoothQuat.w() = smoothQuat.w() + (currentQuat.w() - smoothQuat.w()) * SMOOTH_FACTOR;
  smoothQuat.x() = smoothQuat.x() + (currentQuat.x() - smoothQuat.x()) * SMOOTH_FACTOR;
  smoothQuat.y() = smoothQuat.y() + (currentQuat.y() - smoothQuat.y()) * SMOOTH_FACTOR;
  smoothQuat.z() = smoothQuat.z() + (currentQuat.z() - smoothQuat.z()) * SMOOTH_FACTOR;

  // Normalisieren (wichtig für Quaternionen!)
  float length = sqrt(smoothQuat.w() * smoothQuat.w() +
                      smoothQuat.x() * smoothQuat.x() +
                      smoothQuat.y() * smoothQuat.y() +
                      smoothQuat.z() * smoothQuat.z());

  if (length > 0.0001) {
    smoothQuat.w() /= length;
    smoothQuat.x() /= length;
    smoothQuat.y() /= length;
    smoothQuat.z() /= length;
  }
}

// ===== TOUCH HANDLING =====

void handleTouch() {
  uint16_t x, y;
  if (!lcd.getTouch(&x, &y)) return;

  // Debouncing
  if (millis() - lastTouchTime < 500) return;
  lastTouchTime = millis();

  // Linker Bereich: Demo-Modus toggle
  if (x < 80) {
    demoMode = !demoMode;
    Serial.printf("Demo Mode: %s\n", demoMode ? "ON" : "OFF");

    // Feedback
    lcd.fillRect(5, 290, 70, 25, demoMode ? COLOR_CAL_FULL : COLOR_CAL_NONE);
    lcd.setTextSize(1);
    lcd.setTextColor(COLOR_BG);
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString(demoMode ? "DEMO ON" : "DEMO OFF", 40, 302);
    delay(500);
  }
  // Rechter Bereich: Wireframe toggle
  else if (x > 160) {
    showWireframe = !showWireframe;
    Serial.printf("Wireframe: %s\n", showWireframe ? "ON" : "OFF");

    // Feedback
    lcd.fillRect(165, 290, 70, 25, showWireframe ? COLOR_CAL_FULL : COLOR_CAL_NONE);
    lcd.setTextSize(1);
    lcd.setTextColor(COLOR_BG);
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString(showWireframe ? "WIRE ON" : "WIRE OFF", 200, 302);
    delay(500);
  }
  // Mitte: Kalibrierungshilfe
  else {
    if (sensorConnected) {
      drawCalibrationHint();
      delay(2000);
    }
  }

  lcd.fillScreen(COLOR_BG);
}

// ===== 3D RENDERING =====

void renderCube() {
  // 1. Rotation anwenden
  rotateCube();

  // 2. 3D → 2D Projektion
  projectVertices();

  // 3. Back-face Culling & Z-Sortierung
  calculateFaceDepths();
  sortFaces();

  // 4. Flächen zeichnen (auf lcd)
  renderToSprite = false;
  drawFaces();
}

void renderCubeToSprite() {
  // Gleicher Ablauf wie renderCube(), aber zeichnet in Sprite
  rotateCube();
  projectVertices();
  calculateFaceDepths();
  sortFaces();

  // Flächen in Sprite zeichnen
  renderToSprite = true;
  drawFaces();
}

void rotateCube() {
  // Quaternion-Rotation mit konfigurierbarer Achsen-Transformation
  // Vermeidet Gimbal Lock!

  // Erstelle Achsen-Transformations-Quaternion basierend auf Konfiguration
  imu::Quaternion transformedQuat = smoothQuat;

  // ACHSEN-PERMUTATION: Vertausche Quaternion-Komponenten für Achsen-Mapping
  // Standard: x=Pitch, y=Heading, z=Roll (BNO055)
  float qw = smoothQuat.w();
  float qx = smoothQuat.x();
  float qy = smoothQuat.y();
  float qz = smoothQuat.z();

  // Wende Achsen-Mapping an (tausche Komponenten)
  #if defined(CUBE_X_FROM_ROLL) && defined(CUBE_Z_FROM_PITCH)
    // Z und X vertauscht: Roll→X, Pitch→Z
    transformedQuat = imu::Quaternion(qw, qz, qy, qx);  // Tausche x und z
  #else
    // Standard: Pitch→X, Roll→Z
    transformedQuat = smoothQuat;
  #endif

  // VORZEICHEN-INVERSIONEN: Für Kamera-Effekt und falsche Richtung
  // Konjugat = Inverse Rotation (für "Board = Kamera")
  imu::Quaternion invQuat = transformedQuat.conjugate();

  // Zusätzliche Inversionen basierend auf Konfiguration
  #ifdef INVERT_CUBE_Y
    // Y-Achse extra invertieren (negiere y-Komponente)
    invQuat = imu::Quaternion(invQuat.w(), invQuat.x(), -invQuat.y(), invQuat.z());
  #endif

  #ifdef INVERT_CUBE_X
    // X-Achse extra invertieren (negiere x-Komponente)
    invQuat = imu::Quaternion(invQuat.w(), -invQuat.x(), invQuat.y(), invQuat.z());
  #endif

  #ifdef INVERT_CUBE_Z
    // Z-Achse extra invertieren (negiere z-Komponente)
    invQuat = imu::Quaternion(invQuat.w(), invQuat.x(), invQuat.y(), -invQuat.z());
  #endif

  // Für jeden Vertex: Quaternion-Rotation anwenden
  for (int i = 0; i < 8; i++) {
    Vec3 v = cubeVertices[i] * CUBE_SIZE;

    // Konvertiere Vec3 zu Quaternion (0, x, y, z)
    imu::Quaternion vQuat(0, v.x, v.y, v.z);

    // Rotation: q * v * q^(-1)
    imu::Quaternion rotated = invQuat * vQuat * invQuat.conjugate();

    // Extrahiere rotierte Koordinaten
    transformedVertices[i] = Vec3(rotated.x(), rotated.y(), rotated.z());
  }
}

void projectVertices() {
  // Perspektiv-Projektion: 3D → 2D
  for (int i = 0; i < 8; i++) {
    Vec3 v = transformedVertices[i];

    // Perspektiv-Division
    float z = v.z + PERSPECTIVE_DISTANCE;
    if (z < 10) z = 10;  // Vermeide Division durch 0

    float scale = PERSPECTIVE_DISTANCE / z;

    int x = SCREEN_CENTER_X + (int)(v.x * scale);
    int y = SCREEN_CENTER_Y - (int)(v.y * scale);  // Y invertiert (Display-Koordinaten)

    projectedPoints[i] = Point2D(x, y);
  }
}

void calculateFaceDepths() {
  // Berechne durchschnittliche Z-Tiefe für jede Fläche (für Painter's Algorithm)
  for (int i = 0; i < 6; i++) {
    float avgZ = 0;
    for (int j = 0; j < 4; j++) {
      avgZ += transformedVertices[cubeFaces[i].vertices[j]].z;
    }
    cubeFaces[i].avgZ = avgZ / 4.0;
  }
}

void sortFaces() {
  // Bubble Sort nach Z-Tiefe (von hinten nach vorne zeichnen)
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5 - i; j++) {
      if (cubeFaces[j].avgZ > cubeFaces[j + 1].avgZ) {
        // Swap
        Face temp = cubeFaces[j];
        cubeFaces[j] = cubeFaces[j + 1];
        cubeFaces[j + 1] = temp;
      }
    }
  }
}

void drawFaces() {
  // Zeichne Flächen von hinten nach vorne (auf lcd oder Sprite, je nach renderToSprite Flag)
  for (int i = 0; i < 6; i++) {
    Face& face = cubeFaces[i];

    // Back-face Culling (nur Flächen zeichnen die zur Kamera zeigen)
    if (!isFacingCamera(face)) continue;

    // Hole 2D-Punkte
    Point2D p[4];
    for (int j = 0; j < 4; j++) {
      p[j] = projectedPoints[face.vertices[j]];
    }

    if (showWireframe) {
      // Wireframe-Modus: Nur Kanten
      if (renderToSprite) {
        cubeSprite.drawLine(p[0].x, p[0].y, p[1].x, p[1].y, face.color);
        cubeSprite.drawLine(p[1].x, p[1].y, p[2].x, p[2].y, face.color);
        cubeSprite.drawLine(p[2].x, p[2].y, p[3].x, p[3].y, face.color);
        cubeSprite.drawLine(p[3].x, p[3].y, p[0].x, p[0].y, face.color);
      } else {
        lcd.drawLine(p[0].x, p[0].y, p[1].x, p[1].y, face.color);
        lcd.drawLine(p[1].x, p[1].y, p[2].x, p[2].y, face.color);
        lcd.drawLine(p[2].x, p[2].y, p[3].x, p[3].y, face.color);
        lcd.drawLine(p[3].x, p[3].y, p[0].x, p[0].y, face.color);
      }
    } else {
      // Gefülltes Polygon
      drawFilledQuad(p[0], p[1], p[2], p[3], face.color);

      // Kanten (schwarz)
      if (renderToSprite) {
        cubeSprite.drawLine(p[0].x, p[0].y, p[1].x, p[1].y, DICE_EDGE);
        cubeSprite.drawLine(p[1].x, p[1].y, p[2].x, p[2].y, DICE_EDGE);
        cubeSprite.drawLine(p[2].x, p[2].y, p[3].x, p[3].y, DICE_EDGE);
        cubeSprite.drawLine(p[3].x, p[3].y, p[0].x, p[0].y, DICE_EDGE);
      } else {
        lcd.drawLine(p[0].x, p[0].y, p[1].x, p[1].y, DICE_EDGE);
        lcd.drawLine(p[1].x, p[1].y, p[2].x, p[2].y, DICE_EDGE);
        lcd.drawLine(p[2].x, p[2].y, p[3].x, p[3].y, DICE_EDGE);
        lcd.drawLine(p[3].x, p[3].y, p[0].x, p[0].y, DICE_EDGE);
      }

      // Würfelaugen entfernt (Perspektive stimmt nicht in 3D)
      // drawDiceDots(p, face.diceValue);
    }
  }
}

bool isFacingCamera(const Face& face) {
  // Back-face Culling: Prüfe ob Fläche zur Kamera zeigt
  // Berechne Normale der Fläche (Cross-Product)

  Vec3 v0 = transformedVertices[face.vertices[0]];
  Vec3 v1 = transformedVertices[face.vertices[1]];
  Vec3 v2 = transformedVertices[face.vertices[2]];

  Vec3 edge1 = v1 - v0;
  Vec3 edge2 = v2 - v0;
  Vec3 normal = edge1.cross(edge2);

  // Vektor zur Kamera (Kamera ist bei Z = -PERSPECTIVE_DISTANCE)
  Vec3 faceCenter = (v0 + v1 + v2 + transformedVertices[face.vertices[3]]) * 0.25;
  Vec3 toCamera = Vec3(0, 0, -PERSPECTIVE_DISTANCE) - faceCenter;

  // Dot-Product: positiv = zeigt zur Kamera
  return normal.dot(toCamera) > 0;
}

void drawFilledQuad(Point2D p0, Point2D p1, Point2D p2, Point2D p3, uint16_t color) {
  // Zeichne gefülltes Viereck als zwei Dreiecke (auf lcd oder Sprite)
  if (renderToSprite) {
    cubeSprite.fillTriangle(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, color);
    cubeSprite.fillTriangle(p0.x, p0.y, p2.x, p2.y, p3.x, p3.y, color);
  } else {
    lcd.fillTriangle(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, color);
    lcd.fillTriangle(p0.x, p0.y, p2.x, p2.y, p3.x, p3.y, color);
  }
}

void drawDiceDots(Point2D p[4], int value) {
  // Berechne Mittelpunkt und Größe der Fläche
  int cx = (p[0].x + p[1].x + p[2].x + p[3].x) / 4;
  int cy = (p[0].y + p[1].y + p[2].y + p[3].y) / 4;

  // Berechne durchschnittliche Größe (Distanz vom Zentrum zu Ecken)
  float avgDist = 0;
  for (int i = 0; i < 4; i++) {
    float dx = p[i].x - cx;
    float dy = p[i].y - cy;
    avgDist += sqrt(dx*dx + dy*dy);
  }
  avgDist /= 4.0;

  // Punkt-Radius (proportional zur Flächengröße)
  int dotRadius = max(2, (int)(avgDist * 0.15));
  int spacing = (int)(avgDist * 0.5);

  // Zu klein? Nicht zeichnen
  if (dotRadius < 2 || spacing < 6) return;

  // Zeichne Würfelaugen basierend auf Wert
  switch (value) {
    case 1:
      // Mitte
      lcd.fillCircle(cx, cy, dotRadius, DICE_DOT);
      break;

    case 2:
      // Diagonal (links-oben, rechts-unten)
      lcd.fillCircle(cx - spacing/2, cy - spacing/2, dotRadius, DICE_DOT);
      lcd.fillCircle(cx + spacing/2, cy + spacing/2, dotRadius, DICE_DOT);
      break;

    case 3:
      // Diagonal + Mitte
      lcd.fillCircle(cx - spacing/2, cy - spacing/2, dotRadius, DICE_DOT);
      lcd.fillCircle(cx, cy, dotRadius, DICE_DOT);
      lcd.fillCircle(cx + spacing/2, cy + spacing/2, dotRadius, DICE_DOT);
      break;

    case 4:
      // Vier Ecken
      lcd.fillCircle(cx - spacing/2, cy - spacing/2, dotRadius, DICE_DOT);
      lcd.fillCircle(cx + spacing/2, cy - spacing/2, dotRadius, DICE_DOT);
      lcd.fillCircle(cx - spacing/2, cy + spacing/2, dotRadius, DICE_DOT);
      lcd.fillCircle(cx + spacing/2, cy + spacing/2, dotRadius, DICE_DOT);
      break;

    case 5:
      // Vier Ecken + Mitte
      lcd.fillCircle(cx - spacing/2, cy - spacing/2, dotRadius, DICE_DOT);
      lcd.fillCircle(cx + spacing/2, cy - spacing/2, dotRadius, DICE_DOT);
      lcd.fillCircle(cx, cy, dotRadius, DICE_DOT);
      lcd.fillCircle(cx - spacing/2, cy + spacing/2, dotRadius, DICE_DOT);
      lcd.fillCircle(cx + spacing/2, cy + spacing/2, dotRadius, DICE_DOT);
      break;

    case 6:
      // Zwei Spalten mit je 3 Punkten
      lcd.fillCircle(cx - spacing/2, cy - spacing/2, dotRadius, DICE_DOT);
      lcd.fillCircle(cx - spacing/2, cy, dotRadius, DICE_DOT);
      lcd.fillCircle(cx - spacing/2, cy + spacing/2, dotRadius, DICE_DOT);
      lcd.fillCircle(cx + spacing/2, cy - spacing/2, dotRadius, DICE_DOT);
      lcd.fillCircle(cx + spacing/2, cy, dotRadius, DICE_DOT);
      lcd.fillCircle(cx + spacing/2, cy + spacing/2, dotRadius, DICE_DOT);
      break;
  }
}

// ===== UI FUNKTIONEN =====

void drawUI() {
  // Kalibrierungs-Indikator (oben rechts)
  if (sensorConnected && !demoMode) {
    lcd.setTextSize(1);
    lcd.setTextColor(COLOR_TEXT_DIM);
    lcd.setTextDatum(TL_DATUM);
    lcd.drawString("CAL:", 180, 5);

    uint16_t calColor = getCalibrationColor(calSystem);
    lcd.fillCircle(215, 9, 6, calColor);
    lcd.setTextColor(COLOR_BG);
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString(String(calSystem), 215, 9);
  }

  // Demo-Modus Indikator
  if (demoMode) {
    lcd.setTextSize(1);
    lcd.setTextColor(COLOR_CAL_LOW);
    lcd.setTextDatum(TL_DATUM);
    lcd.drawString("DEMO", 5, 5);
  }

  // Orientierungs-Werte (unten)
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setTextDatum(BC_DATUM);

  char buf[40];
  sprintf(buf, "Y:%.0f R:%.0f P:%.0f", smoothHeading, smoothRoll, smoothPitch);
  lcd.drawString(buf, 120, 285);
}

uint16_t getCalibrationColor(uint8_t calValue) {
  switch (calValue) {
    case 0: return COLOR_CAL_NONE;  // Rot
    case 1: return COLOR_CAL_LOW;   // Orange
    case 2: return COLOR_CAL_MED;   // Gelb
    case 3: return COLOR_CAL_FULL;  // Grün
    default: return COLOR_CAL_NONE;
  }
}

// ===== BILDSCHIRME =====

void drawSplashScreen() {
  lcd.fillScreen(COLOR_BG);

  // Würfel-Symbol (einfach)
  int cx = 120, cy = 100;
  lcd.drawRect(cx - 30, cy - 30, 60, 60, COLOR_TEXT);
  lcd.drawRect(cx - 28, cy - 28, 56, 56, COLOR_TEXT);

  // Perspektiv-Linien
  lcd.drawLine(cx - 30, cy - 30, cx - 40, cy - 40, COLOR_TEXT);
  lcd.drawLine(cx + 30, cy - 30, cx + 40, cy - 40, COLOR_TEXT);
  lcd.drawLine(cx + 30, cy + 30, cx + 40, cy + 20, COLOR_TEXT);
  lcd.drawLine(cx - 30, cy + 30, cx - 40, cy + 20, COLOR_TEXT);

  lcd.drawRect(cx - 40, cy - 40, 60, 60, COLOR_TEXT);

  // Titel
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setTextDatum(MC_DATUM);
  lcd.drawString("3D DICE", 120, 180);
  lcd.drawString("BNO055", 120, 200);

  // Version
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.drawString("Example 23", 120, 230);
  lcd.drawString("Rotate to explore", 120, 250);
}

void drawErrorScreen() {
  lcd.fillScreen(COLOR_BG);

  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_CAL_NONE);
  lcd.setTextDatum(MC_DATUM);
  lcd.drawString("BNO055 Error!", 120, 80);

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setTextDatum(TL_DATUM);

  lcd.drawString("Check wiring:", 10, 120);
  lcd.drawString("SDA -> GPIO 22", 10, 140);
  lcd.drawString("SCL -> GPIO 27", 10, 155);
  lcd.drawString("VCC -> 3.3V/5V", 10, 170);
  lcd.drawString("GND -> GND", 10, 185);

  lcd.drawString("I2C Address:", 10, 210);
  lcd.drawString("0x28 or 0x29", 10, 225);

  lcd.setTextColor(COLOR_CAL_LOW);
  lcd.setTextDatum(MC_DATUM);
  lcd.drawString("Running in DEMO mode", 120, 260);

  delay(3000);
}

void drawCalibrationHint() {
  lcd.fillScreen(COLOR_BG);

  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setTextDatum(MC_DATUM);
  lcd.drawString("CALIBRATION", 120, 40);

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.setTextDatum(TL_DATUM);

  lcd.drawString("Move sensor slowly:", 10, 80);

  lcd.drawString("1. Figure-8 motion", 10, 105);
  lcd.drawString("   (Magnetometer)", 10, 120);

  lcd.drawString("2. Tilt all directions", 10, 145);
  lcd.drawString("   (Accel & Gyro)", 10, 160);

  lcd.drawString("3. Rotate fully", 10, 185);
  lcd.drawString("   around all axes", 10, 200);

  lcd.setTextColor(COLOR_TEXT);
  lcd.setTextDatum(MC_DATUM);
  lcd.drawString("Wait for GREEN indicator", 120, 235);

  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_DIM);
  lcd.drawString("Touch to continue", 120, 265);
}
