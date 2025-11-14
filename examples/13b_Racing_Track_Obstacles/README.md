# 🚗💥 Beispiel 13b: Racing Track mit Hindernissen & Kollisionserkennung

## 📝 Was macht dieses Beispiel?

Erweitert das [13_Racing_Track](../13_Racing_Track/) Beispiel um:

- **Zufällige Hindernisse** auf der Rennstrecke (Ölfässer, Verkehrskegel, Steine)
- **Kollisionserkennung** zwischen Auto und Hindernissen
- **Score-System:** Punkte für ausgewichene Hindernisse
- **Crash-System:** Geschwindigkeits-Verlust und visuelles Feedback bei Kollision
- **Game Mechanics:** Risiko vs. Belohnung (schnell fahren = mehr Crashes, aber besser ausweichen = mehr Punkte)

**Steuerung:**
- Touch oben: Gas geben
- Touch unten: Bremsen
- **Ziel:** Hindernissen ausweichen für Punkte sammeln!

---

## 🎯 Lernziele: Kollisionserkennung

Dieses Beispiel fokussiert sich auf **Collision Detection** - eine fundamentale Technik in Game Development, Robotik und UI-Design.

### Was lernst du?

1. **Distanz-basierte Kollisionserkennung** (Circle-Circle Collision)
2. **Objekt-Management** mit Structs und Arrays
3. **Game State Management** (Score, Crashes, Flags)
4. **Visual Feedback** für bessere User Experience
5. **Performance-Trade-offs** bei Collision Detection

---

## 🔍 Kollisionserkennung - Die Theorie

### 1. Was ist Kollisionserkennung?

Collision Detection prüft, ob zwei Objekte sich **überlappen** oder **berühren**.

**Anwendungen:**
- **Games:** Auto crasht in Wand, Projektil trifft Gegner
- **UI:** Finger berührt Button
- **Robotik:** Robot erkennt Hindernisse
- **Simulation:** Physik-Engines (Unity, Unreal)

### 2. Verschiedene Methoden

#### A) Punkt-zu-Punkt Distanz (Circle Collision)

**Was wir in diesem Beispiel verwenden!**

```
Distanz = √((x2-x1)² + (y2-y1)²)

Wenn Distanz < (Radius1 + Radius2) → KOLLISION!
```

**Vorteile:**
- ✅ Einfach zu implementieren
- ✅ Schnell zu berechnen
- ✅ Funktioniert gut für runde Objekte

**Nachteile:**
- ❌ Ungenau für rechteckige Objekte
- ❌ "Konservativ" (meldet manchmal Kollision, wo keine ist)

#### B) Bounding Box Collision (AABB - Axis-Aligned Bounding Box)

```cpp
bool checkAABB(Rect a, Rect b) {
  return (a.x < b.x + b.width &&
          a.x + a.width > b.x &&
          a.y < b.y + b.height &&
          a.y + a.height > b.y);
}
```

**Vorteile:**
- ✅ Sehr schnell (nur 4 Vergleiche)
- ✅ Gut für rechteckige Objekte

**Nachteile:**
- ❌ Nur für achsenausgerichtete Rechtecke
- ❌ Keine Rotation

#### C) Oriented Bounding Box (OBB)

Rechtecke mit Rotation - benötigt **Separating Axis Theorem (SAT)**.

**Komplexer, aber genauer!**

#### D) Pixel-Perfect Collision

Prüft tatsächliche Pixel-Überlappung.

**Vorteile:**
- ✅ 100% genau

**Nachteile:**
- ❌ Sehr langsam
- ❌ Nicht für Echtzeit-Games geeignet

---

## 💻 Code-Analyse

### Hindernis-Struktur

```cpp
struct Obstacle {
  float pathPosition;     // Position auf Pfad (0.0 - 1.0)
  ObstacleType type;      // BARREL, CONE, ROCK
  float radius;           // Kollisions-Radius in Pixel
  bool avoided;           // Score bereits vergeben?
  bool hitThisLap;        // Bereits gecrasht diese Runde?
};
```

