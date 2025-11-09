/*
  Beispiel 09b: Taschenrechner mit LVGL

  Ein einfacher Taschenrechner mit LVGL (Light and Versatile Graphics Library).

  Unterschiede zu 09_Calculator:
  - Verwendet LVGL statt direktes LovyanGFX Drawing
  - Button-Matrix statt einzelne Touch-Bereiche
  - Professionelles Styling und Layout
  - Ereignis-basierte Architektur

  Features:
  - Display im Portrait-Modus (Hochformat)
  - Touch-Buttons für Ziffern und Operatoren
  - Grundrechenarten: +, -, *, /
  - Dezimalpunkt-Unterstützung
  - Clear-Funktion
  - Automatisches Layout durch LVGL

  WICHTIG: Benötigt LVGL Library v8.x!
  Getestet mit v8.3.11
*/


#include <lvgl.h>
#include <CYD_Display_Config.h>

// Display-Variablen
int SCREEN_WIDTH;
int SCREEN_HEIGHT;

// LVGL Display-Buffer (1/10 der Bildschirmgröße)
#define DRAW_BUF_SIZE (240 * 320 / 10)
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[DRAW_BUF_SIZE];

// LVGL Display-Driver und Input-Driver
static lv_disp_drv_t disp_drv;
static lv_indev_drv_t indev_drv;

// UI-Elemente
lv_obj_t *label_display;
lv_obj_t *btnmatrix;

// Calculator-Zustand (identisch zu 09_Calculator)
String currentNumber = "0";
String displayText = "0";
double result = 0;
char operation = ' ';
bool newNumber = true;
bool hasDecimal = false;

// Button-Map für LVGL btnmatrix
static const char *btnm_map[] = {
    "7", "8", "9", "/", "\n",
    "4", "5", "6", "*", "\n",
    "1", "2", "3", "-", "\n",
    "C", "0", ".", "+", "\n",
    "=", ""  // = über volle Breite
};

LGFX lcd;

// Forward-Deklarationen
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void my_touchpad_read(lv_indev_drv_t *indev, lv_indev_data_t *data);
void btnmatrix_event_cb(lv_event_t *e);
void updateDisplay();
void calculate();
void handleButton(const char *label);

void setup() {
  Serial.begin(115200);
  Serial.println("LVGL Calculator starting...");
  Serial.println("LVGL v8.3.11");

  // LovyanGFX initialisieren
  lcd.init();
  lcd.setRotation(0);  // Portrait-Modus

  SCREEN_WIDTH = lcd.width();
  SCREEN_HEIGHT = lcd.height();

  Serial.printf("Display: %dx%d\n", SCREEN_WIDTH, SCREEN_HEIGHT);

  // RGB LED initialisieren
  #ifdef LED_PIN_R
    pinMode(LED_PIN_R, OUTPUT);
    pinMode(LED_PIN_G, OUTPUT);
    pinMode(LED_PIN_B, OUTPUT);
    digitalWrite(LED_PIN_R, HIGH);  // OFF
    digitalWrite(LED_PIN_G, HIGH);  // OFF
    digitalWrite(LED_PIN_B, HIGH);  // OFF
  #endif

  // LVGL initialisieren
  lv_init();

  // Display-Buffer initialisieren
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, DRAW_BUF_SIZE);

  // Display-Driver initialisieren
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = SCREEN_WIDTH;
  disp_drv.ver_res = SCREEN_HEIGHT;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // Touch-Input-Driver initialisieren
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  Serial.println("LVGL initialized");

  // UI erstellen
  createCalculatorUI();

  Serial.println("Calculator UI created");
}

void loop() {
  lv_tick_inc(5);      // LVGL Zeitbasis aktualisieren
  lv_timer_handler();  // LVGL Task-Handler (v8: lv_timer_handler)
  delay(5);
}

// LVGL Display Flush Callback
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  lcd.startWrite();
  lcd.setAddrWindow(area->x1, area->y1, w, h);
  lcd.pushPixels((uint16_t *)&color_p->full, w * h, true);
  lcd.endWrite();

  lv_disp_flush_ready(disp);
}

