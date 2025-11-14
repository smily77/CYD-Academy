/*
  14b_Selection_Menu_LVGL - Scrollbares Auswahlmenü mit LVGL Framework

  Gleiche Funktionalität wie 14_Selection_Menu_Native, aber mit LVGL.
  Zeigt wie viel einfacher komplexe UI mit einem Framework ist!

  Funktionen:
  - Dynamische Anzahl von Menü-Einträgen (identisch zu Native)
  - Touch-basierte Auswahl (automatisch durch LVGL)
  - Automatisches Scrollen (automatisch durch LVGL)
  - Visual Feedback (LVGL Themes)
  - Callback-System für Aktionen
  - Viel weniger Code!

  Lernziele:
  - LVGL Grundlagen
  - Widget-Hierarchie verstehen
  - Event-Callbacks in LVGL
  - Native vs. Framework vergleichen
  - Trade-offs erkennen (Einfachheit vs. Kontrolle)

  Hardware:
  - 1x CYD Display (2.8" oder 3.5")
  - Nur Touch-Input!

  Installation LVGL:
  1. Arduino IDE: Library Manager → "lvgl" suchen → Version 8.x installieren
  2. PlatformIO: lib_deps = lvgl/lvgl @ ^8.3.0

  WICHTIG: LVGL benötigt ~30KB RAM - funktioniert auf ESP32, NICHT auf kleineren MCUs!
*/

#include <lvgl.h>
#include <CYD_Display_Config.h>

// ===== GLOBALE VARIABLEN =====

LGFX lcd;

// LVGL Display Buffer (mindestens 1/10 der Bildschirmgröße empfohlen)
static const uint16_t BUFFER_SIZE = 240 * 40;  // 40 Zeilen
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[BUFFER_SIZE];
static lv_color_t buf2[BUFFER_SIZE];

// LVGL Display und Input Devices
static lv_disp_drv_t disp_drv;
static lv_indev_drv_t indev_drv;

// UI Widgets
lv_obj_t* list;

// ===== MENU ITEMS (identisch zu Native-Version) =====

struct MenuItem {
  const char* title;
  const char* subtitle;
  const char* icon;        // LVGL Symbol (z.B. LV_SYMBOL_WIFI)
  void (*action)();
};

// Callbacks (identisch zu Native)
void wifiAction() { Serial.println("Action: WiFi Settings"); }
void bluetoothAction() { Serial.println("Action: Bluetooth"); }
void displayAction() { Serial.println("Action: Display"); }
void soundAction() { Serial.println("Action: Sound"); }
void storageAction() { Serial.println("Action: Storage"); }
void aboutAction() { Serial.println("Action: About"); }
void batteryAction() { Serial.println("Action: Battery"); }
void securityAction() { Serial.println("Action: Security"); }
void developerAction() { Serial.println("Action: Developer"); }
void resetAction() { Serial.println("Action: Reset"); }

MenuItem menuItems[] = {
  {"WiFi Settings", "Configure network", LV_SYMBOL_WIFI, wifiAction},
  {"Bluetooth", "Pair devices", LV_SYMBOL_BLUETOOTH, bluetoothAction},
  {"Display", "Brightness & timeout", LV_SYMBOL_IMAGE, displayAction},
  {"Sound", "Volume & notifications", LV_SYMBOL_VOLUME_MAX, soundAction},
  {"Storage", "Manage files", LV_SYMBOL_SD_CARD, storageAction},
  {"About", "Device information", LV_SYMBOL_LIST, aboutAction},
  {"Battery", "Power settings", LV_SYMBOL_BATTERY_FULL, batteryAction},
  {"Security", "Lock screen & privacy", LV_SYMBOL_LOCK, securityAction},
  {"Developer", "Advanced options", LV_SYMBOL_SETTINGS, developerAction},
  {"Reset", "Factory reset", LV_SYMBOL_TRASH, resetAction},
};

const int MENU_ITEM_COUNT = sizeof(menuItems) / sizeof(MenuItem);

// ===== LVGL CALLBACKS =====

// Display Flush (LovyanGFX als Backend)
void lvgl_flush_cb(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  lcd.startWrite();
  lcd.setAddrWindow(area->x1, area->y1, w, h);
  lcd.writePixels((lgfx::rgb565_t*)&color_p->full, w * h);
  lcd.endWrite();

  lv_disp_flush_ready(disp);
}

