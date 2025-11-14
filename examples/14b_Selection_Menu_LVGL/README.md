# 📱 Beispiel 14b: Selection Menu (LVGL Framework)

## 📝 Was macht dieses Beispiel?

**Gleiche Funktionalität** wie [14_Selection_Menu_Native](../14_Selection_Menu_Native/), aber mit dem **LVGL GUI Framework**.

**Features:**
- ✅ Dynamische Item-Anzahl (identisch zu Native)
- ✅ Touch-Interaktion (automatisch durch LVGL)
- ✅ Smooth Scrolling (automatisch durch LVGL)
- ✅ Visual Feedback (durch LVGL Themes)
- ✅ Callback-System (identisch zu Native)
- ✅ **Viel weniger Code!** (~300 Zeilen vs. ~470 Zeilen)

**Steuerung:**
- Touch auf Item: Auswählen
- Drag vertikal: Scrollen (automatisch)

---

## 🎯 Lernziele

1. **LVGL Grundlagen** - Widget-basierte UI
2. **Framework vs. Native** - Trade-offs verstehen
3. **LVGL Architecture** - Display Driver, Input Driver, Event System
4. **Widget-Hierarchie** - Parent-Child-Beziehungen
5. **Memory Management** - RAM-Verbrauch von Frameworks
6. **Wann welchen Ansatz?** - Entscheidungskriterien

---

## 📊 Code-Vergleich: Native vs. LVGL

### Zeilen-Anzahl

| Feature | Native (14) | LVGL (14b) | Ersparnis |
|---------|-------------|------------|-----------|
| Touch Handling | ~80 Zeilen | ~20 Zeilen | **75%** |
| Scroll Mechanik | ~60 Zeilen | 0 Zeilen | **100%** |
| Rendering | ~120 Zeilen | ~30 Zeilen | **75%** |
| Clipping/Culling | ~40 Zeilen | 0 Zeilen | **100%** |
| **GESAMT** | **~470** | **~300** | **~36%** |

### Touch-Handling

**Native (14):**
```cpp
// ~80 Zeilen für State Machine
void handleTouch() {
  uint16_t x, y;
  bool touched = lcd.getTouch(&x, &y);

  if (touched && !wasTouched) {
    touchStartY = y;
    scrollStartOffset = scrollOffset;
    isDragging = false;
    highlightedIndex = getItemIndexAtPosition(y);
  } else if (touched && wasTouched) {
    int deltaY = touchStartY - y;
    if (abs(deltaY) > SCROLL_THRESHOLD) {
      isDragging = true;
      highlightedIndex = -1;
    }
    if (isDragging) {
      scrollOffset = scrollStartOffset + deltaY;
      scrollOffset = constrain(scrollOffset, 0, maxScrollOffset);
    }
  } else if (!touched && wasTouched) {
    if (!isDragging && highlightedIndex >= 0) {
      selectedIndex = highlightedIndex;
      menuItems[selectedIndex].action();
    }
  }
}
```

**LVGL (14b):**
```cpp
// ~10 Zeilen für Callback
void list_event_cb(lv_event_t* e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    uint32_t index = lv_obj_get_index(lv_event_get_target(e));
    menuItems[index].action();
  }
}
```

**LVGL übernimmt:** Tap vs. Drag, Scroll, Highlight, State Management!

### Scroll-Mechanik

**Native (14):**
```cpp
// ~60 Zeilen für Offset-Berechnung, Clipping, Culling
int itemY = HEADER_HEIGHT + (i * ITEM_HEIGHT) - scrollOffset;

if (itemY + ITEM_HEIGHT < contentY || itemY > lcd.height()) {
  continue;  // Culling
}

if (itemY < contentY) {
  int clipHeight = ITEM_HEIGHT - (contentY - itemY);
  drawMenuItem(i, contentY, clipHeight, false);
}
```

**LVGL (14b):**
```cpp
// 0 Zeilen - komplett automatisch!
lv_obj_t* list = lv_list_create(screen);
lv_obj_set_size(list, 240, 280);
// Fertig! Scrolling, Clipping, Culling = automatisch
```

### UI-Erstellung

