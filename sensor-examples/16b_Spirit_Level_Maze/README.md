# 🎮 Beispiel 16b: Spirit Level Maze - Labyrinth mit Kollisionserkennung

## 📝 Was macht dieses Beispiel?

Erweitert [16_Spirit_Level](../16_Spirit_Level/) um **Labyrinth-Wände** und **Circle-to-Rectangle Kollision**!

**Features:**
- ✅ **MPU6050 Steuerung** - Gleich wie 16
- ✅ **Labyrinth-Wände** - Rechteckige Hindernisse
- ✅ **Circle-AABB Kollision** - Ball vs. Rechteck
- ✅ **3 Levels** - Steigender Schwierigkeitsgrad
- ✅ **Ziel-System** - Erreiche grünen Kreis
- ✅ **Timer & Moves** - Performance-Tracking
- ✅ **Kollisions-Flash** - Visuelles Feedback
- ✅ **3D-Wände** - Schatten & Highlights

**Steuerung:**
- **Neigen:** Kugel rollt durch Labyrinth
- **Touch:** Nächstes Level nach Goal
- **Ziel:** Erreiche grünen Kreis!

---

## 🎯 Lernziele

1. **Circle-to-Rectangle Kollision** - Wichtigster Kollisions-Typ!
2. **AABB (Axis-Aligned Bounding Box)** - Effiziente Kollision
3. **Collision Response** - Welche Kante getroffen?
4. **Maze Design** - Level-Layouts erstellen
5. **Level-System** - Mehrere Levels verwalten
6. **Game State Management** - Win-Condition, Timer, Stats

---

## ⚙️ Installation & Setup

**Identisch zu [16_Spirit_Level](../16_Spirit_Level/):**
- Adafruit MPU6050 Library
- Verkabelung: VCC→3.3V, GND→GND, SDA→22, SCL→27
- I2C-Adresse: 0x68

---

## 🔍 Kollisions-Algorithmen

### 1. Circle-to-Rectangle (AABB) Kollision

**Der wichtigste Kollisions-Typ in 2D-Spielen!**

#### Algorithmus

```cpp
bool checkCircleRectCollision(float cx, float cy, float radius, Rect& rect) {
  // 1. Finde nächsten Punkt im Rechteck zum Kreis-Zentrum
  float closestX = constrain(cx, rect.x, rect.x + rect.w);
  float closestY = constrain(cy, rect.y, rect.y + rect.h);

  // 2. Berechne Distanz von diesem Punkt zum Kreis-Zentrum
  float dx = cx - closestX;
  float dy = cy - closestY;
  float distSq = dx * dx + dy * dy;

  // 3. Kollision wenn Distanz < Radius
  return distSq < (radius * radius);
}
```

#### Visualisierung

```
Fall 1: Kreis außerhalb
    ┌─────────┐
    │  Rect   │
    └─────────┘
              ○  ← Closest Point = Ecke
              Distanz > Radius → KEINE Kollision

Fall 2: Kreis überlappend
    ┌─────────┐
    │  Re○t   │  ← Closest Point = Kreis-Zentrum
    └─────────┘
    Distanz = 0 < Radius → KOLLISION!

Fall 3: Kreis neben Kante
    ┌─────────┐
○─────│  Rect   │
    └─────────┘
    Closest Point = Kante, Distanz < Radius → KOLLISION!
```

#### Warum dieser Algorithmus?

**Alternative: 4 Kreise an Ecken + 4 Rechtecke an Kanten testen?**
→ Kompliziert, 8 Tests!

**Dieser Algorithmus:**
→ Nur 2 constrain() + 1 Distanz-Check = Elegant!

---

### 2. Collision Response (Kanten-Erkennung)

**Problem:** Wir wissen jetzt dass Kollision ist - aber **von welcher Seite**?

#### Algorithmus

