# Lince
[![License](https://img.shields.io/badge/License-MIT-mediumorchid)](https://www.github.com/jorgefz/lince/blob/main/LICENSE)

Lince is a graphical rendering engine focused on 2D games.
It is currently very early in development.

For some working minigames made with the Lince engine, see https://github.com/jorgefz/lince-minigames.

## Minimal code example - drawing a red square

```c
#include "lince.h"

LinceCamera* camera;
LinceSprite square = {
    .x = 0, .y = 0,     // position
    .w = 1, .h = 1,     // size
    .color = {1, 0, 0, 1} // rgba
};

void OnInit() {
	camera = LinceCreateCamera(LinceGetAspectRatio());
}

void OnUpdate(float dt) {
    LinceBeginScene(camera);
	LinceDrawSprite(&square, NULL);
    LinceEndScene();
}

void OnTerminate() {
	LinceDeleteCamera(camera);
}

int main() {

	LinceApp* app = LinceGetApp();
	app->on_init = OnInit;
	app->on_update = OnUpdate;
	app->on_terminate = OnTerminate;

	LinceRun();

	return 0;
}
```

## Features

* 2D graphics
* Cross-platform (Windows and Linux)
* Integrated GUI (nuklear)
* Audio system (Miniaudio)
* Event system for user input
* Tilemaps and sprite animations
* Lightweight Entity Component System (ECS) with unlimited entities and components
* Collision system for rectangles
* Built-in level and scene editor

## Why Lince

"Lince" means "lynx" in Spanish.
I chose this name to raise awareness of the Iberian Lynx, which is an endangered species in Spain, my home country.

## Documentation

You need to have Doxygen installed from https://www.doxygen.nl/ or via the command line on Linux:
```
sudo apt install doxygen
```
Then, simply run `doxygen`, and check out the HTML documentation on `doxydocs/html/index.html`.

## Dependencies

The project uses Premake5 by default as the build system (https://premake.github.io/).
It also requires OpenGL 4.5 or greater.

On Linux, the following packages are required:
```bash
sudo apt-get install make libx11-dev libxcursor-dev libxrandr-dev libxinerama-dev libxtst-dev libgl-dev uuid-dev`
```

If you are using Windows Subsystem for Linux on Windows 11 (WSLg), as of December 2022, you must manually connect ALSA to PulseAudio following the steps detailed on this thread: https://github.com/microsoft/wslg/issues/864.

On Windows, Visual Studo 2022 is recommended.

Lince uses CMocka as the testing framework.
On Windows, install it with vcpkg and integrate it with the following commands:
```
./path/to/vcpkg.exe install cmocka
./path/to/vcpkg.exe integrate install
```
On Linux, install it using a package manager:
```
sudo apt install libcmocka-dev
```

## Building Lince

See the projects in the repository at https://github.com/jorgefz/lince-minigames for examples on how to integrate `lince` to build your game.

There is also an user guide in `docs/userguide` in this repository.

1. Download the source
Clone the repository with git:
`git clone github.com/jorgefz/lince`

2. Generate build scripts
On the root folder (where this README.md file is located), run `premake5` to produce the build configuration files appropriate to your building tool:

* For Unix: `premake5 gmake2`, then run `make`.
* For Windows: `premake5 vs2020`, then load the Visual Studio solution and compile.

The game executable will be located at `bin/{configuration}-{system}-{architecture}/{project}`, e.g. `bin/Debug-Linux-x86_64/editor`. Currently, this project is being tested on Windows 11 (x64) and WSLg GNU/Linux Ubuntu-22.04 (x64).

This repository contains the source code for the following projects, which are built into binaries as part of the compilation process:

* GLFW: windowing system (https://glfw.org)
* GLAD: OpenGL loader (https://glad.dav1d.de/)
* CGLM: OpenGL vector and matrix mathematics (https://cglm.readthedocs.io/en/latest)
* stb\_image.h: Single-header image loader by Sean Barret (https://github.com/nothings/stb)
* Nucklear: ANSI C single-header UI library (https://github.com/Immediate-Mode-UI/Nuklear)
* Miniaudio: Audio playback and capture library (https://github.com/mackron/miniaudio)
* CMocka: Testing framework (https://cmocka.org/)

The project makes use of `mdbook` to generate the documentation website (https://github.com/rust-lang/mdBook).


## Credit

This repository contains audio files obtained from freesound.org, which are licensed under the Creative Commons Attribution 4.0 license (https://creativecommons.org/licenses/by/4.0/)
