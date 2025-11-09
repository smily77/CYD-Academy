/*
  Pong Game - Klassisches 2-Spieler Pong

  Hardware:
  - CYD Display (320x240)
  - 2x Analog-Regler (Potentiometer):
    * GPIO 34: Linker Spieler
    * GPIO 35: Rechter Spieler
  - 3x Taster:
    * GPIO 16: Auto-Modus linker Spieler EIN (Active LOW)
    * GPIO 5: Auto-Modus linker Spieler AUS (Active LOW)
    * GPIO 17: Spielstand auf 0 setzen (Active LOW)

  Steuerung:
  - Linker Regler: Bewegt linken SchlÃ¤ger hoch/runter (oder AI im Auto-Modus)
  - Rechter Regler: Bewegt rechten SchlÃ¤ger hoch/runter

  Spielregeln:
  - Ball prallt an SchlÃ¤gern und WÃ¤nden ab
  - Trifft Ball links/rechts raus: Punkt fÃ¼r Gegner
  - Spiel lÃ¤uft bis Reset
*/

#include <CYD_Display_Config.h>

// Pin-Mapping: Physische Pins -> Logische Namen
#define POT_LEFT            potiLeft
#define POT_RIGHT           potiRight
#define BUTTON_AUTO_ON      tasteC
#define BUTTON_AUTO_OFF     tasteB
#define BUTTON_RESET_SCORE  tasteA

// Display Objekt
LGFX lcd;

// Spielfeld-Dimensionen
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// Farben
#define COLOR_BG      0x0000  // Schwarz
#define COLOR_FG      0xFFFF  // WeiÃŸ
#define COLOR_PADDLE  0x07FF  // Cyan (Standard SchlÃ¤gerfarbe)
#define COLOR_PADDLE_AUTO 0xF800 // Rot (Auto-Modus SchlÃ¤gerfarbe)
#define COLOR_BALL    0xFFE0  // Gelb
#define COLOR_LINE    0x4208  // Dunkelgrau

// SchlÃ¤ger (Paddle) Struktur
struct Paddle {
  int x, y;
  int w, h;
  uint16_t color;
};

// Ball Struktur
struct Ball {
  float x, y;      // Position (float fÃ¼r smooth movement)
  float vx, vy;    // Geschwindigkeit
  int size;
  uint16_t color;
};

// Game Objekte
Paddle paddleLeft;
Paddle paddleRight;
Ball ball;

// Score
int scoreLeft = 0;
int scoreRight = 0;

// Alte Positionen fÃ¼r effizientes LÃ¶schen und zum Erkennen von Ã„nderungen
int oldPaddleLeftY = 0;
uint16_t oldPaddleLeftColor = COLOR_PADDLE; // Speichert alte Farbe fÃ¼r Paddle
int oldPaddleRightY = 0;
float oldBallX = 0;
float oldBallY = 0;

// Auto-Modus fÃ¼r linken SchlÃ¤ger
bool autoModeLeftPaddle = false;

// Taster-Variablen fÃ¼r Entprellung (jetzt nur noch fÃ¼r ZustandsÃ¼berwachung)
// Taster AUTO_ON
int lastButtonStateAutoOn = HIGH; // Speichert den vorherigen Zustand des Tasters

// Taster AUTO_OFF
int lastButtonStateAutoOff = HIGH; // Speichert den vorherigen Zustand des Tasters

// Taster RESET_SCORE
int lastButtonStateResetScore = HIGH; // Speichert den vorherigen Zustand des Tasters

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== PONG GAME ===\n");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(1);  // Landscape
  lcd.fillScreen(COLOR_BG);
  lcd.setBrightness(255);

  // Analog Inputs konfigurieren
  pinMode(POT_LEFT, INPUT);
  pinMode(POT_RIGHT, INPUT);

  // Taster Inputs konfigurieren
  pinMode(BUTTON_AUTO_ON, INPUT_PULLUP);
  pinMode(BUTTON_AUTO_OFF, INPUT_PULLUP);
  pinMode(BUTTON_RESET_SCORE, INPUT_PULLUP);

  // Zufallszahlengenerator initialisieren
  // Nutzt analogRead von ungenutzten Pins fÃ¼r bessere ZufÃ¤lligkeit
  randomSeed(analogRead(POT_LEFT) + analogRead(POT_RIGHT) + micros());

  // Spiel initialisieren
  initGame();

  // Spielfeld und Score initial zeichnen
  drawField();
  drawScore();

  Serial.println("Spiel gestartet!");
  Serial.println("Linker Regler: GPIO 34");
  Serial.println("Rechter Regler: GPIO 35");
  Serial.println("Auto-Modus EIN: GPIO 16");
  Serial.println("Auto-Modus AUS: GPIO 5");
  Serial.println("Score Reset: GPIO 17");
}