**Warum diese Felder?**

- `pathPosition`: Hindernisse bewegen sich nicht, bleiben am Pfad fixiert
- `radius`: Verschiedene Objekte = verschiedene Größen
- `avoided`: Verhindert doppelte Score-Vergabe
- `hitThisLap`: Verhindert mehrfache Crashes am selben Objekt

### Kollisions-Check

```cpp
void checkCollisions() {
  Point carPos = getPointOnPath(carPosition);

  for (int i = 0; i < MAX_OBSTACLES; i++) {
    Obstacle &obs = obstacles[i];
    Point obsPos = getPointOnPath(obs.pathPosition);

    // Euklidische Distanz berechnen
    float dist = getDistance(carPos, obsPos);

    // KOLLISION: Direkter Treffer
    if (dist < COLLISION_DISTANCE && !obs.hitThisLap) {
      handleCrash(i);
    }
    // SCORE: Knapp ausgewichen
    else if (dist < AVOID_DISTANCE && !obs.avoided && !obs.hitThisLap) {
      handleAvoid(i);
    }
  }
}
```

**Was passiert hier?**

1. **Auto-Position holen:** `getPointOnPath(carPosition)`
2. **Für jedes Hindernis:**
   - Position berechnen
   - Distanz messen
   - Zwei Zonen prüfen:
     - **COLLISION_DISTANCE (12px)**: Direkter Crash
     - **AVOID_DISTANCE (18px)**: Knappes Ausweichen (Score!)

**Warum zwei Distanzen?**

- **Game Design:** Belohne riskantes Fahren!
- Spieler können nah vorbeifahren für Punkte
- Zu nah = Crash (Risiko vs. Reward)

### Distanz-Berechnung

```cpp
float getDistance(Point p1, Point p2) {
  float dx = p2.x - p1.x;
  float dy = p2.y - p1.y;
  return sqrt(dx * dx + dy * dy);  // Pythagoras!
}
```

**Mathematik:** Satz des Pythagoras

```
     c (Hypotenuse = Distanz)
    /|
 dy | |
    |/
    ----
     dx

c² = dx² + dy²
c = √(dx² + dy²)
```

### Performance-Optimierung: Squared Distance

**Problem:** `sqrt()` ist **langsam** (50-100 CPU-Zyklen).

**Lösung:** Vergleiche **quadrierte Distanzen**!

```cpp
float getDistanceSquared(Point p1, Point p2) {
  float dx = p2.x - p1.x;
  float dy = p2.y - p1.y;
  return dx * dx + dy * dy;  // Kein sqrt()!
}

// In checkCollisions():
float distSq = getDistanceSquared(carPos, obsPos);
if (distSq < COLLISION_DISTANCE * COLLISION_DISTANCE) {
  handleCrash(i);
}
```

**Resultat:** 2-3x schneller!

---

## 🎮 Game Design: Crash vs. Avoid

### Crash-Mechanik

```cpp
void handleCrash(int obsIndex) {
  obstacles[obsIndex].hitThisLap = true;  // Flag setzen
  crashes++;

  // Geschwindigkeit um 50% reduzieren
  carSpeed *= CRASH_SPEED_REDUCTION;

  // Visuelles Feedback (roter Flash)
  crashFlashTime = millis();

  Serial.printf("CRASH! Hindernis #%d\n", obsIndex);
}
```

**Was passiert?**
1. ❌ **Penalty:** Geschwindigkeit reduziert (macht Spieler langsamer)
2. 🔴 **Visual Feedback:** Roter Flash oben im Display (200ms)
3. 📊 **Statistik:** Crash-Counter erhöht
4. 🚫 **Flag:** `hitThisLap` verhindert doppelte Crashes

### Avoid-Mechanik (Score)

```cpp
void handleAvoid(int obsIndex) {
  obstacles[obsIndex].avoided = true;  // Flag setzen
  score++;

  Serial.printf("Ausgewichen! Score: %d\n", score);
}
```

