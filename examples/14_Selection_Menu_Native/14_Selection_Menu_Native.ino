/*
  14_Selection_Menu_Native - Scrollbares Auswahlmenü mit Touch

  Dieses Beispiel zeigt ein dynamisches, scrollbares Menü nur mit LovyanGFX.

  Funktionen:
  - Dynamische Anzahl von Menü-Einträgen
  - Touch-basierte Auswahl
  - Automatisches Scrollen bei vielen Einträgen
  - Visual Feedback (Highlight, Selection)
  - Callback-System für Aktionen
  - Smooth Scrolling mit Drag-Geste

  Lernziele:
  - Touch-Event-Handling (Tap vs. Drag)
  - Scroll-Mechanik implementieren
  - Dynamische UI-Elemente
  - State Management (Selected, Highlighted, Normal)
  - Callback-Pattern

  Hardware:
  - 1x CYD Display (2.8" oder 3.5")
  - Nur Touch-Input!

  Steuerung:
  - Touch auf Item: Auswählen
  - Drag vertikal: Scrollen (wenn mehr Items als Platz)
*/

#include <CYD_Display_Config.h>

// ===== KONSTANTEN =====

// Farben
#define COLOR_BG 0x18E3          // Dunkles Grau (#1a1a1a)
#define COLOR_HEADER 0x2945      // Header Background (#252525)
#define COLOR_ITEM_BG 0x2945     // Item Background
#define COLOR_ITEM_HIGHLIGHT 0x4208  // Highlighted Item (#424242)
#define COLOR_ITEM_SELECTED 0x03EF   // Selected Item (Blau)
#define COLOR_TEXT 0xFFFF        // Weißer Text
#define COLOR_TEXT_SECONDARY 0x9CF3  // Grauer Subtitle-Text
#define COLOR_SEPARATOR 0x4208   // Trennlinie

// Layout
const int HEADER_HEIGHT = 40;
const int ITEM_HEIGHT = 60;
const int ITEM_PADDING = 8;
const int SCROLL_THRESHOLD = 10;  // Pixel für Drag vs. Tap

// ===== DATENSTRUKTUREN =====

struct MenuItem {
  const char* title;
  const char* subtitle;
  uint16_t icon;           // Farbe als einfaches Icon (später: echte Icons)
  void (*action)();        // Callback-Funktion
};

// ===== GLOBALE VARIABLEN =====

LGFX lcd;

// Menu Items (HIER KANNST DU EIGENE ITEMS HINZUFÜGEN!)
MenuItem menuItems[] = {
  {"WiFi Settings", "Configure network", 0x001F, []() { Serial.println("Action: WiFi Settings"); }},
  {"Bluetooth", "Pair devices", 0x03EF, []() { Serial.println("Action: Bluetooth"); }},
  {"Display", "Brightness & timeout", 0xFFE0, []() { Serial.println("Action: Display"); }},
  {"Sound", "Volume & notifications", 0xF81F, []() { Serial.println("Action: Sound"); }},
  {"Storage", "Manage files", 0x07E0, []() { Serial.println("Action: Storage"); }},
  {"About", "Device information", 0xFD20, []() { Serial.println("Action: About"); }},
  {"Battery", "Power settings", 0x07E0, []() { Serial.println("Action: Battery"); }},
  {"Security", "Lock screen & privacy", 0xF800, []() { Serial.println("Action: Security"); }},
  {"Developer", "Advanced options", 0x7BEF, []() { Serial.println("Action: Developer"); }},
  {"Reset", "Factory reset", 0xF800, []() { Serial.println("Action: Reset"); }},
};

const int MENU_ITEM_COUNT = sizeof(menuItems) / sizeof(MenuItem);

// Scroll & Selection State
int scrollOffset = 0;        // Pixel-Offset von oben
int maxScrollOffset = 0;     // Maximaler Scroll (berechnet)
int selectedIndex = -1;      // Aktuell ausgewähltes Item (-1 = keines)
int highlightedIndex = -1;   // Unter Touch gehighlightetes Item