**Native (14):**
```cpp
// Manuelles Zeichnen jedes Frames
void drawMenuItem(int index, int y, int height, bool drawAll) {
  lcd.fillRect(0, y, width, height, bgColor);
  lcd.fillCircle(20, y + 30, 12, item.icon);
  lcd.setCursor(45, y + 15);
  lcd.print(item.title);
  // ... etc
}
```

**LVGL (14b):**
```cpp
// Deklarativ - LVGL managed Rendering
for (int i = 0; i < MENU_ITEM_COUNT; i++) {
  lv_obj_t* btn = lv_list_add_btn(list, menuItems[i].icon, menuItems[i].title);
  lv_obj_t* subtitle = lv_label_create(btn);
  lv_label_set_text(subtitle, menuItems[i].subtitle);
}
// LVGL zeichnet automatisch nur bei Änderungen!
```

---

## 🏗️ LVGL Architektur

### Komponenten

```
┌─────────────────────────────────────┐
│         Application (dein Code)      │
│  - createUI()                        │
│  - Event Callbacks                   │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│         LVGL Core Library            │
│  - Widget Management                 │
│  - Layout Engine                     │
│  - Event System                      │
│  - Rendering Engine                  │
└──────────────┬──────────────────────┘
               │
      ┌────────┴────────┐
      ▼                 ▼
┌────────────┐    ┌────────────┐
│  Display   │    │   Input    │
│  Driver    │    │  Driver    │
│ (Flush CB) │    │ (Read CB)  │
└─────┬──────┘    └─────┬──────┘
      │                 │
      ▼                 ▼
┌────────────┐    ┌────────────┐
│ LovyanGFX  │    │ LovyanGFX  │
│  Display   │    │   Touch    │
└────────────┘    └────────────┘
```

### 1. Display Driver

**Verbindet LVGL mit LovyanGFX:**

```cpp
void lvgl_flush_cb(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
  // LVGL gibt uns einen Bildbereich (area) und Pixel-Daten (color_p)

  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  // LovyanGFX schreibt Pixel auf Display
  lcd.startWrite();
  lcd.setAddrWindow(area->x1, area->y1, w, h);
  lcd.writePixels((lgfx::rgb565_t*)&color_p->full, w * h);
  lcd.endWrite();

  lv_disp_flush_ready(disp);  // WICHTIG: LVGL informieren!
}
```

**Was passiert:**
1. LVGL rendert UI in Buffer
2. LVGL ruft `lvgl_flush_cb()` für jeden geänderten Bereich
3. Wir kopieren Pixel auf Display via LovyanGFX
4. Wir melden LVGL "Fertig!" via `lv_disp_flush_ready()`

**Warum Buffer?**

```
Ohne Buffer: LVGL zeichnet direkt auf Display
             → Flackern, langsam

Mit Buffer:  LVGL zeichnet in RAM
             → Dann schnelle Kopie auf Display
             → Smooth, kein Flackern!
```

### 2. Input Driver

**Verbindet LVGL mit Touch:**

```cpp
void lvgl_touch_cb(lv_indev_drv_t* indev, lv_indev_data_t* data) {
  uint16_t x, y;
  bool touched = lcd.getTouch(&x, &y);

  if (touched) {
    data->state = LV_INDEV_STATE_PR;   // Pressed
    data->point.x = x;
    data->point.y = y;
  } else {
    data->state = LV_INDEV_STATE_REL;  // Released
  }
}
```

**LVGL verarbeitet dann:**
- Tap vs. Long Press vs. Drag
- Scroll-Gesten
- Button-States (Pressed, Released, Clicked)
- Focus-Management

**Wir müssen NICHTS davon selbst implementieren!**

### 3. Event System

**Widgets feuern Events:**

```cpp
LV_EVENT_PRESSED       // Touch Down
LV_EVENT_RELEASED      // Touch Up
LV_EVENT_CLICKED       // Tap (Press + Release)
LV_EVENT_LONG_PRESSED  // Touch halten
LV_EVENT_VALUE_CHANGED // Wert geändert (z.B. Slider)
LV_EVENT_SCROLL        // Gescrollt
// ... viele mehr
```