**Was passiert?**
1. ✅ **Belohnung:** Score +1
2. 🚫 **Flag:** `avoided` verhindert doppelte Score-Vergabe
3. 🎯 **Motiviert:** Spieler sollen nah vorbeifahren

### Warum Flags?

**Problem ohne Flags:**

```
Frame 1: dist = 17px → Score +1
Frame 2: dist = 16px → Score +1  (NOCH IMMER IN DER ZONE!)
Frame 3: dist = 15px → Score +1
...
```

Spieler bekommt **zu viele Punkte** für ein Hindernis!

**Lösung mit Flags:**

```
Frame 1: dist = 17px → avoided=false → Score +1, avoided=true
Frame 2: dist = 16px → avoided=true  → SKIP (bereits ausgewichen)
Frame 3: dist = 15px → avoided=true  → SKIP
```

**Flags werden zurückgesetzt** am Ende jeder Runde in `resetObstacleFlags()`.

---

## 🎨 Visuelle Unterscheidung der Hindernisse

### Drei Typen

```cpp
enum ObstacleType {
  OBS_BARREL,   // Ölfass (Orange)
  OBS_CONE,     // Verkehrskegel (Gelb)
  OBS_ROCK      // Stein (Grau)
};
```

**Warum verschiedene Typen?**
- 🎨 **Visuell interessanter** als nur Kreise
- 🎮 **Erweiterbarkeit:** Verschiedene Typen könnten verschiedene Effekte haben
  - Ölfass: Macht Straße rutschig
  - Kegel: Nur langsam, kein Crash
  - Stein: Blockiert komplett

### Zeichnen der Hindernisse

```cpp
case OBS_BARREL:
  // Ölfass (Orange mit schwarzem Ring)
  lcd.fillCircle(pos.x, pos.y, radius, COLOR_BARREL);
  lcd.drawCircle(pos.x, pos.y, radius - 2, 0x0000);
  break;

case OBS_CONE:
  // Verkehrskegel (Gelb mit rotem Ring)
  lcd.fillCircle(pos.x, pos.y, radius, COLOR_CONE);
  lcd.fillTriangle(pos.x, pos.y - 5, pos.x - 4, pos.y + 4,
                   pos.x + 4, pos.y + 4, COLOR_CONE);
  lcd.drawCircle(pos.x, pos.y, radius - 1, 0xF800);
  break;

case OBS_ROCK:
  // Stein (Grau mit Schattierung)
  lcd.fillCircle(pos.x, pos.y, radius, COLOR_ROCK);
  lcd.fillCircle(pos.x - 2, pos.y - 2, 3, 0x5AEB);  // Dunkel
  lcd.fillCircle(pos.x + 2, pos.y + 1, 2, 0x9CF3);  // Hell
  break;
```

**Design-Tricks:**
- Mehrere Kreise überlagern für 3D-Effekt
- Kleine Details (Ringe, Dreiecke) für Erkennbarkeit
- Farbkontrast zur Straße (Orange/Gelb/Grau auf Grau-Asphalt)

---

## 🚀 Experimente & Erweiterungen

### 1. 🎯 Verschiedene Hindernistypen mit Effekten

**Aktuell:** Alle Hindernisse haben gleichen Crash-Effekt.

**Erweitere:**

```cpp
void handleCrash(int obsIndex) {
  Obstacle &obs = obstacles[obsIndex];
  obs.hitThisLap = true;
  crashes++;

  switch (obs.type) {
    case OBS_BARREL:
      // Öl auf Straße: Reduziere Reibung!
      carSpeed *= 0.3;  // Starker Crash
      // TODO: Rutschige Straße für 2 Sekunden
      break;

    case OBS_CONE:
      // Kegel umfahren: Leichter Crash
      carSpeed *= 0.8;
      break;

    case OBS_ROCK:
      // Stein: VOLLSTÄNDIGER STOPP
      carSpeed = 0;
      break;
  }

  crashFlashTime = millis();
}
```

