# ⚖️ Beispiel 16: Spirit Level - Digitale Wasserwaage mit MPU6050

## 📝 Was macht dieses Beispiel?

Eine **realistische digitale Wasserwaage** mit schöner 3D-Kugel und physikalischer Simulation!

**Features:**
- ✅ **MPU6050 Sensor** - Accelerometer & Gyro über I2C
- ✅ **Realistische Physik** - Gravitation, Reibung, Bounce
- ✅ **Schöne 3D-Kugel** - Radial-Gradient mit Lichtreflex
- ✅ **Dynamischer Schatten** - Unter der Kugel
- ✅ **Wasserwaagen-Kreise** - Konzentrisch wie echte Wasserwaage
- ✅ **Level-Indikator** - "LEVEL" wenn waagerecht
- ✅ **Winkel-Anzeige** - X/Y in Grad
- ✅ **Kalibrierung** - Touch zum Kalibrieren
- ✅ **Smooth Animation** - 60 FPS mit Delta-Time

**Steuerung:**
- **Touch irgendwo:** Kalibrieren (aktuellen Winkel als 0° setzen)
- **Neigen:** Kugel rollt entsprechend der Neigung

---

## 🎯 Lernziele

1. **I2C-Kommunikation** - Hardware-Bus verstehen
2. **MPU6050 Sensor** - Accelerometer/Gyro auslesen
3. **Physik-Simulation** - Realistische Bewegung
4. **3D-Grafik** - Radial-Gradient für Tiefe
5. **Delta-Time** - Frame-rate unabhängige Animation
6. **Kalibrierung** - Sensor-Offset kompensieren
7. **Vektor-Mathematik** - Kollision, Reflexion

---

## ⚙️ Installation & Setup

### 1. Library installieren

**Arduino IDE:**
1. Library Manager öffnen (Strg+Shift+I)
2. "Adafruit MPU6050" suchen
3. Installieren (installiert automatisch Dependencies)

**PlatformIO:**
```ini
[env:esp32]
lib_deps =
  lovyan03/LovyanGFX @ ^1.1.0
  adafruit/Adafruit MPU6050 @ ^2.2.0
```

### 2. Hardware anschließen

**MPU6050 (GY-521 Modul):**

```
MPU6050          CYD
├─ VCC    →    3.3V (WICHTIG: NICHT 5V!)
├─ GND    →    GND
├─ SDA    →    GPIO 22 (extSDA)
└─ SCL    →    GPIO 27 (extSCL)
```

**ACHTUNG:** MPU6050 ist **3.3V**! An 5V kann er beschädigt werden!

**I2C Pins (CYD Standard):**
- `extSDA = GPIO 22`
- `extSCL = GPIO 27`

Diese sollten in `CYD_Display_Config.h` definiert sein. Falls nicht, definiert der Code sie automatisch.

### 3. I2C-Adresse prüfen

**Standard:** `0x68`

Wenn Probleme, scanne I2C-Bus:

```cpp
void scanI2C() {
  Wire.begin(extSDA, extSCL);

  Serial.println("Scanning I2C bus...");

  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("Found device at 0x%02X\n", addr);
    }
  }
}
```

**Mögliche Adressen:**
- `0x68` - Standard (AD0 auf GND)
- `0x69` - Alternativ (AD0 auf VCC)

### 4. Kompilieren & Upload

```
Arduino IDE: Upload (Strg+U)
PlatformIO: pio run -t upload
```

### 5. Testen

1. Display zeigt "Calibrating..." (~3 Sekunden)
2. Kugel erscheint in der Mitte
3. Neige CYD → Kugel rollt
4. Waagerecht halten → "LEVEL" erscheint
5. Touch → Kalibriert neu

---

## 🔍 Technische Konzepte

### 1. I2C-Kommunikation

**I2C (Inter-Integrated Circuit)** ist ein serieller Bus für Sensor-Kommunikation.

#### Prinzip

```
Master (ESP32)  ←──SDA──→  Slave (MPU6050)
                ←──SCL──→
```

- **SDA (Serial Data):** Bidirektionale Datenleitung
- **SCL (Serial Clock):** Takt-Signal vom Master
- **Adresse:** Jedes Gerät hat eindeutige Adresse (7-bit)

#### ESP32 I2C-Pins