**Callback registrieren:**

```cpp
lv_obj_add_event_cb(btn, my_callback, LV_EVENT_CLICKED, NULL);

void my_callback(lv_event_t* e) {
  lv_obj_t* btn = lv_event_get_target(e);
  // Reagiere auf Click
}
```

### 4. Widget-Hierarchie

**Widgets haben Parent-Child-Beziehung:**

```
Screen (Root)
├─ Header (Container)
│  ├─ Title (Label)
│  └─ Counter (Label)
└─ List (Container)
   ├─ Button 1
   │  ├─ Icon (Label)
   │  ├─ Title (Label)
   │  └─ Subtitle (Label)
   ├─ Button 2
   │  ├─ Icon
   │  ├─ Title
   │  └─ Subtitle
   └─ ...
```

**Automatische Effekte:**
- Position relativ zu Parent
- Scroll-Container scrollt alle Children
- Löschen von Parent löscht alle Children
- Styles vererben sich (optional)

---

## 💾 Memory Management

### RAM-Verbrauch

**LVGL benötigt:**

| Komponente | Größe | Beschreibung |
|------------|-------|--------------|
| LVGL Core | ~20 KB | Library selbst |
| Display Buffer | ~19 KB | 240x40 Pixel (2 Buffers) |
| Widgets | ~2 KB | 10 List Items |
| **TOTAL** | **~41 KB** | **Nur für UI!** |

**ESP32 hat 520 KB RAM** → Kein Problem!

**Kleinere MCUs (z.B. STM32F1):**
- Nur 20 KB RAM total
- LVGL passt NICHT
- → Native Rendering erforderlich

### Buffer-Größe optimieren

```cpp
// Aktuell: 240 * 40 = 9600 Pixel pro Buffer (19.2 KB total)
static const uint16_t BUFFER_SIZE = 240 * 40;

// Option 1: Kleinerer Buffer (langsamer, weniger RAM)
static const uint16_t BUFFER_SIZE = 240 * 20;  // 9.6 KB

// Option 2: Nur 1 Buffer statt 2 (langsamer)
lv_disp_draw_buf_init(&draw_buf, buf1, NULL, BUFFER_SIZE);

// Option 3: Größerer Buffer (schneller, mehr RAM)
static const uint16_t BUFFER_SIZE = 240 * 80;  // 38.4 KB
```

**Trade-off:**
- **Größerer Buffer** → Weniger Flush-Calls → Schneller
- **Kleinerer Buffer** → Mehr Flush-Calls → Langsamer, aber weniger RAM

---

## 🎨 LVGL Features die Native nicht hat

### 1. Themes & Styling

```cpp
// Dark Theme (built-in)
lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE),
                      lv_palette_main(LV_PALETTE_RED),
                      true, LV_FONT_DEFAULT);

// Custom Styles
static lv_style_t style_btn;
lv_style_init(&style_btn);
lv_style_set_bg_color(&style_btn, lv_color_hex(0x4CAF50));
lv_style_set_radius(&style_btn, 10);
lv_obj_add_style(btn, &style_btn, 0);
```

### 2. Animationen

```cpp
lv_anim_t a;
lv_anim_init(&a);
lv_anim_set_var(&a, btn);
lv_anim_set_values(&a, 0, 100);
lv_anim_set_time(&a, 1000);
lv_anim_set_exec_cb(&a, [](void* var, int32_t value) {
  lv_obj_set_x((lv_obj_t*)var, value);
});
lv_anim_start(&a);
```

### 3. Viele Widget-Typen

```cpp
lv_btn_create()       // Button
lv_label_create()     // Text
lv_slider_create()    // Slider
lv_switch_create()    // Toggle Switch
lv_dropdown_create()  // Dropdown
lv_chart_create()     // Chart/Graph
lv_keyboard_create()  // Virtuelle Tastatur
lv_calendar_create()  // Kalender
// ... 40+ Widgets!
```

### 4. Flexbox & Grid Layout

