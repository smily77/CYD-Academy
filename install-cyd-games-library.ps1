# ============================================
# CYD_Games Library Symlink Creator (PowerShell)
# ============================================
# Erstellt einen symbolischen Link zur CYD_Games Library
# im Arduino Libraries Verzeichnis
#
# WICHTIG: Muss als Administrator ausgeführt werden!
# ============================================

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "CYD_Games Library Symlink Creator" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Prüfe Administrator-Rechte
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "FEHLER: Dieses Script muss als Administrator ausgeführt werden!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Rechtsklick auf die Datei -> 'Mit PowerShell als Administrator ausführen'" -ForegroundColor Yellow
    Write-Host ""
    Read-Host "Drücke Enter zum Beenden"
    exit 1
}

# Pfade definieren
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$SourceDir = Join-Path $ScriptDir "libraries\CYD_Games"
$ArduinoLibs = Join-Path $env:USERPROFILE "Documents\Arduino\libraries"
$TargetLink = Join-Path $ArduinoLibs "CYD_Games"

Write-Host "Quelle:      $SourceDir" -ForegroundColor White
Write-Host "Ziel:        $TargetLink" -ForegroundColor White
Write-Host ""

# Prüfe ob Quelle existiert
if (-not (Test-Path $SourceDir)) {
    Write-Host "FEHLER: CYD_Games Library nicht gefunden!" -ForegroundColor Red
    Write-Host "Stelle sicher dass du dieses Script im CYD-Academy Root-Verzeichnis ausführst." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Erwartet: $SourceDir" -ForegroundColor Gray
    Write-Host ""
    Read-Host "Drücke Enter zum Beenden"
    exit 1
}

# Prüfe ob Arduino Libraries Verzeichnis existiert
if (-not (Test-Path $ArduinoLibs)) {
    Write-Host "FEHLER: Arduino Libraries Verzeichnis nicht gefunden!" -ForegroundColor Red
    Write-Host "Erwartet: $ArduinoLibs" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Bitte erstelle das Verzeichnis oder passe den Pfad im Script an." -ForegroundColor Yellow
    Write-Host ""
    Read-Host "Drücke Enter zum Beenden"
    exit 1
}

# Prüfe ob bereits existiert
if (Test-Path $TargetLink) {
    Write-Host "WARNUNG: $TargetLink existiert bereits!" -ForegroundColor Yellow
    Write-Host ""

    $item = Get-Item $TargetLink
    if ($item.Attributes -band [System.IO.FileAttributes]::ReparsePoint) {
        Write-Host "Es ist bereits ein Symlink vorhanden:" -ForegroundColor Cyan
        Write-Host "  Ziel: $($item.Target)" -ForegroundColor Gray
    } else {
        Write-Host "Es ist ein normales Verzeichnis (kein Symlink)!" -ForegroundColor Yellow
    }

    Write-Host ""
    $choice = Read-Host "Soll es gelöscht und neu erstellt werden? (J/N)"

    if ($choice -ne "J" -and $choice -ne "j") {
        Write-Host ""
        Write-Host "Abgebrochen." -ForegroundColor Yellow
        Read-Host "Drücke Enter zum Beenden"
        exit 0
    }

    Write-Host ""
    Write-Host "Lösche alten Eintrag..." -ForegroundColor Yellow

    try {
        Remove-Item $TargetLink -Force -Recurse -ErrorAction Stop
        Write-Host "Erfolgreich gelöscht." -ForegroundColor Green
    } catch {
        Write-Host "FEHLER beim Löschen: $_" -ForegroundColor Red
        Read-Host "Drücke Enter zum Beenden"
        exit 1
    }
}

# Erstelle Symlink
Write-Host ""
Write-Host "Erstelle symbolischen Link..." -ForegroundColor Cyan

try {
    New-Item -ItemType SymbolicLink -Path $TargetLink -Target $SourceDir -ErrorAction Stop | Out-Null

    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "ERFOLG!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Symbolischer Link wurde erstellt:" -ForegroundColor White
    Write-Host "  $TargetLink" -ForegroundColor Cyan
    Write-Host "    --> verweist auf -->" -ForegroundColor Gray
    Write-Host "  $SourceDir" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Arduino IDE sollte die Library jetzt finden!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Nächste Schritte:" -ForegroundColor Yellow
    Write-Host "  1. Arduino IDE neu starten" -ForegroundColor White
    Write-Host "  2. Sketch -> Include Library -> CYD_Games sollte erscheinen" -ForegroundColor White
    Write-Host "  3. Beispiel kompilieren: examples/12_Pong/12_Pong.ino" -ForegroundColor White
    Write-Host ""

} catch {
    Write-Host ""
    Write-Host "FEHLER: Konnte Symlink nicht erstellen!" -ForegroundColor Red
    Write-Host "Fehlermeldung: $_" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Mögliche Ursachen:" -ForegroundColor Yellow
    Write-Host "  - Nicht als Administrator gestartet" -ForegroundColor Gray
    Write-Host "  - Pfad enthält Sonderzeichen" -ForegroundColor Gray
    Write-Host "  - Dateisystem unterstützt keine Symlinks" -ForegroundColor Gray
    Write-Host ""
}

Read-Host "Drücke Enter zum Beenden"
