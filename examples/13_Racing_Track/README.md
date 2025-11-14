# 13_Racing_Track - Pfad-basierte Animation

## 🎓 Was lernst du hier?

Dieses Beispiel zeigt **Pfad-basierte Animation** - ein Auto folgt einer geschwungenen Rennstrecke mit realistischer Bewegung und Rotation.

### Lernziele

- ✅ Komplexe Pfade zeichnen (Bézierkurven)
- ✅ Pfad-Interpolation (Bewegung entlang eines Pfades)
- ✅ Sprite-Rotation berechnen (Tangenten)
- ✅ Frame-Rate unabhängige Bewegung
- ✅ Geschwindigkeits-Physik (Beschleunigung, Reibung)
- ✅ Zeit-Messung und Performance-Tracking

## 🎮 Was macht das Beispiel?

### Features

1. **Geschwungene Rennstrecke** - 8-förmiger Kurs von oben
2. **Auto-Animation** - Folgt dem Pfad automatisch
3. **Touch-Steuerung** - Beschleunigen & Bremsen
4. **Realistische Physik** - Reibung, Geschwindigkeits-Limits
5. **Rundenzeiten** - Misst jede Runde + beste Zeit
6. **Sprite-Rotation** - Auto dreht sich mit der Kurve

### Steuerung

```
╔════════════════════════════════╗
║  Touch OBEN    = Beschleunigen ║
║  Touch UNTEN   = Bremsen       ║
║  Kein Touch    = Ausrollen     ║
╚════════════════════════════════╝
```

## 📦 Hardware

**Benötigt:**
- 1x CYD Display (2.8" oder 3.5")
- Nur Touch-Input!

## 🎨 Die Rennstrecke

```
        🌳
    ╔═══════╗
 🌳 ║ ░░░░░ ║ 🌳
    ║░ ─ ─ ░║
    ║░  🏎  ░║
    ║ ░░░░░ ║
    ╚═══╦═══╝
        ║
    ╔═══╩═══╗
 🌳 ║ ░░░░░ ║ 🌳
    ║░ ─ ─ ░║
    ╚═══════╝
        🌳
```

**Elemente:**
- 🟢 Grünes Gras (Hintergrund)
- ⬛ Dunkler Asphalt (Äußere Straße)
- ⬜ Heller Asphalt (Innere Straße)
- - - Weiße gestrichelte Mittellinie
- 🌳 Bäume als Dekoration
- 🏎 Rotes Auto mit Richtungs-Indikator

## 🔍 Code-Analyse: Wichtige Konzepte

### 1. Pfad-Generierung

```cpp
void generateTrackPath() {
  for (int i = 0; i < PATH_POINTS; i++) {
    float t = (float)i / PATH_POINTS;  // 0.0 bis 1.0
    float angle = t * 2 * PI;

    // 8-Form durch Sinus-Kombination
    float x = 120 + sin(angle) * 80;
    float y = 160 + sin(angle * 2) * 120;

    trackPath[i] = {x, y};
  }
}
```

**Was passiert?**
- `t` läuft von 0.0 bis 1.0 (eine Runde)
- `sin(angle)` = horizontale Bewegung (links-rechts)
- `sin(angle * 2)` = vertikale Bewegung mit **doppelter Frequenz**
- Resultat: Eine 8-förmige Kurve!

**Warum funktioniert das?**
- 1x Frequenz horizontal = 1 Schwingung
- 2x Frequenz vertikal = 2 Schwingungen
- Kombination = Lissajous-Kurve (8-Form)

### 2. Pfad-Interpolation

```cpp
Point getPointOnPath(float t) {
  float index = t * PATH_POINTS;
  int i0 = (int)index;
  int i1 = (i0 + 1) % PATH_POINTS;
  float frac = index - i0;

  // Lineare Interpolation zwischen zwei Punkten
  Point p;
  p.x = trackPath[i0].x + (trackPath[i1].x - trackPath[i0].x) * frac;
  p.y = trackPath[i0].y + (trackPath[i1].y - trackPath[i0].y) * frac;

  return p;
}
```