```cpp
// Flexbox (wie CSS!)
lv_obj_set_layout(container, LV_LAYOUT_FLEX);
lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW);
lv_obj_set_flex_align(container, LV_FLEX_ALIGN_SPACE_AROUND,
                      LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

// Grid Layout
static lv_coord_t col_dsc[] = {100, 100, LV_GRID_TEMPLATE_LAST};
static lv_coord_t row_dsc[] = {50, 50, 50, LV_GRID_TEMPLATE_LAST};
lv_obj_set_grid_dsc_array(container, col_dsc, row_dsc);
```

---

## 🚀 Experimente & Erweiterungen

### 1. 🎨 Theme Switcher

```cpp
bool isDarkTheme = true;

void toggleTheme() {
  if (isDarkTheme) {
    lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE),
                          lv_palette_main(LV_PALETTE_CYAN),
                          false, LV_FONT_DEFAULT);  // Light
  } else {
    lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE),
                          lv_palette_main(LV_PALETTE_RED),
                          true, LV_FONT_DEFAULT);   // Dark
  }

  isDarkTheme = !isDarkTheme;
  lv_obj_invalidate(lv_scr_act());  // Neu zeichnen
}

// Button zum Umschalten
lv_obj_t* themeBtn = lv_btn_create(header);
lv_obj_add_event_cb(themeBtn, [](lv_event_t* e) {
  toggleTheme();
}, LV_EVENT_CLICKED, NULL);
```

---

### 2. 🔍 Suchfeld mit Tastatur

```cpp
lv_obj_t* searchField;
lv_obj_t* keyboard;

void createSearchUI() {
  // Text-Feld
  searchField = lv_textarea_create(screen);
  lv_textarea_set_one_line(searchField, true);
  lv_textarea_set_placeholder_text(searchField, "Search...");

  // Virtuelle Tastatur
  keyboard = lv_keyboard_create(screen);
  lv_keyboard_set_textarea(keyboard, searchField);

  // Event: Text geändert
  lv_obj_add_event_cb(searchField, [](lv_event_t* e) {
    const char* text = lv_textarea_get_text(searchField);
    filterMenuItems(text);  // Implementiere Filterung!
  }, LV_EVENT_VALUE_CHANGED, NULL);
}

void filterMenuItems(const char* query) {
  // Alle Buttons durchgehen
  for (int i = 0; i < MENU_ITEM_COUNT; i++) {
    lv_obj_t* btn = lv_obj_get_child(list, i);

    if (strstr(menuItems[i].title, query) != nullptr) {
      lv_obj_clear_flag(btn, LV_OBJ_FLAG_HIDDEN);  // Zeigen
    } else {
      lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);    // Verstecken
    }
  }
}
```

---

### 3. 🎭 Animationen bei Selection

```cpp
void list_event_cb(lv_event_t* e) {
  lv_obj_t* btn = lv_event_get_target(e);

  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    // Animation: Button pulst
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, btn);
    lv_anim_set_exec_cb(&a, [](void* var, int32_t v) {
      lv_obj_set_style_transform_zoom((lv_obj_t*)var, v, 0);
    });
    lv_anim_set_values(&a, 256, 280);  // 256 = 100%, 280 = 110%
    lv_anim_set_time(&a, 200);
    lv_anim_set_playback_time(&a, 200);
    lv_anim_start(&a);

    // Dann Callback
    uint32_t index = lv_obj_get_index(btn);
    menuItems[index].action();
  }
}
```

---

### 4. 📊 Settings mit verschiedenen Widget-Typen

```cpp
void createSettingsUI() {
  // Switch (Toggle)
  lv_obj_t* wifiSwitch = lv_switch_create(list);
  lv_obj_add_event_cb(wifiSwitch, [](lv_event_t* e) {
    bool enabled = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
    Serial.printf("WiFi: %s\n", enabled ? "ON" : "OFF");
  }, LV_EVENT_VALUE_CHANGED, NULL);

  // Slider
  lv_obj_t* brightnessSlider = lv_slider_create(list);
  lv_slider_set_range(brightnessSlider, 0, 255);
  lv_slider_set_value(brightnessSlider, 128, LV_ANIM_OFF);
  lv_obj_add_event_cb(brightnessSlider, [](lv_event_t* e) {
    int value = lv_slider_get_value(lv_event_get_target(e));
    lcd.setBrightness(value);
  }, LV_EVENT_VALUE_CHANGED, NULL);

  // Dropdown
  lv_obj_t* dropdown = lv_dropdown_create(list);
  lv_dropdown_set_options(dropdown, "Auto\nLight\nDark");
  lv_obj_add_event_cb(dropdown, [](lv_event_t* e) {
    uint16_t selected = lv_dropdown_get_selected(lv_event_get_target(e));
    Serial.printf("Theme: %d\n", selected);
  }, LV_EVENT_VALUE_CHANGED, NULL);
}
```

