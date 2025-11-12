#!/bin/sh

echo "Setting up compile command and flags"

# Compiler und Flags setzen
compiler="g++"
FLAGS=""
exeName="Furfront"

#! Statische Verlinkung erzwingen (bei fehlern ggf. -static weglassen)
# Alle benötigten SDL3-Module sowie Abhängigkeiten verlinken
linkingFlags="-static -lSDL3 -lSDL3_image -lSDL3_mixer -lSDL3_net -lSDL3_rtf -lSDL3_ttf "

# Include- und Library-Pfade setzen
lAndIPaths="-I./include -I"F:/Dropbox/Dropbox/CPP_LIBARIES/hppLibs" -I/home/lr6549/Dropbox/CPP_LIBARIES/linux/include/ -L/home/lr6549/Dropbox/CPP_LIBARIES/linux/lib/"

# Compile-Command
compileCommand="$compiler main.cpp -o ${exeName}.exe $lAndIPaths $linkingFlags"

echo "Compiling $exeName Script ..."
echo "$compileCommand"

# Kompilieren
eval "$compileCommand"
if [ $? -ne 0 ]; then
    echo "Fehler: Compiling not successful!"
    exit 1
fi

echo "Compilation successful."

# Prüfen, ob Datei existiert
if [ ! -f "${exeName}.exe" ]; then
    echo "Fehler: ${exeName}.exe was not created!"
    exit 1
fi

# Nachfragen, ob das Programm gestartet werden soll
echo -n "Enter 't' to run the program for tests: "
read runTest
if [ "$runTest" = "t" ] || [ "$runTest" = "T" ]; then
    echo "Running: ${exeName}.exe $FLAGS"
    "./${exeName}.exe" $FLAGS
else
    echo "Skipping test run."
fi

exit 0
