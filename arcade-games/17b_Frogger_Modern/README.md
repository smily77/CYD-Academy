# Frogger Modern - Mit Animationen

Straßen-Klassiker mit animiertem Wasser, Smooth Movement und Partikel-Effekten!

## Was ist neu?

### Visuelle Effekte
- **Animiertes Wasser:** Fließende Wellen-Animation
- **Gradient-Straße:** Asphalt-Textur
- **Smooth Movement:** Interpolierte Frosch-Bewegung
- **Partikel-Splash:** Beim Ins-Wasser-Fallen
- **Auto-Schatten:** 3D-Effekt
- **Log-Glanz:** Highlights auf Logs
- **Turtle-Animation:** Tauch-Effekte
- **Neon-Ziele:** Leuchtende Goal-Slots

### Gameplay
Identisch mit 17_Frogger:
- Gleiche Steuerung
- Gleiche Spielregeln
- Grid-basierte Bewegung
- Zeit-Limit

## Hardware

- **Display:** CYD 2.8" oder 3.5" (320x240 Landscape)
- **Buttons:** 4x Digital (tasteA, tasteB, tasteC, tasteD)

**Hinweis:** Alle Pins sind in `CYD_Display_Config.h` definiert.

## Steuerung

- **tasteA:** Oben (vorwärts)
- **tasteB:** Links
- **tasteC:** Rechts
- **tasteD:** Unten (rückwärts)

## Features

- **Animated Frog:** Smooth Sprung-Animation
- **Flowing Water:** Wellen bewegen sich
- **3D Cars:** Schatten und Glanz
- **Splash Particles:** Beim Ins-Wasser-Fallen
- **Glowing Goals:** Neon-Ziele pulsieren
- **Modern UI:** Gradient-Bars für Zeit/Leben
- **Log Highlights:** Holz-Textur-Effekt

## Design-Philosophie

### Retro vs Modern

| Feature | Retro | Modern |
|---------|-------|--------|
| Wasser | Blau, statisch | Animierte Wellen |
| Frosch | Grün, flach | Smooth Animation |
| Autos | Flach, einfarbig | 3D + Schatten |
| Logs | Braun, flach | Glanz + Highlights |
| Straße | Grau | Gradient-Asphalt |
| Splash | Keine | Partikel-Effekt |
| Ziele | Einfach | Neon-Glow |
| Performance | 60 FPS | 55-60 FPS |

## Code-Highlights

### Neue Funktionen
```cpp
void drawAnimatedWater()             // Fließende Wellen
void drawModernFrog(...)             // Smooth Frosch
void drawModernCar(...)              // 3D-Auto mit Schatten
void drawModernLog(...)              // Log mit Highlights
void drawTurtleAnimation(...)        // Tauch-Effekt
void spawnSplashParticles(...)       // Wasser-Splash
void updateWaterAnimation()          // Wellen-Phase
```

### Farbpalette
```cpp
#define COLOR_WATER_LIGHT  0x1D3F  // Hellblau
#define COLOR_WATER_DARK   0x0218  // Dunkelblau
#define COLOR_FROG         0x07E0  // Grün
#define COLOR_LOG          0x8400  // Braun
#define COLOR_ROAD         0x4208  // Grau
#define COLOR_GOAL         0xFFE0  // Gelb-Neon
```

## Quick Start

```cpp
#include <CYD_Display_Config.h>
#include <FroggerGameModern.h>

LGFX lcd;
FroggerGameModern game;

void setup() {
  lcd.init();
  lcd.setRotation(1);
  game.init(&lcd);
}

void loop() {
  game.update();
}
```

## Performance

- **~55-60 FPS** mit allen Effekten
- **Wasser-Animation:** 4 Phasen, smooth
- **Splash-Partikel:** 10 pro Event
- **Smooth Movement:** Interpoliert über 100ms

## Effekte im Detail

### Wasser-Animation
- **4 Wellen-Phasen:** Sinus-basiert
- **Horizontaler Flow:** Von links nach rechts
- **2 Farben:** Hell/Dunkelblau alternierend
- **Smooth:** 10 FPS Animation

### Frosch-Movement
- **Interpoliert:** Grid zu Grid über 6 Frames
- **Rotation:** Zeigt in Bewegungsrichtung
- **Bounce-Effekt:** Sprung-Animation
- **Smooth:** Kein abruptes Snapping

### Splash-Effekte
- **10 Partikel** beim Ins-Wasser-Fallen
- **Blaue Tropfen** in alle Richtungen
- **Gravitation:** Nach unten
- **Fade-Out:** Über 15 Frames

### Auto-Rendering
- **Gradient-Body:** Hell nach Dunkel
- **Schatten:** Unter Auto (schwarz-transparent)
- **Highlights:** Oben (weiß)
- **Verschiedene Farben:** Per Reihe

## Anpassungen

### Wasser-Speed ändern
```cpp
// In updateWaterAnimation():
waterPhase += 0.2;  // statt 0.1 (schneller)
```

### Mehr Splash-Partikel
```cpp
spawnSplashParticles(x, y, 20);  // statt 10
```

### Frosch-Movement-Speed
```cpp
#define MOVE_INTERPOLATION 4  // statt 6 (schneller)
```

### Andere Wasser-Farben
```cpp
#define COLOR_WATER_LIGHT  0x07FF  // Cyan
#define COLOR_WATER_DARK   0x03FF  // Dunkel-Cyan
```

## Troubleshooting

### Animationen ruckeln
- Normal bei niedriger FPS
- Reduziere Partikel-Count
- Vereinfache Wasser-Animation

### Wasser flackert
- Normal bei komplexer Animation
- Erhöhe delay() leicht
- Reduziere Wellen-Phasen

### Frosch-Bewegung zu langsam
- Ändere MOVE_INTERPOLATION
- Reduziere von 6 auf 4 Frames

## Credits

- **Original:** Frogger (1981, Konami)
- **Retro-Version:** 17_Frogger
- **Modern Design:** 17b_Frogger_Modern

## Genieße den animierten Frosch!