---

### 5. 🗂️ Tab View (Multi-Page Menu)

```cpp
void createTabView() {
  lv_obj_t* tabview = lv_tabview_create(screen, LV_DIR_TOP, 40);

  // Tab 1: Network
  lv_obj_t* tab1 = lv_tabview_add_tab(tabview, LV_SYMBOL_WIFI " Network");
  lv_obj_t* list1 = lv_list_create(tab1);
  lv_list_add_btn(list1, LV_SYMBOL_WIFI, "WiFi");
  lv_list_add_btn(list1, LV_SYMBOL_BLUETOOTH, "Bluetooth");

  // Tab 2: Display
  lv_obj_t* tab2 = lv_tabview_add_tab(tabview, LV_SYMBOL_IMAGE " Display");
  lv_obj_t* list2 = lv_list_create(tab2);
  lv_list_add_btn(list2, LV_SYMBOL_EYE_OPEN, "Brightness");
  lv_list_add_btn(list2, LV_SYMBOL_SETTINGS, "Rotation");

  // Tab 3: System
  lv_obj_t* tab3 = lv_tabview_add_tab(tabview, LV_SYMBOL_SETTINGS " System");
  // ...
}
```

---

### 6. 🎯 Context Menu (Rechtsklick-Menü)

```cpp
void list_event_cb(lv_event_t* e) {
  lv_obj_t* btn = lv_event_get_target(e);
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_LONG_PRESSED) {
    // Long Press = Context Menu
    showContextMenu(btn);
  }
}

void showContextMenu(lv_obj_t* targetBtn) {
  lv_obj_t* menu = lv_menu_create(lv_scr_act());

  lv_obj_t* page = lv_menu_page_create(menu, NULL);

  lv_obj_t* section = lv_menu_section_create(page);
  lv_obj_t* item1 = lv_menu_cont_create(section);
  lv_label_create(item1)->text = "Edit";

  lv_obj_t* item2 = lv_menu_cont_create(section);
  lv_label_create(item2)->text = "Delete";

  lv_menu_set_page(menu, page);
}
```

---

## 🐛 Troubleshooting

### Problem: "Kompiliert nicht - lvgl.h nicht gefunden"

**Lösung:** LVGL installieren!

**Arduino IDE:**
1. Library Manager öffnen (Strg+Shift+I)
2. "lvgl" suchen
3. Version 8.3.x installieren (NICHT 9.x!)

**PlatformIO:**
```ini
[env:esp32]
lib_deps =
  lovyan03/LovyanGFX @ ^1.1.0
  lvgl/lvgl @ ^8.3.0
```

---

### Problem: "Display bleibt schwarz"

**Ursache:** Flush Callback vergessen oder falsch.

**Debug:**

```cpp
void lvgl_flush_cb(...) {
  Serial.println("Flush called!");  // Wird aufgerufen?

  // ... flush code

  lv_disp_flush_ready(disp);  // ← WICHTIG! Nicht vergessen!
}
```

**Überprüfe:**
- `lv_disp_flush_ready()` wird aufgerufen?
- Display Driver registriert? `lv_disp_drv_register(&disp_drv)`

---

### Problem: "Touch funktioniert nicht"

**Debug:**

```cpp
void lvgl_touch_cb(lv_indev_drv_t* indev, lv_indev_data_t* data) {
  uint16_t x, y;
  bool touched = lcd.getTouch(&x, &y);

  Serial.printf("Touch: %d, x=%d, y=%d\n", touched, x, y);

  // ...
}
```