// LVGL Touch Input Callback
void my_touchpad_read(lv_indev_drv_t *indev, lv_indev_data_t *data) {
  uint16_t x, y;

  if (getTouchCalibrated(&lcd, x, y)) {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = x;
    data->point.y = y;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

// Erstellt die Calculator-UI
void createCalculatorUI() {
  // Hintergrund schwarz
  lv_obj_t *screen = lv_scr_act();
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);

  // Display-Label (oben, 25% der Höhe)
  int displayHeight = SCREEN_HEIGHT / 4;

  label_display = lv_label_create(screen);
  lv_obj_set_size(label_display, SCREEN_WIDTH, displayHeight);
  lv_obj_align(label_display, LV_ALIGN_TOP_MID, 0, 0);
  lv_label_set_text(label_display, "0");
  lv_obj_set_style_text_align(label_display, LV_TEXT_ALIGN_RIGHT, 0);

  // Font-Größe: Verwende den größten verfügbaren Font
  #if LV_FONT_MONTSERRAT_32
    lv_obj_set_style_text_font(label_display, &lv_font_montserrat_32, 0);
  #elif LV_FONT_MONTSERRAT_24
    lv_obj_set_style_text_font(label_display, &lv_font_montserrat_24, 0);
  #elif LV_FONT_MONTSERRAT_20
    lv_obj_set_style_text_font(label_display, &lv_font_montserrat_20, 0);
  #endif

  lv_obj_set_style_bg_color(label_display, lv_color_hex(0x404040), 0);
  lv_obj_set_style_bg_opa(label_display, LV_OPA_COVER, 0);
  lv_obj_set_style_text_color(label_display, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_pad_right(label_display, 10, 0);
  lv_obj_set_style_border_width(label_display, 2, 0);
  lv_obj_set_style_border_color(label_display, lv_color_hex(0xFFFFFF), 0);

  // Button-Matrix (unten, 75% der Höhe)
  int btnHeight = SCREEN_HEIGHT - displayHeight;

  btnmatrix = lv_btnmatrix_create(screen);
  lv_obj_set_size(btnmatrix, SCREEN_WIDTH, btnHeight);
  lv_obj_align(btnmatrix, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_btnmatrix_set_map(btnmatrix, btnm_map);
  lv_obj_add_event_cb(btnmatrix, btnmatrix_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // Button-Styling
  // Ziffern: Keine speziellen Controls nötig
  for (int i = 0; i < 10; i++) {
    String num = String(i);
    int btn_id = getBtnIndex(num.c_str());
    if (btn_id >= 0) {
      lv_btnmatrix_set_btn_ctrl(btnmatrix, btn_id, LV_BTNMATRIX_CTRL_NO_REPEAT);
    }
  }

  // "." Button
  int dot_id = getBtnIndex(".");
  if (dot_id >= 0) {
    lv_btnmatrix_set_btn_ctrl(btnmatrix, dot_id, LV_BTNMATRIX_CTRL_NO_REPEAT);
  }

  // "C" Button
  int c_id = getBtnIndex("C");
  if (c_id >= 0) {
    lv_btnmatrix_set_btn_ctrl(btnmatrix, c_id, LV_BTNMATRIX_CTRL_NO_REPEAT);
  }

  // "=" Button - volle Breite
  int eq_id = getBtnIndex("=");
  if (eq_id >= 0) {
    lv_btnmatrix_set_btn_width(btnmatrix, eq_id, 4);  // 4 Spalten breit
    lv_btnmatrix_set_btn_ctrl(btnmatrix, eq_id, LV_BTNMATRIX_CTRL_NO_REPEAT);
  }

  // Operatoren
  const char *ops[] = {"+", "-", "*", "/"};
  for (int i = 0; i < 4; i++) {
    int op_id = getBtnIndex(ops[i]);
    if (op_id >= 0) {
      lv_btnmatrix_set_btn_ctrl(btnmatrix, op_id, LV_BTNMATRIX_CTRL_NO_REPEAT);
    }
  }

  Serial.println("UI styling complete");
}

// Findet Button-Index in der Map
int getBtnIndex(const char *label) {
  for (int i = 0; btnm_map[i][0] != '\0'; i++) {
    if (strcmp(btnm_map[i], "\n") == 0) continue;
    if (strcmp(btnm_map[i], label) == 0) {
      return i;
    }
  }
  return -1;
}

// Button-Matrix Event Callback
void btnmatrix_event_cb(lv_event_t *e) {
  lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
  uint32_t btn_id = lv_btnmatrix_get_selected_btn(obj);
  const char *btn_text = lv_btnmatrix_get_btn_text(obj, btn_id);

  if (btn_text != NULL && strcmp(btn_text, "\n") != 0) {
    Serial.printf("Button pressed: %s\n", btn_text);
    handleButton(btn_text);
  }
}

// Button-Handler (identische Logik wie 09_Calculator)
void handleButton(const char *label) {
  String labelStr = String(label);

  if (labelStr == "C") {
    // Clear
    currentNumber = "0";
    displayText = "0";
    result = 0;
    operation = ' ';
    newNumber = true;
    hasDecimal = false;
  }
  else if (labelStr == "=") {
    // Berechnung durchführen
    calculate();
    operation = ' ';
    newNumber = true;
    hasDecimal = false;
  }
  else if (labelStr == "+" || labelStr == "-" || labelStr == "*" || labelStr == "/") {
    // Operator
    if (operation != ' ' && !newNumber) {
      calculate();
    } else {
      result = currentNumber.toDouble();
    }
    operation = labelStr[0];
    newNumber = true;
    hasDecimal = false;
    displayText = String(result);
  }
  else if (labelStr == ".") {
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
      currentNumber = labelStr;
      displayText = labelStr;
      newNumber = false;
      hasDecimal = false;
    } else {
      if (currentNumber.length() < 12) {
        currentNumber += labelStr;
        displayText = currentNumber;
      }
    }
  }

  updateDisplay();
}

// Display aktualisieren
void updateDisplay() {
  String displayStr = displayText;
  if (displayStr.length() > 15) {
    displayStr = displayStr.substring(0, 15);
  }
  lv_label_set_text(label_display, displayStr.c_str());
}

// Berechnung durchführen (identisch zu 09_Calculator)
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
          digitalWrite(LED_PIN_R, LOW);  // Rot bei Fehler
          delay(500);
          digitalWrite(LED_PIN_R, HIGH);
        #endif

        updateDisplay();
        return;
      }
      break;
    default:
      result = value;
  }

  // Ergebnis formatieren
  if (result == (int)result && abs(result) < 1e9) {
    displayText = String((int)result);
  } else {
    displayText = String(result, 6);

    // Trailing Zeros entfernen
    while (displayText.indexOf('.') != -1 &&
           (displayText.endsWith("0") || displayText.endsWith("."))) {
      displayText.remove(displayText.length() - 1);
    }
  }

  currentNumber = displayText;

  Serial.printf("Result: %f\n", result);

  #ifdef LED_PIN_G
    digitalWrite(LED_PIN_G, LOW);  // Grün bei erfolgreicher Berechnung
    delay(100);
    digitalWrite(LED_PIN_G, HIGH);
  #endif
}
