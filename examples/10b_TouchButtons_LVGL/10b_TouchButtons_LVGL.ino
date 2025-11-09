/*
  Beispiel 10b: Touch-Buttons mit LVGL

  Demonstriert drei verschiedene Button-Typen mit LVGL:
  1. Momentary Button - Aktiv solange gedrückt
  2. Toggle Button - Schaltet bei jedem Druck um (ein/aus)
  3. Radio Button Gruppe - Nur einer kann aktiv sein

  Unterschiede zu 10_TouchButtons:
  - Verwendet LVGL statt direktes LovyanGFX Drawing
  - Event-basierte Architektur statt Touch-Polling
  - Professionelles Styling mit Schatten und Animationen
  - Automatisches Layout und Skalierung
  - Moderne UI-Elemente

  Features:
  - Display im Landscape-Modus (Querformat)
  - RGB LED Integration
  - Touch-Feedback mit Animationen
  - Status-Anzeige für jeden Button-Typ
  - Dynamisch skalierbar für verschiedene Display-Größen

  WICHTIG: Benötigt LVGL Library v8.x!
  Getestet mit v8.3.11
*/

#include <lvgl.h>
#include <CYD_Display_Config.h>

// Display-Variablen
int SCREEN_WIDTH;
int SCREEN_HEIGHT;

// LVGL Display-Buffer (1/10 der Bildschirmgröße)
#define DRAW_BUF_SIZE (320 * 240 / 10)
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[DRAW_BUF_SIZE];

// LVGL Display-Driver und Input-Driver
static lv_disp_drv_t disp_drv;
static lv_indev_drv_t indev_drv;

// UI-Elemente
lv_obj_t *label_title;
lv_obj_t *label_status;

// Buttons
lv_obj_t *btn_momentary;
lv_obj_t *btn_toggle;
lv_obj_t *btn_radio_a;
lv_obj_t *btn_radio_b;
lv_obj_t *btn_radio_c;

// Button-Labels
lv_obj_t *label_momentary;
lv_obj_t *label_toggle;
lv_obj_t *label_radio_a;
lv_obj_t *label_radio_b;
lv_obj_t *label_radio_c;

// Button-Zustände
bool button1Active = false;  // Momentary
bool button2Active = false;  // Toggle
int radioActive = 0;         // Radio Group (0=A, 1=B, 2=C)

LGFX lcd;

// Forward-Deklarationen
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void my_touchpad_read(lv_indev_drv_t *indev, lv_indev_data_t *data);
void btn_momentary_event_cb(lv_event_t *e);
void btn_toggle_event_cb(lv_event_t *e);
void btn_radio_event_cb(lv_event_t *e);
void updateLED();
void updateStatusLabel();
void createButtonUI();

void setup() {
  Serial.begin(115200);
  Serial.println("LVGL Touch Buttons starting...");
  Serial.println("LVGL v8.3.11");

  // LovyanGFX initialisieren
  lcd.init();
  lcd.setRotation(1);  // Landscape-Modus

  SCREEN_WIDTH = lcd.width();
  SCREEN_HEIGHT = lcd.height();

  Serial.printf("Display: %dx%d (Landscape)\n", SCREEN_WIDTH, SCREEN_HEIGHT);

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
  createButtonUI();

  Serial.println("Touch Button UI created");
  Serial.println("Button 1: Momentary (aktiv solange gedrückt)");
  Serial.println("Button 2: Toggle (ein/aus)");
  Serial.println("Button 3-5: Radio Group (nur einer aktiv)");
}