```cpp
void resolveCollision(Rect& wall) {
  // Distanz zu allen 4 Kanten berechnen
  float left   = abs(ballX - wall.x);
  float right  = abs(ballX - (wall.x + wall.w));
  float top    = abs(ballY - wall.y);
  float bottom = abs(ballY - (wall.y + wall.h));

  // Finde minimale Distanz = nächste Kante
  float minDist = min(min(left, right), min(top, bottom));

  if (minDist == left) {
    // Linke Kante getroffen
    ballX = wall.x - BALL_RADIUS;        // Verschiebe aus Wand
    velocityX = -abs(velocityX) * BOUNCE; // Reflektiere X, invertiere
  } else if (minDist == right) {
    // Rechte Kante
    ballX = wall.x + wall.w + BALL_RADIUS;
    velocityX = abs(velocityX) * BOUNCE;  // Reflektiere X, positiv
  } else if (minDist == top) {
    // Obere Kante
    ballY = wall.y - BALL_RADIUS;
    velocityY = -abs(velocityY) * BOUNCE;
  } else {
    // Untere Kante
    ballY = wall.y + wall.h + BALL_RADIUS;
    velocityY = abs(velocityY) * BOUNCE;
  }
}
```

#### Visualisierung

```
Ball kommt von links:
     ●→
    ┌─────┐
    │     │
    └─────┘

→ minDist = left
→ Reflektiere velocityX negativ
→ Ball prallt zurück nach links

Ball kommt von oben:
       ●
       ↓
    ┌─────┐
    │     │
    └─────┘

→ minDist = top
→ Reflektiere velocityY negativ
→ Ball prallt nach oben
```

#### Warum abs()?

```cpp
velocityX = -abs(velocityX) * BOUNCE;

Beispiel:
Ball kommt von links: vx = +5
→ -abs(+5) = -5 → Ball prallt zurück ✓

Ball kommt von rechts: vx = -5
→ -abs(-5) = -5 → Ball prallt nach links ✗ (falsch!)

ABER: Bei left-Kante KANN Ball nur von links kommen!
→ abs() macht Code robuster gegen Edge Cases
```

---

### 3. Verschiedene Kollisions-Typen im Vergleich

| Typ | Genauigkeit | Performance | Anwendung |
|-----|-------------|-------------|-----------|
| **Circle-Circle** | Gut für Kugeln | Sehr schnell | Kugeln, Projektile |
| **AABB-AABB** | Gut für Rechtecke | Sehr schnell | Plattformen, Wände |
| **Circle-AABB** | Hybrid | Schnell | **DIESES BEISPIEL!** |
| **OBB-OBB** | Präzise (rotiert) | Langsam | Rotierte Objekte |
| **Pixel-Perfect** | 100% genau | Sehr langsam | Retro-Games |

#### Circle-Circle (wie in 16)

```cpp
bool checkCircleCircle(Circle a, Circle b) {
  float dx = b.x - a.x;
  float dy = b.y - a.y;
  float dist = sqrt(dx*dx + dy*dy);
  return dist < (a.radius + b.radius);
}
```

**Vorteil:** Einfach, symmetrisch
**Nachteil:** Nur für runde Objekte

#### AABB-AABB

```cpp
bool checkAABB(Rect a, Rect b) {
  return (a.x < b.x + b.w &&
          a.x + a.w > b.x &&
          a.y < b.y + b.h &&
          a.y + a.h > b.y);
}
```

**Vorteil:** Extrem schnell (nur 4 Vergleiche)
**Nachteil:** Nur für Rechtecke, keine Rotation

#### OBB (Oriented Bounding Box)

**Rotierte Rechtecke** - benötigt **SAT (Separating Axis Theorem)**.

```cpp
// SAT: Projiziere beide Shapes auf Achsen
// Wenn alle Achsen überlappen → Kollision
// Komplex! Siehe separate Tutorials
```

**Vorteil:** Funktioniert mit Rotation
**Nachteil:** Viel langsamer als AABB

---

## 🎨 Maze-Design

### Level-Definition

