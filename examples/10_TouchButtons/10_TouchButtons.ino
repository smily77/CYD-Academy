/*
  Beispiel 10: Touch-Buttons mit verschiedenen Verhaltensweisen

  Demonstriert drei verschiedene Button-Typen:
  1. Momentary Button - Aktiv solange gedrückt
  2. Toggle Button - Schaltet bei jedem Druck um (ein/aus)
  3. Radio Button Gruppe - Nur einer kann aktiv sein

  - Touch-Eingabe mit visueller Rückmeldung
  - Verschiedene Farben für aktive/inaktive Zustände
  - Dynamisch skalierbar für verschiedene Display-Größen
*/

#include <CYD_Display_Config.h>

// Display-Variablen (werden dynamisch gesetzt)
int SCREEN_WIDTH;
int SCREEN_HEIGHT;

// Button-Definitionen
struct Button {
  int x, y, w, h;
  String label1;  // Erste Zeile
  String label2;  // Zweite Zeile
  bool active;
  int type;  // 0 = momentary, 1 = toggle, 2 = radio
};

Button buttons[5];

// Button-Zustände
bool button1Active = false;  // Momentary
bool button2Active = false;  // Toggle
int radioActive = 0;         // Radio Group (0, 1, 2 für Button 3, 4, 5)

// Touch-Status für Momentary Button
bool wasTouching = false;
int touchedButton = -1;

LGFX lcd;

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.setRotation(1);  // Landscape

  // Display-Größe ermitteln
  SCREEN_WIDTH = lcd.width();
  SCREEN_HEIGHT = lcd.height();

  // Optional: RGB LED initialisieren
  #ifdef LED_PIN_R
    pinMode(LED_PIN_R, OUTPUT);
    pinMode(LED_PIN_G, OUTPUT);
    pinMode(LED_PIN_B, OUTPUT);
    digitalWrite(LED_PIN_R, HIGH);  // HIGH = OFF
    digitalWrite(LED_PIN_G, HIGH);  // HIGH = OFF
    digitalWrite(LED_PIN_B, HIGH);  // HIGH = OFF
  #endif

  // Button-Layout berechnen mit Platz für Titel
  int headerHeight = SCREEN_HEIGHT / 8;  // Platz für Titel reservieren
  int availableHeight = SCREEN_HEIGHT - headerHeight - 10;  // 10px Rand
  int spacing = SCREEN_WIDTH / 40;  // Dynamischer Abstand

  int topButtonW = (SCREEN_WIDTH - 3 * spacing) / 2;
  int topButtonH = (availableHeight - spacing) / 2;
  int bottomButtonW = (SCREEN_WIDTH - 4 * spacing) / 3;
  int bottomButtonH = (availableHeight - spacing) / 2;

  // Button 1: Momentary (oben links)
  buttons[0].x = spacing;
  buttons[0].y = headerHeight + spacing;
  buttons[0].w = topButtonW;
  buttons[0].h = topButtonH;
  buttons[0].label1 = "MOMENTARY";
  buttons[0].label2 = "(Halten)";
  buttons[0].type = 0;
  buttons[0].active = false;

  // Button 2: Toggle (oben rechts)
  buttons[1].x = 2 * spacing + topButtonW;
  buttons[1].y = headerHeight + spacing;
  buttons[1].w = topButtonW;
  buttons[1].h = topButtonH;
  buttons[1].label1 = "TOGGLE";
  buttons[1].label2 = "(Ein/Aus)";
  buttons[1].type = 1;
  buttons[1].active = false;

  // Button 3: Radio 1 (unten links)
  buttons[2].x = spacing;
  buttons[2].y = headerHeight + spacing + topButtonH + spacing;
  buttons[2].w = bottomButtonW;
  buttons[2].h = bottomButtonH;
  buttons[2].label1 = "RADIO";
  buttons[2].label2 = "A";
  buttons[2].type = 2;
  buttons[2].active = true;  // Standard aktiv

  // Button 4: Radio 2 (unten mitte)
  buttons[3].x = 2 * spacing + bottomButtonW;
  buttons[3].y = headerHeight + spacing + topButtonH + spacing;
  buttons[3].w = bottomButtonW;
  buttons[3].h = bottomButtonH;
  buttons[3].label1 = "RADIO";
  buttons[3].label2 = "B";
  buttons[3].type = 2;
  buttons[3].active = false;

  // Button 5: Radio 3 (unten rechts)
  buttons[4].x = 3 * spacing + 2 * bottomButtonW;
  buttons[4].y = headerHeight + spacing + topButtonH + spacing;
  buttons[4].w = bottomButtonW;
  buttons[4].h = bottomButtonH;
  buttons[4].label1 = "RADIO";
  buttons[4].label2 = "C";
  buttons[4].type = 2;
  buttons[4].active = false;

  lcd.fillScreen(TFT_BLACK);

  // Titel
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextDatum(TC_DATUM);
  int fontSize = headerHeight / 20;
  if (fontSize < 1) fontSize = 1;
  if (fontSize > 2) fontSize = 2;
  lcd.setTextSize(fontSize);
  lcd.drawString("Touch Button Demo", SCREEN_WIDTH / 2, headerHeight / 2);

  // Alle Buttons zeichnen
  drawAllButtons();

  Serial.println("Touch Button Demo gestartet");
  Serial.println("Button 1: Momentary (aktiv solange gedrückt)");
  Serial.println("Button 2: Toggle (ein/aus)");
  Serial.println("Button 3-5: Radio Group (nur einer aktiv)");
}

