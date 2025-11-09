/*
  Pong Game MODERN - Modernes 2-Spieler Pong mit Fancy Design

  Hardware:
  - CYD Display (320x240)
  - 2x Analog-Regler (Potentiometer):
    * GPIO 34: Linker Spieler
    * GPIO 35: Rechter Spieler
  - 3x Taster:
    * GPIO 16: Auto-Modus linker Spieler EIN (Active LOW)
    * GPIO 5: Auto-Modus linker Spieler AUS (Active LOW)
    * GPIO 17: Spielstand auf 0 setzen (Active LOW)

  Design-Features:
  - Gradient-Hintergrund (Dunkelblau bis Schwarz)
  - Neon-Glüheffekte für Schläger und Ball
  - 3D-Schläger mit Schatten und Highlights
  - Partikeleffekt beim Ball-Treffer
  - Trail-Effekt für den Ball
  - Moderne Farbpalette
  - Animierte UI-Elemente
*/

#include <CYD_Display_Config.h>

// Display Objekt
LGFX lcd;

// Spielfeld-Dimensionen
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// ===== MODERNE FARBPALETTE =====
// Hintergrund: Dunkler Gradient
#define COLOR_BG_TOP     0x0814  // Dunkelblau
#define COLOR_BG_BOTTOM  0x0000  // Schwarz

// Neon-Farben für Spielelemente
#define COLOR_NEON_CYAN   0x07FF  // Helles Cyan
#define COLOR_NEON_PINK   0xF81F  // Magenta/Pink
#define COLOR_NEON_YELLOW 0xFFE0  // Gelb
#define COLOR_NEON_GREEN  0x07E0  // Grün
#define COLOR_NEON_ORANGE 0xFD20  // Orange

// UI-Farben
#define COLOR_TEXT       0xFFFF  // Weiß
#define COLOR_TEXT_DIM   0x8410  // Grau
#define COLOR_SHADOW     0x2104  // Sehr Dunkelgrau
#define COLOR_HIGHLIGHT  0xFFFF  // Weiß

// Schläger-Farben
#define COLOR_PADDLE_MANUAL  COLOR_NEON_CYAN    // Cyan für manuell
#define COLOR_PADDLE_AUTO    COLOR_NEON_PINK    // Pink für AI
#define COLOR_PADDLE_RIGHT   COLOR_NEON_GREEN   // Grün für rechts

// SchlÃ¤ger (Paddle) Struktur
struct Paddle {
  int x, y;
  int w, h;
  uint16_t color;
};

// Ball Struktur
struct Ball {
  float x, y;
  float vx, vy;
  int size;
  uint16_t color;
};

// Partikel für Treffer-Effekte
struct Particle {
  float x, y;
  float vx, vy;
  int life;
  uint16_t color;
  bool active;
};

// Ball-Trail für Motion-Blur-Effekt
struct TrailPoint {
  int x, y;
  bool active;
};

// Game Objekte
Paddle paddleLeft;
Paddle paddleRight;
Ball ball;

// Partikel-System
#define MAX_PARTICLES 20
Particle particles[MAX_PARTICLES];

// Ball-Trail
#define TRAIL_LENGTH 8
TrailPoint trail[TRAIL_LENGTH];
int trailIndex = 0;

// Score
int scoreLeft = 0;
int scoreRight = 0;

// Alte Positionen
int oldPaddleLeftY = 0;
uint16_t oldPaddleLeftColor = COLOR_PADDLE_MANUAL;
int oldPaddleRightY = 0;
float oldBallX = 0;
float oldBallY = 0;

// Auto-Modus
bool autoModeLeftPaddle = false;

// Taster-Variablen
int lastButtonStateAutoOn = HIGH;
int lastButtonStateAutoOff = HIGH;
int lastButtonStateResetScore = HIGH;

