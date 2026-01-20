# OffScreenRendering Project

## Prerequisites

- **CMake**: 3.15 or newer.
- **C++ Compiler**: MSVC (Visual Studio) or MinGW supporting C++17.
- **GLAD**: OpenGL Loader Library.

## Setting up GLAD

Because GLAD is a generator, you need to generate the source files and place them in the `extern/glad` folder.

1. Go to [https://glad.dav1d.de/](https://glad.dav1d.de/)
2. **Language**: C/C++
3. **Specification**: OpenGL
4. **API**: gl -> Version 3.3 (or higher) -> Profile: **Core**
5. **Options**: Check "Generate a loader"
6. Click **Generate** and download the zip file.
7. Extract the contents.
8. Copy the folders `include` and `src` into `extern/glad/`.

The structure should look like this:
```
extern/
  glad/
    include/
      glad/
        glad.h
      KHR/
        khrplatform.h
    src/
      glad.c
```

## Building and Running

1. Open this folder in VS Code.
2. If you have the **CMake Tools** extension:
    - Select your kit (e.g., Visual Studio Community 2022 Release - x64).
    - Click "Build" in the bottom bar.
    - Click "Run" (play button).
3. Via command line:
    ```bash
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ./Debug/OffScreenRendering.exe
    ```

## Controls
- Rendering logic is in `src/Renderer.cpp`.
- Scene shader: `shaders/scene.frag`.
- Post-process shader: `shaders/screen.frag` (Currently applies Grayscale).