```cpp
Wire.begin(SDA_PIN, SCL_PIN);

// CYD hat zwei I2C-Busse möglich:
// 1. Internal (für Display-Touch)
// 2. External (GPIO 22/27) - für Sensoren
```

**Warum extSDA/extSCL?**

CYD nutzt bereits I2C intern für Touch-Controller → zweiter Bus für externe Sensoren!

#### Kommunikation

```cpp
// 1. Initialisierung
Wire.begin(extSDA, extSCL);

// 2. Gerät ansprechen
Wire.beginTransmission(0x68);  // MPU6050 Adresse

// 3. Register schreiben
Wire.write(0x6B);  // Register-Adresse (z.B. PWR_MGMT_1)
Wire.write(0x00);  // Wert (0x00 = Wake up)

// 4. Transaktion abschließen
Wire.endTransmission();

// 5. Daten lesen
Wire.beginTransmission(0x68);
Wire.write(0x3B);  // ACCEL_XOUT_H Register
Wire.endTransmission(false);  // Repeated Start

Wire.requestFrom(0x68, 6);  // 6 Bytes lesen (X,Y,Z je 2 Bytes)

int16_t accelX = Wire.read() << 8 | Wire.read();
int16_t accelY = Wire.read() << 8 | Wire.read();
int16_t accelZ = Wire.read() << 8 | Wire.read();
```

**Glücklicherweise:** Adafruit Library macht das alles automatisch!

---

### 2. MPU6050 Sensor

**MPU6050** kombiniert **Accelerometer** (Beschleunigung) und **Gyroscope** (Drehrate).

#### Accelerometer

**Misst Beschleunigung in g (Erdbeschleunigung = 9.81 m/s²):**

```
Axes (CYD liegt flach auf Tisch):
  X: Links/Rechts
  Y: Vor/Zurück
  Z: Oben/Unten (zeigt nach oben = +1g)

Neigung nach rechts:
  X: +0.5g (nach rechts gezogen)
  Y: 0g
  Z: +0.87g (noch etwas nach oben)
```

**Verwendung für Wasserwaage:**

```cpp
sensors_event_t a, g, temp;
mpu.getEvent(&a, &g, &temp);

float accelX = a.acceleration.x;  // in g
float accelY = a.acceleration.y;
float accelZ = a.acceleration.z;
```

**Winkel berechnen:**

```cpp
// atan2(y, z) gibt Winkel um X-Achse
float angleX = atan2(accelY, accelZ) * 57.2958;  // in Grad

// atan2(x, z) gibt Winkel um Y-Achse
float angleY = atan2(accelX, accelZ) * 57.2958;
```

**Warum atan2?**

```
      accelY
        ↑
        │    / (Vektor)
        │  /
        │/) angleX
        └────────→ accelZ

atan2(y, z) berechnet Winkel korrekt in allen Quadranten!
```

#### Gyroscope

**Misst Drehrate in °/s:**

Wird in diesem Beispiel nicht verwendet (nur Accelerometer für statische Neigung).

**Für Bewegung:** Kombiniere Accel + Gyro (Sensor Fusion)!

---

### 3. Kalibrierung

**Problem:** Sensor hat kleine Offsets (nicht exakt 0g bei waagerecht).

**Lösung:** Messe Offset bei bekannter Lage und subtrahiere.

```cpp
void calibrate() {
  float sumX = 0, sumY = 0;
  const int samples = 100;

  for (int i = 0; i < samples; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    sumX += a.acceleration.x;
    sumY += a.acceleration.y;

    delay(10);
  }

  offsetX = sumX / samples;
  offsetY = sumY / samples;

  Serial.printf("Offsets: X=%.3f, Y=%.3f\n", offsetX, offsetY);
}

// Später beim Lesen:
float accelX = a.acceleration.x - offsetX;
float accelY = a.acceleration.y - offsetY;
```

**Warum 100 Samples?**

Sensor hat Rauschen → Mittelwert reduziert Rauschen!

---

### 4. Physik-Simulation

#### Bewegungsgleichungen

```cpp
// 1. Kraft aus Sensor
float forceX = -accelX * GRAVITY_SCALE;
float forceY = accelY * GRAVITY_SCALE;

// 2. Geschwindigkeit aktualisieren (Integration)
velocityX += forceX * deltaTime;
velocityY += forceY * deltaTime;

// 3. Reibung anwenden
velocityX *= FRICTION;  // z.B. 0.98 = 2% Verlust pro Frame

// 4. Position aktualisieren (Integration)
ballX += velocityX * deltaTime * 60.0;
ballY += velocityY * deltaTime * 60.0;
```