// Touch State
bool wasTouched = false;
int touchStartY = 0;
int scrollStartOffset = 0;
bool isDragging = false;

// ===== SETUP =====

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Selection Menu Demo (Native) ===");

  // Display initialisieren
  lcd.init();
  lcd.setRotation(0);  // Portrait
  lcd.setBrightness(255);

  // Maximalen Scroll berechnen
  int totalHeight = MENU_ITEM_COUNT * ITEM_HEIGHT;
  int visibleHeight = lcd.height() - HEADER_HEIGHT;
  maxScrollOffset = max(0, totalHeight - visibleHeight);

  Serial.printf("Menu Items: %d\n", MENU_ITEM_COUNT);
  Serial.printf("Max Scroll: %d px\n", maxScrollOffset);

  // Initiales Zeichnen
  drawUI();

  Serial.println("Ready! Touch items to select, drag to scroll.");
}

// ===== LOOP =====

void loop() {
  handleTouch();
  drawUI();
  delay(16);  // ~60 FPS
}

// ===== FUNKTIONEN =====

void handleTouch() {
  uint16_t x, y;
  bool touched = lcd.getTouch(&x, &y);

  if (touched && !wasTouched) {
    // Touch Start
    touchStartY = y;
    scrollStartOffset = scrollOffset;
    isDragging = false;
    wasTouched = true;

    // Prüfe ob Item unter Touch ist
    highlightedIndex = getItemIndexAtPosition(y);

  } else if (touched && wasTouched) {
    // Touch Move (Drag)
    int deltaY = touchStartY - y;

    if (abs(deltaY) > SCROLL_THRESHOLD) {
      isDragging = true;
      highlightedIndex = -1;  // Kein Highlight während Drag
    }

    if (isDragging) {
      // Scrollen
      scrollOffset = scrollStartOffset + deltaY;
      scrollOffset = constrain(scrollOffset, 0, maxScrollOffset);
    } else {
      // Noch kein Drag - Highlight unter Touch updaten
      highlightedIndex = getItemIndexAtPosition(y);
    }

  } else if (!touched && wasTouched) {
    // Touch End
    if (!isDragging && highlightedIndex >= 0) {
      // Tap auf Item - Auswahl!
      selectedIndex = highlightedIndex;

      // Visuelles Feedback
      drawUI();
      delay(150);  // Kurze Pause für visuelle Bestätigung

      // Callback ausführen
      if (menuItems[selectedIndex].action != nullptr) {
        menuItems[selectedIndex].action();
      }

      Serial.printf("Selected: %s\n", menuItems[selectedIndex].title);
    }

    highlightedIndex = -1;
    wasTouched = false;
    isDragging = false;
  }
}

int getItemIndexAtPosition(int touchY) {
  // Header überspringen
  if (touchY < HEADER_HEIGHT) {
    return -1;
  }

  // Position relativ zum Content-Bereich
  int contentY = touchY - HEADER_HEIGHT + scrollOffset;

  // Index berechnen
  int index = contentY / ITEM_HEIGHT;

  // Gültigkeit prüfen
  if (index >= 0 && index < MENU_ITEM_COUNT) {
    return index;
  }

  return -1;
}

void drawUI() {
  // Hintergrund
  lcd.fillScreen(COLOR_BG);

  // Header zeichnen
  drawHeader();

  // Scrollbarer Content-Bereich
  drawMenuItems();

  // Scroll-Indikator (wenn scrollbar)
  if (maxScrollOffset > 0) {
    drawScrollIndicator();
  }
}

void drawHeader() {
  // Header Background
  lcd.fillRect(0, 0, lcd.width(), HEADER_HEIGHT, COLOR_HEADER);

  // Titel
  lcd.setTextSize(2);
  lcd.setTextColor(COLOR_TEXT);
  lcd.setCursor(10, 12);
  lcd.print("Settings");

  // Item Counter (rechts)
  lcd.setTextSize(1);
  lcd.setTextColor(COLOR_TEXT_SECONDARY);
  lcd.setCursor(lcd.width() - 50, 15);
  lcd.printf("%d items", MENU_ITEM_COUNT);

  // Separator
  lcd.drawFastHLine(0, HEADER_HEIGHT - 1, lcd.width(), COLOR_SEPARATOR);
}

