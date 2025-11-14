# 📋 Beispiel 14: Selection Menu (Native LovyanGFX)

## 📝 Was macht dieses Beispiel?

Zeigt ein **vollständig scrollbares Auswahlmenü** nur mit LovyanGFX - ohne externe GUI-Frameworks.

**Features:**
- ✅ **Dynamische Item-Anzahl** (einfach Items hinzufügen/entfernen)
- ✅ **Touch-Interaktion:** Tap zum Auswählen, Drag zum Scrollen
- ✅ **Smooth Scrolling** mit automatischem Clipping
- ✅ **Visual States:** Normal, Highlighted, Selected
- ✅ **Callback-System** für Item-Aktionen
- ✅ **Performance-Optimierung:** Culling für nicht-sichtbare Items
- ✅ **Scroll-Indikator** (Scrollbar rechts)

**Steuerung:**
- **Touch auf Item:** Auswählen und Aktion ausführen
- **Drag vertikal:** Scrollen (wenn Liste länger als Bildschirm)

---

## 🎯 Lernziele

1. **Touch-Event-Handling** - Unterscheidung Tap vs. Drag
2. **Scroll-Mechanik** - Offset-basiertes Scrollen implementieren
3. **State Management** - UI-States verwalten
4. **Rendering-Optimierung** - Culling & Clipping
5. **Callback-Pattern** - Funktions-Pointer für Aktionen
6. **Dynamische UI** - Flexible Item-Anzahl

---

## 🔍 Technische Konzepte

### 1. Touch-Event State Machine

Das Beispiel unterscheidet zwischen **Tap** (kurzer Touch) und **Drag** (Wischen).

#### State Machine

```
┌─────────────┐
│   IDLE      │
│ (no touch)  │
└──────┬──────┘
       │ Touch Down
       ▼
┌─────────────┐
│ TOUCH_START │  ← touchStartY speichern
│             │    highlightedIndex setzen
└──────┬──────┘
       │
       ├─────► abs(deltaY) > THRESHOLD? ──YES──► DRAGGING
       │                                          (Scrollen)
       │                                          highlightedIndex = -1
       │
       └─────► abs(deltaY) <= THRESHOLD ────────► HIGHLIGHTING
                                                   (Item unter Touch)

       Touch Up
       │
       ▼
┌─────────────┐
│  EVALUATE   │
│             │
└──────┬──────┘
       │
       ├─────► Was DRAGGING? ────► Scroll-Position beibehalten
       │
       └─────► Was HIGHLIGHTING? ► Item auswählen + Callback
```

#### Code

```cpp
void handleTouch() {
  uint16_t x, y;
  bool touched = lcd.getTouch(&x, &y);

  if (touched && !wasTouched) {
    // ===== TOUCH START =====
    touchStartY = y;
    scrollStartOffset = scrollOffset;
    isDragging = false;
    highlightedIndex = getItemIndexAtPosition(y);

  } else if (touched && wasTouched) {
    // ===== TOUCH MOVE =====
    int deltaY = touchStartY - y;

    if (abs(deltaY) > SCROLL_THRESHOLD) {
      isDragging = true;               // Ab jetzt Scrollen
      highlightedIndex = -1;           // Kein Highlight mehr
    }

    if (isDragging) {
      scrollOffset = scrollStartOffset + deltaY;
      scrollOffset = constrain(scrollOffset, 0, maxScrollOffset);
    } else {
      highlightedIndex = getItemIndexAtPosition(y);
    }

  } else if (!touched && wasTouched) {
    // ===== TOUCH END =====
    if (!isDragging && highlightedIndex >= 0) {
      selectedIndex = highlightedIndex;
      menuItems[selectedIndex].action();  // Callback!
    }

    highlightedIndex = -1;
    wasTouched = false;
    isDragging = false;
  }
}
```

**Warum SCROLL_THRESHOLD?**

Ohne Threshold würde **jede kleinste Bewegung** als Drag erkannt → Item kann nicht ausgewählt werden!

```
SCROLL_THRESHOLD = 10 Pixel

Touch Move: 3px  → NOCH Tap (Finger zittert)
Touch Move: 15px → JETZT Drag (bewusste Bewegung)
```

---

### 2. Scroll-Mechanik

#### Konzept: Offset-basiertes Scrollen