```cpp
Wall level1_walls[] = {
  // Rand (Rahmen)
  {10, 50, 220, 5},    // Oben: x=10, y=50, w=220, h=5
  {10, 255, 220, 5},   // Unten
  {10, 50, 5, 210},    // Links
  {225, 50, 5, 210},   // Rechts

  // Innere Wände
  {60, 50, 5, 100},    // Vertikale Wand
  {140, 110, 5, 100},
  {100, 140, 80, 5},   // Horizontale Wand
};
```

### Design-Prinzipien

#### 1. Immer geschlossener Rahmen

```
✓ Gut:
┌─────────────┐
│             │  ← Rahmen verhindert Ball außerhalb
│   Maze      │
└─────────────┘

✗ Schlecht:
┌─────────────
│   Maze      │  ← Ball kann entkommen!
└─────────────┘
```

#### 2. Start und Ziel zugänglich

```
✓ Gut:
Start
  ●   ╔══════╗
  │   ║      ║
  └───╫──→ ○ ║  ← Pfad zum Ziel existiert
      ╚══════╝

✗ Schlecht:
Start
  ●   ╔══════╗
  │   ║  ○   ║  ← Ziel eingeschlossen!
  X   ║      ║
      ╚══════╝
```

#### 3. Keine zu engen Passagen

```
Passage-Breite >= 2 * BALL_RADIUS + 2px

✓ Gut (20px breit):
║                    ║  ← Ball (16px) passt durch

✗ Zu eng (10px):
║          ║  ← Ball steckt fest!
```

#### 4. Schwierigkeits-Progression

**Level 1:** Einfach, wenige Wände, breite Wege

```
┌──────────────┐
│ ●            │
│              │
│   ║          │
│   ║      ○   │
└──────────────┘
```

**Level 2:** Mittel, mehr Wände, engere Wege

```
┌──────────────┐
│ ●  ║    ║    │
│    ║ ══ ║    │
│ ══    ══ ║   │
│          ║ ○ │
└──────────────┘
```

**Level 3:** Schwer, viele Wände, Sackgassen

```
┌──────────────┐
│ ● ║   ║ ══ ║ │
│ ══║══ ║    ║ │
│   ║ ══╬══    │
│   ║    ║  ○  │
└──────────────┘
```

---

## 🚀 Experimente & Erweiterungen

### 1. 🔄 Bewegliche Wände

**Idee:** Wände die sich bewegen!

```cpp
struct MovingWall {
  Wall wall;
  float vx, vy;       // Geschwindigkeit
  float minX, maxX;   // Bewegungsgrenzen
  float minY, maxY;
};

MovingWall movingWalls[] = {
  {{100, 100, 40, 5}, 2.0, 0, 60, 180, 0, 0},  // Horizontal
  {{150, 80, 5, 60}, 0, 1.5, 0, 0, 80, 200},   // Vertikal
};

void updateMovingWalls(float dt) {
  for (MovingWall& mw : movingWalls) {
    // Position aktualisieren
    mw.wall.x += mw.vx * dt * 60;
    mw.wall.y += mw.vy * dt * 60;

    // Grenzen prüfen & umkehren
    if (mw.wall.x < mw.minX || mw.wall.x > mw.maxX) {
      mw.vx *= -1;
    }
    if (mw.wall.y < mw.minY || mw.wall.y > mw.maxY) {
      mw.vy *= -1;
    }

    // Kollision prüfen (wie normale Wände)
    if (checkCircleRectCollision(ballX, ballY, BALL_RADIUS, mw.wall)) {
      resolveCollision(mw.wall);

      // BONUS: Übertrage Wand-Geschwindigkeit auf Ball!
      velocityX += mw.vx * 0.5;
      velocityY += mw.vy * 0.5;
    }
  }
}
```

---

### 2. 🕳️ Löcher (Instant-Fail)

**Idee:** Schwarze Löcher im Boden - Ball fällt rein = Level Reset!

