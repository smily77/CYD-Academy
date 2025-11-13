# ModernGames Menu - Moderne Arcade-Spiele

Zentrale Menü-Anwendung mit allen 7 modernen Arcade-Spielen - Neon-Effekte, Partikel & Animationen!

## Enthaltene Spiele (Modern Versions)

1. **Pong Modern** - Neon-Paddle mit Partikeln
2. **Snake Modern** - Gradient-Schlange & Trail
3. **Breakout Modern** - 3D-Blöcke & Explosionen
4. **Space Invaders Modern** - Glow-Aliens & Schilde
5. **Asteroids Modern** - Partikel & Bullet-Trails
6. **Frogger Modern** - Animiertes Wasser & Smooth
7. **Tetris Modern** - Ghost-Piece & Rainbow! (Portrait-Modus)

## Was ist neu?

### Visuelle Effekte
- **Neon-Glow:** Alle Spielelemente leuchten
- **Partikel-Systeme:** Explosionen und Effekte
- **Gradient-Hintergründe:** Atmosphärische Farbverläufe
- **Trail-Effekte:** Motion-Blur für bewegte Objekte
- **3D-Shading:** Schatten und Highlights
- **Animationen:** Pulsierender Glow, animiertes Wasser

### Performance
- **~55-60 FPS:** Mit allen Effekten
- **Optimiertes Rendering:** Selective Redraw
- **Partikel-Pooling:** Effiziente Memory-Nutzung

## Hardware

- **Display:** CYD 2.8" oder 3.5" (320x240 Landscape, Tetris: 240x320 Portrait)
- **Touch Screen:** Menu-Navigation
- **Buttons:** 4x Digital (tasteA, tasteB, tasteC, tasteD)
- **Potentiometer:** 2x Analog (potiLeft, potiRight) - für Pong/Breakout

**Hinweis:** Alle Pins sind in `CYD_Display_Config.h` definiert.

## Steuerung

### Im Menu
- **Touch Screen:** Spiel auswählen und starten

### Spiel-spezifisch
Identisch mit RetroGames:
- **Pong:** potiLeft & potiRight (Analog-Regler)
- **Snake:** tasteA/B/C/D (Richtungstasten)
- **Breakout:** potiLeft (Schläger-Steuerung)
- **Space Invaders:** tasteB/C (Links/Rechts), tasteA (Schießen)
- **Asteroids:** tasteB/C (Drehen), tasteA (Schießen), tasteD (Schub)
- **Frogger:** tasteA/B/C/D (Richtungstasten)
- **Tetris:** tasteD/A (Links/Rechts), tasteC (Runter), tasteB (Drehen)

## Features

- **7 Modern Games:** Komplette Sammlung mit Effekten
- **Touch-Menu:** Einfache Navigation (Neon-Design)
- **Partikel-Systeme:** In allen Spielen
- **Gradient-Backgrounds:** Atmosphärische Tiefe
- **Neon-Farbpalette:** Cyberpunk-Ästhetik
- **Automatischer Mode-Wechsel:** Tetris → Portrait

## Retro vs Modern Vergleich

| Feature | Retro | Modern |
|---------|-------|--------|
| Hintergrund | Schwarz | Gradient |
| Spielelemente | Flach | 3D + Glow |
| Effekte | Keine | Partikel, Trail |
| Farbpalette | Primär | Neon |
| Performance | 60 FPS | 55-60 FPS |
| Code-Größe | 100% | 150% |
| Stil | 1970er | 2025 |

## Quick Start

```cpp
#include <CYD_Display_Config.h>
#include <PongGameModern.h>
#include <SnakeGameModern.h>
// + weitere Modern Game-Header

LGFX lcd;
// Modern Game-Instanzen...

void setup() {
  lcd.init();
  lcd.setRotation(1);  // Landscape
  drawMenu();
}

void loop() {
  // Modern Game-Loop mit Effekten
}
```

## Code-Struktur