void drawMenuItems() {
  // Clip-Bereich für Content
  int contentY = HEADER_HEIGHT;
  int contentHeight = lcd.height() - HEADER_HEIGHT;

  for (int i = 0; i < MENU_ITEM_COUNT; i++) {
    // Position berechnen (mit Scroll-Offset)
    int itemY = contentY + (i * ITEM_HEIGHT) - scrollOffset;

    // Culling: Nur sichtbare Items zeichnen
    if (itemY + ITEM_HEIGHT < contentY || itemY > lcd.height()) {
      continue;  // Item außerhalb Sichtbereich
    }

    // Clipping für Items die teilweise sichtbar sind
    if (itemY < contentY) {
      // Oberer Teil abgeschnitten - zeichne nur sichtbaren Teil
      int clipHeight = ITEM_HEIGHT - (contentY - itemY);
      drawMenuItem(i, contentY, clipHeight, false);
    } else if (itemY + ITEM_HEIGHT > lcd.height()) {
      // Unterer Teil abgeschnitten
      int clipHeight = lcd.height() - itemY;
      drawMenuItem(i, itemY, clipHeight, false);
    } else {
      // Vollständig sichtbar
      drawMenuItem(i, itemY, ITEM_HEIGHT, true);
    }
  }
}

void drawMenuItem(int index, int y, int height, bool drawAll) {
  MenuItem &item = menuItems[index];

  // Hintergrundfarbe bestimmen
  uint16_t bgColor = COLOR_ITEM_BG;

  if (index == highlightedIndex) {
    bgColor = COLOR_ITEM_HIGHLIGHT;
  }

  if (index == selectedIndex) {
    bgColor = COLOR_ITEM_SELECTED;
  }

  // Background
  lcd.fillRect(0, y, lcd.width(), height, bgColor);

  // Nur vollen Inhalt zeichnen wenn nicht geclippt
  if (drawAll) {
    // Icon (farbiger Kreis)
    lcd.fillCircle(20, y + ITEM_HEIGHT / 2, 12, item.icon);
    lcd.drawCircle(20, y + ITEM_HEIGHT / 2, 12, COLOR_TEXT);

    // Titel
    lcd.setTextSize(1);
    lcd.setTextColor(COLOR_TEXT, bgColor);
    lcd.setCursor(45, y + 15);
    lcd.print(item.title);

    // Subtitle
    lcd.setTextColor(COLOR_TEXT_SECONDARY, bgColor);
    lcd.setCursor(45, y + 32);
    lcd.print(item.subtitle);

    // Arrow (Pfeil nach rechts)
    int arrowX = lcd.width() - 20;
    int arrowY = y + ITEM_HEIGHT / 2;
    lcd.fillTriangle(arrowX - 5, arrowY - 5,
                     arrowX - 5, arrowY + 5,
                     arrowX + 3, arrowY,
                     COLOR_TEXT_SECONDARY);
  }

  // Separator (immer zeichnen)
  lcd.drawFastHLine(0, y + height - 1, lcd.width(), COLOR_SEPARATOR);
}

void drawScrollIndicator() {
  // Scrollbar rechts
  int barX = lcd.width() - 4;
  int barY = HEADER_HEIGHT;
  int barHeight = lcd.height() - HEADER_HEIGHT;

  // Berechne Thumb-Position und -Größe
  int totalHeight = MENU_ITEM_COUNT * ITEM_HEIGHT;
  float visibleRatio = (float)barHeight / totalHeight;
  int thumbHeight = max(20, (int)(barHeight * visibleRatio));

  float scrollRatio = (float)scrollOffset / maxScrollOffset;
  int thumbY = barY + (int)((barHeight - thumbHeight) * scrollRatio);

  // Zeichne Thumb
  lcd.fillRect(barX, thumbY, 3, thumbHeight, COLOR_TEXT_SECONDARY);
}