**Schritt für Schritt:**
1. Konvertiere `t` (0-1) zu Array-Index
2. Finde zwei benachbarte Punkte: `i0` und `i1`
3. Berechne Zwischen-Wert: `frac` (0-1)
4. **Lineare Interpolation** (Lerp) zwischen Punkten

**Warum wichtig?**
- Smooth Bewegung zwischen diskreten Punkten
- Funktioniert für beliebige Geschwindigkeiten
- Fundamentales Konzept in Animationen!

### 3. Tangenten-Berechnung (Rotation)

```cpp
float getAngle(float t) {
  Point p1 = getPointOnPath(t);
  Point p2 = getPointOnPath(t + 0.01);  // Leicht voraus

  return atan2(p2.y - p1.y, p2.x - p1.x);
}
```

**Mathematik:**
- Tangente = Richtungsvektor am Punkt
- Berechne durch "Ein Stück voraus schauen"
- `atan2()` = Winkel aus X/Y-Differenz
- Ergebnis: Auto zeigt immer in Fahrtrichtung!

### 4. Sprite-Rotation

```cpp
// Lokale Auto-Koordinaten (vor Rotation)
float corners[4][2] = {
  {-7, -4},  // Hinten Links
  {7, -4},   // Vorne Links
  {7, 4},    // Vorne Rechts
  {-7, 4}    // Hinten Rechts
};

// Rotieren
for (int i = 0; i < 4; i++) {
  float rx = corners[i][0] * cos(angle) - corners[i][1] * sin(angle);
  float ry = corners[i][0] * sin(angle) + corners[i][1] * cos(angle);
  // ... + Position
}
```

**Rotations-Matrix:**
```
x' = x * cos(θ) - y * sin(θ)
y' = x * sin(θ) + y * cos(θ)
```

**Warum so?**
- Standard 2D Rotations-Formel
- `cos()` und `sin()` drehen Punkt um Ursprung
- Erst rotieren, dann verschieben!

### 5. Geschwindigkeits-Physik

```cpp
// In loop():
carSpeed -= FRICTION;           // Reibung immer aktiv

// Bei Touch:
if (top_touch) {
  carSpeed += ACCELERATION;     // Gas
  carSpeed = min(carSpeed, MAX_SPEED);
}

if (bottom_touch) {
  carSpeed -= BRAKE;            // Bremse
  carSpeed = max(carSpeed, 0);
}

// Position aktualisieren
carPosition += carSpeed;
```

**Realistische Bewegung:**
- **Reibung** = Auto wird langsamer ohne Gas
- **Beschleunigung** = Geschwindigkeit erhöht sich graduell
- **Max Speed** = Verhindert unrealistisch schnelles Fahren
- **Brake** = Stärker als Reibung

## 🎯 Experimente & Erweiterungen

### Experiment 1: Verschiedene Strecken

**Challenge:** Ändere die Strecken-Form!

**Beispiel - Kreis:**
```cpp
float x = 120 + cos(angle) * 80;
float y = 160 + sin(angle) * 80;
```

**Beispiel - Spirale:**
```cpp
float radius = 50 + t * 50;
float x = 120 + cos(angle) * radius;
float y = 160 + sin(angle) * radius;
```

**Beispiel - Rechteck-Strecke:**
```cpp
if (t < 0.25) {
  x = 40 + t * 4 * 160;  // Oben
  y = 40;
} else if (t < 0.5) {
  x = 200;               // Rechts
  y = 40 + (t - 0.25) * 4 * 240;
}
// ... etc
```

### Experiment 2: Mehrere Autos (Gegner)

**Idee:** Füge KI-Gegner hinzu!