```cpp
struct Hole {
  int x, y;
  int radius;
};

Hole holes[] = {
  {120, 150, 15},
  {180, 200, 12},
};

void checkHoles() {
  for (Hole& hole : holes) {
    float dx = ballX - hole.x;
    float dy = ballY - hole.y;
    float dist = sqrt(dx*dx + dy*dy);

    if (dist < hole.radius) {
      // Ball ist im Loch!
      lcd.fillScreen(COLOR_BG);
      lcd.setTextSize(3);
      lcd.setTextColor(0xF800);
      lcd.setCursor(50, 150);
      lcd.print("FELL!");

      delay(1000);

      // Reset Level
      startLevel(currentLevel);
      return;
    }
  }
}

void drawHoles() {
  for (Hole& hole : holes) {
    // Gradient für Tiefe-Effekt
    for (int r = hole.radius; r > 0; r -= 2) {
      uint16_t color = lcd.color565(r*2, r*2, r*2);  // Dunkler zur Mitte
      lcd.fillCircle(hole.x, hole.y, r, color);
    }

    // Komplett schwarz in Mitte
    lcd.fillCircle(hole.x, hole.y, 5, 0x0000);
  }
}
```

---

### 3. 🎁 Power-Ups

**Idee:** Sammelbare Items mit temporären Effekten.

```cpp
enum PowerUpType {
  POWERUP_SPEED,     // Schneller
  POWERUP_SMALL,     // Kleiner Ball
  POWERUP_JUMP,      // Spring über Wände
  POWERUP_TIME       // Freeze Timer
};

struct PowerUp {
  int x, y;
  PowerUpType type;
  bool collected;
};

PowerUp powerups[] = {
  {100, 120, POWERUP_SPEED, false},
  {180, 180, POWERUP_SMALL, false},
};

unsigned long powerUpEndTime = 0;
PowerUpType activePowerUp = POWERUP_SPEED;  // Dummy

void checkPowerUps() {
  for (PowerUp& pu : powerups) {
    if (pu.collected) continue;

    float dx = ballX - pu.x;
    float dy = ballY - pu.y;
    if (sqrt(dx*dx + dy*dy) < 15) {
      // Collected!
      pu.collected = true;
      activatePowerUp(pu.type);
    }
  }
}

void activatePowerUp(PowerUpType type) {
  activePowerUp = type;
  powerUpEndTime = millis() + 5000;  // 5 Sekunden

  switch (type) {
    case POWERUP_SPEED:
      GRAVITY_SCALE = 18.0;  // Schneller
      break;
    case POWERUP_SMALL:
      BALL_RADIUS = 5.0;     // Kleiner (passt durch enge Gänge!)
      break;
    case POWERUP_JUMP:
      // Ignoriere Wand-Kollisionen für 5s
      break;
    case POWERUP_TIME:
      // Pausiere Timer
      break;
  }
}

void updatePowerUps() {
  if (millis() > powerUpEndTime) {
    // Power-Up abgelaufen
    GRAVITY_SCALE = 12.0;
    BALL_RADIUS = 8.0;
    activePowerUp = POWERUP_SPEED;  // Reset
  }
}

void drawPowerUps() {
  for (PowerUp& pu : powerups) {
    if (pu.collected) continue;

    // Rotierendes Icon
    float angle = (millis() / 200.0);

    switch (pu.type) {
      case POWERUP_SPEED:
        // Blitz-Symbol (vereinfacht: Stern)
        lcd.fillCircle(pu.x, pu.y, 10, 0xFFE0);
        lcd.setTextColor(0x0000);
        lcd.setCursor(pu.x - 4, pu.y - 4);
        lcd.print(">");
        break;

      case POWERUP_SMALL:
        // Kleiner Kreis
        lcd.drawCircle(pu.x, pu.y, 8, 0x03EF);
        lcd.fillCircle(pu.x, pu.y, 5, 0x03EF);
        break;

      // ... etc
    }
  }
}
```

---