void loop() {
  uint16_t x, y;
  bool isTouching = getTouchCalibrated(&lcd, x, y);

  if (isTouching) {
    // Prüfen welcher Button berührt wird
    int buttonIndex = getTouchedButton(x, y);

    if (buttonIndex != -1) {
      // Neuer Touch oder anderer Button
      if (!wasTouching || touchedButton != buttonIndex) {
        handleButtonPress(buttonIndex);
        touchedButton = buttonIndex;
        wasTouching = true;
      }

      // Für Momentary Button: Kontinuierlich aktiv halten
      if (buttonIndex == 0 && !button1Active) {
        button1Active = true;
        buttons[0].active = true;
        drawButton(0);
        updateLED();
        Serial.println("Button 1 (Momentary): GEDRÜCKT");
      }
    }
  } else {
    // Touch losgelassen
    if (wasTouching) {
      // Momentary Button deaktivieren
      if (button1Active) {
        button1Active = false;
        buttons[0].active = false;
        drawButton(0);
        updateLED();
        Serial.println("Button 1 (Momentary): LOSGELASSEN");
      }

      wasTouching = false;
      touchedButton = -1;
    }
  }

  delay(50);  // Kleine Pause
}

// Ermittelt welcher Button berührt wurde
int getTouchedButton(int x, int y) {
  for (int i = 0; i < 5; i++) {
    if (x >= buttons[i].x && x <= buttons[i].x + buttons[i].w &&
        y >= buttons[i].y && y <= buttons[i].y + buttons[i].h) {
      return i;
    }
  }
  return -1;
}

// Behandelt Button-Druck
void handleButtonPress(int index) {
  Serial.print("Button ");
  Serial.print(index + 1);
  Serial.print(" gedrückt (");

  if (index == 0) {
    // Momentary - wird in loop() behandelt
    Serial.println("Momentary)");
  }
  else if (index == 1) {
    // Toggle
    button2Active = !button2Active;
    buttons[1].active = button2Active;
    drawButton(1);
    updateLED();
    Serial.print("Toggle) - Status: ");
    Serial.println(button2Active ? "EIN" : "AUS");
  }
  else if (index >= 2 && index <= 4) {
    // Radio Group
    int radioIndex = index - 2;  // 0, 1, 2

    if (radioActive != radioIndex) {
      // Alten Button deaktivieren
      buttons[radioActive + 2].active = false;
      drawButton(radioActive + 2);

      // Neuen Button aktivieren
      radioActive = radioIndex;
      buttons[index].active = true;
      drawButton(index);
      updateLED();

      Serial.print("Radio) - Aktiv: ");
      Serial.println((char)('A' + radioIndex));
    } else {
      Serial.println("Radio) - Bereits aktiv");
    }
  }
}

// Zeichnet alle Buttons
void drawAllButtons() {
  for (int i = 0; i < 5; i++) {
    drawButton(i);
  }
}

// Zeichnet einen einzelnen Button
void drawButton(int index) {
  Button* btn = &buttons[index];

  // Farbe je nach Status
  uint16_t bgColor = btn->active ? TFT_GREEN : TFT_DARKGREY;
  uint16_t textColor = btn->active ? TFT_BLACK : TFT_LIGHTGREY;
  uint16_t borderColor = btn->active ? TFT_WHITE : 0x5AEB;  // Mittelgrau

  // Button-Hintergrund
  lcd.fillRoundRect(btn->x, btn->y, btn->w, btn->h, 8, bgColor);
  lcd.drawRoundRect(btn->x, btn->y, btn->w, btn->h, 8, borderColor);

  // Dickerer Rahmen für aktive Buttons
  if (btn->active) {
    lcd.drawRoundRect(btn->x + 1, btn->y + 1, btn->w - 2, btn->h - 2, 8, borderColor);
  }

  // Labels (zwei Zeilen)
  lcd.setTextColor(textColor, bgColor);
  lcd.setTextDatum(MC_DATUM);

  // Schriftgröße basierend auf Button-Größe
  int fontSize = btn->h / 50;
  if (fontSize < 1) fontSize = 1;
  if (fontSize > 2) fontSize = 2;
  lcd.setTextSize(fontSize);

  int centerX = btn->x + btn->w / 2;
  int centerY = btn->y + btn->h / 2;

  // Erste Zeile etwas höher
  lcd.drawString(btn->label1, centerX, centerY - fontSize * 5);
  // Zweite Zeile etwas tiefer
  lcd.drawString(btn->label2, centerX, centerY + fontSize * 5);

  // Status-Indikator (kleiner Kreis)
  int indicatorRadius = btn->w / 25;
  if (indicatorRadius < 3) indicatorRadius = 3;
  if (indicatorRadius > 6) indicatorRadius = 6;

  int indicatorX = btn->x + btn->w - indicatorRadius - 5;
  int indicatorY = btn->y + indicatorRadius + 5;

  lcd.fillCircle(indicatorX, indicatorY, indicatorRadius,
                 btn->active ? TFT_YELLOW : TFT_RED);
  lcd.drawCircle(indicatorX, indicatorY, indicatorRadius, TFT_BLACK);
}

// Aktualisiert RGB LED basierend auf Button-Status
void updateLED() {
  #ifdef LED_PIN_R
    // LED-Status:
    // Rot: Button 1 (Momentary) aktiv
    // Grün: Button 2 (Toggle) aktiv
    // Blau: NICHT verwendet (Radio-Buttons zeigen nichts an)

    digitalWrite(LED_PIN_R, button1Active ? LOW : HIGH);
    digitalWrite(LED_PIN_G, button2Active ? LOW : HIGH);
    // LED_PIN_B bleibt immer HIGH (OFF)
  #endif
}