```cpp
struct Car {
  float position;
  float speed;
  uint32_t color;
};

Car cars[3] = {
  {0.0, 0.008, 0xF800},   // Spieler (rot)
  {0.3, 0.006, 0x001F},   // Gegner 1 (blau)
  {0.6, 0.007, 0x07E0}    // Gegner 2 (grün)
};
```

### Experiment 3: Kollisions-Erkennung

**Challenge:** Erkenne wenn Auto von der Strecke abkommt!

```cpp
Point pos = getPointOnPath(carPosition);
float distanceFromCenter = ... // Berechne Abstand

if (distanceFromCenter > TRACK_WIDTH/2) {
  // Auto ist off-track!
  carSpeed *= 0.5;  // Langsamer im Gras
}
```

### Experiment 4: Power-Ups

**Idee:** Items auf der Strecke zum Einsammeln!

```cpp
struct PowerUp {
  float position;  // 0-1 auf Pfad
  int type;        // 0=Speed, 1=Shield, etc.
  bool collected;
};

// Im Loop:
if (distance(car, powerup) < 10 && !powerup.collected) {
  applyPowerUp(powerup.type);
  powerup.collected = true;
}
```

**Types:**
- 🚀 Speed Boost (temporär schneller)
- 🛡️ Shield (nicht abbremsen im Gras)
- ⏱️ Time Slow (Gegner langsamer)

### Experiment 5: Partikel-Effekte

**Idee:** Rauch/Staub hinter Auto!

```cpp
struct Particle {
  float x, y;
  float vx, vy;
  int lifetime;
};

Particle particles[20];

// Beim Fahren:
if (carSpeed > 0.005) {
  // Erzeuge Partikel hinter Auto
  Point pos = getPointOnPath(carPosition - 0.01);
  addParticle(pos.x, pos.y);
}
```

### Experiment 6: Besseres Auto-Sprite

**Aktuell:** Einfaches Rechteck

**Besser:** Detailliertes Auto!

```cpp
void drawDetailedCar(Point pos, float angle) {
  // Karosserie
  drawRotatedRect(pos, angle, 14, 8, 0xF800);

  // Fenster
  drawRotatedRect(pos + offset, angle, 6, 6, 0x39C7);

  // Räder (4 kleine Kreise)
  drawWheels(pos, angle);

  // Scheinwerfer (vorne gelb)
  drawLights(pos, angle);
}
```

## 📊 Performance-Optimierung

### Aktuelles Setup

```cpp
delay(16);  // ~60 FPS
```

**Problem:** Fest codierte Frame-Rate!

**Besser:** Delta-Time basiert

```cpp
void loop() {
  static unsigned long lastFrame = 0;
  unsigned long now = millis();
  float deltaTime = (now - lastFrame) / 1000.0;
  lastFrame = now;

  // Geschwindigkeit mit deltaTime
  carPosition += carSpeed * deltaTime * 60;  // Normalisiert auf 60 FPS

  // Kein delay() nötig!
}
```

**Vorteil:**
- Läuft gleich schnell auf jedem Display
- Smooth auch bei unterschiedlicher Render-Time
- Professioneller Ansatz!

### Rendering-Optimierung

**Problem:** Ganzer Screen wird jedes Frame neu gezeichnet

**Lösung:** Dirty-Rectangle / Double-Buffering

```cpp
// Nur Auto-Bereich neu zeichnen
void updateCar(Point oldPos, Point newPos) {
  // Lösche altes Auto
  drawTrackSection(oldPos, CLEAR);

  // Zeichne neues Auto
  drawCar(newPos);
}
```

## 🐛 Troubleshooting

### Problem: Auto "springt"

**Symptom:** Ungleichmäßige Bewegung

**Ursachen:**
1. Zu wenige Pfadpunkte (`PATH_POINTS` erhöhen)
2. Touch-Input blockiert Rendering
3. Zu große `carSpeed` Schritte

**Lösung:**
```cpp
#define PATH_POINTS 128  // Mehr Punkte = smoother
```

### Problem: Auto zeigt in falsche Richtung