### 4. ⏱️ Time Trial Modus

**Idee:** Bestzeit pro Level, Gesamt-Zeit für alle Levels.

```cpp
unsigned long levelBestTimes[LEVEL_COUNT] = {0, 0, 0};
unsigned long totalTime = 0;
bool timerRunning = false;

void startLevel(int level) {
  // ... existing code

  if (level == 0) {
    totalTime = 0;  // Reset bei Level 1
  }

  timerRunning = true;
}

void handleLevelComplete() {
  timerRunning = false;
  totalTime += levelTime;

  // Bestzeit checken
  if (levelBestTimes[currentLevel] == 0 ||
      levelTime < levelBestTimes[currentLevel]) {
    levelBestTimes[currentLevel] = levelTime;

    lcd.setTextColor(0xFFE0);  // Gelb
    lcd.setCursor(50, 200);
    lcd.print("NEW BEST TIME!");
  }

  // Zeige Stats
  lcd.setCursor(50, 215);
  lcd.printf("Best: %.1f s", levelBestTimes[currentLevel] / 1000.0);

  if (currentLevel == LEVEL_COUNT - 1) {
    // Alle Levels fertig - Gesamt-Zeit
    lcd.setCursor(40, 250);
    lcd.printf("Total: %.1f s", totalTime / 1000.0);
  }
}
```

---

### 5. 🎯 Multiple Balls (Co-Op Modus)

**Idee:** Zwei Bälle gleichzeitig steuern!

```cpp
struct Ball {
  float x, y;
  float vx, vy;
  uint16_t color;
};

Ball balls[2] = {
  {30, 70, 0, 0, 0xF800},   // Rot
  {200, 230, 0, 0, 0x001F}, // Blau
};

Goal dualGoals[2] = {
  {200, 230, 10},  // Ziel für Ball 1
  {30, 70, 10},    // Ziel für Ball 2
};

bool ballsAtGoal[2] = {false, false};

void updatePhysics(float dt) {
  for (int i = 0; i < 2; i++) {
    // Gleiche Sensor-Daten für beide Bälle
    float forceX = -accelX * GRAVITY_SCALE;
    float forceY = accelY * GRAVITY_SCALE;

    balls[i].vx += forceX * dt;
    balls[i].vy += forceY * dt;

    balls[i].vx *= FRICTION;
    balls[i].vy *= FRICTION;

    balls[i].x += balls[i].vx * dt * 60;
    balls[i].y += balls[i].vy * dt * 60;

    // Wand-Kollision
    checkWallCollisions(balls[i]);

    // Ziel-Check
    float dx = balls[i].x - dualGoals[i].x;
    float dy = balls[i].y - dualGoals[i].y;
    if (sqrt(dx*dx + dy*dy) < dualGoals[i].radius + BALL_RADIUS) {
      ballsAtGoal[i] = true;
    }
  }

  // Ball-zu-Ball Kollision!
  checkBallToBall(balls[0], balls[1]);

  // Level complete wenn BEIDE Bälle am Ziel
  if (ballsAtGoal[0] && ballsAtGoal[1]) {
    levelComplete = true;
  }
}

void checkBallToBall(Ball& a, Ball& b) {
  float dx = b.x - a.x;
  float dy = b.y - a.y;
  float dist = sqrt(dx*dx + dy*dy);
  float minDist = BALL_RADIUS * 2;

  if (dist < minDist && dist > 0) {
    // Kollision!

    // Normalisiere Richtung
    float nx = dx / dist;
    float ny = dy / dist;

    // Separiere Bälle
    float overlap = minDist - dist;
    a.x -= overlap * 0.5 * nx;
    a.y -= overlap * 0.5 * ny;
    b.x += overlap * 0.5 * nx;
    b.y += overlap * 0.5 * ny;

    // Impuls-Transfer
    float dvx = b.vx - a.vx;
    float dvy = b.vy - a.vy;
    float dotProduct = dvx * nx + dvy * ny;

    if (dotProduct < 0) {
      a.vx -= dotProduct * nx * 0.5;
      a.vy -= dotProduct * ny * 0.5;
      b.vx += dotProduct * nx * 0.5;
      b.vy += dotProduct * ny * 0.5;
    }
  }
}
```

