/*
 * CYD Academy - Bouncing Ball Beispiel
 *
 * Dieses Beispiel demonstriert Animationen und Physik-Simulation:
 * - Bouncing Ball mit Gravitation
 * - Mehrere Bälle
 * - Kollisionserkennung mit Wänden
 * - FPS-Anzeige
 * - Touch-Interaktion (neue Bälle spawnen)
 *
 * Hardware: CYD 2.8" oder 3.5" ESP32 Display
 * Library: LovyanGFX
 */

#include <CYD_Display_Config.h>

// Display-Objekt erstellen
static LGFX lcd;

// Display-Auflösung (wird dynamisch ermittelt)
int SCREEN_WIDTH;
int SCREEN_HEIGHT;

// Ball-Struktur
struct Ball {
  float x, y;           // Position
  float vx, vy;         // Geschwindigkeit
  float radius;         // Radius
  uint16_t color;       // Farbe
  bool active;          // Ist Ball aktiv?
};

// Array von Bällen
const int MAX_BALLS = 10;
Ball balls[MAX_BALLS];

// Physik-Konstanten
const float GRAVITY = 0.5;
const float BOUNCE_DAMPING = 0.8;  // Energie-Verlust beim Aufprall
const float FRICTION = 0.99;       // Luft-Reibung

// FPS-Zähler
unsigned long lastFpsUpdate = 0;
int frameCount = 0;
int fps = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("CYD Bouncing Ball Demo gestartet!");

  // Display initialisieren
  lcd.init();

  // Display-Auflösung ermitteln
  SCREEN_WIDTH = lcd.width();
  SCREEN_HEIGHT = lcd.height();

  Serial.print("Display-Auflösung: ");
  Serial.print(SCREEN_WIDTH);
  Serial.print("x");
  Serial.println(SCREEN_HEIGHT);

  // Optionale Pins initialisieren (falls definiert)
  #ifdef LED_PIN_R
    pinMode(LED_PIN_R, OUTPUT);
    pinMode(LED_PIN_G, OUTPUT);
    pinMode(LED_PIN_B, OUTPUT);
    digitalWrite(LED_PIN_R, HIGH);  // HIGH = OFF
    digitalWrite(LED_PIN_G, HIGH);  // HIGH = OFF
    digitalWrite(LED_PIN_B, HIGH);  // HIGH = OFF
    Serial.println("RGB LED initialisiert");
  #endif

  #ifdef photoR_PIN
    // pinMode nicht nötig - ADC Pins sind standardmäßig INPUT
    Serial.println("Photoresistor initialisiert");
  #endif

  lcd.setBrightness(200);
  lcd.setRotation(1);

  // Alle Bälle inaktiv setzen
  for (int i = 0; i < MAX_BALLS; i++) {
    balls[i].active = false;
  }

  // Erste Bälle erstellen (relativ zur Bildschirmgröße)
  createBall(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 5, random(-3, 3), 0, 10, TFT_RED);
  createBall(SCREEN_WIDTH / 3, SCREEN_HEIGHT / 4, random(-2, 2), 0, 8, TFT_GREEN);
  createBall(2 * SCREEN_WIDTH / 3, SCREEN_HEIGHT / 5, random(-2, 2), 0, 12, TFT_BLUE);

  // Bildschirm löschen
  lcd.fillScreen(TFT_BLACK);

  // Anleitung
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(1);
  lcd.setCursor(5, 5);
  lcd.println("Touch: Neuer Ball");
}

void loop() {
  // Bildschirm löschen (mit leichtem Schatten-Effekt)
  lcd.fillScreen(TFT_BLACK);

  // Boden zeichnen
  lcd.fillRect(0, SCREEN_HEIGHT - 5, SCREEN_WIDTH, 5, TFT_DARKGREY);

  // Touch prüfen (neuen Ball erstellen)
  checkTouch();

  // Alle Bälle aktualisieren und zeichnen
  for (int i = 0; i < MAX_BALLS; i++) {
    if (balls[i].active) {
      updateBall(i);
      drawBall(i);
    }
  }

  // FPS anzeigen
  updateFPS();
  displayInfo();

  // Frame-Verzögerung für konstante Framerate
  delay(16);  // ~60 FPS
}