void loop() {
  // Taster-Eingaben verarbeiten (Auto-Modus, Reset)
  handleButtons();

  // SchlÃ¤ger-Positionen aktualisieren (Analog-Inputs oder AI)
  updatePaddles();

  // Ball bewegen
  updateBall();

  // Kollisionen prÃ¼fen (SchlÃ¤ger, WÃ¤nde, Tor)
  checkCollisions();

  // Alles auf dem Display zeichnen
  drawGame();

  // Frame-Rate ~60 FPS (1000ms / 60 = ~16.6ms)
  delay(16);
}

// ===== INITIALISIERUNG =====

void initGame() {
  // Linker SchlÃ¤ger
  paddleLeft.x = 10;
  paddleLeft.y = SCREEN_HEIGHT / 2 - 30;
  paddleLeft.w = 5;
  paddleLeft.h = 60;
  // Standardfarbe und Auto-Modus ausschalten
  autoModeLeftPaddle = false;
  paddleLeft.color = COLOR_PADDLE;

  // Rechter SchlÃ¤ger
  paddleRight.x = SCREEN_WIDTH - 15;
  paddleRight.y = SCREEN_HEIGHT / 2 - 30;
  paddleRight.w = 5;
  paddleRight.h = 60;
  paddleRight.color = COLOR_PADDLE;

  // Ball in der Mitte
  resetBall();

  // Score auf Null
  scoreLeft = 0;
  scoreRight = 0;

  // Alte Positionen fÃ¼r das erste Zeichnen initialisieren
  oldPaddleLeftY = paddleLeft.y;
  oldPaddleLeftColor = paddleLeft.color;
  oldPaddleRightY = paddleRight.y;
  oldBallX = ball.x;
  oldBallY = ball.y;

  // Hintergrund neu zeichnen und Score aktualisieren
  lcd.fillScreen(COLOR_BG);
  drawField();
  drawScore();
}

void resetBall() {
  ball.x = SCREEN_WIDTH / 2;
  ball.y = SCREEN_HEIGHT / 2;
  ball.size = 6;
  ball.color = COLOR_BALL;

  // ZufÃ¤llige Start-Richtung
  ball.vx = (random(0, 2) == 0) ? -3.5 : 3.5; // Startet links oder rechts
  ball.vy = random(-2, 3); // Leichte vertikale Abweichung

  delay(500);  // Kurze Pause nach Reset, bevor das Spiel weitergeht
}

// ===== SPIELFELD ZEICHNEN =====

void drawField() {
  // Mittellinie (gestrichelt)
  int centerX = SCREEN_WIDTH / 2;
  for (int y = 0; y < SCREEN_HEIGHT; y += 10) {
    lcd.fillRect(centerX - 1, y, 2, 5, COLOR_LINE);
  }
}

void drawScore() {
  lcd.setTextSize(3);
  // Hintergrund fÃ¼r Text explizit auf COLOR_BG setzen, damit alte Zahlen sauber Ã¼berschrieben werden
  lcd.setTextColor(COLOR_FG, COLOR_BG);

  // Linker Score (vorherigen Score lÃ¶schen, falls er grÃ¶ÃŸer war)
  lcd.fillRect(SCREEN_WIDTH / 4 - 20, 10, 40, 24, COLOR_BG); // GrÃ¶ÃŸe fÃ¼r zwei Ziffern
  lcd.setCursor(SCREEN_WIDTH / 4 - 20, 10);
  lcd.printf("%d", scoreLeft);

  // Rechter Score
  lcd.fillRect(SCREEN_WIDTH * 3 / 4 - 20, 10, 40, 24, COLOR_BG); // GrÃ¶ÃŸe fÃ¼r zwei Ziffern
  lcd.setCursor(SCREEN_WIDTH * 3 / 4 - 20, 10);
  lcd.printf("%d", scoreRight);
}

// ===== UPDATE FUNKTIONEN =====