**Challenge:** Bälle müssen **zusammenarbeiten** um Ziele zu erreichen!

---

### 6. 🎨 Maze-Editor

**Idee:** Eigene Levels im Spiel designen!

```cpp
bool editorMode = false;
int editorWallCount = 4;  // Start mit Rahmen
Wall editorWalls[50];

int dragStartX = 0, dragStartY = 0;
bool dragging = false;

void enterEditorMode() {
  editorMode = true;

  // Init mit Rahmen
  editorWalls[0] = {10, 50, 220, 5};
  editorWalls[1] = {10, 255, 220, 5};
  editorWalls[2] = {10, 50, 5, 210};
  editorWalls[3] = {225, 50, 5, 210};

  Serial.println("Editor Mode - Drag to create walls");
}

void handleEditorTouch() {
  uint16_t x, y;
  bool touched = lcd.getTouch(&x, &y);

  if (touched && !dragging) {
    // Start Drag
    dragStartX = x;
    dragStartY = y;
    dragging = true;

  } else if (touched && dragging) {
    // Continue Drag - zeige Vorschau
    lcd.drawRect(min(dragStartX, x), min(dragStartY, y),
                 abs(x - dragStartX), abs(y - dragStartY),
                 0xFFFF);

  } else if (!touched && dragging) {
    // End Drag - Wand erstellen
    if (editorWallCount < 50) {
      editorWalls[editorWallCount++] = {
        min(dragStartX, x),
        min(dragStartY, y),
        abs(x - dragStartX),
        abs(y - dragStartY)
      };
    }

    dragging = false;
  }
}

void saveLevel() {
  // Serialisiere zu Code
  Serial.println("Wall editorLevel[] = {");
  for (int i = 0; i < editorWallCount; i++) {
    Serial.printf("  {%d, %d, %d, %d},\n",
                  editorWalls[i].x, editorWalls[i].y,
                  editorWalls[i].w, editorWalls[i].h);
  }
  Serial.println("};");
}
```

---

### 7. 🏆 Leaderboard (SPIFFS)

**Idee:** Speichere Bestzeiten persistent!

```cpp
#include <SPIFFS.h>

void saveLeaderboard() {
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS failed");
    return;
  }

  File file = SPIFFS.open("/leaderboard.txt", FILE_WRITE);

  for (int i = 0; i < LEVEL_COUNT; i++) {
    file.printf("Level %d: %lu\n", i+1, levelBestTimes[i]);
  }

  file.close();
  Serial.println("Leaderboard saved");
}

void loadLeaderboard() {
  if (!SPIFFS.begin(true)) return;

  File file = SPIFFS.open("/leaderboard.txt", FILE_READ);
  if (!file) return;

  for (int i = 0; i < LEVEL_COUNT; i++) {
    String line = file.readStringUntil('\n');
    int colonPos = line.indexOf(':');
    if (colonPos > 0) {
      levelBestTimes[i] = line.substring(colonPos + 2).toInt();
    }
  }

  file.close();
  Serial.println("Leaderboard loaded");
}
```

---

## 🐛 Troubleshooting

### Problem: Ball steckt in Wand fest

**Symptom:** Ball vibriert in Wand, kann nicht raus.

**Ursache:** Collision Response schiebt nicht weit genug.

**Lösung:**

```cpp
if (minDist == left) {
  ballX = wall.x - BALL_RADIUS - 1;  // +1 für Sicherheit
  velocityX = -abs(velocityX) * BOUNCE;
}
```

---

### Problem: Ball kann durch Wand-Ecken clippen

**Symptom:** Bei hoher Geschwindigkeit durchdringt Ball Ecken.