**Lernziel:** Verschiedene Game Mechanics für Spieltiefe.

---

### 2. 📦 Bounding Box Collision

**Aktuell:** Circle-Circle (ungenau für rechteckiges Auto).

**Erweitere auf AABB:**

```cpp
struct AABB {
  float x, y;        // Top-left corner
  float width, height;
};

AABB getCarAABB() {
  Point pos = getPointOnPath(carPosition);
  float carWidth = 8;
  float carLength = 14;

  // TODO: Rotation berücksichtigen für OBB!
  return {pos.x - carLength/2, pos.y - carWidth/2, carLength, carWidth};
}

bool checkAABBvsCircle(AABB box, Point center, float radius) {
  // Nächster Punkt im Rechteck zum Kreis-Zentrum finden
  float closestX = constrain(center.x, box.x, box.x + box.width);
  float closestY = constrain(center.y, box.y, box.y + box.height);

  // Distanz von diesem Punkt zum Kreis-Zentrum
  float dx = center.x - closestX;
  float dy = center.y - closestY;

  return (dx * dx + dy * dy) < (radius * radius);
}
```

**Lernziel:** Präzisere Kollisionserkennung, hybride Shapes.

---

### 3. 🌟 Power-Ups statt nur Hindernisse

**Idee:** Manche Objekte sind **gut**!

```cpp
enum ItemType {
  ITEM_OBSTACLE,  // Hindernis (ausweichen)
  ITEM_POWERUP    // Power-Up (einsammeln!)
};

struct Item {
  float pathPosition;
  ItemType itemType;
  PowerUpType powerType;  // SPEED_BOOST, SHIELD, SCORE_MULTIPLIER
  // ...
};

void checkCollisions() {
  // ...
  if (item.itemType == ITEM_POWERUP) {
    collectPowerUp(i);
  } else {
    // Normale Crash-Logik
  }
}

void collectPowerUp(int index) {
  switch (items[index].powerType) {
    case SPEED_BOOST:
      carSpeed *= 1.5;  // 50% schneller für 3 Sekunden
      powerUpEndTime = millis() + 3000;
      break;

    case SHIELD:
      hasShield = true;  // Nächster Crash = kein Schaden
      break;

    case SCORE_MULTIPLIER:
      scoreMultiplier = 2;  // Doppelte Punkte für 10 Sekunden
      break;
  }
}
```

**Lernziel:** Positive Kollisionen, Game Balancing.

---

### 4. 🔄 Spatial Partitioning für Performance

**Problem:** Bei 100+ Hindernissen wird `checkCollisions()` langsam.

**Aktuell:** O(n) - Jedes Hindernis wird geprüft.

**Optimierung:** Nur Hindernisse in der Nähe prüfen!

#### Grid-basierter Ansatz

```cpp
// Teile Strecke in Segmente
const int GRID_CELLS = 8;
Obstacle* grid[GRID_CELLS][10];  // 8 Segmente, max 10 pro Segment

void buildGrid() {
  // Leere Grid
  memset(grid, 0, sizeof(grid));

  // Hindernisse in Grid einsortieren
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    int cell = (int)(obstacles[i].pathPosition * GRID_CELLS);
    // ... füge zu grid[cell] hinzu
  }
}

void checkCollisions() {
  // Auto-Position → Grid-Zelle
  int carCell = (int)(carPosition * GRID_CELLS);

  // Prüfe nur aktuelle + benachbarte Zellen
  for (int cell = carCell - 1; cell <= carCell + 1; cell++) {
    int wrappedCell = (cell + GRID_CELLS) % GRID_CELLS;

    // Prüfe nur Hindernisse in dieser Zelle
    for (Obstacle* obs : grid[wrappedCell]) {
      if (obs != nullptr) {
        // Collision Check...
      }
    }
  }
}
```

**Resultat:** O(n) → O(1) amortisiert!

**Lernziel:** Spatial Data Structures (Grid, Quadtree, BSP).

---

### 5. 🎯 Collision Layers & Bitmasks