**Überprüfe:**
- Touch-Koordinaten korrekt?
- Input Driver registriert? `lv_indev_drv_register(&indev_drv)`

---

### Problem: "ESP32 crasht mit 'Out of Memory'"

**Ursache:** Buffer zu groß oder zu viele Widgets.

**Lösung 1:** Reduziere Buffer-Größe

```cpp
static const uint16_t BUFFER_SIZE = 240 * 20;  // Statt 40
```

**Lösung 2:** Nur 1 Buffer

```cpp
lv_disp_draw_buf_init(&draw_buf, buf1, NULL, BUFFER_SIZE);
```

**Lösung 3:** Lösche nicht genutzte Widgets

```cpp
lv_obj_del(oldWidget);  // Gibt RAM frei
```

---

### Problem: "UI laggt / ist langsam"

**Ursache:** `lv_timer_handler()` zu selten aufgerufen.

**Lösung:**

```cpp
void loop() {
  lv_timer_handler();  // Mindestens alle 10ms!
  delay(5);            // Empfehlung: 5ms
}
```

**Nicht:**
```cpp
void loop() {
  lv_timer_handler();
  delay(100);  // ← Zu langsam! UI reagiert träge
}
```

---

## 📊 Native vs. LVGL - Die Entscheidung

### ✅ Verwende LVGL wenn:

- 🏗️ **Komplexe UI** mit vielen verschiedenen Widgets
- ⏱️ **Schnelle Entwicklung** wichtiger als volle Kontrolle
- 📱 **Standard-Widgets** ausreichen (Buttons, Slider, etc.)
- 💾 **Genug RAM** verfügbar (ESP32: ✓, STM32F1: ✗)
- 🎨 **Themes/Animationen** gewünscht
- 👥 **Wiederverwendbarkeit** (LVGL ist Standard)

### ✅ Verwende Native (LovyanGFX) wenn:

- 🎯 **Einfache UI** mit wenigen Elementen
- 🚀 **Maximale Performance** erforderlich
- 💾 **Wenig RAM** verfügbar (< 64 KB)
- 🎨 **Vollständig custom Look** gewünscht
- 📚 **Lernziel:** Verstehen wie UI funktioniert
- ⚡ **Minimaler Overhead** wichtig (Battery-powered)

### 🤝 Hybrid-Ansatz

**Kombiniere beide!**

```cpp
// LVGL für Menü
void showMenu() {
  lv_obj_t* list = lv_list_create(lv_scr_act());
  // ... LVGL UI
}

// Native für Performance-kritische Animation
void runGame() {
  lv_obj_clean(lv_scr_act());  // LVGL-UI löschen

  // Native Rendering
  while (gameRunning) {
    lcd.fillScreen(0x0000);
    drawGameSprites();
    // ... schnelles Rendering
  }

  showMenu();  // Zurück zu LVGL
}
```

---

## 🎓 Was hast du gelernt?

Nach diesem Beispiel verstehst du:

- ✅ **LVGL Grundlagen** - Display Driver, Input Driver, Widgets
- ✅ **Framework-Vorteile** - Weniger Code, mehr Features
- ✅ **Memory Trade-offs** - RAM vs. Entwicklungszeit
- ✅ **Event-System** - Callback-basierte UI-Logik
- ✅ **Entscheidungskriterien** - Wann Native, wann Framework?

**Vergleiche:** [14_Selection_Menu_Native](../14_Selection_Menu_Native/) für direkten Code-Vergleich!

---

## 🔗 Siehe auch

- [14_Selection_Menu_Native](../14_Selection_Menu_Native/) - Native LovyanGFX Version
- [LVGL Documentation](https://docs.lvgl.io/8.3/)
- [LVGL Examples](https://github.com/lvgl/lvgl/tree/master/examples)
- [LovyanGFX](https://github.com/lovyan03/LovyanGFX)

**Online Tools:**
- [LVGL Simulator](https://sim.lvgl.io/) - UI im Browser testen
- [SquareLine Studio](https://squareline.io/) - LVGL GUI Designer

---

**Framework = Schneller ans Ziel, Native = Volle Kontrolle! 🚀**