// Animationen
unsigned long lastParticleSpawn = 0;
float glowPhase = 0;  // Für Glow-Animation

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== PONG GAME MODERN ===\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);
  lcd.setBrightness(255);

  // Analog Inputs konfigurieren
  pinMode(POT_LEFT, INPUT);
  pinMode(POT_RIGHT, INPUT);

  // Taster Inputs konfigurieren
  pinMode(BUTTON_AUTO_ON, INPUT_PULLUP);
  pinMode(BUTTON_AUTO_OFF, INPUT_PULLUP);
  pinMode(BUTTON_RESET_SCORE, INPUT_PULLUP);

  // Zufallszahlengenerator
  randomSeed(analogRead(POT_LEFT) + analogRead(POT_RIGHT) + micros());

  // Partikel initialisieren
  for (int i = 0; i < MAX_PARTICLES; i++) {
    particles[i].active = false;
  }

  // Trail initialisieren
  for (int i = 0; i < TRAIL_LENGTH; i++) {
    trail[i].active = false;
  }

  // Spiel initialisieren
  initGame();

  Serial.println("Modern Pong gestartet!");
  Serial.println("Steuerung wie Original-Pong");
}

void loop() {
  // Glow-Animation aktualisieren
  glowPhase += 0.05;
  if (glowPhase > 6.28) glowPhase = 0;  // 2*PI

  // Taster-Eingaben
  handleButtons();

  // Schläger aktualisieren
  updatePaddles();

  // Ball bewegen
  updateBall();

  // Kollisionen prüfen
  checkCollisions();

  // Partikel aktualisieren
  updateParticles();

  // Trail aktualisieren
  updateTrail();

  // Alles zeichnen
  drawGame();

  // Frame-Rate ~60 FPS
  delay(16);
}

// ===== INITIALISIERUNG =====

void initGame() {
  // Linker Schläger
  paddleLeft.x = 15;
  paddleLeft.y = SCREEN_HEIGHT / 2 - 35;
  paddleLeft.w = 8;
  paddleLeft.h = 70;
  autoModeLeftPaddle = false;
  paddleLeft.color = COLOR_PADDLE_MANUAL;

  // Rechter Schläger
  paddleRight.x = SCREEN_WIDTH - 23;
  paddleRight.y = SCREEN_HEIGHT / 2 - 35;
  paddleRight.w = 8;
  paddleRight.h = 70;
  paddleRight.color = COLOR_PADDLE_RIGHT;

  // Ball
  resetBall();

  // Score
  scoreLeft = 0;
  scoreRight = 0;

  // Alte Positionen
  oldPaddleLeftY = paddleLeft.y;
  oldPaddleLeftColor = paddleLeft.color;
  oldPaddleRightY = paddleRight.y;
  oldBallX = ball.x;
  oldBallY = ball.y;

  // Gradient-Hintergrund zeichnen
  drawGradientBackground();
}

void resetBall() {
  ball.x = SCREEN_WIDTH / 2;
  ball.y = SCREEN_HEIGHT / 2;
  ball.size = 8;
  ball.color = COLOR_NEON_YELLOW;

  // Zufällige Start-Richtung
  ball.vx = (random(0, 2) == 0) ? -3.5 : 3.5;
  ball.vy = random(-2, 3);

  delay(500);
}

// ===== BUTTON HANDLING =====

void handleButtons() {
  int currentButtonStateAutoOn = digitalRead(BUTTON_AUTO_ON);
  if (currentButtonStateAutoOn == LOW && lastButtonStateAutoOn == HIGH) {
    if (!autoModeLeftPaddle) {
      autoModeLeftPaddle = true;
      paddleLeft.color = COLOR_PADDLE_AUTO;
      Serial.println("Auto-Modus EIN");
    }
  }
  lastButtonStateAutoOn = currentButtonStateAutoOn;

  int currentButtonStateAutoOff = digitalRead(BUTTON_AUTO_OFF);
  if (currentButtonStateAutoOff == LOW && lastButtonStateAutoOff == HIGH) {
    if (autoModeLeftPaddle) {
      autoModeLeftPaddle = false;
      paddleLeft.color = COLOR_PADDLE_MANUAL;
      Serial.println("Auto-Modus AUS");
    }
  }
  lastButtonStateAutoOff = currentButtonStateAutoOff;

  int currentButtonStateResetScore = digitalRead(BUTTON_RESET_SCORE);
  if (currentButtonStateResetScore == LOW && lastButtonStateResetScore == HIGH) {
    Serial.println("Spielstand zurückgesetzt!");
    initGame();
  }
  lastButtonStateResetScore = currentButtonStateResetScore;
}

// ===== UPDATE FUNKTIONEN =====

