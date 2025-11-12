# CYD Games Sync Script
# Synchronisiert alle Spiele-Dateien und stellt sicher dass Änderungen übernommen werden
#
# Verwendung:
#   .\sync-games.ps1
#
# Was macht dieses Script:
# 1. Prüft ob es Updates in den einzelnen Spiele-Ordnern gibt
# 2. Zeigt eine Übersicht aller Spiele und deren letztes Änderungsdatum
# 3. Optional: Erstellt Backups vor dem Überschreiben
#
# WARUM GIBT ES KEINE MERGE-KONFLIKTE?
# ====================================
#
# Das Script arbeitet mit einer "One-Way-Sync" Strategie:
#
# 1. SINGLE SOURCE OF TRUTH:
#    - Jedes Spiel hat GENAU EINEN Quell-Ordner (z.B. examples/12_Pong/)
#    - Änderungen werden IMMER im Original-Ordner gemacht
#    - Das Menu (00_GameMenu) referenziert nur, ändert aber nie die Spiele
#
# 2. KEINE DUPLIKATE:
#    - Spiele werden NICHT kopiert oder dupliziert
#    - Es gibt keine "modifizierte Kopie" die mit dem Original kollidieren könnte
#    - Das Menu ist nur eine Übersicht, keine Code-Integration
#
# 3. KLARE VERANTWORTLICHKEITEN:
#    - Original-Spiele in examples/XX_GameName/ → Hier werden Änderungen gemacht
#    - Menu in examples/00_GameMenu/ → Nur Anzeige, keine Spiel-Logik
#    - Beide sind komplett getrennt
#
# 4. GIT-WORKFLOW:
#    - Jede Änderung wird einzeln committed (z.B. "FIX: Frogger - Logs fix")
#    - Das Menu wird separat committed wenn es geändert wird
#    - Keine automatischen Merges zwischen Spiel-Code und Menu
#
# 5. WENN DOCH EIN KONFLIKT ENTSTEHT:
#    - Das ist nur möglich wenn zwei Personen GLEICHZEITIG am selben Spiel arbeiten
#    - Git zeigt dann den Konflikt an
#    - Du entscheidest welche Version du behalten willst
#    - Der Konflikt ist im ORIGINAL-File, nicht durch das Sync-Script
#
# BEISPIEL WORKFLOW:
# ==================
# 1. Du verbesserst Frogger → Änderungen in examples/17_Frogger/17_Frogger.ino
# 2. Du commitest: git commit -m "FIX: Frogger - Logs fix"
# 3. Du pushst: git push
# 4. Das Menu zeigt automatisch den richtigen Ordner an
# 5. Kein Sync nötig - alles funktioniert!
#
# Das Sync-Script ist also eigentlich OPTIONAL und nur für:
# - Übersicht über alle Spiele
# - Prüfung wann etwas zuletzt geändert wurde
# - Backup-Erstellung
#

param(
    [switch]$ShowStatus,    # Zeigt nur den Status, macht keine Änderungen
    [switch]$CreateBackup   # Erstellt Backups vor Änderungen
)

# Farben für Output
$ColorInfo = "Cyan"
$ColorSuccess = "Green"
$ColorWarning = "Yellow"
$ColorError = "Red"

Write-Host "`n=== CYD GAMES SYNC SCRIPT ===" -ForegroundColor $ColorInfo
Write-Host ""

# Spiele-Ordner definieren
$games = @(
    @{Name="Pong"; Folder="12_Pong"},
    @{Name="Pong Modern"; Folder="12b_Pong_Modern"},
    @{Name="Snake"; Folder="13_Snake"},
    @{Name="Snake Modern"; Folder="13b_Snake_Modern"},
    @{Name="Breakout"; Folder="14_Breakout"},
    @{Name="Breakout Modern"; Folder="14b_Breakout_Modern"},
    @{Name="Space Invaders"; Folder="15_SpaceInvaders"},
    @{Name="Space Invaders Modern"; Folder="15b_SpaceInvaders_Modern"},
    @{Name="Asteroids"; Folder="16_Asteroids"},
    @{Name="Frogger"; Folder="17_Frogger"},
    @{Name="Tetris"; Folder="18_Tetris"}
)

# Basis-Pfad
$basePath = $PSScriptRoot
$examplesPath = Join-Path $basePath "examples"

