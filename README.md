# Asteroids

Game is written with C using Raylib for drawing stuff on the screen and Nob.h for building the project.

Check creative work of awesome people behind used technology:
    - Tsoding creator of Nob.h
    - Raysan5 creator of Raylib

Game is avilable for x86_64 both Windows and Linux.
Sounds were taken from Pixabay and are under OpenSource licence.

## Building

1. Build the build system
    ```cc -o build build.c```
2. Run build script for your OS
    - For linux: ```./build linux-native```
    - For windows: ```./build window-native```
    - For cross-compilation: ```./build linux-to-windows```
    - optionaly add `run` ot `run-exe` to run build result.