**Warum Delta-Time?**

```
Ohne Delta-Time:
  60 FPS: Ball bewegt sich 1 Pixel/Frame → 60 px/s
  30 FPS: Ball bewegt sich 1 Pixel/Frame → 30 px/s
  → Inkonsistent!

Mit Delta-Time:
  60 FPS: dt=0.016s → Ball bewegt sich v*0.016 → v px/s
  30 FPS: dt=0.033s → Ball bewegt sich v*0.033 → v px/s
  → Konsistent!
```

#### Reibung

```cpp
velocityX *= 0.98;  // 2% Verlust pro Frame

Nach 10 Frames:  v * 0.98^10 = v * 0.817  (82%)
Nach 50 Frames:  v * 0.98^50 = v * 0.364  (36%)
Nach 100 Frames: v * 0.98^100 = v * 0.133 (13%)

→ Kugel wird langsamer und stoppt!
```

#### Kollision mit Kreis

```cpp
// Distanz vom Zentrum
float dx = ballX - centerX;
float dy = ballY - centerY;
float dist = sqrt(dx * dx + dy * dy);

float maxDist = AREA_SIZE - BALL_RADIUS;

if (dist > maxDist) {
  // Ball ist außerhalb Kreis

  // 1. Normalisiere Richtung
  float nx = dx / dist;
  float ny = dy / dist;

  // 2. Setze Ball auf Rand
  ballX = centerX + nx * maxDist;
  ballY = centerY + ny * maxDist;

  // 3. Reflektiere Geschwindigkeit
  // Formel: v' = v - 2(v·n)n
  float dotProduct = velocityX * nx + velocityY * ny;
  velocityX -= 2 * dotProduct * nx;
  velocityY -= 2 * dotProduct * ny;

  // 4. Bounce-Dämpfung
  velocityX *= BOUNCE;  // z.B. 0.6 = 60% Energie bleibt
  velocityY *= BOUNCE;
}
```

**Reflexion - Die Mathematik:**

```
Incident Vector (v):     Reflected Vector (v'):
      ↓                       ↗
       \                     /
        \                   /
         \                 /
    ──────●────────  (Wand, Normale n)
           ↑
         Normal (n)

v' = v - 2(v·n)n

Dot Product: v·n = vx*nx + vy*ny
```

---

### 5. 3D-Grafik: Radial-Gradient

**Problem:** Flacher Kreis sieht 2D aus.

**Lösung:** Radial-Gradient simuliert 3D-Kugel!

```cpp
void drawBall() {
  // Lichtquelle oben-links
  int lightOffsetX = -6;
  int lightOffsetY = -6;

  for (int r = BALL_RADIUS; r >= 0; r--) {
    // t = 0.0 (Rand) bis 1.0 (Zentrum)
    float t = 1.0 - (float)r / BALL_RADIUS;

    uint16_t color;

    if (t < 0.3) {
      color = COLOR_BALL_DARK;   // Außen dunkel
    } else if (t < 0.7) {
      color = COLOR_BALL;        // Mitte normal
    } else {
      color = COLOR_BALL_LIGHT;  // Zentrum hell
    }

    lcd.fillCircle(ballX, ballY, r, color);
  }

  // Highlight (heller Fleck versetzt)
  for (int r = BALL_RADIUS * 0.3; r >= 0; r--) {
    lcd.fillCircle(ballX + lightOffsetX, ballY + lightOffsetY, r, COLOR_BALL_LIGHT);
  }
}
```

**Resultat:**

```
    ○  ← Highlight (heller Fleck)
   ╱ ╲
  │   │  ← Gradient (dunkel → hell → dunkel)
   ╲ ╱
    ●  ← Schatten-Seite

→ Sieht aus wie 3D-Kugel!
```

#### Schatten

```cpp
void drawShadow() {
  int shadowX = ballX + 5;  // Versetzt (Licht von oben-links)
  int shadowY = ballY + 5;

  // Mehrere Kreise mit abnehmender Größe = Blur-Effekt
  for (int r = BALL_RADIUS; r >= 0; r -= 2) {
    lcd.fillCircle(shadowX, shadowY, r, COLOR_SHADOW);
  }
}
```

