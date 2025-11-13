# CYD_Games Library Installation

Anleitung zur Installation der CYD_Games Library für Arduino IDE.

## 🎯 Empfohlene Methode: Symlink (Verweis)

Ein symbolischer Link verbindet die Library im Repository direkt mit Arduino Libraries.
**Vorteil:** Keine Code-Duplikation, Git Updates wirken sofort!

### Windows - Automatische Installation

#### Option 1: PowerShell Script (Empfohlen)

1. **Rechtsklick** auf `install-cyd-games-library.ps1`
2. Wähle **"Mit PowerShell als Administrator ausführen"**
3. Folge den Anweisungen
4. Arduino IDE neu starten

#### Option 2: Batch Script

1. **Rechtsklick** auf `install-cyd-games-library.bat`
2. Wähle **"Als Administrator ausführen"**
3. Folge den Anweisungen
4. Arduino IDE neu starten

### Windows - Manuelle Installation

1. Öffne **Eingabeaufforderung als Administrator**:
   - Windows-Taste drücken
   - "cmd" eingeben
   - **Rechtsklick** auf "Eingabeaufforderung"
   - **"Als Administrator ausführen"**

2. Navigiere zum CYD-Academy Verzeichnis:
   ```cmd
   cd C:\Users\<DeinName>\Documents\GitHub\CYD-Academy
   ```

3. Erstelle Symlink:
   ```cmd
   mklink /D "%USERPROFILE%\Documents\Arduino\libraries\CYD_Games" "%CD%\libraries\CYD_Games"
   ```

4. Erfolg prüfen:
   ```cmd
   dir "%USERPROFILE%\Documents\Arduino\libraries\"
   ```
   Du solltest `CYD_Games <SYMLINK>` sehen.

5. Arduino IDE neu starten

### Symlink überprüfen

**Erfolgreich:** Im Windows Explorer siehst du bei `Arduino\libraries\CYD_Games` ein kleines Pfeil-Symbol.

**Im Zweifel:**
```cmd
dir "%USERPROFILE%\Documents\Arduino\libraries\CYD_Games"
```
Sollte zeigen: `<SYMLINK>` oder `<JUNCTION>`

## 📦 Alternative: Kopieren (Nicht empfohlen)

Falls Symlinks nicht funktionieren:

1. Kopiere den ganzen Ordner:
   ```
   CYD-Academy/libraries/CYD_Games/
   ```

2. Füge ihn hier ein:
   ```
   C:\Users\<DeinName>\Documents\Arduino\libraries\CYD_Games\
   ```

3. Arduino IDE neu starten

**Nachteil:** Updates aus Git werden NICHT automatisch übernommen!

## ✅ Installation überprüfen

### In Arduino IDE:

1. Arduino IDE öffnen
2. **Sketch** → **Include Library**
3. In der Liste sollte **"CYD_Games"** erscheinen

### Testprogramm kompilieren:

1. Öffne `examples/12_Pong/12_Pong.ino`
2. Klicke auf **Verify** (Haken-Symbol)
3. Sollte ohne Fehler kompilieren!

## 🔧 Troubleshooting

### Fehler: "PongGame.h: No such file or directory"

**Lösung 1:** Arduino IDE neu starten

**Lösung 2:** Prüfe ob Symlink korrekt ist:
```cmd
dir "%USERPROFILE%\Documents\Arduino\libraries\"
```
Du solltest `CYD_Games <SYMLINK>` sehen.

**Lösung 3:** Symlink neu erstellen:
```cmd
rmdir "%USERPROFILE%\Documents\Arduino\libraries\CYD_Games"
mklink /D "%USERPROFILE%\Documents\Arduino\libraries\CYD_Games" "C:\Users\<DeinName>\Documents\GitHub\CYD-Academy\libraries\CYD_Games"
```

### Fehler: "Sie benötigen Administratorrechte"

- Script als **Administrator** ausführen (Rechtsklick → "Als Administrator ausführen")
- Oder: Eingabeaufforderung als Administrator öffnen

### Symlink wird nicht angezeigt

Prüfe ob Entwicklermodus aktiviert ist (Windows 10/11):
1. **Einstellungen** → **Update & Sicherheit** → **Für Entwickler**
2. **Entwicklermodus** aktivieren
3. Script erneut ausführen

### Arduino Libraries Verzeichnis falsch

Standard-Pfad:
```
C:\Users\<DeinName>\Documents\Arduino\libraries\
```

Falls anders konfiguriert:
- Arduino IDE → **File** → **Preferences**
- Sieh nach "Sketchbook location"
- Libraries sind unter: `<Sketchbook>/libraries/`

## 📂 Library Struktur

Nach erfolgreicher Installation:

```
Arduino/libraries/CYD_Games/     ← Symlink
  ├── library.properties
  ├── README.md
  ├── keywords.txt
  └── src/
      ├── SnakeGame.h
      ├── PongGame.h
      └── BreakoutGame.h
```

## 🎮 Verwendung

Nach Installation kannst du die Library verwenden:

```cpp
#include <CYD_Display_Config.h>
#include <SnakeGame.h>

LGFX lcd;
SnakeGame game;

void setup() {
  lcd.init();
  game.init(&lcd);
}

void loop() {
  game.update();
}
```

## 📝 Weitere Beispiele

- `examples/12_Pong/` - Pong Standalone
- `examples/13_Snake/` - Snake Standalone
- `examples/14_Breakout/` - Breakout Standalone
- `examples/00_RetroGames/` - **Alle Spiele in einem Menu!**

## ❓ Fragen?

Bei Problemen:
1. Prüfe ob Symlink existiert
2. Arduino IDE neu starten
3. Als Administrator ausführen
4. Entwicklermodus aktivieren (Windows)