**Ursache:** Tunneling (Ball überspringt Wand in einem Frame).

**Lösung 1:** Reduziere MAX_VELOCITY

```cpp
const float MAX_VELOCITY = 4.0;  // Statt 6.0
```

**Lösung 2:** Continuous Collision Detection (CCD)

```cpp
// Prüfe Pfad von alter zu neuer Position
bool checkRaycast(float x1, float y1, float x2, float y2, Wall& wall) {
  // Line-Rectangle Intersection
  // Komplex! Siehe: https://stackoverflow.com/q/99353
}
```

---

### Problem: Ziel nicht erreichbar

**Symptom:** Kugel kommt nicht ins Ziel, egal wie versucht.

**Ursache:** Ziel ist eingeschlossen oder zu eng.

**Lösung:** Visualisiere Ziel-Bereich im Editor:

```cpp
void drawGoal() {
  Goal& goal = levelGoals[currentLevel];

  // Zeige Kollisions-Radius
  lcd.drawCircle(goal.x, goal.y, goal.radius + BALL_RADIUS, 0x07E0);

  // Normales Goal-Rendering
  // ...
}
```

Wenn Kreis Wände überschneidet → Ziel unerreichbar!

---

### Problem: Performance-Probleme bei vielen Wänden

**Symptom:** FPS-Drops bei Level 3.

**Ursache:** Zu viele Kollisions-Checks (O(n) pro Frame).

**Lösung:** Spatial Partitioning (Grid)

```cpp
const int GRID_SIZE = 40;  // 40x40 pixel Zellen
const int GRID_W = 6;      // 240/40 = 6 Zellen
const int GRID_H = 8;      // 320/40 = 8 Zellen

std::vector<Wall*> grid[GRID_W][GRID_H];

void buildGrid() {
  // Leere Grid
  for (int i = 0; i < GRID_W; i++) {
    for (int j = 0; j < GRID_H; j++) {
      grid[i][j].clear();
    }
  }

  // Füge Wände zu Grid-Zellen hinzu
  Wall* walls = levels[currentLevel];
  for (int i = 0; i < levelWallCounts[currentLevel]; i++) {
    Wall& w = walls[i];

    // Welche Grid-Zellen überschneidet diese Wand?
    int gridX1 = w.x / GRID_SIZE;
    int gridY1 = w.y / GRID_SIZE;
    int gridX2 = (w.x + w.w) / GRID_SIZE;
    int gridY2 = (w.y + w.h) / GRID_SIZE;

    for (int gx = gridX1; gx <= gridX2 && gx < GRID_W; gx++) {
      for (int gy = gridY1; gy <= gridY2 && gy < GRID_H; gy++) {
        grid[gx][gy].push_back(&w);
      }
    }
  }
}

void checkWallCollisions() {
  // Ball-Position → Grid-Zelle
  int gridX = (int)ballX / GRID_SIZE;
  int gridY = (int)ballY / GRID_SIZE;

  gridX = constrain(gridX, 0, GRID_W - 1);
  gridY = constrain(gridY, 0, GRID_H - 1);

  // Prüfe nur Wände in dieser Zelle!
  for (Wall* wall : grid[gridX][gridY]) {
    if (checkCircleRectCollision(ballX, ballY, BALL_RADIUS, *wall)) {
      resolveCollision(*wall);
    }
  }
}
```

**Resultat:** O(n) → O(k) wo k = Wände pro Zelle (meist 1-3)!

---

## 📚 Weiterführende Konzepte

### 1. SAT (Separating Axis Theorem)

**Für rotierte Rechtecke (OBB):**

```cpp
// Projiziere beide Shapes auf alle relevanten Achsen
// Wenn ALLE Projektionen überlappen → Kollision

bool checkOBB(OBB a, OBB b) {
  // Achsen: 2 von A, 2 von B
  Vector2 axes[4] = {
    a.getAxis(0), a.getAxis(1),
    b.getAxis(0), b.getAxis(1)
  };

  for (Vector2 axis : axes) {
    // Projiziere A und B auf Achse
    Projection projA = a.project(axis);
    Projection projB = b.project(axis);

    // Überlappen sie?
    if (!projA.overlaps(projB)) {
      return false;  // Separating Axis gefunden = keine Kollision
    }
  }

  return true;  // Alle Achsen überlappen = Kollision
}
```