// Neuen Ball erstellen
void createBall(float x, float y, float vx, float vy, float radius, uint16_t color) {
  for (int i = 0; i < MAX_BALLS; i++) {
    if (!balls[i].active) {
      balls[i].x = x;
      balls[i].y = y;
      balls[i].vx = vx;
      balls[i].vy = vy;
      balls[i].radius = radius;
      balls[i].color = color;
      balls[i].active = true;

      Serial.print("Ball erstellt: #");
      Serial.println(i);
      break;
    }
  }
}

// Ball aktualisieren (Physik)
void updateBall(int index) {
  Ball* ball = &balls[index];

  // Gravitation anwenden
  ball->vy += GRAVITY;

  // Reibung anwenden
  ball->vx *= FRICTION;
  ball->vy *= FRICTION;

  // Position aktualisieren
  ball->x += ball->vx;
  ball->y += ball->vy;

  // Kollision mit Boden
  if (ball->y + ball->radius > SCREEN_HEIGHT - 5) {
    ball->y = SCREEN_HEIGHT - 5 - ball->radius;
    ball->vy = -ball->vy * BOUNCE_DAMPING;

    // Wenn Ball fast still ist, auf Boden fixieren
    if (abs(ball->vy) < 0.5) {
      ball->vy = 0;
    }
  }

  // Kollision mit Decke
  if (ball->y - ball->radius < 0) {
    ball->y = ball->radius;
    ball->vy = -ball->vy * BOUNCE_DAMPING;
  }

  // Kollision mit linker Wand
  if (ball->x - ball->radius < 0) {
    ball->x = ball->radius;
    ball->vx = -ball->vx * BOUNCE_DAMPING;
  }

  // Kollision mit rechter Wand
  if (ball->x + ball->radius > SCREEN_WIDTH) {
    ball->x = SCREEN_WIDTH - ball->radius;
    ball->vx = -ball->vx * BOUNCE_DAMPING;
  }

  // Ball entfernen wenn er zu langsam und am Boden ist
  if (abs(ball->vx) < 0.1 && abs(ball->vy) < 0.1 && ball->y > SCREEN_HEIGHT - 20) {
    // Ball kann für neue Bälle wiederverwendet werden
    // Optional: ball->active = false;
  }
}

// Ball zeichnen
void drawBall(int index) {
  Ball* ball = &balls[index];

  // Ball mit Schatten
  lcd.fillCircle(ball->x + 2, ball->y + 2, ball->radius, TFT_DARKGREY);  // Schatten
  lcd.fillCircle(ball->x, ball->y, ball->radius, ball->color);           // Ball

  // Glanzpunkt für 3D-Effekt
  lcd.fillCircle(ball->x - ball->radius/3, ball->y - ball->radius/3, ball->radius/4, TFT_WHITE);
}

// Touch prüfen
void checkTouch() {
  uint16_t x, y;
  bool touched = lcd.getTouch(&x, &y);

  if (touched) {
    // Neuen Ball an Touch-Position erstellen
    uint16_t colors[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_CYAN, TFT_MAGENTA, TFT_ORANGE, TFT_PINK};
    int colorIndex = random(0, 8);
    int radius = random(6, 15);

    createBall(x, y, random(-4, 4), random(-2, 0), radius, colors[colorIndex]);

    delay(200);  // Debounce
  }
}

// FPS aktualisieren
void updateFPS() {
  frameCount++;

  if (millis() - lastFpsUpdate >= 1000) {
    fps = frameCount;
    frameCount = 0;
    lastFpsUpdate = millis();
  }
}

// Info anzeigen
void displayInfo() {
  // FPS
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(1);
  lcd.setCursor(5, 5);
  lcd.print("Touch: Neuer Ball");

  lcd.setCursor(SCREEN_WIDTH - 60, 5);
  lcd.print("FPS: ");
  lcd.print(fps);

  // Anzahl aktiver Bälle
  int activeBalls = 0;
  for (int i = 0; i < MAX_BALLS; i++) {
    if (balls[i].active) activeBalls++;
  }

  lcd.setCursor(5, 15);
  lcd.print("Baelle: ");
  lcd.print(activeBalls);
  lcd.print("/");
  lcd.print(MAX_BALLS);

  // Display-Größe anzeigen (klein unten)
  lcd.setTextColor(TFT_DARKGREY);
  lcd.setCursor(5, SCREEN_HEIGHT - 10);
  lcd.print(SCREEN_WIDTH);
  lcd.print("x");
  lcd.print(SCREEN_HEIGHT);
}