void handleButtons() {
  // --- Taster fÃ¼r Auto-Modus EIN ---
  int currentButtonStateAutoOn = digitalRead(BUTTON_AUTO_ON);
  // FÃ¼hre Aktion nur aus, wenn der Taster gerade gedrÃ¼ckt wurde (Ãœbergang von HIGH zu LOW)
  if (currentButtonStateAutoOn == LOW && lastButtonStateAutoOn == HIGH) {
    if (!autoModeLeftPaddle) {
      autoModeLeftPaddle = true;
      paddleLeft.color = COLOR_PADDLE_AUTO;
      Serial.println("Auto-Modus EIN");
    }
  }
  lastButtonStateAutoOn = currentButtonStateAutoOn; // Speichere den aktuellen Zustand fÃ¼r den nÃ¤chsten Durchlauf

  // --- Taster fÃ¼r Auto-Modus AUS ---
  int currentButtonStateAutoOff = digitalRead(BUTTON_AUTO_OFF);
  // FÃ¼hre Aktion nur aus, wenn der Taster gerade gedrÃ¼ckt wurde (Ãœbergang von HIGH zu LOW)
  if (currentButtonStateAutoOff == LOW && lastButtonStateAutoOff == HIGH) {
    if (autoModeLeftPaddle) {
      autoModeLeftPaddle = false;
      paddleLeft.color = COLOR_PADDLE;
      Serial.println("Auto-Modus AUS");
    }
  }
  lastButtonStateAutoOff = currentButtonStateAutoOff;

  // --- Taster fÃ¼r Spielstand RESET ---
  int currentButtonStateResetScore = digitalRead(BUTTON_RESET_SCORE);
  // FÃ¼hre Aktion nur aus, wenn der Taster gerade gedrÃ¼ckt wurde (Ãœbergang von HIGH zu LOW)
  if (currentButtonStateResetScore == LOW && lastButtonStateResetScore == HIGH) {
    Serial.println("Spielstand wird zurÃ¼ckgesetzt!");
    initGame(); // Setzt alles zurÃ¼ck und zeichnet neu
  }
  lastButtonStateResetScore = currentButtonStateResetScore;
}

void updatePaddles() {
  // Alte Positionen und Farbe speichern fÃ¼r effizientes Zeichnen
  oldPaddleLeftY = paddleLeft.y;
  oldPaddleLeftColor = paddleLeft.color; // Speichere alte Farbe
  oldPaddleRightY = paddleRight.y;

  // Linker SchlÃ¤ger (Spieler oder AI)
  if (autoModeLeftPaddle) {
    // Einfache AI: Folgt dem Ball
    // Ziel-Y des Paddles ist die Mitte des Balls minus die HÃ¤lfte der Paddle-HÃ¶he
    int targetY = ball.y - (paddleLeft.h / 2);
    // Sanfte Bewegung zur Zielposition
    paddleLeft.y += (targetY - paddleLeft.y) * 0.1; // 10% des Weges pro Frame

  } else {
    // Manueller Modus: Analog-Wert lesen
    int potLeftValue = analogRead(POT_LEFT);
    // Hinweis: ESP32 ADC hat standardmÃ¤ÃŸig einen Bereich von 0-4095.
    // Die Original-Map-Werte (1000,0) deuten auf eine spezielle Potentiometer-Verkabelung oder Nutzungsbereich hin.
    paddleLeft.y = map(potLeftValue, 1000, 0, 0, SCREEN_HEIGHT - paddleLeft.h);
  }

  // Rechter SchlÃ¤ger (immer manuell)
  int potRightValue = analogRead(POT_RIGHT);
  paddleRight.y = map(potRightValue, 1000, 0, 0, SCREEN_HEIGHT - paddleRight.h);

  // Grenzen prÃ¼fen fÃ¼r beide SchlÃ¤ger
  paddleLeft.y = constrain(paddleLeft.y, 0, SCREEN_HEIGHT - paddleLeft.h);
  paddleRight.y = constrain(paddleRight.y, 0, SCREEN_HEIGHT - paddleRight.h);
}

void updateBall() {
  // Alte Position speichern
  oldBallX = ball.x;
  oldBallY = ball.y;

  // Ball bewegen
  ball.x += ball.vx;
  ball.y += ball.vy;

  // Kollision mit oberer/unterer Wand
  if (ball.y <= 0 || ball.y >= SCREEN_HEIGHT - ball.size) {
    ball.vy = -ball.vy;
    // Ball-Position innerhalb der Grenzen korrigieren, um Feststecken zu vermeiden
    ball.y = constrain(ball.y, 0, SCREEN_HEIGHT - ball.size);
  }
}