```
┌────────────────────────┐ ▲
│      HEADER (fix)      │ │ 40px
├────────────────────────┤ │
│                        │ │
│    VISIBLE WINDOW      │ │ 280px (bei 320px Höhe)
│                        │ │
│  Items werden hier     │ │
│  gezeichnet mit        │ │
│  Y-Offset!             │ │
│                        │ │
└────────────────────────┘ ▼

scrollOffset = 0    → Item 0 oben
scrollOffset = 60   → Item 0 teilweise versteckt, Item 1 oben
scrollOffset = 120  → Item 2 oben
```

#### Berechnung

```cpp
// Item-Position = Basis-Position - Scroll-Offset
int itemY = HEADER_HEIGHT + (itemIndex * ITEM_HEIGHT) - scrollOffset;

// Beispiel: Item 3, scrollOffset = 120
itemY = 40 + (3 * 60) - 120 = 40 + 180 - 120 = 100
```

#### Maximaler Scroll

```cpp
int totalHeight = MENU_ITEM_COUNT * ITEM_HEIGHT;
int visibleHeight = lcd.height() - HEADER_HEIGHT;
maxScrollOffset = max(0, totalHeight - visibleHeight);

// Beispiel: 10 Items à 60px = 600px total
//           Sichtbar: 280px
maxScrollOffset = 600 - 280 = 320px
```

**Warum constrain()?**

```cpp
scrollOffset = constrain(scrollOffset, 0, maxScrollOffset);
```

Verhindert:
- ❌ Scrollen über Anfang hinaus (scrollOffset < 0)
- ❌ Scrollen über Ende hinaus (scrollOffset > max)

---

### 3. Rendering-Optimierung: Culling & Clipping

Bei vielen Items (z.B. 100) ist es **ineffizient**, alle zu zeichnen!

#### A) Culling (Überspringen nicht-sichtbarer Items)

```cpp
for (int i = 0; i < MENU_ITEM_COUNT; i++) {
  int itemY = contentY + (i * ITEM_HEIGHT) - scrollOffset;

  // Culling: Skip if outside visible area
  if (itemY + ITEM_HEIGHT < contentY || itemY > lcd.height()) {
    continue;  // ← Item nicht zeichnen!
  }

  // Item ist sichtbar - zeichnen
  drawMenuItem(i, itemY, ITEM_HEIGHT, true);
}
```

**Performance-Gewinn:**
- Ohne Culling: Zeichne 100 Items → ~500ms pro Frame
- Mit Culling: Zeichne 5-6 Items → ~30ms pro Frame

#### B) Clipping (Teilweise sichtbare Items)

Was wenn Item nur **halb** sichtbar ist?

```
┌────────────┐ ▲
│ HEADER     │ │
├────────────┤ ← contentY (40px)
│ Item 0 ████│ │ ← Nur untere Hälfte sichtbar!
├────────────┤ │
│ Item 1     │ │
│            │ │
...
```

**Lösung:** Zeichne nur sichtbaren Teil!

```cpp
if (itemY < contentY) {
  // Oberer Teil abgeschnitten
  int clipHeight = ITEM_HEIGHT - (contentY - itemY);
  drawMenuItem(i, contentY, clipHeight, false);  // false = kein Text
}
```

**Warum `drawAll = false`?**

Bei geclippten Items würde Text **abgeschnitten** aussehen → besser komplett weglassen!

---

### 4. MenuItem-Struktur mit Callbacks

```cpp
struct MenuItem {
  const char* title;       // Haupttext
  const char* subtitle;    // Untertitel
  uint16_t icon;           // Farbe (später: echte Icon-IDs)
  void (*action)();        // Funktions-Pointer!
};
```

#### Callback-Pattern

**Lambda-Funktionen:**

```cpp
MenuItem menuItems[] = {
  {"WiFi", "Network settings", 0x001F, []() {
    Serial.println("WiFi selected");
  }},

  {"Bluetooth", "Pair devices", 0x03EF, []() {
    Serial.println("Bluetooth selected");
  }},
};
```

**Normale Funktionen:**

```cpp
void wifiAction() {
  Serial.println("Opening WiFi settings...");
  // Hier könnte ein Sub-Menü geöffnet werden
}

MenuItem menuItems[] = {
  {"WiFi", "Network settings", 0x001F, wifiAction},
};
```

**Callback ausführen:**

```cpp
if (menuItems[selectedIndex].action != nullptr) {
  menuItems[selectedIndex].action();
}
```

**Warum Callbacks?**

- ✅ **Flexibel:** Jedes Item kann eigene Aktion haben
- ✅ **Erweiterbar:** Neue Actions ohne Code-Änderung
- ✅ **Sauber:** Trennung von UI und Logik

---

### 5. Visual States

Drei Zustände für Items:

```cpp
enum ItemState {
  NORMAL,       // Normaler Zustand
  HIGHLIGHTED,  // Touch ist über Item (noch nicht losgelassen)
  SELECTED      // Item wurde ausgewählt (letzter Tap)
};
```

**Farben:**

```cpp
uint16_t bgColor = COLOR_ITEM_BG;          // Default

if (index == highlightedIndex) {
  bgColor = COLOR_ITEM_HIGHLIGHT;          // Dunkler Grau
}

if (index == selectedIndex) {
  bgColor = COLOR_ITEM_SELECTED;           // Blau
}
```

**Visuelle Hierarchie:**

```
SELECTED > HIGHLIGHTED > NORMAL

Item kann nur einen Zustand haben!
(außer: highlightedIndex == selectedIndex)
```

---

## 🎨 UI-Design-Entscheidungen

### Layout-Struktur

```
┌─────────────────────────────┐
│  HEADER (40px, fix)         │
│  "Settings"     "10 items"  │
├─────────────────────────────┤
│  ┌──┐                    ▶  │ ← Item (60px hoch)
│  │🔵│ WiFi Settings         │   - Icon (24px circle)
│  └──┘ Configure network     │   - Title (groß)
│                             │   - Subtitle (klein, grau)
├─────────────────────────────┤   - Arrow rechts
│  ┌──┐                    ▶  │
│  │🔵│ Bluetooth             │
│  └──┘ Pair devices          │
├─────────────────────────────┤
│          ...                │
│                             │
│                          ┃  │ ← Scrollbar (4px)
└─────────────────────────────┘
```

### Warum 60px Item-Höhe?

- **Touch-Target-Größe:** Mindestens 44-48px laut UX-Guidelines
- **Lesbarkeit:** Genug Platz für Title + Subtitle
- **Anzahl sichtbar:** ~4.5 Items bei 280px Höhe (gut für Kontext)

### Warum Icons als Farb-Kreise?

- **Einfachheit:** Keine Icon-Fonts oder Bitmaps nötig
- **Performance:** Kreis zeichnen ist schnell
- **Erweiterbar:** Kann später durch echte Icons ersetzt werden

---

## 🚀 Experimente & Erweiterungen

### 1. 🖼️ Echte Icons statt Farb-Kreise

**Aktuell:** `uint16_t icon` = Farbe

**Erweitere auf Icon-IDs:**

```cpp
struct MenuItem {
  const char* title;
  const char* subtitle;
  const uint8_t* iconBitmap;  // Pointer auf Icon-Daten
  uint16_t iconColor;
  void (*action)();
};

// 16x16 Icon (Bitmap)
const uint8_t ICON_WIFI[] PROGMEM = {
  0b00000000, 0b00000000,
  0b00000111, 0b11100000,
  0b00011111, 0b11111000,
  0b00111100, 0b00111100,
  // ... 16 Zeilen
};

void drawMenuItem(...) {
  // Icon aus PROGMEM zeichnen
  lcd.drawBitmap(x, y, iconBitmap, 16, 16, iconColor);
}
```

