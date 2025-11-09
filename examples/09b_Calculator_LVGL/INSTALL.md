# Installation: LVGL Calculator

Dieses Beispiel benötigt **LVGL v8.3.11**.

Die benötigte `lv_conf.h` ist **bereits im Projekt-Verzeichnis enthalten** - keine manuelle Konfiguration nötig!

## Schritt 1: LVGL installieren

1. Arduino IDE → `Tools` → `Manage Libraries...`
2. Suche nach "lvgl"
3. Installiere **LVGL by LVGL** Version **8.3.11**

## Schritt 2: Kompilieren

1. `09b_Calculator_LVGL.ino` öffnen
2. Board: **ESP32 Dev Module** auswählen
3. Upload

## Troubleshooting

### Fehler: "lv_conf.h: No such file or directory"
→ Die `lv_conf.h` sollte im gleichen Ordner wie die `.ino` Datei liegen
→ Falls du das Projekt neu geklont hast, stelle sicher dass alle Dateien vorhanden sind
→ Versuche Arduino IDE neu zu starten

### Display bleibt schwarz
→ Prüfe Serial Monitor für Fehlermeldungen
→ Stelle sicher dass `CYD_Display_Config.h` korrekt ist

### Compile-Fehler in LVGL
→ Stelle sicher dass du **Version 8.3.11** von LVGL installiert hast (nicht v9.x)
→ Arduino IDE neu starten nach Library-Installation