**Problem:** Was wenn Auto mit anderen Autos kollidieren soll, aber nicht mit Power-Ups anderer Spieler?

**Lösung:** Collision Layers (wie Unity!)

```cpp
enum CollisionLayer {
  LAYER_PLAYER = 1 << 0,      // 0001
  LAYER_OBSTACLE = 1 << 1,    // 0010
  LAYER_POWERUP = 1 << 2,     // 0100
  LAYER_OTHER_PLAYER = 1 << 3 // 1000
};

struct GameObject {
  uint8_t layer;           // Welchem Layer gehöre ich?
  uint8_t collidesWith;    // Mit welchen Layern kollidiere ich?
};

// Player kollidiert mit Hindernissen und anderen Playern
player.layer = LAYER_PLAYER;
player.collidesWith = LAYER_OBSTACLE | LAYER_OTHER_PLAYER;

// Collision Check
if (objA.layer & objB.collidesWith && objB.layer & objA.collidesWith) {
  // Kollision!
}
```

**Lernziel:** Bitmasks, Layer-basierte Physik (Unity, Godot).

---

### 6. 🧪 Debug-Visualisierung

**Problem:** Schwer zu sehen, warum Kollision auftritt oder nicht.

**Lösung:** Zeichne Kollisions-Kreise!

```cpp
void drawObstacles() {
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    Point pos = getPointOnPath(obstacles[i].pathPosition);

    // Normales Hindernis zeichnen
    // ...

    // DEBUG: Zeige Kollisions-Zonen
    #ifdef DEBUG_COLLISION
      // Crash-Zone (rot)
      lcd.drawCircle(pos.x, pos.y, COLLISION_DISTANCE, 0xF800);

      // Avoid-Zone (grün)
      lcd.drawCircle(pos.x, pos.y, AVOID_DISTANCE, 0x07E0);
    #endif
  }
}

void drawCar() {
  // ...

  // DEBUG: Zeige Auto-Kollisions-Kreis
  #ifdef DEBUG_COLLISION
    Point pos = getPointOnPath(carPosition);
    lcd.drawCircle(pos.x, pos.y, 7, 0x001F);  // Blau
  #endif
}
```

**Aktivieren:**

```cpp
// Am Anfang der Datei
#define DEBUG_COLLISION
```

**Lernziel:** Visual Debugging, Conditional Compilation.

---

## 🐛 Troubleshooting

### Problem: "Auto crasht, obwohl es nicht berührt"

**Ursache:** Kollisions-Radius zu groß.

**Lösung:**

```cpp
// Reduziere COLLISION_DISTANCE
const float COLLISION_DISTANCE = 10.0;  // War 12.0

// Oder reduziere Hindernis-Radien
obstacles[i].radius = 4.0;  // War 6.0 für Barrel
```

---

### Problem: "Kein Score, obwohl ich ausweiche"

**Ursache:** `AVOID_DISTANCE` zu klein oder Auto zu schnell.

**Debug:**

```cpp
void checkCollisions() {
  // ...
  float dist = getDistance(carPos, obsPos);

  // DEBUG-Ausgabe
  if (dist < 30) {  // Wenn nah am Hindernis
    Serial.printf("Hindernis #%d: dist=%.1f, avoided=%d\n",
                  i, dist, obs.avoided);
  }

  // ...
}
```

**Lösung:** Vergrößere `AVOID_DISTANCE` oder reduziere `MAX_SPEED`.

---

### Problem: "Mehrere Punkte für ein Hindernis"

**Ursache:** `avoided` Flag wird nicht gesetzt.

**Überprüfe:**

```cpp
void handleAvoid(int obsIndex) {
  obstacles[obsIndex].avoided = true;  // ← Diese Zeile vorhanden?
  score++;
}
```

---

### Problem: "Nach Runde sind Hindernisse weg"

**Ursache:** `resetObstacleFlags()` löscht Hindernisse.

**Überprüfe:**

