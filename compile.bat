@echo off
setlocal enabledelayedexpansion

echo setting up compile command and flags

:: Compiler und Flags setzen
set compiler=g++
set FLAGS= 
set exeName=Furfront
::! Statische Verlinkung erzwingen (bei fehlern ggf. -static weglassen)
:: Alle benötigten SFML Module sowie Abhängigkeiten verlinken
set linkingFlags=-static -lSDL3 -lSDL3_image -lSDL3_mixer -lSDL3_net -lSDL3_rtf -lSDL3_ttf -lfreetype -lharfbuzz -luser32 -lgdi32 -lole32 -luuid -loleaut32 -limm32 -ldinput8 -lwinmm -lshell32 -lsetupapi -lversion -lwininet -lws2_32

:: Include- und Library-Pfade setzen
set lAndIPaths=-I"./include" -I"F:/Dropbox/Dropbox/CPP_LIBARIES/hppLibs" -I"F:/Dropbox/Dropbox/CPP_LIBARIES/windows/include/" -L"F:/Dropbox/Dropbox/CPP_LIBARIES/windows/lib/"

:: Setzen des Compile-Commands inkl. statischer Verlinkung
set compileCommand=%compiler% main.cpp -o %exeName%.exe %lAndIPaths% %linkingFlags%

echo Compiling %exeName% Script ...

echo %compileCommand%

%compileCommand%

:: Fehlerprüfung
if %errorlevel% neq 0 (
    echo Fehler: Compiling Not successful!
    pause
    exit /b 1
)

echo Compilation successful.

:: Prüfen, ob Datei existiert
if not exist %exeName%.exe (
    echo Fehler: %exeName%.exe was not created!
    pause
    exit /b 1
)

:: Nachfragen, ob das Programm gestartet werden soll
set /p runTest=Enter 't' to run the program for tests: 
if /i "%runTest%" == "t" (
    echo Running: %exeName%.exe %FLAGS%
    %exeName%.exe %FLAGS%
) else (
    echo Skipping test run.
)

pause
exit