// Touch Read (LovyanGFX als Backend)
void lvgl_touch_cb(lv_indev_drv_t* indev_driver, lv_indev_data_t* data) {
  uint16_t x, y;
  bool touched = lcd.getTouch(&x, &y);

  if (touched) {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = x;
    data->point.y = y;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

// List Item Click Callback
void list_event_cb(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t* btn = lv_event_get_target(e);

  if (code == LV_EVENT_CLICKED) {
    // Finde Index des geklickten Items
    uint32_t index = lv_obj_get_index(btn);

    Serial.printf("Selected: %s (index %d)\n", menuItems[index].title, index);

    // Callback ausführen
    if (menuItems[index].action != nullptr) {
      menuItems[index].action();
    }

    // Visuelles Feedback durch LVGL (Button wird automatisch gehighlighted)
  }
}

// ===== SETUP =====

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Selection Menu Demo (LVGL) ===");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(0);
  lcd.setBrightness(255);
  lcd.fillScreen(TFT_BLACK);

  // LVGL initialisieren
  lv_init();

  // Display Buffer registrieren
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, BUFFER_SIZE);

  // Display Driver initialisieren
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 240;
  disp_drv.ver_res = 320;
  disp_drv.flush_cb = lvgl_flush_cb;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // Touch Driver initialisieren
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = lvgl_touch_cb;
  lv_indev_drv_register(&indev_drv);

  // UI erstellen
  createUI();

  Serial.printf("LVGL initialized! Menu Items: %d\n", MENU_ITEM_COUNT);
  Serial.println("Touch items to select!");
}

// ===== LOOP =====

void loop() {
  lv_timer_handler();  // LVGL Event Loop
  delay(5);            // LVGL empfiehlt 5-10ms
}

// ===== UI CREATION =====

void createUI() {
  // Hauptcontainer (Vollbild)
  lv_obj_t* screen = lv_scr_act();

  // Header erstellen
  lv_obj_t* header = lv_obj_create(screen);
  lv_obj_set_size(header, 240, 40);
  lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(header, lv_color_hex(0x252525), 0);
  lv_obj_set_style_border_width(header, 0, 0);
  lv_obj_set_style_radius(header, 0, 0);

  // Header-Titel
  lv_obj_t* headerLabel = lv_label_create(header);
  lv_label_set_text(headerLabel, "Settings");
  lv_obj_set_style_text_font(headerLabel, &lv_font_montserrat_18, 0);
  lv_obj_align(headerLabel, LV_ALIGN_LEFT_MID, 10, 0);

  // Item Counter (rechts)
  lv_obj_t* counterLabel = lv_label_create(header);
  char buf[16];
  sprintf(buf, "%d items", MENU_ITEM_COUNT);
  lv_label_set_text(counterLabel, buf);
  lv_obj_set_style_text_color(counterLabel, lv_color_hex(0x9CF3), 0);
  lv_obj_align(counterLabel, LV_ALIGN_RIGHT_MID, -10, 0);

  // Liste erstellen (scrollbar automatisch!)
  list = lv_list_create(screen);
  lv_obj_set_size(list, 240, 280);
  lv_obj_align(list, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_bg_color(list, lv_color_hex(0x1a1a1a), 0);
  lv_obj_set_style_border_width(list, 0, 0);
  lv_obj_set_style_radius(list, 0, 0);

  // Menü-Items hinzufügen (DYNAMIC!)
  for (int i = 0; i < MENU_ITEM_COUNT; i++) {
    // List Button erstellen (mit Icon und Text)
    lv_obj_t* btn = lv_list_add_btn(list, menuItems[i].icon, menuItems[i].title);

    // Subtitle hinzufügen (als zweite Zeile)
    lv_obj_t* subtitle = lv_label_create(btn);
    lv_label_set_text(subtitle, menuItems[i].subtitle);
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0x9CF3), 0);
    lv_obj_set_style_text_font(subtitle, &lv_font_montserrat_12, 0);
    lv_obj_align(subtitle, LV_ALIGN_BOTTOM_LEFT, 40, -5);

    // Event Callback registrieren
    lv_obj_add_event_cb(btn, list_event_cb, LV_EVENT_CLICKED, NULL);

    // Button-Höhe anpassen für Subtitle
    lv_obj_set_height(btn, 60);
  }

  Serial.println("UI created successfully!");
}
