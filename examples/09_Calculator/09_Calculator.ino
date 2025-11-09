/*
  Beispiel 09: Taschenrechner

  Ein einfacher Taschenrechner mit Touch-Eingabe.
  - Display im Portrait-Modus (Hochformat)
  - Touch-Buttons für Ziffern und Operatoren
  - Grundrechenarten: +, -, *, /
  - Dezimalpunkt-Unterstützung
  - Clear-Funktion
  - Dynamisch skalierbar für verschiedene Display-Größen
*/

#include <CYD_Display_Config.h>

// Display-Variablen (werden dynamisch gesetzt)
int SCREEN_WIDTH;
int SCREEN_HEIGHT;

// Button-Layout
int DISPLAY_HEIGHT;  // Höhe des Anzeigebereichs
int BUTTON_ROWS = 5;
int BUTTON_COLS = 4;
int BUTTON_WIDTH;
int BUTTON_HEIGHT;
int BUTTON_SPACING = 2;

// Taschenrechner-Zustand
String currentNumber = "0";
String displayText = "0";
double result = 0;
char operation = ' ';
bool newNumber = true;
bool hasDecimal = false;

// Button-Definitionen (5 Zeilen x 4 Spalten)
const char* buttonLabels[5][4] = {
  {"7", "8", "9", "/"},
  {"4", "5", "6", "*"},
  {"1", "2", "3", "-"},
  {"C", "0", ".", "+"},
  {"=", "=", "=", "="}  // = über volle Breite
};

LGFX lcd;

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.setRotation(0);  // Portrait-Modus!

  // Display-Größe ermitteln
  SCREEN_WIDTH = lcd.width();
  SCREEN_HEIGHT = lcd.height();

  // Layout berechnen
  DISPLAY_HEIGHT = SCREEN_HEIGHT / 4;  // 25% für Display
  int buttonAreaHeight = SCREEN_HEIGHT - DISPLAY_HEIGHT;
  BUTTON_HEIGHT = (buttonAreaHeight - (BUTTON_ROWS + 1) * BUTTON_SPACING) / BUTTON_ROWS;
  BUTTON_WIDTH = (SCREEN_WIDTH - (BUTTON_COLS + 1) * BUTTON_SPACING) / BUTTON_COLS;

  // Optional: RGB LED initialisieren
  #ifdef LED_PIN_R
    pinMode(LED_PIN_R, OUTPUT);
    pinMode(LED_PIN_G, OUTPUT);
    pinMode(LED_PIN_B, OUTPUT);
    digitalWrite(LED_PIN_R, HIGH);  // OFF
    digitalWrite(LED_PIN_G, HIGH);  // OFF
    digitalWrite(LED_PIN_B, HIGH);  // OFF
  #endif

  lcd.fillScreen(TFT_BLACK);

  // Taschenrechner-UI zeichnen
  drawCalculator();
}

void loop() {
  uint16_t x, y;

  if (getTouchCalibrated(&lcd, x, y)) {
    // Prüfen ob Touch im Button-Bereich
    if (y > DISPLAY_HEIGHT) {
      handleButtonPress(x, y);
      delay(200);  // Debounce
    }
  }
}

// Zeichnet den kompletten Taschenrechner
void drawCalculator() {
  lcd.fillScreen(TFT_BLACK);

  // Display-Bereich
  drawDisplay();

  // Buttons zeichnen
  for (int row = 0; row < BUTTON_ROWS; row++) {
    for (int col = 0; col < BUTTON_COLS; col++) {
      // Für "=" Button: nur einmal zeichnen (volle Breite)
      if (row == 4 && col > 0) continue;

      drawButton(row, col);
    }
  }
}

// Zeichnet den Display-Bereich mit aktuellem Text
void drawDisplay() {
  // Display-Hintergrund
  lcd.fillRect(0, 0, SCREEN_WIDTH, DISPLAY_HEIGHT, TFT_DARKGREY);
  lcd.drawRect(0, 0, SCREEN_WIDTH, DISPLAY_HEIGHT, TFT_WHITE);

  // Text anzeigen (rechtsbündig)
  lcd.setTextDatum(MR_DATUM);  // Middle Right
  lcd.setTextColor(TFT_WHITE, TFT_DARKGREY);

  // Schriftgröße dynamisch anpassen
  int fontSize = DISPLAY_HEIGHT / 50;
  if (fontSize < 2) fontSize = 2;
  if (fontSize > 7) fontSize = 7;
  lcd.setTextSize(fontSize);

  int textY = DISPLAY_HEIGHT / 2;
  int textX = SCREEN_WIDTH - 10;

  // Text kürzen wenn zu lang
  String displayStr = displayText;
  if (displayStr.length() > 15) {
    displayStr = displayStr.substring(0, 15);
  }

  lcd.drawString(displayStr, textX, textY);
}

