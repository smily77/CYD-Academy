/*
  Beispiel 11: Scrollendes Diagramm (Chart)

  Klassisches scrollendes Liniendiagramm:
  - Neue Datenpunkte kommen rechts herein
  - Chart scrollt kontinuierlich nach links
  - Y-Achse mit Skala und Grid-Linien
  - Simulierte Daten (Sinus mit Rauschen)
  - Dynamisch skalierbar für verschiedene Display-Breiten (2.8" vs 3.5")

  Perfekt für Live-Daten-Visualisierung!
*/

#include <CYD_Display_Config.h>

// Display-Variablen (werden dynamisch gesetzt)
int SCREEN_WIDTH;
int SCREEN_HEIGHT;

// Chart-Bereich
int CHART_X;          // Start X
int CHART_Y;          // Start Y
int CHART_WIDTH;      // Breite des Chart-Bereichs
int CHART_HEIGHT;     // Höhe des Chart-Bereichs
int Y_AXIS_WIDTH;     // Breite der Y-Achse (für Labels)
int X_AXIS_HEIGHT;    // Höhe der X-Achse (für Labels)

// Datenpunkte
int* dataPoints;      // Dynamisches Array für Datenpunkte
int numPoints;        // Anzahl Punkte (= CHART_WIDTH)

// Chart-Parameter
const int MIN_VALUE = 0;
const int MAX_VALUE = 100;
int updateInterval = 100;  // ms zwischen Updates
unsigned long lastUpdate = 0;

// Simulation
float simulationValue = 50.0;
float simulationAngle = 0.0;

LGFX lcd;

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.setRotation(1);  // Landscape

  // Display-Größe ermitteln
  SCREEN_WIDTH = lcd.width();
  SCREEN_HEIGHT = lcd.height();

  // Chart-Bereich berechnen
  Y_AXIS_WIDTH = SCREEN_WIDTH / 10;  // 10% für Y-Achse
  X_AXIS_HEIGHT = SCREEN_HEIGHT / 8; // 12.5% für X-Achse

  CHART_X = Y_AXIS_WIDTH;
  CHART_Y = 5;  // Kleiner Rand oben
  CHART_WIDTH = SCREEN_WIDTH - Y_AXIS_WIDTH - 5;  // 5px Rand rechts
  CHART_HEIGHT = SCREEN_HEIGHT - X_AXIS_HEIGHT - 5;

  // Datenpunkte-Array initialisieren
  numPoints = CHART_WIDTH;
  dataPoints = new int[numPoints];

  // Mit Mittelwerten initialisieren
  for (int i = 0; i < numPoints; i++) {
    dataPoints[i] = 50;
  }

  // RGB LED initialisieren
  #ifdef LED_PIN_R
    pinMode(LED_PIN_R, OUTPUT);
    pinMode(LED_PIN_G, OUTPUT);
    pinMode(LED_PIN_B, OUTPUT);
    digitalWrite(LED_PIN_R, HIGH);  // HIGH = OFF
    digitalWrite(LED_PIN_G, HIGH);  // HIGH = OFF
    digitalWrite(LED_PIN_B, HIGH);  // HIGH = OFF
  #endif

  lcd.fillScreen(TFT_BLACK);

  // Statische Elemente einmalig zeichnen
  drawStaticElements();

  // Initial Chart zeichnen
  updateChart();

  Serial.println("Scrolling Chart gestartet");
  Serial.print("Chart-Breite: ");
  Serial.print(CHART_WIDTH);
  Serial.println(" Pixel");
}

void loop() {
  unsigned long now = millis();

  if (now - lastUpdate >= updateInterval) {
    lastUpdate = now;

    // Neuen Datenpunkt holen
    int newValue = getNewDataPoint();

    // Datenpunkte nach links verschieben
    for (int i = 0; i < numPoints - 1; i++) {
      dataPoints[i] = dataPoints[i + 1];
    }

    // Neuen Punkt rechts einfügen
    dataPoints[numPoints - 1] = newValue;

    // Chart aktualisieren
    updateChart();

    // LED basierend auf Wert
    #ifdef LED_PIN_R
      updateLED(newValue);
    #endif
  }

  delay(10);
}

// Holt einen neuen Datenpunkt (simuliert)
int getNewDataPoint() {
  // Simulierte Daten: Sinus-Welle mit etwas Rauschen
  simulationAngle += 0.05;
  float sinValue = sin(simulationAngle) * 30 + 50;  // 20-80
  float noise = random(-5, 6);  // -5 bis +5
  simulationValue = sinValue + noise;

  int value = (int)constrain(simulationValue, MIN_VALUE, MAX_VALUE);

  return value;
}