**Oder verwende Icon-Font:**
- [Font Awesome](https://fontawesome.com/)
- [Material Icons](https://fonts.google.com/icons)

---

### 2. 📊 Multi-Column Layout

**Idee:** Zwei Spalten für mehr Items auf einmal.

```cpp
const int COLUMNS = 2;
const int ITEM_WIDTH = lcd.width() / COLUMNS;

void drawMenuItems() {
  for (int i = 0; i < MENU_ITEM_COUNT; i++) {
    int col = i % COLUMNS;
    int row = i / COLUMNS;

    int itemX = col * ITEM_WIDTH;
    int itemY = HEADER_HEIGHT + (row * ITEM_HEIGHT) - scrollOffset;

    drawMenuItem(i, itemX, itemY, ITEM_WIDTH, ITEM_HEIGHT);
  }
}
```

---

### 3. 🎯 Swipe-Aktionen (iOS-Style)

**Idee:** Nach links wischen zeigt "Delete"-Button.

```cpp
int swipeOffset = 0;  // Pro Item

void handleTouch() {
  // Horizontales Drag erkennen
  if (abs(deltaX) > abs(deltaY)) {
    // Horizontal Swipe!
    swipeOffset = constrain(-deltaX, -80, 0);
  }
}

void drawMenuItem(...) {
  // Item nach links verschieben
  lcd.fillRect(x + swipeOffset, y, width, height, bgColor);

  // Delete-Button rechts (wenn geswiped)
  if (swipeOffset < -20) {
    lcd.fillRect(x + width - 80, y, 80, height, 0xF800);  // Rot
    lcd.setCursor(x + width - 70, y + 20);
    lcd.print("Delete");
  }
}
```

---

### 4. 🔍 Such-Funktion

**Idee:** Header mit Suchfeld, Items filtern.

```cpp
char searchQuery[32] = "";
int filteredIndices[MAX_ITEMS];
int filteredCount = 0;

void filterItems(const char* query) {
  filteredCount = 0;

  for (int i = 0; i < MENU_ITEM_COUNT; i++) {
    if (strstr(menuItems[i].title, query) != nullptr) {
      filteredIndices[filteredCount++] = i;
    }
  }
}

void drawMenuItems() {
  for (int i = 0; i < filteredCount; i++) {
    int itemIndex = filteredIndices[i];
    // Zeichne menuItems[itemIndex]
  }
}
```

**Touch-Keyboard:** Verwende virtuelle Tastatur aus Beispiel 03_Touch_Keyboard (falls vorhanden).

---

### 5. 🎨 Sections mit Headers

**Idee:** Gruppiere Items in Kategorien.

```cpp
enum ItemType {
  TYPE_HEADER,
  TYPE_ITEM
};

struct MenuItem {
  ItemType type;
  const char* title;
  // ...
};

MenuItem menuItems[] = {
  {TYPE_HEADER, "Network", nullptr, 0, nullptr},
  {TYPE_ITEM, "WiFi", "Configure", 0x001F, wifiAction},
  {TYPE_ITEM, "Bluetooth", "Pair", 0x03EF, btAction},

  {TYPE_HEADER, "System", nullptr, 0, nullptr},
  {TYPE_ITEM, "Display", "Brightness", 0xFFE0, displayAction},
  // ...
};

void drawMenuItem(int index, ...) {
  if (menuItems[index].type == TYPE_HEADER) {
    // Zeichne Kategorie-Header (kleiner, fett)
    lcd.setTextSize(1);
    lcd.setTextColor(COLOR_TEXT_SECONDARY);
    lcd.setCursor(x + 10, y + 5);
    lcd.print(menuItems[index].title);
  } else {
    // Normales Item
  }
}
```

---

### 6. ⚡ Kinetic Scrolling (Momentum)

**Aktuell:** Scrollen stoppt sofort bei Touch Release.

**Erweitere:** Scroll läuft mit Trägheit weiter!

```cpp
float scrollVelocity = 0;
const float FRICTION = 0.92;  // 0.0 - 1.0

void handleTouch() {
  static int lastY = 0;
  static unsigned long lastTime = 0;

  if (touched && isDragging) {
    // Berechne Geschwindigkeit
    unsigned long now = millis();
    int deltaY = y - lastY;
    int deltaTime = now - lastTime;

    if (deltaTime > 0) {
      scrollVelocity = (float)deltaY / deltaTime * 10;  // px/s
    }

    lastY = y;
    lastTime = now;
  }

  if (!touched && wasTouched && isDragging) {
    // Touch losgelassen - Velocity bleibt!
  }
}

void loop() {
  // Velocity-basiertes Scrollen
  if (!isDragging && abs(scrollVelocity) > 0.1) {
    scrollOffset -= (int)scrollVelocity;
    scrollOffset = constrain(scrollOffset, 0, maxScrollOffset);

    scrollVelocity *= FRICTION;  // Abbremsen
  }

  handleTouch();
  drawUI();
}
```

---

### 7. 📱 Pull-to-Refresh

**Idee:** Nach unten ziehen wenn schon ganz oben = Refresh.

```cpp
const int PULL_THRESHOLD = 80;
bool isPulling = false;

void handleTouch() {
  if (scrollOffset == 0 && deltaY < 0) {
    // Bereits ganz oben + weitere Bewegung nach unten
    isPulling = true;
    int pullAmount = -deltaY;

    if (pullAmount > PULL_THRESHOLD) {
      // REFRESH TRIGGERED!
      Serial.println("Refreshing...");
      refreshMenuItems();
      isPulling = false;
    }
  }
}

void drawHeader() {
  if (isPulling) {
    // Zeige Refresh-Icon
    lcd.drawCircle(120, 20, 10, COLOR_TEXT);
    // Rotierender Kreis-Animation
  }
}
```

---

## 🐛 Troubleshooting

### Problem: "Touch reagiert nicht"

**Ursache:** Touch-Kalibrierung falsch.

**Debug:**

```cpp
void loop() {
  uint16_t x, y;
  if (lcd.getTouch(&x, &y)) {
    Serial.printf("Touch: x=%d, y=%d\n", x, y);
  }
}
```

**Lösung:** Passe `cfg.x_min/x_max/y_min/y_max` in LGFX-Konfiguration an.

---

### Problem: "Scrollen ist ruckelig"

**Ursache:** Zu viele Items = langsames Rendering.

**Lösung 1:** Aktiviere Culling (bereits im Code).

**Lösung 2:** Reduziere `delay(16)` auf `delay(10)`.

**Lösung 3:** Nutze Sprites (double buffering):

```cpp
LGFX_Sprite sprite(&lcd);

void setup() {
  sprite.createSprite(lcd.width(), lcd.height());
}

void drawUI() {
  sprite.fillScreen(COLOR_BG);
  // Zeichne alles auf sprite...
  sprite.pushSprite(0, 0);  // Auf Display kopieren
}
```

---

### Problem: "Items 'verschwinden' beim Scrollen"

**Ursache:** Clipping-Logik fehlerhaft.

**Debug:** Deaktiviere Clipping temporär:

```cpp
void drawMenuItems() {
  for (int i = 0; i < MENU_ITEM_COUNT; i++) {
    int itemY = HEADER_HEIGHT + (i * ITEM_HEIGHT) - scrollOffset;

    // Kein Culling/Clipping für Debug
    drawMenuItem(i, itemY, ITEM_HEIGHT, true);
  }
}
```

---

### Problem: "Kann nicht bis zum Ende scrollen"

**Ursache:** `maxScrollOffset` falsch berechnet.

**Debug:**

```cpp
void setup() {
  Serial.printf("Total height: %d\n", MENU_ITEM_COUNT * ITEM_HEIGHT);
  Serial.printf("Visible height: %d\n", lcd.height() - HEADER_HEIGHT);
  Serial.printf("Max scroll: %d\n", maxScrollOffset);
}
```

**Überprüfe:** Ist maxScrollOffset > 0 wenn mehr Items als Platz?

---

## 📚 Native vs. Framework - Wann was?

### ✅ Verwende Native (LovyanGFX direkt) wenn:

- 🎯 **Einfache UI** mit wenigen Elementen
- 🚀 **Performance kritisch** (vollständige Kontrolle)
- 💾 **Wenig RAM** verfügbar (kein Framework-Overhead)
- 🎨 **Custom Look** gewünscht (keine Standard-Widgets)
- 📚 **Lernziel:** UI-Grundlagen verstehen

### ❌ Verwende Framework (LVGL) wenn:

- 🏗️ **Komplexe UI** mit vielen Widgets
- ⏱️ **Schnelle Entwicklung** wichtiger als Performance
- 🎨 **Standard-Look** ausreichend
- 🔧 **Viele Features:** Animationen, Themes, Input-Methoden
- 👥 **Team-Arbeit:** Standard-Bibliothek = bessere Wartbarkeit

**Siehe auch:** [14b_Selection_Menu_LVGL](../14b_Selection_Menu_LVGL/) für Framework-Vergleich!

---

## 🎓 Was hast du gelernt?

Nach diesem Beispiel verstehst du:

- ✅ **Touch-State-Machine** - Unterscheidung Tap vs. Drag
- ✅ **Scroll-Mechanik** - Offset-basiertes Rendering
- ✅ **Rendering-Optimierung** - Culling & Clipping
- ✅ **Callback-Pattern** - Funktions-Pointer für flexible Aktionen
- ✅ **UI-State-Management** - Visual Feedback für Interaktion
- ✅ **Performance-Trade-offs** - Native vs. Framework

**Nächste Schritte:**
1. Vergleiche mit [14b_Selection_Menu_LVGL](../14b_Selection_Menu_LVGL/)
2. Implementiere eine der Erweiterungen (Icons, Swipe, Kinetic Scroll)
3. Baue eigene Menu-Struktur für dein Projekt

---

## 🔗 Siehe auch

- [14b_Selection_Menu_LVGL](../14b_Selection_Menu_LVGL/) - Gleiche Funktion mit LVGL
- [03_Touch_Zones](../03_Touch_Zones/) - Touch-Grundlagen
- [00_Hello_CYD](../00_Hello_CYD/) - Display-Basics

**Externe Ressourcen:**
- [LovyanGFX Documentation](https://github.com/lovyan03/LovyanGFX)
- [Touch UI Design Guidelines](https://developer.apple.com/design/human-interface-guidelines/inputs/touchscreen-gestures/)

---

**Viel Spaß beim Experimentieren! 📱✨**
