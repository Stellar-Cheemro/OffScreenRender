# OffScreenRendering 项目

## 先决条件

- **CMake**: 3.15 或更新版本。
- **C++ 编译器**: 支持 C++17 的 MSVC (Visual Studio) 或 MinGW。
- **GLAD**: OpenGL 加载库。

## 设置 GLAD

由于 GLAD 是一个生成器，您需要生成源代码文件并将其放在 `extern/glad` 文件夹中。

1. 访问 [https://glad.dav1d.de/](https://glad.dav1d.de/)
2. **语言**: C/C++
3. **规范**: OpenGL
4. **API**: gl -> 版本 3.3 (或更高) -> 配置文件: **Core**
5. **选项**: 勾选 "Generate a loader"
6. 点击 **Generate** 并下载 zip 文件。
7. 解压内容。
8. 将 `include` 和 `src` 文件夹复制到 `extern/glad/` 中。

结构应该如下所示：

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

## 构建和运行

1. 在 VS Code 中打开此文件夹。
2. 如果您有 **CMake Tools** 扩展：
    - 选择您的工具包（例如，Visual Studio Community 2022 Release - x64）。
    - 点击底部栏中的 "Build"。
    - 点击 "Run"（播放按钮）。
3. 通过命令行：
    ```bash
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ./Debug/OffScreenRendering.exe
    ```

## 控制
- 渲染逻辑位于 `src/Renderer.cpp`。
- 场景着色器: `shaders/scene.frag`。
- 后处理着色器: `shaders/screen.frag`（当前应用灰度效果）。