```cpp
void resetObstacleFlags() {
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    obstacles[i].avoided = false;      // ← Nur Flags!
    obstacles[i].hitThisLap = false;
    // NICHT: obstacles[i].pathPosition = 0;  // ← Das würde sie löschen!
  }
}
```

---

## 📚 Weiterführende Konzepte

### 1. Broad Phase vs. Narrow Phase

**Game Engines nutzen zwei Phasen:**

#### Broad Phase (Grobe Suche)
- Schnelle, ungenaue Prüfung
- Findet **potenzielle** Kollisionen
- Methoden: AABB, Grid, Quadtree

#### Narrow Phase (Präzise Prüfung)
- Genaue Prüfung nur für potenzielle Paare
- Methoden: OBB, SAT, GJK

**Beispiel:**

```cpp
void checkCollisions() {
  // BROAD PHASE: Grobe Distanz-Prüfung
  for (Obstacle &obs : obstacles) {
    float dist = getDistanceSquared(carPos, obsPos);

    if (dist < (AVOID_DISTANCE + 20) * (AVOID_DISTANCE + 20)) {
      // Potenziell nah - füge zu Kandidaten-Liste hinzu
      candidates.push_back(&obs);
    }
  }

  // NARROW PHASE: Präzise Prüfung nur für Kandidaten
  for (Obstacle* obs : candidates) {
    float exactDist = getDistance(carPos, obsPos);  // Mit sqrt()

    if (exactDist < COLLISION_DISTANCE) {
      handleCrash(obs);
    }
  }
}
```

### 2. Continuous Collision Detection (CCD)

**Problem:** Bei hoher Geschwindigkeit kann Auto **durch** Hindernis tunneln.

**Frame 1:**
```
Auto  -->     [Hindernis]
```

**Frame 2:**
```
                [Hindernis]  <-- Auto
```

Auto ist **durch** Hindernis geflogen!

**Lösung:** CCD prüft den **Pfad zwischen Frames**.

```cpp
bool checkContinuousCollision(Point start, Point end, Point obstacle, float radius) {
  // Ray-Circle Intersection Test
  // Komplexe Mathematik, siehe: https://stackoverflow.com/q/1073336
}
```

### 3. Physik-Engines

Für komplexe Projekte: Verwende eine Physik-Engine!

**Optionen für ESP32:**
- **Box2D-lite:** 2D Physik (klein genug für ESP32)
- **Chipmunk2D:** Sehr schnell, für eingebettete Systeme

**Vorteile:**
- ✅ Realistische Kollisionen
- ✅ Rotation, Reibung, Elastizität
- ✅ Optimiert und getestet

---

## 🎓 Was hast du gelernt?

Nach diesem Beispiel verstehst du:

- ✅ **Circle-Circle Collision Detection** mit Distanz-Berechnung
- ✅ **Game State Management** mit Flags und Countern
- ✅ **Visual Feedback** für bessere UX
- ✅ **Trade-offs:** Genauigkeit vs. Performance
- ✅ **Objekt-Management** mit Arrays und Structs
- ✅ **Game Design:** Risiko vs. Belohnung Mechaniken

**Nächste Schritte:**
1. Implementiere AABB oder OBB für präzisere Kollisionen
2. Füge Power-Ups hinzu (positives Feedback)
3. Experimentiere mit Spatial Partitioning (Grid)
4. Lerne über Physik-Engines (Box2D)

---

## 🔗 Siehe auch

- [13_Racing_Track](../13_Racing_Track/) - Basis ohne Hindernisse
- [11_Sprite_Animation](../11_Sprite_Animation/) - Sprite-Bewegung
- [00_Hello_CYD](../00_Hello_CYD/) - Display-Basics

**Externe Ressourcen:**
- [Real-Time Collision Detection (Buch)](https://realtimecollisiondetection.net/)
- [2D Collision Detection Tutorial](https://developer.mozilla.org/en-US/docs/Games/Techniques/2D_collision_detection)
- [Box2D Documentation](https://box2d.org/documentation/)

---

**Viel Spaß beim Experimentieren! 🚗💨**