void loop() {
  lv_tick_inc(5);      // LVGL Zeitbasis aktualisieren
  lv_timer_handler();  // LVGL Task-Handler
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

// Erstellt die Button-UI
void createButtonUI() {
  lv_obj_t *screen = lv_scr_act();

  // Dunkler Hintergrund mit Gradient-Effekt
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x1a1a1a), 0);
  lv_obj_set_style_bg_grad_color(screen, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);

  // Titel-Label (oben)
  label_title = lv_label_create(screen);
  lv_label_set_text(label_title, "Touch Button Demo");
  lv_obj_set_style_text_color(label_title, lv_color_hex(0xFFFFFF), 0);

  #if LV_FONT_MONTSERRAT_20
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_20, 0);
  #elif LV_FONT_MONTSERRAT_16
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_16, 0);
  #endif

  lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 5);

  // Status-Label (ganz unten)
  label_status = lv_label_create(screen);
  lv_label_set_text(label_status, "Status: Radio A aktiv");
  lv_obj_set_style_text_color(label_status, lv_color_hex(0x00FF00), 0);
  lv_obj_align(label_status, LV_ALIGN_BOTTOM_MID, 0, -5);

  // Layout-Berechnungen
  int header_h = 30;
  int footer_h = 20;
  int available_h = SCREEN_HEIGHT - header_h - footer_h - 20;
  int button_area_y = header_h + 10;

  int spacing = 8;
  int top_btn_w = (SCREEN_WIDTH - 3 * spacing) / 2;
  int top_btn_h = (available_h - spacing) / 2;
  int bot_btn_w = (SCREEN_WIDTH - 4 * spacing) / 3;
  int bot_btn_h = (available_h - spacing) / 2;

  // ===== Button 1: MOMENTARY (oben links) =====
  btn_momentary = lv_btn_create(screen);
  lv_obj_set_size(btn_momentary, top_btn_w, top_btn_h);
  lv_obj_set_pos(btn_momentary, spacing, button_area_y);

  // Styling
  lv_obj_set_style_bg_color(btn_momentary, lv_color_hex(0x404040), LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(btn_momentary, lv_color_hex(0xFF4444), LV_STATE_PRESSED);
  lv_obj_set_style_shadow_width(btn_momentary, 8, 0);
  lv_obj_set_style_shadow_opa(btn_momentary, LV_OPA_50, 0);
  lv_obj_set_style_radius(btn_momentary, 12, 0);

  // Event Callbacks
  lv_obj_add_event_cb(btn_momentary, btn_momentary_event_cb, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(btn_momentary, btn_momentary_event_cb, LV_EVENT_RELEASED, NULL);

  // Label
  label_momentary = lv_label_create(btn_momentary);
  lv_label_set_text(label_momentary, "MOMENTARY\n(Halten)");
  lv_obj_set_style_text_align(label_momentary, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_center(label_momentary);

  // ===== Button 2: TOGGLE (oben rechts) =====
  btn_toggle = lv_btn_create(screen);
  lv_obj_set_size(btn_toggle, top_btn_w, top_btn_h);
  lv_obj_set_pos(btn_toggle, 2 * spacing + top_btn_w, button_area_y);

  // Styling
  lv_obj_set_style_bg_color(btn_toggle, lv_color_hex(0x404040), LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_width(btn_toggle, 8, 0);
  lv_obj_set_style_shadow_opa(btn_toggle, LV_OPA_50, 0);
  lv_obj_set_style_radius(btn_toggle, 12, 0);

  // Event
  lv_obj_add_event_cb(btn_toggle, btn_toggle_event_cb, LV_EVENT_CLICKED, NULL);

  // Label
  label_toggle = lv_label_create(btn_toggle);
  lv_label_set_text(label_toggle, "TOGGLE\n(Ein/Aus)");
  lv_obj_set_style_text_align(label_toggle, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_center(label_toggle);

  // ===== Button 3: RADIO A (unten links) =====
  btn_radio_a = lv_btn_create(screen);
  lv_obj_set_size(btn_radio_a, bot_btn_w, bot_btn_h);
  lv_obj_set_pos(btn_radio_a, spacing, button_area_y + top_btn_h + spacing);

  // Styling (aktiv)
  lv_obj_set_style_bg_color(btn_radio_a, lv_color_hex(0x4444FF), LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_width(btn_radio_a, 8, 0);
  lv_obj_set_style_shadow_opa(btn_radio_a, LV_OPA_50, 0);
  lv_obj_set_style_radius(btn_radio_a, 12, 0);
  lv_obj_set_style_border_width(btn_radio_a, 3, 0);
  lv_obj_set_style_border_color(btn_radio_a, lv_color_hex(0xFFFFFF), 0);

  // Event
  lv_obj_add_event_cb(btn_radio_a, btn_radio_event_cb, LV_EVENT_CLICKED, (void*)0);

  // Label
  label_radio_a = lv_label_create(btn_radio_a);
  lv_label_set_text(label_radio_a, "RADIO\nA");
  lv_obj_set_style_text_align(label_radio_a, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_center(label_radio_a);

  // ===== Button 4: RADIO B (unten mitte) =====
  btn_radio_b = lv_btn_create(screen);
  lv_obj_set_size(btn_radio_b, bot_btn_w, bot_btn_h);
  lv_obj_set_pos(btn_radio_b, 2 * spacing + bot_btn_w, button_area_y + top_btn_h + spacing);

  // Styling (inaktiv)
  lv_obj_set_style_bg_color(btn_radio_b, lv_color_hex(0x404040), LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_width(btn_radio_b, 8, 0);
  lv_obj_set_style_shadow_opa(btn_radio_b, LV_OPA_50, 0);
  lv_obj_set_style_radius(btn_radio_b, 12, 0);
  lv_obj_set_style_border_width(btn_radio_b, 0, 0);

  // Event
  lv_obj_add_event_cb(btn_radio_b, btn_radio_event_cb, LV_EVENT_CLICKED, (void*)1);

  // Label
  label_radio_b = lv_label_create(btn_radio_b);
  lv_label_set_text(label_radio_b, "RADIO\nB");
  lv_obj_set_style_text_align(label_radio_b, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_center(label_radio_b);

  // ===== Button 5: RADIO C (unten rechts) =====
  btn_radio_c = lv_btn_create(screen);
  lv_obj_set_size(btn_radio_c, bot_btn_w, bot_btn_h);
  lv_obj_set_pos(btn_radio_c, 3 * spacing + 2 * bot_btn_w, button_area_y + top_btn_h + spacing);

  // Styling (inaktiv)
  lv_obj_set_style_bg_color(btn_radio_c, lv_color_hex(0x404040), LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_width(btn_radio_c, 8, 0);
  lv_obj_set_style_shadow_opa(btn_radio_c, LV_OPA_50, 0);
  lv_obj_set_style_radius(btn_radio_c, 12, 0);
  lv_obj_set_style_border_width(btn_radio_c, 0, 0);

  // Event
  lv_obj_add_event_cb(btn_radio_c, btn_radio_event_cb, LV_EVENT_CLICKED, (void*)2);

  // Label
  label_radio_c = lv_label_create(btn_radio_c);
  lv_label_set_text(label_radio_c, "RADIO\nC");
  lv_obj_set_style_text_align(label_radio_c, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_center(label_radio_c);
}

// Momentary Button Event Handler
void btn_momentary_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_PRESSED) {
    button1Active = true;
    updateLED();
    updateStatusLabel();
    Serial.println("Button 1 (Momentary): GEDRÜCKT");
  }
  else if (code == LV_EVENT_RELEASED) {
    button1Active = false;
    updateLED();
    updateStatusLabel();
    Serial.println("Button 1 (Momentary): LOSGELASSEN");
  }
}

// Toggle Button Event Handler
void btn_toggle_event_cb(lv_event_t *e) {
  button2Active = !button2Active;

  if (button2Active) {
    lv_obj_set_style_bg_color(btn_toggle, lv_color_hex(0x44FF44), LV_STATE_DEFAULT);
  } else {
    lv_obj_set_style_bg_color(btn_toggle, lv_color_hex(0x404040), LV_STATE_DEFAULT);
  }

  updateLED();
  updateStatusLabel();
  Serial.printf("Button 2 (Toggle): %s\n", button2Active ? "EIN" : "AUS");
}

// Radio Button Event Handler
void btn_radio_event_cb(lv_event_t *e) {
  int new_radio = (int)lv_event_get_user_data(e);

  if (radioActive == new_radio) {
    Serial.printf("Radio %c bereits aktiv\n", 'A' + new_radio);
    return;
  }

  // Alten Button deaktivieren
  lv_obj_t *old_btn = (radioActive == 0) ? btn_radio_a :
                      (radioActive == 1) ? btn_radio_b : btn_radio_c;
  lv_obj_set_style_bg_color(old_btn, lv_color_hex(0x404040), LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(old_btn, 0, 0);

  // Neuen Button aktivieren
  lv_obj_t *new_btn = (new_radio == 0) ? btn_radio_a :
                      (new_radio == 1) ? btn_radio_b : btn_radio_c;
  lv_obj_set_style_bg_color(new_btn, lv_color_hex(0x4444FF), LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(new_btn, 3, 0);
  lv_obj_set_style_border_color(new_btn, lv_color_hex(0xFFFFFF), 0);

  radioActive = new_radio;
  updateStatusLabel();
  Serial.printf("Radio %c aktiviert\n", 'A' + new_radio);
}

// Aktualisiert RGB LED basierend auf Button-Status
void updateLED() {
  #ifdef LED_PIN_R
    // LED-Status:
    // Rot: Button 1 (Momentary) aktiv
    // Grün: Button 2 (Toggle) aktiv
    // Blau: NICHT verwendet

    digitalWrite(LED_PIN_R, button1Active ? LOW : HIGH);
    digitalWrite(LED_PIN_G, button2Active ? LOW : HIGH);
  #endif
}

// Aktualisiert Status-Label
void updateStatusLabel() {
  String status = "Status: ";

  if (button1Active) {
    status += "Momentary GEDRÜCKT";
  } else if (button2Active) {
    status += "Toggle EIN";
  } else {
    status += "Radio ";
    status += (char)('A' + radioActive);
    status += " aktiv";
  }

  lv_label_set_text(label_status, status.c_str());

  // Farbe je nach Status
  if (button1Active) {
    lv_obj_set_style_text_color(label_status, lv_color_hex(0xFF4444), 0);
  } else if (button2Active) {
    lv_obj_set_style_text_color(label_status, lv_color_hex(0x44FF44), 0);
  } else {
    lv_obj_set_style_text_color(label_status, lv_color_hex(0x4444FF), 0);
  }
}
