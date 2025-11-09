# Calculator mit LVGL

Dieses Beispiel demonstriert einen Taschenrechner mit **LVGL (Light and Versatile Graphics Library)** statt direktem LovyanGFX Drawing.

## Unterschiede zu 09_Calculator

| Feature | 09_Calculator | 09b_Calculator_LVGL |
|---------|---------------|---------------------|
| UI-Framework | Direktes LovyanGFX Drawing | LVGL Widgets |
| Buttons | Manuelle Touch-Koordinaten | Button-Matrix Widget |
| Layout | Manuelle Berechnung | LVGL Layout-Engine |
| Styling | Manuelles Zeichnen | LVGL Styles |
| Event-Handling | Touch-Polling + Koordinaten | Event-Callbacks |
| Code-Komplexität | Höher | Niedriger |
| Speicherverbrauch | Minimal | Höher (LVGL-Overhead) |

## Installation

### 1. LVGL Library installieren

**Über Arduino Library Manager:**
1. Arduino IDE öffnen
2. `Tools` → `Manage Libraries...`
3. Suche nach "lvgl"
4. Installiere **LVGL by LVGL** (Version 9.x oder höher)

**Oder manuell:**
```bash
cd ~/Documents/Arduino/libraries
git clone https://github.com/lvgl/lvgl.git
```

### 2. LVGL konfigurieren

LVGL benötigt eine Konfigurationsdatei `lv_conf.h`.

**Option A: Verwende die mitgelieferte Template:**
1. Gehe zu `libraries/lvgl/`
2. Kopiere `lv_conf_template.h` nach `libraries/lv_conf.h`
3. Öffne `lv_conf.h` und ändere:
   ```c
   #if 1  // Zeile 15: von 0 auf 1 ändern
   ```

**Option B: Minimale lv_conf.h erstellen:**

Erstelle `~/Documents/Arduino/libraries/lv_conf.h` mit folgendem Inhalt:

```c
#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_USE_LOG 0
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0
#define LV_MEM_SIZE (64U * 1024U)

// Font aktivieren
#define LV_FONT_MONTSERRAT_32 1

// Widgets aktivieren
#define LV_USE_LABEL 1
#define LV_USE_BUTTON 1
#define LV_USE_BUTTONMATRIX 1

#endif
```

### 3. Beispiel kompilieren und hochladen

1. Arduino IDE öffnen
2. Datei öffnen: `09b_Calculator_LVGL.ino`
3. Board auswählen: **ESP32 Dev Module**
4. Upload

## Verwendung

Der Taschenrechner funktioniert identisch wie `09_Calculator`:

- **Ziffern (0-9):** Zahlen eingeben
- **Operatoren (+, -, *, /):** Rechenoperation wählen
- **=** : Berechnung durchführen
- **C** : Löschen (Clear)
- **.** : Dezimalpunkt

## LVGL Vorteile

### 1. Weniger Code für UI
**Ohne LVGL (09_Calculator):**
```cpp
// Manuelles Zeichnen jedes Buttons
void drawButton(int row, int col) {
  int x = BUTTON_SPACING + col * (BUTTON_WIDTH + BUTTON_SPACING);
  int y = DISPLAY_HEIGHT + BUTTON_SPACING + row * (BUTTON_HEIGHT + BUTTON_SPACING);
  int width = BUTTON_WIDTH;
  // ... 50+ Zeilen für Farben, Rahmen, Text ...
}

// Manuelle Touch-Koordinaten-Berechnung
void handleButtonPress(int touchX, int touchY) {
  int buttonY = touchY - DISPLAY_HEIGHT - BUTTON_SPACING;
  int row = buttonY / (BUTTON_HEIGHT + BUTTON_SPACING);
  int col = (touchX - BUTTON_SPACING) / (BUTTON_WIDTH + BUTTON_SPACING);
  // ...
}
```

**Mit LVGL (09b_Calculator_LVGL):**
```cpp
// LVGL erstellt alle Buttons automatisch!
btnmatrix = lv_buttonmatrix_create(screen);
lv_buttonmatrix_set_map(btnmatrix, btnm_map);
lv_obj_add_event_cb(btnmatrix, btnmatrix_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
```

### 2. Professionelles Styling
- Schatten, Farbverläufe, Animationen
- Konsistentes Look-and-Feel
- Theme-Support

### 3. Flexibles Layout
- Automatische Größenanpassung
- Responsive Design
- Alignment-System

### 4. Event-System
- Saubere Event-Callbacks statt Touch-Polling
- Mehrere Events pro Widget
- Gesten-Unterstützung

## Nachteile

- **Höherer Speicherverbrauch:** LVGL braucht ~64 KB RAM
- **Größerer Flash:** LVGL-Library ist groß (~300 KB)
- **Komplexere Setup:** Benötigt `lv_conf.h` Konfiguration
- **Lernkurve:** LVGL-API muss gelernt werden

## Wann LVGL verwenden?

**LVGL ist gut für:**
- Komplexe UIs mit vielen Widgets
- Apps die professionelles Design brauchen
- Projekte wo Code-Wartbarkeit wichtig ist
- Multi-Screen-Anwendungen

**Direktes Drawing (wie 09_Calculator) ist besser für:**
- Einfache UIs mit wenigen Elementen
- Speicher-kritische Anwendungen
- Maximale Performance
- Custom-Grafik (Charts, Spiele)

## Troubleshooting

### Compile-Fehler: "lv_conf.h not found"
→ Siehe Installation Schritt 2

### Compile-Fehler: "LV_FONT_MONTSERRAT_32 not defined"
→ In `lv_conf.h` aktivieren:
```c
#define LV_FONT_MONTSERRAT_32 1
```

### Display bleibt schwarz
→ Prüfe Serial Monitor für Fehlermeldungen
→ Stelle sicher dass `CYD_Display_Config.h` korrekt ist

### Touch funktioniert nicht
→ Prüfe ob `lcd.getTouch()` funktioniert
→ Ggf. Touch-Kalibrierung nötig

## Ressourcen

- [LVGL Dokumentation](https://docs.lvgl.io/)
- [LVGL Beispiele](https://github.com/lvgl/lvgl/tree/master/examples)
- [LVGL Forum](https://forum.lvgl.io/)

## Code-Vergleich: Zeilen

- **09_Calculator:** ~335 Zeilen (mit manuellem Drawing)
- **09b_Calculator_LVGL:** ~410 Zeilen (mit LVGL + Setup)

Aber: LVGL-Version ist wartbarer und erweiterbarer!
