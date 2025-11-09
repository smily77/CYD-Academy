# Beispiel 10b: Touch-Buttons mit LVGL

Moderne Touch-Button-Demo mit LVGL (Light and Versatile Graphics Library).

## Features

### Button-Typen

1. **Momentary Button** (oben links)
   - Aktiv solange der Button gedrückt wird
   - Rot wenn aktiv
   - Steuert rote LED

2. **Toggle Button** (oben rechts)
   - Schaltet zwischen Ein/Aus bei jedem Klick
   - Grün wenn aktiv, grau wenn inaktiv
   - Steuert grüne LED

3. **Radio Button Gruppe** (unten: A, B, C)
   - Nur ein Button kann gleichzeitig aktiv sein
   - Blauer Hintergrund und weißer Rahmen wenn aktiv
   - Keine LED-Steuerung

### LVGL Features

- **Event-basierte Architektur**: Keine Touch-Polling, sondern Callbacks
- **Modernes Design**:
  - Schatten-Effekte
  - Gradient-Hintergrund
  - Abgerundete Ecken
  - Smooth Transitions
- **Dynamische Status-Anzeige**: Farbige Status-Labels unten
- **Responsive Layout**: Automatische Skalierung für verschiedene Display-Größen

## Unterschiede zu Beispiel 10

| Feature | 10_TouchButtons | 10b_TouchButtons_LVGL |
|---------|----------------|----------------------|
| Framework | LovyanGFX Direct | LVGL |
| Touch-Handling | Polling in loop() | Event Callbacks |
| Code-Zeilen | ~300 | ~400 |
| Styling | Manuell | LVGL Themes |
| Animationen | Keine | Eingebaut |
| Wartbarkeit | Gut | Sehr gut |

## Hardware

- **Display**: CYD 2.8" oder 3.5"
- **Orientation**: Landscape (Querformat)
- **RGB LED**: Optional (zeigt Momentary/Toggle Status)

## Installation

1. **LVGL v8.3.11** installieren:
   - Arduino IDE → Tools → Manage Libraries
   - Suche "lvgl"
   - Installiere Version **8.3.11**

2. **Board-Typ wählen** in `CYD_Display_Config.h`:
   ```cpp
   #define CYD28_USB_C
   //#define CYD35_USB_C
   //#define CYD28_USB_M
   ```

3. **Sketch hochladen**

Die benötigte `lv_conf.h` ist bereits im Projekt-Verzeichnis enthalten.

## Verwendung

### Serial Monitor

Öffne den Serial Monitor (115200 Baud) um Button-Events zu sehen:

```
Button 1 (Momentary): GEDRÜCKT
Button 1 (Momentary): LOSGELASSEN
Button 2 (Toggle): EIN
Radio B aktiviert
```

### LED-Anzeige

- **Rote LED**: Momentary Button gedrückt
- **Grüne LED**: Toggle Button aktiv
- **Blaue LED**: Nicht verwendet

### Status-Label

Unten am Display zeigt der aktuelle Zustand:
- Rot: "Momentary GEDRÜCKT"
- Grün: "Toggle EIN"
- Blau: "Radio A/B/C aktiv"

## Code-Struktur

```
createButtonUI()          - Erstellt alle UI-Elemente
btn_momentary_event_cb()  - Handler für Momentary Button
btn_toggle_event_cb()     - Handler für Toggle Button
btn_radio_event_cb()      - Handler für Radio Buttons
updateLED()               - Aktualisiert RGB LED
updateStatusLabel()       - Aktualisiert Status-Anzeige
```

## LVGL v8 API

Dieses Beispiel zeigt wichtige LVGL v8 Konzepte:

```cpp
// Button erstellen
lv_obj_t *btn = lv_btn_create(screen);

// Styling
lv_obj_set_style_bg_color(btn, lv_color_hex(0xFF0000), LV_STATE_DEFAULT);
lv_obj_set_style_shadow_width(btn, 8, 0);

// Event Callback
lv_obj_add_event_cb(btn, my_callback, LV_EVENT_CLICKED, user_data);

// Label im Button
lv_obj_t *label = lv_label_create(btn);
lv_label_set_text(label, "Click me");
lv_obj_center(label);
```

## Lernziele

- LVGL Button-Widgets und Styling
- Event-basierte Programmierung
- Dynamische UI-Updates
- State-Management in LVGL
- Moderne UI/UX Patterns

## Erweiterungsideen

1. **Mehr Button-Typen**: Checkbox, Switch, Slider
2. **Animationen**: Fade-in/out beim Zustandswechsel
3. **Themes**: Light/Dark Mode umschalten
4. **Gestures**: Long-press, Swipe erkennen
5. **Charts**: Visualisierung der Button-Aktivität

## Siehe auch

- [Beispiel 09b: Calculator mit LVGL](../09b_Calculator_LVGL/)
- [LVGL Documentation](https://docs.lvgl.io/8.3/)
- [LVGL Examples](https://github.com/lvgl/lvgl/tree/release/v8.3/examples)