**Symptom:** Auto fährt "rückwärts" oder seitwärts

**Ursache:** Falsche Tangenten-Berechnung

**Debug:**
```cpp
Serial.printf("Angle: %.2f deg\n", getAngle(carPosition) * 180 / PI);
```

### Problem: Langsame Performance

**Symptom:** Ruckelt, <60 FPS

**Ursachen:**
1. Zu viele `fillCircle()` Calls
2. Zu detaillierte Strecke
3. Serial-Output im Loop

**Lösung:**
```cpp
// Serial nur alle 1s
static unsigned long lastDebug = 0;
if (millis() - lastDebug > 1000) {
  Serial.println("...");
  lastDebug = millis();
}
```

## 💡 Mathe-Konzepte erklärt

### Lineare Interpolation (Lerp)

```
value = start + (end - start) * t

Beispiel:
start = 10
end = 20
t = 0.5

value = 10 + (20 - 10) * 0.5 = 15
```

**Verwendung:**
- Smooth Bewegung
- Farb-Übergänge
- Animation allgemein

### Lissajous-Kurven

```
x(t) = A * sin(a*t + δ)
y(t) = B * sin(b*t)

a/b = Frequenz-Verhältnis
```

**Formen:**
- 1:1 = Kreis/Ellipse
- 1:2 = 8-Form (unser Beispiel!)
- 2:3 = Komplexe Kurven

### Vektoren & Tangenten

```
Tangente am Punkt P = (P₂ - P₁) / |P₂ - P₁|

Winkel = atan2(dy, dx)
```

**Wichtig:**
- Tangente = Richtung der Kurve
- Normalisieren für Einheitsvektor
- `atan2()` besser als `atan()` (4-Quadranten!)

## 🎓 Was hast du gelernt?

Nach diesem Beispiel verstehst du:

1. ✅ **Pfad-basierte Animation** - Objekte folgen komplexen Kurven
2. ✅ **Mathematische Kurven** - Sinus, Lissajous-Kurven
3. ✅ **Interpolation** - Smooth Bewegung zwischen Punkten
4. ✅ **Rotation** - Matrix-Transformationen
5. ✅ **Physik-Simulation** - Geschwindigkeit, Reibung, Beschleunigung
6. ✅ **Performance** - FPS, Delta-Time, Optimierung

## 🚀 Nächste Schritte

### Level-Up Projekte

1. **Multiplayer-Rennen** - 2 CYDs via ESP-NOW
2. **Strecken-Editor** - Touch zum Zeichnen eigener Strecken
3. **Realistic Racing** - Drift, Kollisionen, Schaden
4. **Top-Down RPG** - Charakter folgt Pfad
5. **Tower Defense** - Gegner folgen Pfad zum Ziel

### Verwandte Beispiele

- `06_BouncingBall` - Einfachere Animation
- `11_ScrollingChart` - Pfad-Zeichnung
- `12_CYD_to_CYD` - Multi-Device für Multiplayer

## 📚 Weiterführende Ressourcen

- [Bézierkurven erklärt](https://pomax.github.io/bezierinfo/)
- [Lissajous Curves](https://en.wikipedia.org/wiki/Lissajous_curve)
- [Game Loop Design](https://gameprogrammingpatterns.com/game-loop.html)
- [Sprite Rotation](https://www.geeksforgeeks.org/rotation-matrix-in-2d/)

## ✨ Fazit

Dieses Beispiel zeigt **professionelle Animations-Techniken**:
- Mathematisch definierte Pfade
- Smooth Interpolation
- Realistische Physik
- Performance-Bewusstsein

Du kannst diese Konzepte für unzählige Projekte nutzen:
- Spiele (Racing, Tower Defense, RPG)
- Visualisierungen (Daten folgen Pfaden)
- UI-Animationen (Smooth Transitions)
- Robotik (Pfad-Planung)

**Viel Spaß beim Experimentieren! 🏎️💨**
