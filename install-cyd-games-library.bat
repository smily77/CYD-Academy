@echo off
REM ============================================
REM CYD_Games Library Symlink Creator
REM ============================================
REM Erstellt einen symbolischen Link zur CYD_Games Library
REM im Arduino Libraries Verzeichnis
REM
REM WICHTIG: Muss als Administrator ausgeführt werden!
REM ============================================

echo.
echo ========================================
echo CYD_Games Library Symlink Creator
echo ========================================
echo.

REM Prüfe ob als Administrator gestartet
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo FEHLER: Dieses Script muss als Administrator ausgefuehrt werden!
    echo.
    echo Rechtsklick auf die Datei -^> "Als Administrator ausfuehren"
    echo.
    pause
    exit /b 1
)

REM Aktuelles Verzeichnis (sollte CYD-Academy Root sein)
set "SOURCE_DIR=%~dp0libraries\CYD_Games"

REM Arduino Libraries Verzeichnis (Standard Windows Pfad)
set "ARDUINO_LIBS=%USERPROFILE%\Documents\Arduino\libraries"

echo Quelle:      %SOURCE_DIR%
echo Ziel:        %ARDUINO_LIBS%\CYD_Games
echo.

REM Prüfe ob Quell-Verzeichnis existiert
if not exist "%SOURCE_DIR%" (
    echo FEHLER: CYD_Games Library nicht gefunden!
    echo Stelle sicher dass du dieses Script im CYD-Academy Root-Verzeichnis ausfuehrst.
    echo.
    pause
    exit /b 1
)

REM Prüfe ob Arduino Libraries Verzeichnis existiert
if not exist "%ARDUINO_LIBS%" (
    echo FEHLER: Arduino Libraries Verzeichnis nicht gefunden!
    echo Erwartet: %ARDUINO_LIBS%
    echo.
    echo Bitte erstelle das Verzeichnis oder passe den Pfad im Script an.
    echo.
    pause
    exit /b 1
)

REM Prüfe ob bereits ein Symlink oder Verzeichnis existiert
if exist "%ARDUINO_LIBS%\CYD_Games" (
    echo WARNUNG: %ARDUINO_LIBS%\CYD_Games existiert bereits!
    echo.
    choice /C JN /M "Soll es geloescht und neu erstellt werden"
    if errorlevel 2 (
        echo.
        echo Abgebrochen.
        pause
        exit /b 0
    )

    echo.
    echo Loesche alten Eintrag...
    rmdir "%ARDUINO_LIBS%\CYD_Games" 2>nul
    del "%ARDUINO_LIBS%\CYD_Games" 2>nul
)

REM Erstelle Symlink
echo.
echo Erstelle symbolischen Link...
mklink /D "%ARDUINO_LIBS%\CYD_Games" "%SOURCE_DIR%"

if %errorLevel% equ 0 (
    echo.
    echo ========================================
    echo ERFOLG!
    echo ========================================
    echo.
    echo Symbolischer Link wurde erstellt:
    echo %ARDUINO_LIBS%\CYD_Games
    echo   --^> verweist auf --^>
    echo %SOURCE_DIR%
    echo.
    echo Arduino IDE sollte die Library jetzt finden!
    echo.
    echo Naechste Schritte:
    echo 1. Arduino IDE neu starten
    echo 2. Sketch -^> Include Library -^> CYD_Games sollte erscheinen
    echo 3. Beispiel kompilieren: examples/12_Pong/12_Pong.ino
    echo.
) else (
    echo.
    echo FEHLER: Konnte Symlink nicht erstellen!
    echo.
    echo Moegliche Ursachen:
    echo - Nicht als Administrator gestartet
    echo - Pfad enthaelt Sonderzeichen
    echo - Dateisystem unterstuetzt keine Symlinks
    echo.
)

pause
