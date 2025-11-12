# CYD Game Menu - Übersicht aller Arcade-Spiele

Ein Touch-basiertes Menü für alle CYD Arcade-Spiele im Repository.

## 📋 Verfügbare Spiele

| Nr. | Spiel | Ordner | Modus |
|-----|-------|--------|-------|
| 12 | Pong | `12_Pong` | Landscape |
| 12b | Pong Modern | `12b_Pong_Modern` | Landscape |
| 13 | Snake | `13_Snake` | Landscape |
| 13b | Snake Modern | `13b_Snake_Modern` | Landscape |
| 14 | Breakout | `14_Breakout` | Landscape |
| 14b | Breakout Modern | `14b_Breakout_Modern` | Landscape |
| 15 | Space Invaders | `15_SpaceInvaders` | Landscape |
| 15b | Space Invaders Modern | `15b_SpaceInvaders_Modern` | Landscape |
| 16 | Asteroids | `16_Asteroids` | Landscape |
| 17 | Frogger | `17_Frogger` | Landscape |
| 18 | Tetris | `18_Tetris` | **Portrait** |

## 🎮 Wie benutzen

### Option 1: Menu als Übersicht (Empfohlen)

1. **Menu auf CYD laden:**
   - Öffne `examples/00_GameMenu/00_GameMenu.ino` in Arduino IDE
   - Upload auf dein CYD Board
   - Das Menu zeigt alle verfügbaren Spiele

2. **Spiel auswählen:**
   - Touch ein Spiel um Details zu sehen
   - Notiere den Ordner-Namen

3. **Spiel laden:**
   - Öffne die entsprechende `.ino` Datei aus dem Spiele-Ordner
   - Upload auf dein CYD Board
   - Spiel starten!

### Option 2: Direktes Laden (Schnell)

Wenn du schon weißt welches Spiel du spielen willst:

1. Öffne direkt `examples/XX_GameName/XX_GameName.ino`
2. Upload auf CYD Board
3. Spielen!

## 🔄 Sync-Script (sync-games.ps1)

Das PowerShell Script hilft dir bei der Verwaltung aller Spiele.

### Verwendung:

```powershell
# Status aller Spiele anzeigen
.\sync-games.ps1

# Backup aller Spiele erstellen
.\sync-games.ps1 -CreateBackup
```

### Was macht das Script?

- ✅ Zeigt Status aller Spiele (Ordner, letzte Änderung, Dateigröße)
- ✅ Prüft Git Status
- ✅ Erstellt optional Backups
- ✅ Gibt Übersicht über den Workflow

## 🤔 Warum gibt es KEINE Merge-Konflikte?

**TL;DR:** Jedes Spiel ist in seinem eigenen Ordner → Keine Duplikate → Keine Konflikte!

### Das Konzept: "Single Source of Truth"

```
examples/
├── 00_GameMenu/          ← Menu (nur Anzeige)
│   └── 00_GameMenu.ino
├── 12_Pong/              ← Original-Spiel #1
│   └── 12_Pong.ino
├── 13_Snake/             ← Original-Spiel #2
│   └── 13_Snake.ino
└── ...

Jedes Spiel = EIN Ordner = EINE .ino Datei
```

### Warum keine Konflikte?

#### 1. **Keine Duplikate**
- Jedes Spiel existiert nur EINMAL im Repository
- Das Menu kopiert NICHTS, es zeigt nur eine Liste
- Es gibt keine "modifizierte Kopie" die mit dem Original kollidieren könnte

#### 2. **Getrennte Ordner**
- Änderungen an Pong → Nur `12_Pong/12_Pong.ino` betroffen
- Änderungen an Snake → Nur `13_Snake/13_Snake.ino` betroffen
- Änderungen am Menu → Nur `00_GameMenu/00_GameMenu.ino` betroffen
- Git sieht das als **komplett separate Dateien**

#### 3. **Klarer Workflow**
```
1. Du änderst ein Spiel:        examples/17_Frogger/17_Frogger.ino
2. Du committest:               git commit -m "FIX: Frogger - Bug fix"
3. Du pushst:                   git push
4. Fertig!
```

Keine Synchronisation, kein Merging, kein Konflikt!

#### 4. **Menu ist passiv**
- Das Menu **liest nicht** den Code der Spiele
- Das Menu **ändert nicht** den Code der Spiele
- Das Menu **kopiert nicht** den Code der Spiele
- Das Menu zeigt nur: "Es gibt Spiel X im Ordner Y"

