# Tetris Modern - Mit Ghost-Piece & Rainbow

Der Puzzle-Klassiker mit Ghost-Piece, Rainbow-Effekten und Partikel-Explosionen!

**Besonderheit:** Nutzt **Portrait-Modus** (240x320) wie das Original!

## Was ist neu?

### Visuelle Effekte
- **Ghost-Piece:** Zeigt wo Teil landen wird (transparent)
- **Rainbow-Effekte:** Bunte Partikel bei Line-Clear
- **Gradient-Board:** Schöner Hintergrund
- **Neon-Glow:** Alle Teile leuchten
- **Smooth Drop:** Interpolierte Bewegung
- **Partikel-Explosionen:** Bei jedem Line-Clear
- **Modern UI:** Gradient-Bars und Neon-Text

### Gameplay
Identisch mit 18_Tetris:
- Gleiche Steuerung
- Gleiche Spielregeln
- 7 Tetromino-Typen
- Score & Level System

## Hardware

- **Display:** CYD 2.8" oder 3.5" (240x320 Portrait!)
- **Buttons:** 4x Digital (tasteA, tasteB, tasteC, tasteD)

**Hinweis:** Alle Pins sind in `CYD_Display_Config.h` definiert.

## Steuerung

- **tasteD:** Links bewegen
- **tasteA:** Rechts bewegen
- **tasteC:** Schnell-Fall (Hard Drop) + Explosions-Effekt
- **tasteB:** Drehen + Rotation-Partikel

## Features

- **Ghost-Piece:** Sehe Lande-Position (50% transparent)
- **Rainbow Line-Clear:** Bunte Partikel-Explosion
- **Neon-Tetrominos:** Alle Farben leuchten
- **Smooth Drop-Animation:** Interpoliert
- **Rotation-Effekt:** Partikel beim Drehen
- **Combo-Counter:** Zeigt aufeinanderfolgende Clears
- **Modern Score-Display:** Gradient-Bars
- **Next-Piece Glow:** Pulsierender Preview

## Design-Philosophie

### Retro vs Modern

| Feature | Retro | Modern |
|---------|-------|--------|
| Hintergrund | Schwarz | Gradient Blau→Schwarz |
| Tetrominos | Flach, Farben | Neon-Glow + Schatten |
| Ghost-Piece | Keine | Transparent-Preview |
| Line-Clear | Einfach | Rainbow-Partikel |
| Drop | Instant | Smooth Animation |
| Rotation | Instant | Partikel-Effekt |
| UI | Einfach | Gradient-Bars |
| Performance | 60 FPS | 55-60 FPS |

## Code-Highlights

### Neue Funktionen
```cpp
void drawGradientBackground()        // Atmosphäre
void drawGhostPiece()                // Lande-Preview
void drawModernTetromino(...)        // Neon-Teil mit Glow
void spawnLineClearParticles(...)    // Rainbow-Explosion
void drawSmoothDrop(...)             // Interpolation
void drawRotationEffect(...)         // Dreh-Partikel
void drawModernUI()                  // Gradient-Bars
```

### Farbpalette (Neon)
```cpp
// Tetrominos
#define COLOR_I    0x07FF  // Cyan
#define COLOR_O    0xFFE0  // Gelb
#define COLOR_T    0xF81F  // Magenta
#define COLOR_S    0x07E0  // Grün
#define COLOR_Z    0xF800  // Rot
#define COLOR_L    0xFD20  // Orange
#define COLOR_J    0x001F  // Blau
```

## Quick Start

```cpp
#include <CYD_Display_Config.h>
#include <TetrisGameModern.h>

LGFX lcd;
TetrisGameModern game;

void setup() {
  lcd.init();
  lcd.setRotation(0);  // Portrait
  game.init(&lcd);
}

void loop() {
  game.update();
}
```

## Performance

- **~55-60 FPS** mit allen Effekten
- **Ghost-Piece:** Real-time Berechnung
- **Line-Clear Partikel:** 50 pro Clear
- **Smooth Drop:** 4-Frame Interpolation

## Effekte im Detail

### Ghost-Piece
- **Auto-Berechnung:** Zeigt Lande-Position
- **50% Transparent:** Gut sichtbar
- **Gleiche Form:** Wie aktuelles Teil
- **Real-time Update:** Bei jeder Bewegung

### Rainbow Line-Clear
- **50 Partikel** pro gelöschte Zeile
- **Regenbogen-Farben:** Zufällig verteilt
- **Explosion:** Von Zeilenmitte nach außen
- **Lifetime:** 30 Frames
- **Gravity:** Nach unten

### Smooth Drop
- **4 Frames Interpolation:** Smooth Movement
- **Drop-Trail:** Motion-Blur Effekt
- **Hard-Drop:** Sofort mit Particles
- **Visual Feedback:** Klar erkennbar

### Rotation-Partikel
- **8 Partikel** pro Drehung
- **Farbe:** Vom Tetromino
- **Kreis-Pattern:** Um Teil herum
- **Kurze Lifetime:** 10 Frames

## Anpassungen

### Mehr Line-Clear Partikel
```cpp
spawnLineClearParticles(y, 80);  // statt 50
```

### Ghost-Piece-Transparenz ändern
```cpp
// In drawGhostPiece():
uint16_t ghostColor = dimColor(color, 0.3);  // statt 0.5 (dunkler)
```

### Andere Rainbow-Farben
```cpp
uint16_t rainbowColors[] = {
  COLOR_NEON_RED,
  COLOR_NEON_ORANGE,
  COLOR_NEON_YELLOW,
  COLOR_NEON_GREEN,
  COLOR_NEON_CYAN,
  COLOR_NEON_BLUE,
  COLOR_NEON_MAGENTA
};
```

### Smooth Drop-Speed
```cpp
#define DROP_INTERPOLATION 2  // statt 4 (schneller)
```

## Spieltipps mit Modern Features

### Ghost-Piece nutzen
- **Perfekte Platzierung:** Sehe Endposition
- **Schnelle Entscheidungen:** Weniger Fehler
- **Hard-Drop Timing:** Exakte Landung
- **Brunnen füllen:** Präzise I-Piece-Drops

### Rainbow-Combos
- **4-Zeilen Clear:** Spektakuläre Explosion
- **Mehrfach-Clear:** Particle-Overload
- **Visual Feedback:** Erfolg sichtbar
- **Motivierend:** Belohnung für gutes Spiel

## Troubleshooting

### Ghost-Piece flackert
- Normal bei schneller Bewegung
- Kein Bug, nur Rendering-Update
- Reduziere delay() für smootheren Look

### Zu viele Partikel
- Reduziere Line-Clear Partikel (50 → 30)
- Deaktiviere Rotation-Particles
- Erhöhe delay()

### FPS zu niedrig
- Deaktiviere Ghost-Piece temporär
- Reduziere alle Partikel-Counts
- Vereinfache Gradient-Background

### Ghost-Piece falsch positioniert
- Bug! Prüfe Kollisions-Check
- Sollte korrekte Lande-Position zeigen
- Bei Wall-Kicks aktualisieren

## Credits

- **Original:** Tetris (1984, Alexey Pajitnov)
- **Retro-Version:** 18_Tetris
- **Modern Design:** 18b_Tetris_Modern

## Genieße Tetris mit Ghost-Piece!