**Warum versetzt?**

```
  Licht
    ↓
    ●  ← Kugel
     ╲
      ○ ← Schatten (versetzt)
```

Schatten erscheint gegenüber der Lichtrichtung!

---

## 🚀 Experimente & Erweiterungen

### 1. 🎨 Verschiedene Kugel-Designs

**Idee:** Metall-Kugel, Glas-Kugel, Billard-Ball, etc.

```cpp
// Metall-Kugel (Chrome-Effekt)
void drawMetalBall() {
  for (int r = BALL_RADIUS; r >= 0; r--) {
    float t = (float)r / BALL_RADIUS;

    // Chrome: Starke Kontraste
    uint16_t color;
    if (t > 0.8) {
      color = 0xFFFF;  // Weiß (Reflex)
    } else if (t > 0.6) {
      color = 0xC618;  // Hellgrau
    } else if (t > 0.4) {
      color = 0x8410;  // Grau
    } else {
      color = 0x4208;  // Dunkelgrau
    }

    lcd.fillCircle(ballX, ballY, r, color);
  }

  // Mehrere Highlights
  lcd.fillCircle(ballX - 8, ballY - 8, 4, 0xFFFF);
  lcd.fillCircle(ballX + 6, ballY - 5, 2, 0xCE79);
}

// Billard-Ball (mit Nummer)
void drawBillardBall() {
  // Basis-Kugel (z.B. gelb für Ball 1)
  for (int r = BALL_RADIUS; r >= 0; r--) {
    lcd.fillCircle(ballX, ballY, r, 0xFFE0);
  }

  // Highlight
  lcd.fillCircle(ballX - 6, ballY - 6, 6, 0xFFFF);

  // Nummer (weiße Scheibe + schwarze Nummer)
  lcd.fillCircle(ballX, ballY, 10, 0xFFFF);
  lcd.setTextColor(0x0000);
  lcd.setTextSize(2);
  lcd.setCursor(ballX - 5, ballY - 8);
  lcd.print("1");
}

// Glas-Kugel (transparent-Effekt)
void drawGlassBall() {
  // Äußerer Ring (stark reflektierend)
  lcd.drawCircle(ballX, ballY, BALL_RADIUS, 0xFFFF);
  lcd.drawCircle(ballX, ballY, BALL_RADIUS - 1, 0xCE79);

  // Inneres (leicht gefüllt)
  for (int r = BALL_RADIUS - 2; r >= 0; r--) {
    uint16_t color = lcd.color565(200, 220, 255);  // Leichtes Blau
    lcd.fillCircle(ballX, ballY, r, color);
  }

  // Starker Highlight (Reflektion)
  lcd.fillCircle(ballX - 8, ballY - 8, 8, 0xFFFF);
  lcd.fillCircle(ballX - 8, ballY - 8, 6, 0xE73F);
}
```

---

### 2. 🎯 Multiple Bälle

**Idee:** Mehrere Kugeln die miteinander kollidieren!

```cpp
const int NUM_BALLS = 3;

struct Ball {
  float x, y;
  float vx, vy;
  float radius;
  uint16_t color;
};

Ball balls[NUM_BALLS] = {
  {100, 160, 0, 0, 15, 0xF800},  // Rot
  {140, 160, 0, 0, 15, 0x07E0},  // Grün
  {120, 140, 0, 0, 15, 0x001F},  // Blau
};

void updatePhysics(float dt) {
  for (int i = 0; i < NUM_BALLS; i++) {
    // Sensor-Kraft auf alle Bälle
    balls[i].vx += -accelX * GRAVITY_SCALE * dt;
    balls[i].vy += accelY * GRAVITY_SCALE * dt;

    balls[i].vx *= FRICTION;
    balls[i].vy *= FRICTION;

    balls[i].x += balls[i].vx * dt * 60;
    balls[i].y += balls[i].vy * dt * 60;

    // Kollision mit Rand
    // ... (wie vorher)
  }

  // Ball-zu-Ball Kollision
  for (int i = 0; i < NUM_BALLS; i++) {
    for (int j = i + 1; j < NUM_BALLS; j++) {
      checkBallCollision(balls[i], balls[j]);
    }
  }
}

void checkBallCollision(Ball& a, Ball& b) {
  float dx = b.x - a.x;
  float dy = b.y - a.y;
  float dist = sqrt(dx * dx + dy * dy);
  float minDist = a.radius + b.radius;

  if (dist < minDist) {
    // Kollision!

    // Normalisiere Richtung
    float nx = dx / dist;
    float ny = dy / dist;

    // Relative Geschwindigkeit
    float dvx = b.vx - a.vx;
    float dvy = b.vy - a.vy;

    // Dot product
    float dotProduct = dvx * nx + dvy * ny;

    // Nur wenn Bälle sich aufeinander zu bewegen
    if (dotProduct < 0) {
      // Impuls-basierte Kollision (elastisch)
      a.vx -= dotProduct * nx;
      a.vy -= dotProduct * ny;
      b.vx += dotProduct * nx;
      b.vy += dotProduct * ny;

      // Separiere Bälle (verschiebe auseinander)
      float overlap = minDist - dist;
      a.x -= overlap * 0.5 * nx;
      a.y -= overlap * 0.5 * ny;
      b.x += overlap * 0.5 * nx;
      b.y += overlap * 0.5 * ny;
    }
  }
}
```