// Zeichnet statische Elemente (einmalig beim Start)
void drawStaticElements() {
  // Chart-Rahmen
  lcd.drawRect(CHART_X - 1, CHART_Y - 1, CHART_WIDTH + 2, CHART_HEIGHT + 2, TFT_WHITE);

  // Y-Achsen-Beschriftung
  lcd.setTextDatum(MR_DATUM);
  lcd.setTextColor(TFT_CYAN);
  lcd.setTextSize(1);

  for (int i = 0; i <= 4; i++) {
    int value = MAX_VALUE - (MAX_VALUE - MIN_VALUE) * i / 4;
    int y = CHART_Y + (CHART_HEIGHT * i / 4);
    lcd.drawNumber(value, Y_AXIS_WIDTH - 3, y);
  }

  // Info-Text unten
  lcd.setTextDatum(BL_DATUM);
  lcd.setTextColor(TFT_YELLOW);
  lcd.setTextSize(1);
  lcd.drawString("Simuliert (Sinus)", 5, SCREEN_HEIGHT - 2);

  lcd.setTextDatum(BR_DATUM);
  String fps = String(1000 / updateInterval) + " Updates/s";
  lcd.drawString(fps, SCREEN_WIDTH - 5, SCREEN_HEIGHT - 2);
}

// Aktualisiert Chart-Bereich
void updateChart() {
  // Chart-Bereich löschen
  lcd.fillRect(CHART_X, CHART_Y, CHART_WIDTH, CHART_HEIGHT, TFT_BLACK);

  // Grid-Linien zeichnen
  lcd.startWrite();  // Batch-Modus für schnelleres Zeichnen

  for (int i = 0; i <= 4; i++) {
    int y = CHART_Y + (CHART_HEIGHT * i / 4);
    lcd.drawFastHLine(CHART_X, y, CHART_WIDTH, TFT_DARKGREY);
  }

  // Datenpunkte zeichnen
  drawDataPoints();

  lcd.endWrite();  // Batch-Modus beenden
}

// Zeichnet die Datenpunkte als Linie
void drawDataPoints() {
  // Punkte von links nach rechts durchgehen
  for (int i = 0; i < numPoints - 1; i++) {
    int x1 = CHART_X + i;
    int x2 = CHART_X + i + 1;

    // Y-Position berechnen (invertiert: hoher Wert = oben)
    int y1 = mapValue(dataPoints[i]);
    int y2 = mapValue(dataPoints[i + 1]);

    // Farbe basierend auf Wert (Gradient)
    uint16_t color = getColorForValue(dataPoints[i]);

    // Linie zeichnen
    lcd.drawLine(x1, y1, x2, y2, color);
  }

  // Neuester Punkt rechts hervorheben
  int x = CHART_X + numPoints - 1;
  int y = mapValue(dataPoints[numPoints - 1]);
  lcd.fillCircle(x, y, 2, TFT_YELLOW);
  lcd.drawCircle(x, y, 3, TFT_WHITE);
}

// Mappt einen Datenwert auf Y-Koordinate
int mapValue(int value) {
  // Hoher Wert = oben, niedriger Wert = unten
  int y = map(value, MIN_VALUE, MAX_VALUE,
              CHART_Y + CHART_HEIGHT - 1, CHART_Y);
  return constrain(y, CHART_Y, CHART_Y + CHART_HEIGHT - 1);
}

// Gibt Farbe basierend auf Wert zurück (Gradient)
uint16_t getColorForValue(int value) {
  if (value < 25) {
    return TFT_BLUE;
  } else if (value < 50) {
    return TFT_CYAN;
  } else if (value < 75) {
    return TFT_GREEN;
  } else {
    return TFT_RED;
  }
}

#ifdef LED_PIN_R
// Aktualisiert RGB LED basierend auf Wert
void updateLED(int value) {
  // Rot: Hohe Werte (>75)
  digitalWrite(LED_PIN_R, (value > 75) ? LOW : HIGH);

  // Grün: Mittlere Werte (25-75)
  digitalWrite(LED_PIN_G, (value >= 25 && value <= 75) ? LOW : HIGH);

  // Blau: Niedrige Werte (<25)
  digitalWrite(LED_PIN_B, (value < 25) ? LOW : HIGH);
}
#endif