// Zeichnet einen einzelnen Button
void drawButton(int row, int col) {
  int x = BUTTON_SPACING + col * (BUTTON_WIDTH + BUTTON_SPACING);
  int y = DISPLAY_HEIGHT + BUTTON_SPACING + row * (BUTTON_HEIGHT + BUTTON_SPACING);

  // Für "=" Button: volle Breite
  int width = BUTTON_WIDTH;
  if (row == 4) {
    width = SCREEN_WIDTH - 2 * BUTTON_SPACING;
  }

  // Button-Farbe je nach Typ
  uint16_t buttonColor = TFT_DARKGREY;
  uint16_t textColor = TFT_WHITE;

  String label = buttonLabels[row][col];

  if (label == "C") {
    buttonColor = TFT_RED;
  } else if (label == "=") {
    buttonColor = TFT_BLUE;
  } else if (label == "+" || label == "-" || label == "*" || label == "/") {
    buttonColor = TFT_ORANGE;
  } else if (label == "0" || label == "." ||
             (label[0] >= '1' && label[0] <= '9')) {
    buttonColor = TFT_LIGHTGREY;
    textColor = TFT_BLACK;
  }

  // Button zeichnen
  lcd.fillRoundRect(x, y, width, BUTTON_HEIGHT, 8, buttonColor);
  lcd.drawRoundRect(x, y, width, BUTTON_HEIGHT, 8, TFT_WHITE);

  // Label zentriert
  lcd.setTextDatum(MC_DATUM);
  lcd.setTextColor(textColor, buttonColor);

  // Schriftgröße für Buttons
  int fontSize = BUTTON_HEIGHT / 30;
  if (fontSize < 2) fontSize = 2;
  if (fontSize > 4) fontSize = 4;
  lcd.setTextSize(fontSize);

  int centerX = x + width / 2;
  int centerY = y + BUTTON_HEIGHT / 2;

  lcd.drawString(label, centerX, centerY);
}

// Behandelt Button-Druck
void handleButtonPress(int touchX, int touchY) {
  // Button-Position berechnen
  int buttonY = touchY - DISPLAY_HEIGHT - BUTTON_SPACING;
  int row = buttonY / (BUTTON_HEIGHT + BUTTON_SPACING);
  int col = (touchX - BUTTON_SPACING) / (BUTTON_WIDTH + BUTTON_SPACING);

  // Validierung
  if (row < 0 || row >= BUTTON_ROWS || col < 0 || col >= BUTTON_COLS) {
    return;
  }

  String label = buttonLabels[row][col];

  Serial.print("Button gedrückt: ");
  Serial.println(label);

  // Button-Aktion ausführen
  if (label == "C") {
    // Clear
    currentNumber = "0";
    displayText = "0";
    result = 0;
    operation = ' ';
    newNumber = true;
    hasDecimal = false;
  }
  else if (label == "=") {
    // Berechnung durchführen
    calculate();
    operation = ' ';
    newNumber = true;
    hasDecimal = false;
  }
  else if (label == "+" || label == "-" || label == "*" || label == "/") {
    // Operator
    if (operation != ' ' && !newNumber) {
      calculate();
    } else {
      result = currentNumber.toDouble();
    }
    operation = label[0];
    newNumber = true;
    hasDecimal = false;
    displayText = String(result);
  }
  else if (label == ".") {
    // Dezimalpunkt
    if (!hasDecimal) {
      if (newNumber) {
        currentNumber = "0.";
        displayText = "0.";
        newNumber = false;
      } else {
        currentNumber += ".";
        displayText = currentNumber;
      }
      hasDecimal = true;
    }
  }
  else {
    // Ziffer (0-9)
    if (newNumber) {
      currentNumber = label;
      displayText = label;
      newNumber = false;
      hasDecimal = false;
    } else {
      // Limitierung auf max. 12 Zeichen
      if (currentNumber.length() < 12) {
        currentNumber += label;
        displayText = currentNumber;
      }
    }
  }

  // Display aktualisieren
  drawDisplay();

  // Visuelles Feedback: Button kurz highlighten
  int x = BUTTON_SPACING + col * (BUTTON_WIDTH + BUTTON_SPACING);
  int y = DISPLAY_HEIGHT + BUTTON_SPACING + row * (BUTTON_HEIGHT + BUTTON_SPACING);
  int width = (row == 4) ? SCREEN_WIDTH - 2 * BUTTON_SPACING : BUTTON_WIDTH;

  lcd.drawRoundRect(x, y, width, BUTTON_HEIGHT, 8, TFT_YELLOW);
  delay(50);
  lcd.drawRoundRect(x, y, width, BUTTON_HEIGHT, 8, TFT_WHITE);
}

// Führt die Berechnung durch
void calculate() {
  double value = currentNumber.toDouble();

  switch (operation) {
    case '+':
      result = result + value;
      break;
    case '-':
      result = result - value;
      break;
    case '*':
      result = result * value;
      break;
    case '/':
      if (value != 0) {
        result = result / value;
      } else {
        displayText = "Error";
        result = 0;
        Serial.println("Division durch Null!");

        #ifdef LED_PIN_R
          digitalWrite(LED_PIN_R, HIGH);  // HIGH = OFF  // Rot bei Fehler
          delay(500);
          digitalWrite(LED_PIN_R, HIGH);
        #endif

        drawDisplay();
        return;
      }
      break;
    default:
      result = value;
  }

  // Ergebnis formatieren
  // Wenn Ganzzahl, ohne Dezimalstellen anzeigen
  if (result == (int)result && abs(result) < 1e9) {
    displayText = String((int)result);
  } else {
    displayText = String(result, 6);  // Max. 6 Dezimalstellen

    // Trailing Zeros entfernen
    while (displayText.indexOf('.') != -1 &&
           (displayText.endsWith("0") || displayText.endsWith("."))) {
      displayText.remove(displayText.length() - 1);
    }
  }

  currentNumber = displayText;

  Serial.print("Ergebnis: ");
  Serial.println(result);

  #ifdef LED_PIN_G
    digitalWrite(LED_PIN_G, HIGH);  // HIGH = OFF  // Grün bei erfolgreicher Berechnung
    delay(100);
    digitalWrite(LED_PIN_G, HIGH);
  #endif
}