---

### 3. 🌊 Wasser-Simulation

**Idee:** Kugel schwimmt in Wasser (bewegt sich langsamer).

```cpp
// Wasser-Level (y-Koordinate)
float waterLevel = 200;

void updatePhysics(float dt) {
  // Normale Physik...
  velocityX += -accelX * GRAVITY_SCALE * dt;
  velocityY += accelY * GRAVITY_SCALE * dt;

  // Wasser-Check
  if (ballY > waterLevel - BALL_RADIUS) {
    // Ball ist (teilweise) im Wasser

    // Auftrieb (nach oben)
    float buoyancy = 0.8;  // 80% der Gravitation
    velocityY -= buoyancy * GRAVITY_SCALE * dt;

    // Wasser-Widerstand (extra Dämpfung)
    velocityX *= 0.90;  // Stärker als Luft-Reibung
    velocityY *= 0.90;
  } else {
    // In Luft - normale Reibung
    velocityX *= FRICTION;
    velocityY *= FRICTION;
  }

  // Position update
  ballX += velocityX * dt * 60;
  ballY += velocityY * dt * 60;
}

void drawWater() {
  // Wasser (blau, halbtransparent simuliert)
  int waterHeight = lcd.height() - waterLevel;
  lcd.fillRect(0, waterLevel, lcd.width(), waterHeight, 0x03EF);

  // Wellen (kleine Linien)
  for (int x = 0; x < lcd.width(); x += 10) {
    int waveY = waterLevel + sin((millis() / 200.0) + x * 0.1) * 3;
    lcd.drawPixel(x, waveY, 0xFFFF);
  }

  // Blasen wenn Ball eintaucht
  if (ballY > waterLevel - BALL_RADIUS && abs(velocityY) > 2) {
    for (int i = 0; i < 5; i++) {
      int bubbleX = ballX + random(-10, 10);
      int bubbleY = ballY + random(0, 20);
      lcd.drawCircle(bubbleX, bubbleY, 2, 0xFFFF);
    }
  }
}
```

---

### 4. 🎮 Labyrinth-Spiel

**Idee:** Hindernisse auf dem Feld, navigiere Ball zum Ziel!