#### 5. **Git-Workflow**
Konflikte entstehen nur wenn:
- **Zwei Personen** die **gleiche Zeile** in der **gleichen Datei** ändern
- **Zur gleichen Zeit**

Bei uns:
- ✅ Jedes Spiel in separater Datei
- ✅ Menu in separater Datei
- ✅ Keine gemeinsamen Code-Zeilen
- ✅ **→ Kein Konflikt möglich!**

### Beispiel-Scenario

**Situation:** Du verbesserst Frogger und Tetris gleichzeitig

```bash
# Vorher
examples/17_Frogger/17_Frogger.ino  ← Version 1
examples/18_Tetris/18_Tetris.ino    ← Version 1

# Du änderst beide
examples/17_Frogger/17_Frogger.ino  ← Version 2 (Bug fix)
examples/18_Tetris/18_Tetris.ino    ← Version 2 (Button fix)

# Git sieht:
# - Datei A geändert
# - Datei B geändert
# → Kein Problem, das sind verschiedene Dateien!

git add .
git commit -m "FIX: Frogger + Tetris improvements"
git push
# ✅ Kein Konflikt!
```

**Konflikt würde nur entstehen wenn:**
- Person A ändert Zeile 50 in `17_Frogger.ino`
- Person B ändert Zeile 50 in `17_Frogger.ino`
- **Zur gleichen Zeit** (parallel branches)
- Dann: Git kann nicht entscheiden welche Version richtig ist

**Lösung:** Du arbeitest alleine → Kein Problem!

## 📝 Entwicklungs-Workflow

### Spiel verbessern

```bash
# 1. Spiel-Datei öffnen
code examples/17_Frogger/17_Frogger.ino

# 2. Änderungen machen
# (z.B. Bug fix, neue Features, etc.)

# 3. Testen mit Arduino IDE
# (Upload & Test auf CYD Board)

# 4. Committen
git add examples/17_Frogger/
git commit -m "FIX: Frogger - Logs carry frog correctly"

# 5. Pushen
git push
```

### Menu aktualisieren

Nur wenn ein **neues Spiel** hinzugefügt wurde:

```bash
# 1. Menu-Datei öffnen
code examples/00_GameMenu/00_GameMenu.ino

# 2. Neues Spiel zur games[] Array hinzufügen
# {"SpielName", "19_NewGame", "Beschreibung", 0xCOLOR}

# 3. Committen
git add examples/00_GameMenu/
git commit -m "UPDATE: Menu - Added NewGame"

# 4. Pushen
git push
```

## 🛠️ Technische Details

### Menu-Programm

- **Sprache:** Arduino C++
- **Display:** 320x240 Landscape
- **Touch:** XPT2046 Touch Controller
- **Features:**
  - Touch-basierte Spiele-Liste
  - Farbige Icons für jedes Spiel
  - Detail-Ansicht mit Upload-Anleitung
  - Scroll-Funktion (wenn viele Spiele)

### Dateigröße

- Menu: ~10 KB
- Typisches Spiel: 10-25 KB
- Alles passt problemlos auf ESP32

## ❓ FAQ

**Q: Muss ich das Sync-Script ausführen nach jeder Änderung?**
A: Nein! Das Script ist optional und nur zur Übersicht.

**Q: Kann ich Spiele direkt laden ohne das Menu?**
A: Ja! Öffne einfach direkt die .ino Datei und upload sie.

**Q: Was passiert wenn ich ein Spiel ändere?**
A: Nichts spezielles. Ändere die Datei, committe, pushe. Fertig!

**Q: Kann ich eigene Spiele hinzufügen?**
A: Ja! Erstelle einen neuen Ordner `examples/19_MeinSpiel/` und füge es zum Menu hinzu.

**Q: Warum ist Tetris im Portrait-Modus?**
A: Tetris funktioniert besser im vertikalen Format (wie beim Original). Die Buttons sind entsprechend angepasst.

## 🎯 Zusammenfassung

✅ **Ein Menu** für alle Spiele
✅ **Einfacher Workflow** ohne Komplikationen
✅ **Keine Merge-Konflikte** durch clevere Struktur
✅ **Sync-Script** für Übersicht (optional)
✅ **Flexibel:** Menu nutzen ODER Spiel direkt laden

Viel Spaß beim Spielen! 🎮