### Hauptfunktionen
```cpp
void drawMenu()              // Modern Menu mit Neon
void checkMenuTouch()        // Touch-Eingabe
void switchToGame(GameState) // Spiel mit Effekten starten
void returnToMenu()          // Zurück zum Menu
```

### Neue Effekt-Funktionen
Jedes Spiel enthält:
- `drawGradientBackground()` - Atmosphärischer Hintergrund
- `spawnParticles()` - Partikel-Explosionen
- `updateParticles()` - Partikel-Physik
- `drawTrail()` - Motion-Blur Effekte
- `lerpColor()` - Smooth Farbübergänge
- `dimColor()` - Glow-Effekte

## Technische Details

### Memory Usage
- **7 Modern Game-Instanzen:** ~25 KB
- **Partikel-Systeme:** ~2-3 KB pro Spiel
- **Trail-Buffer:** ~1 KB pro Spiel
- **Gesamt:** ~30-35 KB RAM

### Visuelle Features
- **Neon-Glow:** Multi-Layer Rendering
- **Partikel:** Physik-Simulation mit Gravitation
- **Trail:** Ring-Buffer für Bewegungsspur
- **Gradient:** Cached Background für Performance

## Installation

1. **CYD_Display_Config.h** vorhanden?
2. **CYD_Games Library** installiert?
3. **LovyanGFX** Library installiert?
4. **Genug Flash:** Modern-Versionen größer (~2x)
5. **Sketch hochladen**
6. **Genießen!**

## Performance-Tipps

### Wenn FPS zu niedrig:
- Reduziere Partikel-Count in Game-Headern
- Verkürze Trail-Length
- Deaktiviere Glow temporär

### Wenn Flackern auftritt:
- Normal bei vielen Partikeln
- Reduziere MAX_PARTICLES
- Erhöhe delay() in loop()

## Anpassungen

### Neon-Intensität ändern
```cpp
// In Game-Headern:
uint16_t glowColor = dimColor(baseColor, 0.8);  // statt 0.6 (heller)
```

### Mehr Partikel
```cpp
#define MAX_PARTICLES 50  // statt 20-40 (je nach Spiel)
```

### Andere Farbpalette
```cpp
// Pastellfarben statt Neon:
#define COLOR_GLOW_CYAN   0xAD55  // Hellblau
#define COLOR_GLOW_PINK   0xFBEA  // Rosa
```

## Troubleshooting

### Zu viel Flackern
- Normal bei Partikel-Systemen
- Reduziere Partikel-Count
- Nutze längere Trails für smootheren Look

### Farben zu grell
- Passe Neon-Farben an (weniger gesättigt)
- Nutze dimColor() mit niedrigeren Faktoren
- Ändere Gradient zu dunkleren Tönen

### Memory-Fehler beim Kompilieren
- Normal! Modern-Versionen sind größer
- Flash voll? Lösche andere Sketches
- RAM voll? Reduziere Partikel/Trail-Buffer

## Vergleich: Retro vs Modern

Bevorzugst du **klassisches Design**?
Siehe: `00_RetroGames/` - Alle Spiele im Retro-Look!

## Credits

- **Original Games:** Atari, Nintendo, Taito, u.a.
- **Retro Implementation:** CYD_Games Library
- **Modern Design:** CYD_Games Modern-Versionen
- **Platform:** ESP32-2432S028R (CYD)
- **Inspiration:** Tron, Blade Runner, Synthwave

## Viel Spaß mit den modernen Spielen!

```
    __  _______  ____  __________  _   __   ________    __  _____________
   /  |/  / __ \/ __ \/ ____/ __ \/ | / /  / ____/ /   / / / / ____/ ___/
  / /|_/ / / / / / / / __/ / /_/ /  |/ /  / / __/ /   / / / / __/  \__ \
 / /  / / /_/ / /_/ / /___/ _, _/ /|  /  / /_/ / /___/ /_/ / /___ ___/ /
/_/  /_/\____/_____/_____/_/ |_/_/ |_/   \____/_____/\____/_____//____/
```
