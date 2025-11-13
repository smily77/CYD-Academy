# Asteroids Modern - Mit Spektakulären Effekten

Weltraum-Shooter mit Neon-Effekten, Partikel-Explosionen und Bullet-Trails!

## Was ist neu?

### Visuelle Effekte
- **Neon-Glow:** Raumschiff und Asteroids leuchten
- **Gradient-Weltraum:** Sternenfeld-Hintergrund
- **Partikel-Explosionen:** Spektakuläre Asteroid-Zerstörung
- **Bullet-Trails:** Motion-Blur für Schüsse
- **Schub-Effekt:** Flammen-Partikel beim Thrust
- **UFO-Glow:** Pulsierendes Leuchten
- **3D-Asteroids:** Schatten und Highlights

### Gameplay
Identisch mit 16_Asteroids:
- Gleiche Steuerung
- Gleiche Physik
- Gleiche Spielregeln
- Wrap-Around Mechanik

## Hardware

- **Display:** CYD 2.8" oder 3.5" (320x240 Landscape)
- **Buttons:** 4x Digital (tasteA, tasteB, tasteC, tasteD)

**Hinweis:** Alle Pins sind in `CYD_Display_Config.h` definiert.

## Steuerung

- **tasteB:** Links drehen
- **tasteC:** Rechts drehen
- **tasteA:** Schießen
- **tasteD:** Schub (Thrust) + Flammen-Effekt

## Features

- **Neon-Raumschiff:** Cyan-Glow mit Thrust-Flammen
- **3D-Asteroids:** Verschiedene Farben nach Größe
- **Partikel-System:** Explosionen in Asteroid-Farbe
- **Bullet-Trails:** Smooth Motion-Blur
- **Sternenfeld:** Animierter Hintergrund
- **UFO-Effekte:** Pulsierender Glow + Trail
- **Modern UI:** Neon-Score und Leben-Anzeige

## Design-Philosophie

### Retro vs Modern

| Feature | Retro | Modern |
|---------|-------|--------|
| Hintergrund | Schwarz | Gradient + Sterne |
| Raumschiff | Weiß, Vektor | Neon-Cyan + Glow |
| Thrust | Einfacher Strich | Flammen-Partikel |
| Asteroids | Grau, flach | Farbcodiert, 3D |
| Explosionen | Keine | Partikel-System (15 Partikel) |
| Bullets | Einfach | Trail-Effekte |
| UFO | Weiß | Pulsierender Glow |
| Performance | 60 FPS | 55-60 FPS |

## Code-Highlights

### Neue Funktionen
```cpp
void drawGradientBackground()        // Weltraum mit Sternen
void drawModernShip(...)             // Raumschiff mit Glow
void drawThrustFlame(...)            // Schub-Partikel
void drawModernAsteroid(...)         // 3D-Asteroid
void spawnParticles(...)             // Explosionen
void updateParticles()               // Partikel-Physik
void drawBulletTrail(...)            // Motion-Blur
```

### Farbpalette
```cpp
#define COLOR_SHIP         0x07FF  // Cyan
#define COLOR_ASTEROID_L   0xF800  // Rot (Groß)
#define COLOR_ASTEROID_M   0xFD20  // Orange (Mittel)
#define COLOR_ASTEROID_S   0xFFE0  // Gelb (Klein)
#define COLOR_UFO          0xF81F  // Pink
#define COLOR_BULLET       0x07E0  // Grün
```

## Quick Start

```cpp
#include <CYD_Display_Config.h>
#include <AsteroidsGameModern.h>

LGFX lcd;
AsteroidsGameModern game;

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
- **Partikel-Limit:** 50 gleichzeitig
- **Thrust-Partikel:** 5 gleichzeitig
- **Bullet-Trail:** 8 Punkte pro Bullet

## Effekte im Detail

### Partikel-Explosionen
- **15 Partikel** pro Asteroid
- **Farbe:** Entspricht Asteroid-Größe
- **Physik:** Gravitation + Reibung
- **Lifetime:** 20-30 Frames

### Thrust-Flammen
- **5 Flammen-Partikel** beim Schub
- **Orange/Gelb** Gradient
- **Richtungsabhängig:** Entgegengesetzt zur Bewegung
- **Fade-Out:** Über Lebensdauer

### Bullet-Trails
- **8 Punkte** pro Schuss
- **Fade-Out:** Älter = transparenter
- **Grüner Neon-Glow**
- **Smooth Motion-Blur**

## Anpassungen

### Mehr Partikel
```cpp
#define MAX_PARTICLES 80  // statt 50
spawnParticles(x, y, color, 25);  // statt 15
```

### Längere Trails
```cpp
#define TRAIL_LENGTH 12  // statt 8
```

### Andere Asteroid-Farben
```cpp
#define COLOR_ASTEROID_L  0x001F  // Blau statt Rot
#define COLOR_ASTEROID_M  0x07FF  // Cyan statt Orange
```

### Intensivere Thrust-Flammen
```cpp
// In drawThrustFlame():
for (int i = 0; i < 8; i++)  // statt 5
```

## Troubleshooting

### FPS zu niedrig
- Reduziere MAX_PARTICLES (50 → 30)
- Deaktiviere Sternenfeld
- Verkürze Bullet-Trails

### Zu viel Flackern
- Normal bei vielen Explosionen
- Reduziere Partikel pro Explosion
- Erhöhe delay()

### Thrust-Flammen nicht sichtbar
- Vergrößere Partikel-Größe
- Ändere Farbe zu heller
- Erhöhe Partikel-Count

## Credits

- **Original:** Asteroids (1979, Atari)
- **Retro-Version:** 16_Asteroids
- **Modern Design:** 16b_Asteroids_Modern

## Genieße den Weltraum in Neon!