void checkCollisions() {
  // Kollision mit linkem SchlÃ¤ger
  if (ball.x <= paddleLeft.x + paddleLeft.w &&
      ball.x >= paddleLeft.x &&
      ball.y + ball.size >= paddleLeft.y &&
      ball.y <= paddleLeft.y + paddleLeft.h) {

    ball.vx = -ball.vx;
    // Ball-Position an SchlÃ¤ger anpassen, um Feststecken zu vermeiden
    ball.x = paddleLeft.x + paddleLeft.w;

    // Winkel abhÃ¤ngig von Treffpunkt
    float hitPos = (ball.y + ball.size/2) - (paddleLeft.y + paddleLeft.h/2);
    ball.vy += hitPos * 0.05; // Leichte GeschwindigkeitsÃ¤nderung basierend auf Treffpunkt

    Serial.println("Hit: Linker SchlÃ¤ger");
  }

  // Kollision mit rechtem SchlÃ¤ger
  if (ball.x + ball.size >= paddleRight.x &&
      ball.x + ball.size <= paddleRight.x + paddleRight.w &&
      ball.y + ball.size >= paddleRight.y &&
      ball.y <= paddleRight.y + paddleRight.h) {

    ball.vx = -ball.vx;
    // Ball-Position an SchlÃ¤ger anpassen, um Feststecken zu vermeiden
    ball.x = paddleRight.x - ball.size;

    // Winkel abhÃ¤ngig von Treffpunkt
    float hitPos = (ball.y + ball.size/2) - (paddleRight.y + paddleRight.h/2);
    ball.vy += hitPos * 0.05; // Leichte GeschwindigkeitsÃ¤nderung basierend auf Treffpunkt

    Serial.println("Hit: Rechter SchlÃ¤ger");
  }

  // Ball links raus - Punkt fÃ¼r Rechts
  if (ball.x < 0) {
    scoreRight++;
    Serial.printf("PUNKT! Rechts: %d - Links: %d\n", scoreRight, scoreLeft);
    drawScore(); // Score sofort aktualisieren
    resetBall();
  }

  // Ball rechts raus - Punkt fÃ¼r Links
  if (ball.x > SCREEN_WIDTH) {
    scoreLeft++;
    Serial.printf("PUNKT! Links: %d - Rechts: %d\n", scoreLeft, scoreRight);
    drawScore(); // Score sofort aktualisieren
    resetBall();
  }
}

// ===== ZEICHNEN =====

void drawGame() {
  // --- Rendern der statischen Hintergrundelemente ---
  // Wenn der Ball Ã¼ber die Mittellinie oder Punktanzeige fliegt,
  // werden diese Elemente schwarz. Durch das erneute Zeichnen dieser Elemente
  // am Anfang jedes Frames wird sichergestellt, dass sie immer sichtbar sind.
  drawField();
  drawScore();

  // --- SchlÃ¤ger zeichnen ---
  // Linken SchlÃ¤ger lÃ¶schen (alte Position/Farbe) und neu zeichnen
  // Neuzeichnen, wenn sich Y-Position oder Farbe geÃ¤ndert hat
  if (oldPaddleLeftY != paddleLeft.y || oldPaddleLeftColor != paddleLeft.color) {
    // Zuerst alten Bereich mit Hintergrundfarbe Ã¼bermalen
    lcd.fillRect(paddleLeft.x, oldPaddleLeftY, paddleLeft.w, paddleLeft.h, COLOR_BG);
    // Dann SchlÃ¤ger an neuer Position mit aktueller Farbe zeichnen
    lcd.fillRect(paddleLeft.x, paddleLeft.y, paddleLeft.w, paddleLeft.h, paddleLeft.color);
    oldPaddleLeftColor = paddleLeft.color; // Aktuelle Farbe fÃ¼r den nÃ¤chsten Frame speichern
  }

  // Rechten SchlÃ¤ger lÃ¶schen (alte Position) und neu zeichnen
  if (oldPaddleRightY != paddleRight.y) {
    lcd.fillRect(paddleRight.x, oldPaddleRightY, paddleRight.w, paddleRight.h, COLOR_BG);
    lcd.fillRect(paddleRight.x, paddleRight.y, paddleRight.w, paddleRight.h, paddleRight.color);
  }

  // --- Ball zeichnen ---
  // Ball lÃ¶schen (alte Position)
  // Dies Ã¼bermalt den alten Ball mit COLOR_BG.
  // Da drawField() und drawScore() am Anfang jedes Frames aufgerufen werden,
  // werden die Teile der Mittellinie/Score, die unter dem alten Ball lagen,
  // sofort wiederhergestellt.
  lcd.fillRect(oldBallX, oldBallY, ball.size, ball.size, COLOR_BG);
  // Ball an neuer Position zeichnen
  lcd.fillRect(ball.x, ball.y, ball.size, ball.size, ball.color);
}
