# Snake Game

Klassisches Snake-Spiel für das CYD Display mit Hardware-Button-Steuerung.

## Hardware

- **CYD Display** (320x240 Pixel)
- **4x Hardware-Buttons:**
  - tasteB: Links
  - tasteA: Oben
  - tasteC: Rechts
  - tasteD: Unten
  - (Buttons sind LOW wenn gedrückt)

**Hinweis:** Alle Pins (tasteA, tasteB, tasteC, tasteD) sind in `CYD_Display_Config.h` definiert.

## Steuerung

- Benutze die 4 Hardware-Buttons um die Richtung der Schlange zu steuern
- Sammle die roten Food-Items um zu wachsen
- Vermeide Kollisionen mit den Wänden und deinem eigenen Körper

## Spielregeln

- Jedes Food-Item gibt +10 Punkte und lässt die Schlange wachsen
- Die Schlange wird schneller, je länger sie wird
- Kollision mit Wand oder eigenem Körper = Game Over
- Bei Game Over: Beliebigen Button drücken zum Neustart

## Features

- Grid-basiertes Spielfeld (32x24)
- Progressive Geschwindigkeitserhöhung
- Score- und Längen-Anzeige
- Game Over Screen mit Neustart-Funktion
- Farbcodierung:
  - Grün: Schlangen-Körper
  - Cyan: Schlangen-Kopf
  - Rot: Food
  - Grau: Wände

## Technische Details

- Grid-Größe: 10x10 Pixel pro Zelle
- Start-Geschwindigkeit: 200ms pro Bewegung
- Min-Geschwindigkeit: 80ms pro Bewegung
- Max. Schlangen-Länge: 768 Segmente (32x24)