void updatePaddles() {
  oldPaddleLeftY = paddleLeft.y;
  oldPaddleLeftColor = paddleLeft.color;
  oldPaddleRightY = paddleRight.y;

  // Linker Schläger (Spieler oder AI)
  if (autoModeLeftPaddle) {
    int targetY = ball.y - (paddleLeft.h / 2);
    paddleLeft.y += (targetY - paddleLeft.y) * 0.1;
  } else {
    int potLeftValue = analogRead(POT_LEFT);
    paddleLeft.y = map(potLeftValue, 1000, 0, 0, SCREEN_HEIGHT - paddleLeft.h);
  }

  // Rechter Schläger (immer manuell)
  int potRightValue = analogRead(POT_RIGHT);
  paddleRight.y = map(potRightValue, 1000, 0, 0, SCREEN_HEIGHT - paddleRight.h);

  // Grenzen
  paddleLeft.y = constrain(paddleLeft.y, 0, SCREEN_HEIGHT - paddleLeft.h);
  paddleRight.y = constrain(paddleRight.y, 0, SCREEN_HEIGHT - paddleRight.h);
}

void updateBall() {
  oldBallX = ball.x;
  oldBallY = ball.y;

  ball.x += ball.vx;
  ball.y += ball.vy;

  // Kollision mit oberer/unterer Wand
  if (ball.y <= 0 || ball.y >= SCREEN_HEIGHT - ball.size) {
    ball.vy = -ball.vy;
    ball.y = constrain(ball.y, 0, SCREEN_HEIGHT - ball.size);

    // Partikel spawnen
    spawnParticles(ball.x + ball.size/2, ball.y + ball.size/2, COLOR_NEON_YELLOW, 5);
  }
}

void checkCollisions() {
  // Kollision mit linkem Schläger
  if (ball.x <= paddleLeft.x + paddleLeft.w &&
      ball.x >= paddleLeft.x &&
      ball.y + ball.size >= paddleLeft.y &&
      ball.y <= paddleLeft.y + paddleLeft.h) {

    ball.vx = -ball.vx;
    ball.x = paddleLeft.x + paddleLeft.w;

    float hitPos = (ball.y + ball.size/2) - (paddleLeft.y + paddleLeft.h/2);
    ball.vy += hitPos * 0.05;

    // Partikel spawnen
    spawnParticles(paddleLeft.x + paddleLeft.w, ball.y + ball.size/2, paddleLeft.color, 8);

    Serial.println("Hit: Linker Schläger");
  }

  // Kollision mit rechtem Schläger
  if (ball.x + ball.size >= paddleRight.x &&
      ball.x + ball.size <= paddleRight.x + paddleRight.w &&
      ball.y + ball.size >= paddleRight.y &&
      ball.y <= paddleRight.y + paddleRight.h) {

    ball.vx = -ball.vx;
    ball.x = paddleRight.x - ball.size;

    float hitPos = (ball.y + ball.size/2) - (paddleRight.y + paddleRight.h/2);
    ball.vy += hitPos * 0.05;

    // Partikel spawnen
    spawnParticles(paddleRight.x, ball.y + ball.size/2, paddleRight.color, 8);

    Serial.println("Hit: Rechter Schläger");
  }

  // Ball links raus
  if (ball.x < 0) {
    scoreRight++;
    Serial.printf("PUNKT! Rechts: %d - Links: %d\n", scoreRight, scoreLeft);
    resetBall();
  }

  // Ball rechts raus
  if (ball.x > SCREEN_WIDTH) {
    scoreLeft++;
    Serial.printf("PUNKT! Links: %d - Rechts: %d\n", scoreLeft, scoreRight);
    resetBall();
  }
}

// ===== PARTIKEL-SYSTEM =====

void spawnParticles(int x, int y, uint16_t color, int count) {
  for (int i = 0; i < count; i++) {
    // Freien Partikel-Slot finden
    for (int j = 0; j < MAX_PARTICLES; j++) {
      if (!particles[j].active) {
        particles[j].x = x;
        particles[j].y = y;
        particles[j].vx = random(-30, 30) / 10.0;
        particles[j].vy = random(-30, 30) / 10.0;
        particles[j].life = random(10, 25);
        particles[j].color = color;
        particles[j].active = true;
        break;
      }
    }
  }
}

void updateParticles() {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (particles[i].active) {
      particles[i].x += particles[i].vx;
      particles[i].y += particles[i].vy;
      particles[i].vy += 0.3;  // Gravitation
      particles[i].life--;

      if (particles[i].life <= 0) {
        particles[i].active = false;
      }
    }
  }
}