# Status-Übersicht
Write-Host "STATUS ALLER SPIELE:" -ForegroundColor $ColorInfo
Write-Host ("=" * 70)
Write-Host ""

foreach ($game in $games) {
    $gamePath = Join-Path $examplesPath $game.Folder
    $inoFile = Get-ChildItem -Path $gamePath -Filter "*.ino" -ErrorAction SilentlyContinue | Select-Object -First 1

    if ($inoFile) {
        $lastModified = $inoFile.LastWriteTime
        $fileSize = [math]::Round($inoFile.Length / 1KB, 1)

        Write-Host ("[{0,-25}]" -f $game.Name) -NoNewline -ForegroundColor $ColorSuccess
        Write-Host (" {0,-20}" -f $game.Folder) -NoNewline
        Write-Host (" | " + $lastModified.ToString("yyyy-MM-dd HH:mm:ss")) -NoNewline
        Write-Host (" | {0,6} KB" -f $fileSize)
    } else {
        Write-Host ("[{0,-25}]" -f $game.Name) -NoNewline -ForegroundColor $ColorWarning
        Write-Host (" {0,-20}" -f $game.Folder) -NoNewline
        Write-Host (" | NICHT GEFUNDEN!" -ForegroundColor $ColorError)
    }
}

Write-Host ""
Write-Host ("=" * 70)

# Git Status prüfen
Write-Host "`nGIT STATUS:" -ForegroundColor $ColorInfo
Write-Host ("=" * 70)

try {
    $gitStatus = git status --short 2>&1

    if ($LASTEXITCODE -eq 0) {
        if ($gitStatus) {
            Write-Host $gitStatus
            Write-Host ""
            Write-Host "Es gibt uncommittete Änderungen!" -ForegroundColor $ColorWarning
        } else {
            Write-Host "Working tree ist sauber - keine uncommitteten Änderungen" -ForegroundColor $ColorSuccess
        }
    } else {
        Write-Host "Kein Git Repository oder Git nicht installiert" -ForegroundColor $ColorWarning
    }
} catch {
    Write-Host "Git Status konnte nicht geprüft werden" -ForegroundColor $ColorWarning
}

Write-Host ""
Write-Host ("=" * 70)

# Zusammenfassung
Write-Host "`nZUSAMMENFASSUNG:" -ForegroundColor $ColorInfo
Write-Host "- Alle Spiele sind in ihren eigenen Ordnern (Single Source of Truth)" -ForegroundColor $ColorSuccess
Write-Host "- Das Menu (00_GameMenu) zeigt nur die Übersicht" -ForegroundColor $ColorSuccess
Write-Host "- Änderungen werden DIREKT im Original-Ordner gemacht" -ForegroundColor $ColorSuccess
Write-Host "- Keine Duplikate = Keine Merge-Konflikte!" -ForegroundColor $ColorSuccess
Write-Host ""
Write-Host "WORKFLOW:" -ForegroundColor $ColorInfo
Write-Host "1. Ändere ein Spiel im examples/XX_GameName/ Ordner"
Write-Host "2. Teste es mit der Arduino IDE"
Write-Host "3. Commit die Änderungen: git add . && git commit -m 'FIX: GameName - ...'"
Write-Host "4. Push: git push"
Write-Host "5. Fertig! Keine Sync nötig."
Write-Host ""

# Backup-Option
if ($CreateBackup) {
    Write-Host "`nERSTELLE BACKUPS..." -ForegroundColor $ColorInfo
    $backupPath = Join-Path $basePath "backups"
    $backupFolder = Join-Path $backupPath (Get-Date -Format "yyyy-MM-dd_HH-mm-ss")

    New-Item -Path $backupFolder -ItemType Directory -Force | Out-Null

    foreach ($game in $games) {
        $gamePath = Join-Path $examplesPath $game.Folder
        if (Test-Path $gamePath) {
            $destPath = Join-Path $backupFolder $game.Folder
            Copy-Item -Path $gamePath -Destination $destPath -Recurse -Force
            Write-Host "  ✓ $($game.Name) gesichert" -ForegroundColor $ColorSuccess
        }
    }

    Write-Host "`nBackup erstellt in: $backupFolder" -ForegroundColor $ColorSuccess
}

Write-Host ""
