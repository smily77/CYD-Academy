# Space Invaders Modern - Mit Neon-Effekten

Klassischer Alien-Shooter mit modernem Cyberpunk-Design!

## Was ist neu?

### Visuelle Effekte
- **Neon-Glow:** Aliens und Spieler leuchten
- **Gradient-Hintergrund:** Dunkelblau zu Schwarz
- **Partikel-Explosionen:** Bei jedem Treffer
- **Animierte Aliens:** Pulsierender Glow
- **Trail-Effekte:** Bullet-Trails
- **3D-Schilde:** Gradient-Shading
- **UFO-Glow:** Leuchtende Bonustargets

### Gameplay
Identisch mit 15_SpaceInvaders:
- Gleiche Steuerung
- Gleiche Spielregeln
- Gleiche Punktevergabe
- Progressive Schwierigkeit

## Hardware

- **Display:** CYD 2.8" oder 3.5" (320x240 Landscape)
- **Buttons:** 4x Digital (tasteA, tasteB, tasteC, tasteD)

**Hinweis:** Alle Pins sind in `CYD_Display_Config.h` definiert.

## Steuerung

- **tasteB:** Links bewegen
- **tasteC:** Rechts bewegen
- **tasteA:** Schießen
- **tasteD:** Pause

## Features

- **Neon-Aliens:** Verschiedene Farben pro Typ
- **Partikel-System:** Explosionen bei Treffern
- **Bullet-Trails:** Motion-Blur für Schüsse
- **Gradient-Schilde:** 3D-Effekt
- **Animierter UFO:** Pulsierender Glow
- **Modern UI:** Neon-Score-Anzeige

## Design-Philosophie

### Retro vs Modern

| Feature | Retro | Modern |
|---------|-------|--------|
| Hintergrund | Schwarz | Gradient Blau→Schwarz |
| Aliens | Flach, Grün/Weiß | Neon-Glow, Farbcodiert |
| Explosionen | Keine | Partikel-System |
| Bullets | Einfach | Trail-Effekte |
| Schilde | Grün, flach | Gradient, 3D |
| UFO | Rot, flach | Pulsierender Neon-Glow |
| Performance | 60 FPS | 55-60 FPS |

## Code-Highlights

### Neue Funktionen
```cpp
void drawGradientBackground()        // Atmosphärischer Hintergrund
void drawModernAlien(...)            // Alien mit Glow
void drawModernShield(...)           // 3D-Schild
void spawnParticles(...)             // Explosionen
void updateParticles()               // Partikel-Physik
void drawBulletTrail(...)            // Motion-Blur
```

### Farbpalette
```cpp
#define COLOR_ALIEN_1  0xF81F  // Pink (oben)
#define COLOR_ALIEN_2  0x07FF  // Cyan (mitte)
#define COLOR_ALIEN_3  0x07E0  // Grün (unten)
#define COLOR_UFO      0xFFE0  // Gelb
#define COLOR_PLAYER   0x001F  // Blau
```

## Quick Start

```cpp
#include <CYD_Display_Config.h>
#include <SpaceInvadersGameModern.h>

LGFX lcd;
SpaceInvadersGameModern game;

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
- **Partikel-Limit:** 40 gleichzeitig
- **Selektives Rendering:** Nur geänderte Bereiche

## Anpassungen

### Mehr Partikel
```cpp
#define MAX_PARTICLES 60  // statt 40
spawnParticles(x, y, color, 20);  // statt 12
```

### Glow-Intensität
```cpp
uint16_t glowColor = dimColor(baseColor, 0.8);  // statt 0.6
```

### Trail-Länge
```cpp
#define TRAIL_LENGTH 8  // statt 5
```

## Troubleshooting

### FPS zu niedrig
- Reduziere MAX_PARTICLES
- Verkürze TRAIL_LENGTH
- Deaktiviere Glow temporär

### Zu viel Flackern
- Normal bei vielen Partikeln
- Reduziere Partikel-Count
- Erhöhe delay()

## Credits

- **Original:** Space Invaders (1978, Taito)
- **Retro-Version:** 15_SpaceInvaders
- **Modern Design:** 15b_SpaceInvaders_Modern

## Viel Spaß mit dem Neon-Shooter!