void updateTrail() {
  // Neuen Trail-Point hinzufügen
  trail[trailIndex].x = ball.x + ball.size/2;
  trail[trailIndex].y = ball.y + ball.size/2;
  trail[trailIndex].active = true;

  trailIndex = (trailIndex + 1) % TRAIL_LENGTH;
}

// ===== ZEICHNEN =====

void drawGradientBackground() {
  // Vertikaler Gradient von oben nach unten
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    // Interpoliere zwischen TOP und BOTTOM
    uint16_t color = lerpColor(COLOR_BG_TOP, COLOR_BG_BOTTOM, (float)y / SCREEN_HEIGHT);
    lcd.drawFastHLine(0, y, SCREEN_WIDTH, color);
  }
}

void drawGame() {
  // Hintergrund (alle paar Frames neu zeichnen für Partikel-Clearing)
  static int frameCount = 0;
  if (frameCount % 30 == 0) {
    drawGradientBackground();
  }
  frameCount++;

  // Mittellinie (gepunktet, Neon-Stil)
  drawModernCenterLine();

  // Score mit modernem Design
  drawModernScore();

  // Schläger mit 3D-Effekt zeichnen
  drawModernPaddle(paddleLeft, oldPaddleLeftY, oldPaddleLeftColor);
  drawModernPaddle(paddleRight, oldPaddleRightY, paddleRight.color);

  // Ball-Trail zeichnen
  drawTrail();

  // Ball mit Glow-Effekt zeichnen
  drawModernBall();

  // Partikel zeichnen
  drawParticles();
}

void drawModernCenterLine() {
  int centerX = SCREEN_WIDTH / 2;
  for (int y = 5; y < SCREEN_HEIGHT - 5; y += 15) {
    // Neon-Linie mit Glow
    lcd.fillRect(centerX - 2, y, 4, 8, COLOR_NEON_CYAN);
    // Glow-Effekt
    lcd.drawPixel(centerX - 3, y + 3, dimColor(COLOR_NEON_CYAN, 0.3));
    lcd.drawPixel(centerX + 3, y + 3, dimColor(COLOR_NEON_CYAN, 0.3));
  }
}

void drawModernScore() {
  lcd.setTextSize(4);

  // Linker Score mit Schatten
  lcd.setTextColor(COLOR_SHADOW);
  lcd.setCursor(SCREEN_WIDTH / 4 - 22, 12);
  lcd.printf("%d", scoreLeft);

  lcd.setTextColor(autoModeLeftPaddle ? COLOR_PADDLE_AUTO : COLOR_PADDLE_MANUAL);
  lcd.setCursor(SCREEN_WIDTH / 4 - 24, 10);
  lcd.printf("%d", scoreLeft);

  // Rechter Score mit Schatten
  lcd.setTextColor(COLOR_SHADOW);
  lcd.setCursor(SCREEN_WIDTH * 3 / 4 - 22, 12);
  lcd.printf("%d", scoreRight);

  lcd.setTextColor(COLOR_PADDLE_RIGHT);
  lcd.setCursor(SCREEN_WIDTH * 3 / 4 - 24, 10);
  lcd.printf("%d", scoreRight);

  // Mode-Anzeige (klein, unten)
  if (autoModeLeftPaddle) {
    lcd.setTextSize(1);
    lcd.setTextColor(COLOR_PADDLE_AUTO);
    lcd.setCursor(5, SCREEN_HEIGHT - 12);
    lcd.print("AI MODE");
  }
}

void drawModernPaddle(Paddle &paddle, int oldY, uint16_t oldColor) {
  if (oldY != paddle.y || oldColor != paddle.color) {
    // Alten Schläger löschen (mit Gradient-Hintergrund)
    for (int y = oldY; y < oldY + paddle.h; y++) {
      uint16_t bgColor = lerpColor(COLOR_BG_TOP, COLOR_BG_BOTTOM, (float)y / SCREEN_HEIGHT);
      lcd.drawFastHLine(paddle.x - 2, y, paddle.w + 4, bgColor);
    }
  }

  // 3D-Effekt: Schatten
  lcd.fillRect(paddle.x + 2, paddle.y + 2, paddle.w, paddle.h, COLOR_SHADOW);

  // Haupt-Schläger mit Gradient
  for (int i = 0; i < paddle.h; i++) {
    float factor = (float)i / paddle.h;
    uint16_t color = lerpColor(paddle.color, dimColor(paddle.color, 0.5), factor);
    lcd.drawFastHLine(paddle.x, paddle.y + i, paddle.w, color);
  }

  // Highlight (oben links)
  lcd.drawFastHLine(paddle.x, paddle.y, paddle.w - 2, COLOR_HIGHLIGHT);
  lcd.drawFastVLine(paddle.x, paddle.y, 8, COLOR_HIGHLIGHT);

  // Glow-Effekt (pulsierend)
  float glow = sin(glowPhase) * 0.5 + 0.5;  // 0 bis 1
  if (glow > 0.3) {
    uint16_t glowColor = dimColor(paddle.color, glow * 0.5);
    lcd.drawRect(paddle.x - 1, paddle.y - 1, paddle.w + 2, paddle.h + 2, glowColor);
  }
}