```cpp
struct Obstacle {
  int x, y;
  int width, height;
};

Obstacle obstacles[] = {
  {80, 100, 80, 10},   // Wand horizontal
  {100, 120, 10, 60},  // Wand vertikal
  {60, 200, 60, 10},   // Wand horizontal
  // ...
};

struct Goal {
  int x, y;
  int radius;
};

Goal goal = {120, 280, 15};

void checkObstacleCollision() {
  for (Obstacle& obs : obstacles) {
    // AABB (Axis-Aligned Bounding Box) Kollision
    if (ballX + BALL_RADIUS > obs.x &&
        ballX - BALL_RADIUS < obs.x + obs.width &&
        ballY + BALL_RADIUS > obs.y &&
        ballY - BALL_RADIUS < obs.y + obs.height) {

      // Kollision! Bounce zurück

      // Finde nächste Kante
      float left = abs((ballX - BALL_RADIUS) - obs.x);
      float right = abs((ballX + BALL_RADIUS) - (obs.x + obs.width));
      float top = abs((ballY - BALL_RADIUS) - obs.y);
      float bottom = abs((ballY + BALL_RADIUS) - (obs.y + obs.height));

      float minDist = min(min(left, right), min(top, bottom));

      if (minDist == left || minDist == right) {
        // Horizontal bounce
        velocityX *= -BOUNCE;
      } else {
        // Vertical bounce
        velocityY *= -BOUNCE;
      }
    }
  }
}

void checkGoal() {
  float dx = ballX - goal.x;
  float dy = ballY - goal.y;
  float dist = sqrt(dx * dx + dy * dy);

  if (dist < goal.radius + BALL_RADIUS) {
    // ZIEL ERREICHT!
    lcd.fillScreen(COLOR_SUCCESS);
    lcd.setTextSize(3);
    lcd.setCursor(50, 150);
    lcd.print("GOAL!");
    delay(2000);

    // Neues Level oder Reset
  }
}

void drawObstacles() {
  for (Obstacle& obs : obstacles) {
    lcd.fillRect(obs.x, obs.y, obs.width, obs.height, 0x8410);
    lcd.drawRect(obs.x, obs.y, obs.width, obs.height, 0xFFFF);
  }
}

void drawGoal() {
  // Blinkendes Ziel
  if ((millis() / 500) % 2) {
    lcd.fillCircle(goal.x, goal.y, goal.radius, COLOR_SUCCESS);
  }
  lcd.drawCircle(goal.x, goal.y, goal.radius, 0xFFFF);
}
```

---

### 5. 🎵 Sound-Effekte

**Idee:** Töne bei Kollision, Level-Erreichen, etc.

```cpp
// Piezo-Buzzer an GPIO (z.B. 26)
#define BUZZER_PIN 26

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  ledcSetup(0, 5000, 8);       // Channel 0, 5kHz, 8-bit
  ledcAttachPin(BUZZER_PIN, 0);
}

void playBounceSound() {
  // Kurzer "Boing" Ton
  ledcWriteTone(0, 800);  // 800 Hz
  delay(50);
  ledcWriteTone(0, 600);  // 600 Hz
  delay(30);
  ledcWriteTone(0, 0);    // Aus
}

void playLevelSound() {
  // Steigender Ton
  for (int freq = 400; freq < 1000; freq += 100) {
    ledcWriteTone(0, freq);
    delay(50);
  }
  ledcWriteTone(0, 0);
}

void playGoalSound() {
  // Triumph-Melodie
  int melody[] = {523, 659, 784, 1047};  // C, E, G, C (oktave höher)
  for (int note : melody) {
    ledcWriteTone(0, note);
    delay(200);
  }
  ledcWriteTone(0, 0);
}

// In updatePhysics() bei Kollision:
if (dist > maxDist) {
  // ... bounce code
  playBounceSound();
}
```

---

### 6. 📊 Gyro-Integration (Sensor Fusion)

**Idee:** Kombiniere Accelerometer + Gyro für präzisere Winkel!

```cpp
// Complementary Filter
float angleX_accel = 0;
float angleX_gyro = 0;
float angleX_fused = 0;

const float ALPHA = 0.98;  // Filter-Gewicht

void updateAngles(float dt) {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Accelerometer-Winkel (statisch genau)
  angleX_accel = atan2(a.acceleration.y, a.acceleration.z) * 57.2958;

  // Gyro-Winkel (integriert, dynamisch gut)
  angleX_gyro += g.gyro.x * dt * 57.2958;  // Gyro in °/s

  // Complementary Filter: Mische beide
  angleX_fused = ALPHA * (angleX_fused + g.gyro.x * dt * 57.2958) +
                 (1 - ALPHA) * angleX_accel;

  // angleX_fused ist genauer als angleX_accel alleine!
}
```

**Warum Sensor Fusion?**

```
Accelerometer:
  ✅ Langfristig genau (keine Drift)
  ❌ Anfällig für Vibrationen/Stöße

Gyroscope:
  ✅ Schnelle Bewegungen präzise
  ❌ Driftet über Zeit

Complementary Filter:
  ✅ Beste von beiden!
  ✅ Schnell + genau + wenig Drift
```

---

### 7. 🌡️ Temperatur-Anzeige

**Idee:** MPU6050 hat auch Temperatursensor!