**Siehe:** [SAT Tutorial](https://www.sevenson.com.au/programming/sat/)

### 2. GJK (Gilbert-Johnson-Keerthi)

**Universeller Kollisions-Algorithmus** für beliebige konvexe Shapes!

```cpp
// Iterativer Algorithmus, findet Simplex
// Komplex aber mächtig!
```

**Siehe:** [GJK Tutorial](https://caseymuratori.com/blog_0003)

### 3. Quadtree

**Effizientere Spatial Partitioning als Grid:**

```cpp
class Quadtree {
  Rect bounds;
  int capacity = 4;
  std::vector<Wall*> walls;
  Quadtree* children[4] = {nullptr};

  void subdivide() {
    // Teile in 4 Quadranten
    children[0] = new Quadtree(topLeft);
    children[1] = new Quadtree(topRight);
    children[2] = new Quadtree(bottomLeft);
    children[3] = new Quadtree(bottomRight);
  }

  void insert(Wall* wall) {
    if (!bounds.intersects(wall)) return;

    if (walls.size() < capacity) {
      walls.push_back(wall);
    } else {
      if (children[0] == nullptr) subdivide();

      for (Quadtree* child : children) {
        child->insert(wall);
      }
    }
  }

  std::vector<Wall*> query(Circle circle) {
    std::vector<Wall*> found;

    if (!bounds.intersects(circle)) return found;

    for (Wall* w : walls) {
      if (checkCircleRectCollision(circle, *w)) {
        found.push_back(w);
      }
    }

    if (children[0] != nullptr) {
      for (Quadtree* child : children) {
        auto childResults = child->query(circle);
        found.insert(found.end(), childResults.begin(), childResults.end());
      }
    }

    return found;
  }
};
```

**Vorteil:** O(log n) statt O(n)!

---

## 🎓 Was hast du gelernt?

Nach diesem Beispiel verstehst du:

- ✅ **Circle-AABB Kollision** - Wichtigster 2D-Kollisions-Typ
- ✅ **Collision Response** - Welche Kante, wie reagieren?
- ✅ **Maze-Design** - Level-Layouts erstellen
- ✅ **Level-System** - Mehrere Levels verwalten
- ✅ **Game State** - Win-Condition, Timer, Moves
- ✅ **Verschiedene Kollisions-Typen** - Circle-Circle, AABB-AABB, Circle-AABB, OBB

**Vergleich zu 16:**

| Aspekt | 16 (Spirit Level) | 16b (Maze) |
|--------|-------------------|------------|
| **Kollision** | Circle-Circle (Rand) | **Circle-AABB (Wände)** |
| **Bewegungsraum** | Kreisförmig | **Labyrinth** |
| **Ziel** | Level erreichen | **Goal erreichen** |
| **Levels** | 1 | **3 (erweiterbar)** |
| **Komplexität** | Einfach | **Mittel-Schwer** |

---

## 🔗 Siehe auch

- [16_Spirit_Level](../16_Spirit_Level/) - Basis ohne Labyrinth
- [13b_Racing_Track_Obstacles](../13b_Racing_Track_Obstacles/) - Andere Kollisions-Art
- [Collision Detection Tutorial](https://developer.mozilla.org/en-US/docs/Games/Techniques/2D_collision_detection)

**Externe Ressourcen:**
- [SAT Explained](https://www.sevenson.com.au/programming/sat/)
- [GJK Algorithm](https://caseymuratori.com/blog_0003)
- [Spatial Hashing](https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/spatial-hashing-r2697/)

---

**Kollisionserkennung gemeistert! 🎮✨**
