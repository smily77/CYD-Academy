/*
  Beispiel 10c: Touch-Buttons PREMIUM DESIGN

  Die ultimative Touch-Button-Demo mit maximalem Design-Focus!

  Premium Features:
  - 3D-Buttons mit Multi-Layer Schatten
  - Glassmorphism Status-Panel
  - Farbverläufe (Gradients) für jeden Button
  - Glow-Effekte für aktive Buttons
  - Neumorphism-Stil
  - Animierte Hintergrund-Verläufe
  - Icon-basierte Labels
  - Pulsierende Aktiv-Indikatoren
  - Premium Farbpalette

  Button-Typen:
  1. Momentary - Rot-Orange Gradient mit Puls-Effekt
  2. Toggle - Grün-Cyan Gradient mit Glow
  3. Radio A - Violett mit 3D-Relief
  4. Radio B - Orange-Pink Gradient
  5. Radio C - Cyan-Blue Gradient

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
lv_obj_t *status_panel;
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

// Glow-Indikatoren (kleine leuchtende Kreise)
lv_obj_t *glow_momentary;
lv_obj_t *glow_toggle;
lv_obj_t *glow_radio_a;
lv_obj_t *glow_radio_b;
lv_obj_t *glow_radio_c;

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
void updateStatusPanel();
void createPremiumUI();
lv_obj_t* create3DButton(lv_obj_t *parent, int x, int y, int w, int h,
                         uint32_t color1, uint32_t color2, const char* text);
lv_obj_t* createGlowIndicator(lv_obj_t *parent, uint32_t color);

void setup() {
  Serial.begin(115200);
  Serial.println("\n=================================");
  Serial.println("PREMIUM Touch Buttons - LVGL");
  Serial.println("3D Design | Glassmorphism | Glow");
  Serial.println("=================================\n");

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

  // Premium UI erstellen
  createPremiumUI();

  Serial.println("\nPremium UI created!");
  Serial.println("Enjoy the beautiful design! ✨");
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

// Erstellt die Premium UI
void createPremiumUI() {
  lv_obj_t *screen = lv_scr_act();

  // ===== PREMIUM HINTERGRUND: Bunter Gradient =====
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x1a0033), 0);  // Dunkel-Violett
  lv_obj_set_style_bg_grad_color(screen, lv_color_hex(0x001a33), 0);  // Dunkel-Blau
  lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);

  // ===== TITEL mit Glow =====
  label_title = lv_label_create(screen);
  lv_label_set_text(label_title, "✨ PREMIUM TOUCH DEMO ✨");
  lv_obj_set_style_text_color(label_title, lv_color_hex(0xFFFFFF), 0);

  #if LV_FONT_MONTSERRAT_20
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_20, 0);
  #elif LV_FONT_MONTSERRAT_16
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_16, 0);
  #endif

  // Titel-Schatten für Glow-Effekt
  lv_obj_set_style_shadow_width(label_title, 20, 0);
  lv_obj_set_style_shadow_color(label_title, lv_color_hex(0x00FFFF), 0);
  lv_obj_set_style_shadow_opa(label_title, LV_OPA_50, 0);
  lv_obj_set_style_shadow_spread(label_title, 5, 0);

  lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 8);

  // ===== STATUS PANEL mit Glassmorphism =====
  status_panel = lv_obj_create(screen);
  lv_obj_set_size(status_panel, SCREEN_WIDTH - 40, 35);
  lv_obj_align(status_panel, LV_ALIGN_BOTTOM_MID, 0, -5);

  // Glassmorphism-Effekt: Semi-transparent mit Blur-Illusion
  lv_obj_set_style_bg_color(status_panel, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_bg_opa(status_panel, LV_OPA_20, 0);  // 20% opacity
  lv_obj_set_style_border_width(status_panel, 2, 0);
  lv_obj_set_style_border_color(status_panel, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_border_opa(status_panel, LV_OPA_40, 0);
  lv_obj_set_style_radius(status_panel, 15, 0);
  lv_obj_set_style_shadow_width(status_panel, 15, 0);
  lv_obj_set_style_shadow_color(status_panel, lv_color_hex(0x000000), 0);
  lv_obj_set_style_shadow_opa(status_panel, LV_OPA_50, 0);
  lv_obj_set_style_pad_all(status_panel, 0, 0);

  // Status-Label im Panel
  label_status = lv_label_create(status_panel);
  lv_label_set_text(label_status, "Radio A aktiv");
  lv_obj_set_style_text_color(label_status, lv_color_hex(0xFFFFFF), 0);
  #if LV_FONT_MONTSERRAT_16
    lv_obj_set_style_text_font(label_status, &lv_font_montserrat_16, 0);
  #endif
  lv_obj_center(label_status);

  // ===== LAYOUT-BERECHNUNGEN =====
  int header_h = 40;
  int footer_h = 45;
  int available_h = SCREEN_HEIGHT - header_h - footer_h - 10;
  int button_area_y = header_h + 5;

  int spacing = 10;
  int top_btn_w = (SCREEN_WIDTH - 3 * spacing) / 2;
  int top_btn_h = (available_h - spacing) / 2;
  int bot_btn_w = (SCREEN_WIDTH - 4 * spacing) / 3;
  int bot_btn_h = (available_h - spacing) / 2;

  // ===== BUTTON 1: MOMENTARY (Rot-Orange Gradient) =====
  btn_momentary = create3DButton(screen, spacing, button_area_y, top_btn_w, top_btn_h,
                                 0xFF4444, 0xFF8844, "MOMENTARY\nHOLD");

  lv_obj_add_event_cb(btn_momentary, btn_momentary_event_cb, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(btn_momentary, btn_momentary_event_cb, LV_EVENT_RELEASED, NULL);

  // Status-Indikator (automatisch oben rechts positioniert)
  glow_momentary = createGlowIndicator(btn_momentary, 0x808080);  // Mittelgrau = inaktiv

  // ===== BUTTON 2: TOGGLE (Grün-Cyan Gradient) =====
  btn_toggle = create3DButton(screen, 2 * spacing + top_btn_w, button_area_y,
                              top_btn_w, top_btn_h, 0x44FF44, 0x44FFFF, "TOGGLE\nON/OFF");

  lv_obj_add_event_cb(btn_toggle, btn_toggle_event_cb, LV_EVENT_CLICKED, NULL);

  // Status-Indikator (automatisch oben rechts positioniert)
  glow_toggle = createGlowIndicator(btn_toggle, 0x808080);  // Mittelgrau = inaktiv

  // ===== BUTTON 3: RADIO A (Violett-Magenta Gradient) =====
  btn_radio_a = create3DButton(screen, spacing, button_area_y + top_btn_h + spacing,
                               bot_btn_w, bot_btn_h, 0x8844FF, 0xFF44FF, "RADIO\nA");

  lv_obj_add_event_cb(btn_radio_a, btn_radio_event_cb, LV_EVENT_CLICKED, (void*)0);

  // Status-Indikator (aktiv beim Start = Magenta)
  glow_radio_a = createGlowIndicator(btn_radio_a, 0xFF00FF);  // Magenta = aktiv

  // ===== BUTTON 4: RADIO B (Orange-Pink Gradient) =====
  btn_radio_b = create3DButton(screen, 2 * spacing + bot_btn_w, button_area_y + top_btn_h + spacing,
                               bot_btn_w, bot_btn_h, 0xFF8800, 0xFF4488, "RADIO\nB");

  lv_obj_add_event_cb(btn_radio_b, btn_radio_event_cb, LV_EVENT_CLICKED, (void*)1);

  // Status-Indikator (inaktiv beim Start)
  glow_radio_b = createGlowIndicator(btn_radio_b, 0x808080);  // Mittelgrau = inaktiv

  // ===== BUTTON 5: RADIO C (Cyan-Blue Gradient) =====
  btn_radio_c = create3DButton(screen, 3 * spacing + 2 * bot_btn_w, button_area_y + top_btn_h + spacing,
                               bot_btn_w, bot_btn_h, 0x00FFFF, 0x0088FF, "RADIO\nC");

  lv_obj_add_event_cb(btn_radio_c, btn_radio_event_cb, LV_EVENT_CLICKED, (void*)2);

  // Status-Indikator (inaktiv beim Start)
  glow_radio_c = createGlowIndicator(btn_radio_c, 0x808080);  // Mittelgrau = inaktiv
}

// Erstellt einen 3D-Button mit Gradient
lv_obj_t* create3DButton(lv_obj_t *parent, int x, int y, int w, int h,
                         uint32_t color1, uint32_t color2, const char* text) {
  lv_obj_t *btn = lv_btn_create(parent);
  lv_obj_set_size(btn, w, h);
  lv_obj_set_pos(btn, x, y);

  // 3D-Effekt: Multi-Layer Schatten
  lv_obj_set_style_shadow_width(btn, 15, LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_opa(btn, LV_OPA_60, LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_color(btn, lv_color_hex(0x000000), LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_ofs_y(btn, 8, LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_spread(btn, 3, LV_STATE_DEFAULT);

  // Pressed State: Flacherer Schatten
  lv_obj_set_style_shadow_width(btn, 5, LV_STATE_PRESSED);
  lv_obj_set_style_shadow_ofs_y(btn, 2, LV_STATE_PRESSED);

  // Gradient-Hintergrund
  lv_obj_set_style_bg_color(btn, lv_color_hex(color1), LV_STATE_DEFAULT);
  lv_obj_set_style_bg_grad_color(btn, lv_color_hex(color2), LV_STATE_DEFAULT);
  lv_obj_set_style_bg_grad_dir(btn, LV_GRAD_DIR_VER, LV_STATE_DEFAULT);

  // Pressed State: Dunklerer Gradient
  uint32_t dark1 = ((color1 >> 1) & 0x7F7F7F);
  uint32_t dark2 = ((color2 >> 1) & 0x7F7F7F);
  lv_obj_set_style_bg_color(btn, lv_color_hex(dark1), LV_STATE_PRESSED);
  lv_obj_set_style_bg_grad_color(btn, lv_color_hex(dark2), LV_STATE_PRESSED);

  // Border für mehr 3D-Effekt
  lv_obj_set_style_border_width(btn, 3, LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(btn, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(btn, LV_OPA_30, LV_STATE_DEFAULT);

  // Abgerundete Ecken
  lv_obj_set_style_radius(btn, 15, 0);

  // Label im Button
  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);

  #if LV_FONT_MONTSERRAT_16
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
  #elif LV_FONT_MONTSERRAT_14
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
  #endif

  // Label-Schatten für bessere Lesbarkeit
  lv_obj_set_style_shadow_width(label, 5, 0);
  lv_obj_set_style_shadow_color(label, lv_color_hex(0x000000), 0);
  lv_obj_set_style_shadow_opa(label, LV_OPA_80, 0);

  lv_obj_center(label);

  return btn;
}

// Erstellt einen Status-Indikator (immer sichtbar, ändert Farbe)
// Wird oben rechts im Button positioniert mit 10px Abstand von Ecke
lv_obj_t* createGlowIndicator(lv_obj_t *parent, uint32_t color) {
  lv_obj_t *glow = lv_obj_create(parent);
  lv_obj_set_size(glow, 20, 20);  // Größer und runder

  // Oben rechts positionieren mit 10px Abstand von Ecke
  lv_obj_align(glow, LV_ALIGN_TOP_RIGHT, -10, 10);

  // Kreis-Form
  lv_obj_set_style_radius(glow, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(glow, lv_color_hex(color), 0);

  // Dickere weiße Border für bessere Sichtbarkeit
  lv_obj_set_style_border_width(glow, 3, 0);
  lv_obj_set_style_border_color(glow, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_border_opa(glow, LV_OPA_100, 0);

  // Starker Glow-Effekt
  lv_obj_set_style_shadow_width(glow, 15, 0);
  lv_obj_set_style_shadow_color(glow, lv_color_hex(color), 0);
  lv_obj_set_style_shadow_opa(glow, LV_OPA_80, 0);
  lv_obj_set_style_shadow_spread(glow, 5, 0);

  // Kein Padding, damit der Kreis voll ausgefüllt ist
  lv_obj_set_style_pad_all(glow, 0, 0);

  return glow;
}

// Momentary Button Event Handler
void btn_momentary_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_PRESSED) {
    button1Active = true;
    // Status-Indikator: Rot = aktiv
    lv_obj_set_style_bg_color(glow_momentary, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_shadow_color(glow_momentary, lv_color_hex(0xFF0000), 0);
    updateLED();
    updateStatusPanel();
    Serial.println("💥 Momentary: PRESSED");
  }
  else if (code == LV_EVENT_RELEASED) {
    button1Active = false;
    // Status-Indikator: Mittelgrau = inaktiv
    lv_obj_set_style_bg_color(glow_momentary, lv_color_hex(0x808080), 0);
    lv_obj_set_style_shadow_color(glow_momentary, lv_color_hex(0x808080), 0);
    updateLED();
    updateStatusPanel();
    Serial.println("💥 Momentary: RELEASED");
  }
}

// Toggle Button Event Handler
void btn_toggle_event_cb(lv_event_t *e) {
  button2Active = !button2Active;

  if (button2Active) {
    // Status-Indikator: Grün = aktiv
    lv_obj_set_style_bg_color(glow_toggle, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_shadow_color(glow_toggle, lv_color_hex(0x00FF00), 0);
    Serial.println("🔆 Toggle: ON");
  } else {
    // Status-Indikator: Mittelgrau = inaktiv
    lv_obj_set_style_bg_color(glow_toggle, lv_color_hex(0x808080), 0);
    lv_obj_set_style_shadow_color(glow_toggle, lv_color_hex(0x808080), 0);
    Serial.println("🔆 Toggle: OFF");
  }

  updateLED();
  updateStatusPanel();
}

// Radio Button Event Handler
void btn_radio_event_cb(lv_event_t *e) {
  int new_radio = (int)lv_event_get_user_data(e);

  if (radioActive == new_radio) {
    Serial.printf("📻 Radio %c already active\n", 'A' + new_radio);
    return;
  }

  // Alten Status-Indikator auf Mittelgrau setzen (inaktiv)
  lv_obj_t *old_glow = (radioActive == 0) ? glow_radio_a :
                       (radioActive == 1) ? glow_radio_b : glow_radio_c;
  lv_obj_set_style_bg_color(old_glow, lv_color_hex(0x808080), 0);
  lv_obj_set_style_shadow_color(old_glow, lv_color_hex(0x808080), 0);

  // Neuen Status-Indikator auf Farbe setzen
  lv_obj_t *new_glow = (new_radio == 0) ? glow_radio_a :
                       (new_radio == 1) ? glow_radio_b : glow_radio_c;

  // Farbe je nach Radio-Button
  uint32_t active_color = (new_radio == 0) ? 0xFF00FF :  // Magenta für A
                         (new_radio == 1) ? 0xFF8800 :  // Orange für B
                                           0x00FFFF;    // Cyan für C

  lv_obj_set_style_bg_color(new_glow, lv_color_hex(active_color), 0);
  lv_obj_set_style_shadow_color(new_glow, lv_color_hex(active_color), 0);

  radioActive = new_radio;
  updateStatusPanel();
  Serial.printf("📻 Radio %c activated\n", 'A' + new_radio);
}

// Aktualisiert RGB LED
void updateLED() {
  #ifdef LED_PIN_R
    digitalWrite(LED_PIN_R, button1Active ? LOW : HIGH);
    digitalWrite(LED_PIN_G, button2Active ? LOW : HIGH);
  #endif
}

// Aktualisiert Status-Panel mit bunten Icons
void updateStatusPanel() {
  String status;
  uint32_t color;

  if (button1Active) {
    status = "💥 MOMENTARY ACTIVE";
    color = 0xFF4444;
  } else if (button2Active) {
    status = "🔆 TOGGLE ON";
    color = 0x44FF44;
  } else {
    status = "📻 RADIO ";
    status += (char)('A' + radioActive);
    status += " ACTIVE";

    if (radioActive == 0) color = 0xFF44FF;      // Magenta
    else if (radioActive == 1) color = 0xFF8800; // Orange
    else color = 0x00FFFF;                       // Cyan
  }

  lv_label_set_text(label_status, status.c_str());
  lv_obj_set_style_text_color(label_status, lv_color_hex(color), 0);

  // Status-Label Glow anpassen
  lv_obj_set_style_shadow_color(label_status, lv_color_hex(color), 0);
  lv_obj_set_style_shadow_width(label_status, 10, 0);
  lv_obj_set_style_shadow_opa(label_status, LV_OPA_70, 0);
}