```cpp
void readTemperature() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float celsius = temp.temperature;
  float fahrenheit = celsius * 9.0 / 5.0 + 32.0;

  Serial.printf("Temp: %.1f°C (%.1f°F)\n", celsius, fahrenheit);

  // Auf Display anzeigen
  lcd.setCursor(10, 270);
  lcd.printf("Temp: %.1f", celsius);
  lcd.print((char)247);  // Grad-Symbol
  lcd.print("C");
}
```

**Genauigkeit:** ±2°C (nicht präzise, aber gut für Umgebung)

---

## 🐛 Troubleshooting

### Problem: "MPU6050 not found!"

**Symptom:** Display zeigt Error-Screen mit Verkabelung.

**Ursachen:**
1. MPU6050 nicht angeschlossen
2. Falsche Verkabelung
3. Falsche I2C-Adresse
4. Defektes Modul

**Debug:**

```cpp
void debugI2C() {
  Wire.begin(extSDA, extSCL);

  Serial.println("Scanning I2C...");

  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.printf("Device found at 0x%02X\n", addr);
    }
  }

  Serial.println("Scan complete");
}
```

**Lösung:**
- Überprüfe Verkabelung (VCC, GND, SDA, SCL)
- **WICHTIG:** VCC an 3.3V (NICHT 5V!)
- Teste mit anderem Modul
- Überprüfe I2C-Adresse (0x68 oder 0x69)

---

### Problem: Kugel bewegt sich nicht

**Symptom:** Kugel bleibt in Mitte, egal wie geneigt.

**Ursachen:**
1. Sensor-Werte sind 0
2. GRAVITY_SCALE zu klein
3. FRICTION zu hoch
4. Kalibrierung falsch

**Debug:**

```cpp
void loop() {
  readSensor();

  Serial.printf("AccelX: %.3f, AccelY: %.3f\n", accelX, accelY);
  Serial.printf("VelX: %.3f, VelY: %.3f\n", velocityX, velocityY);
  Serial.printf("PosX: %.1f, PosY: %.1f\n", ballX, ballY);

  // ...
}
```

**Lösung:**
- Erhöhe GRAVITY_SCALE (z.B. 20.0)
- Reduziere FRICTION (z.B. 0.99)
- Kalibriere neu (Touch)

---

### Problem: Kugel ist zu schnell/unkontrollierbar

**Symptom:** Kleinste Neigung → Kugel rast wild herum.

**Lösung:**

```cpp
const float GRAVITY_SCALE = 8.0;   // Reduzieren!
const float FRICTION = 0.95;       // Mehr Reibung
const float MAX_VELOCITY = 5.0;    // Niedrigeres Limit
```

---

### Problem: Kugel "vibriert" oder zittert

**Symptom:** Kugel bewegt sich ruckartig, auch bei stillhalten.

**Ursachen:**
1. Sensor-Rauschen
2. Zu hoher GRAVITY_SCALE
3. Kein Lowpass-Filter

**Lösung:**

```cpp
// Lowpass-Filter für Sensor-Daten
float filteredAccelX = 0;
float filteredAccelY = 0;
const float FILTER_ALPHA = 0.8;

void readSensor() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Lowpass: neue_wert = alpha * alt + (1-alpha) * neu
  filteredAccelX = FILTER_ALPHA * filteredAccelX +
                   (1 - FILTER_ALPHA) * (a.acceleration.x - offsetX);

  filteredAccelY = FILTER_ALPHA * filteredAccelY +
                   (1 - FILTER_ALPHA) * (a.acceleration.y - offsetY);

  accelX = filteredAccelX;
  accelY = filteredAccelY;
}
```

---

### Problem: "LEVEL" wird nie erreicht

**Symptom:** Auch bei waagerecht steht nie "LEVEL".

**Ursache:** LEVEL_THRESHOLD zu streng (z.B. 0.5°).

**Lösung:**

```cpp
const float LEVEL_THRESHOLD = 3.0;  // Erhöhen auf 3°
```

Oder visualisiere aktuellen Winkel:

```cpp
Serial.printf("Total Angle: %.2f° (Threshold: %.2f°)\n",
              totalAngle, LEVEL_THRESHOLD);
```

---

### Problem: I2C-Fehler "SDA/SCL not defined"

**Symptom:** Compiler-Error bei `Wire.begin(extSDA, extSCL)`.