void drawModernBall() {
  // Alte Position löschen
  int r = ball.size + 3;
  for (int dy = -r; dy <= r; dy++) {
    for (int dx = -r; dx <= r; dx++) {
      int px = oldBallX + ball.size/2 + dx;
      int py = oldBallY + ball.size/2 + dy;
      if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
        uint16_t bgColor = lerpColor(COLOR_BG_TOP, COLOR_BG_BOTTOM, (float)py / SCREEN_HEIGHT);
        lcd.drawPixel(px, py, bgColor);
      }
    }
  }

  // Glow-Effekt (äußerer Ring)
  int centerX = ball.x + ball.size/2;
  int centerY = ball.y + ball.size/2;

  // Mehrere Glow-Ringe
  for (int r = ball.size + 2; r >= ball.size/2; r--) {
    float intensity = 1.0 - (float)(r - ball.size/2) / (ball.size + 2);
    uint16_t glowColor = dimColor(ball.color, intensity * 0.6);
    lcd.drawCircle(centerX, centerY, r, glowColor);
  }

  // Haupt-Ball (gefüllt mit Gradient)
  lcd.fillCircle(centerX, centerY, ball.size/2, ball.color);

  // Highlight
  lcd.fillCircle(centerX - ball.size/4, centerY - ball.size/4, ball.size/4, COLOR_HIGHLIGHT);
}

void drawTrail() {
  // Zeichne Trail-Punkte (älter = transparenter)
  for (int i = 0; i < TRAIL_LENGTH; i++) {
    if (trail[i].active) {
      int age = (trailIndex - i + TRAIL_LENGTH) % TRAIL_LENGTH;
      float alpha = 1.0 - (float)age / TRAIL_LENGTH;
      uint16_t color = dimColor(ball.color, alpha * 0.4);

      lcd.fillCircle(trail[i].x, trail[i].y, 2, color);
    }
  }
}

void drawParticles() {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (particles[i].active) {
      float alpha = (float)particles[i].life / 25.0;
      uint16_t color = dimColor(particles[i].color, alpha);

      int size = 1 + (particles[i].life / 10);
      lcd.fillCircle(particles[i].x, particles[i].y, size, color);
    }
  }
}

// ===== HELPER FUNKTIONEN =====

// Interpoliere zwischen zwei Farben
uint16_t lerpColor(uint16_t color1, uint16_t color2, float t) {
  t = constrain(t, 0.0, 1.0);

  // RGB565 dekodieren
  uint8_t r1 = (color1 >> 11) & 0x1F;
  uint8_t g1 = (color1 >> 5) & 0x3F;
  uint8_t b1 = color1 & 0x1F;

  uint8_t r2 = (color2 >> 11) & 0x1F;
  uint8_t g2 = (color2 >> 5) & 0x3F;
  uint8_t b2 = color2 & 0x1F;

  // Interpolieren
  uint8_t r = r1 + (r2 - r1) * t;
  uint8_t g = g1 + (g2 - g1) * t;
  uint8_t b = b1 + (b2 - b1) * t;

  // RGB565 kodieren
  return (r << 11) | (g << 5) | b;
}

// Dimme eine Farbe (factor: 0.0 = schwarz, 1.0 = original)
uint16_t dimColor(uint16_t color, float factor) {
  factor = constrain(factor, 0.0, 1.0);

  uint8_t r = ((color >> 11) & 0x1F) * factor;
  uint8_t g = ((color >> 5) & 0x3F) * factor;
  uint8_t b = (color & 0x1F) * factor;

  return (r << 11) | (g << 5) | b;
}