**Lösung:** Füge in `CYD_Display_Config.h` hinzu:

```cpp
#ifndef extSDA
#define extSDA 22
#endif

#ifndef extSCL
#define extSCL 27
#endif
```

Oder direkt im Code (wie im Beispiel):

```cpp
#ifndef extSDA
#define extSDA 22
#endif
```

---

## 📚 Weiterführende Konzepte

### 1. Kalman-Filter

**Noch präzisere Sensor-Fusion:**

```cpp
// Vereinfachter Kalman-Filter für Winkel
class KalmanFilter {
  float Q_angle = 0.001;
  float Q_bias = 0.003;
  float R_measure = 0.03;

  float angle = 0;
  float bias = 0;
  float P[2][2] = {{0, 0}, {0, 0}};

public:
  float getAngle(float newAngle, float newRate, float dt) {
    // Predict
    angle += dt * (newRate - bias);
    P[0][0] += dt * (dt*P[1][1] - P[0][1] - P[1][0] + Q_angle);
    P[0][1] -= dt * P[1][1];
    P[1][0] -= dt * P[1][1];
    P[1][1] += Q_bias * dt;

    // Update
    float S = P[0][0] + R_measure;
    float K[2];
    K[0] = P[0][0] / S;
    K[1] = P[1][0] / S;

    float y = newAngle - angle;
    angle += K[0] * y;
    bias += K[1] * y;

    float P00_temp = P[0][0];
    float P01_temp = P[0][1];

    P[0][0] -= K[0] * P00_temp;
    P[0][1] -= K[0] * P01_temp;
    P[1][0] -= K[1] * P00_temp;
    P[1][1] -= K[1] * P01_temp;

    return angle;
  }
};
```

### 2. Madgwick-Filter

**Quaternion-basierte Sensor-Fusion (3D-Orientierung):**

Siehe [Madgwick Library](https://github.com/arduino-libraries/MadgwickAHRS).

### 3. Verlet-Integration

**Alternative zu Euler (genauer für Physik):**

```cpp
// Verlet Integration (keine explizite Velocity)
float ballX_old = ballX;
float ballY_old = ballY;

void updatePhysicsVerlet(float dt) {
  float accelForceX = -accelX * GRAVITY_SCALE;
  float accelForceY = accelY * GRAVITY_SCALE;

  // Verlet: x_new = 2*x - x_old + a*dt²
  float ballX_new = 2*ballX - ballX_old + accelForceX * dt * dt;
  float ballY_new = 2*ballY - ballY_old + accelForceY * dt * dt;

  ballX_old = ballX;
  ballY_old = ballY;

  ballX = ballX_new;
  ballY = ballY_new;
}
```

**Vorteil:** Stabiler, weniger Drift bei langen Simulationen.

---

## 🎓 Was hast du gelernt?

Nach diesem Beispiel verstehst du:

- ✅ **I2C-Kommunikation** - Hardware-Bus für Sensoren
- ✅ **MPU6050 Sensor** - Accelerometer & Gyro
- ✅ **Physik-Simulation** - Euler-Integration, Reibung, Kollision
- ✅ **3D-Grafik** - Radial-Gradient, Schatten, Licht
- ✅ **Kalibrierung** - Sensor-Offset kompensieren
- ✅ **Delta-Time** - Frame-rate unabhängige Animation
- ✅ **Vektor-Mathematik** - Dot-Product, Normalisierung, Reflexion

**Nächste Schritte:**
1. Experimentiere mit verschiedenen Kugel-Designs
2. Implementiere Labyrinth-Spiel
3. Füge Sound-Effekte hinzu
4. Kombiniere mit Gyro (Sensor Fusion)

---

## 🔗 Siehe auch

- [MPU6050 Datasheet](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf)
- [I2C Protocol Explained](https://www.i2c-bus.org/i2c-primer/)
- [Adafruit MPU6050 Guide](https://learn.adafruit.com/mpu6050-6-dof-accelerometer-and-gyro)

**Externe Ressourcen:**
- [Physics Simulation Tutorial](https://www.toptal.com/game/video-game-physics-part-i-an-introduction-to-rigid-body-dynamics)
- [Sensor Fusion Guide](https://www.nxp.com/docs/en/application-note/AN3461.pdf)

---

**Realistische Physik trifft schöne Grafik! ⚖️✨**
