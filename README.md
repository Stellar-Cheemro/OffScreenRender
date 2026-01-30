# 多线程渲染 Demo (Multi-Threaded Rendering Demo)

一个基于 C++、OpenGL (GLFW、GLAD、GLM) 和 ImGui 的演示项目，用于展示单线程渲染与多线程渲染在不同负载场景下的性能差异。

## 1. 项目目的

直观地体现**渲染线程与主逻辑线程解耦**带来的性能优势。
在现代图形应用和游戏中，渲染与逻辑如果串行执行，短板效应明显。
本项目的目的是：

*   **可视化性能对比**：通过实时控制面板，让用户直观看到单线程与多线程在不同负载下的帧率（FPS）和帧时差异。
*   **验证并行优势**：展示当渲染任务剥离到独立 Worker 线程后，即使渲染负载极高，主线程（UI响应）依然能保持流畅；或当两者负载均衡时，通过并行执行显著由于串行执行的总耗时。
*   **教学演示**：作为 OpenGL 多线程上下文共享、离屏渲染（FBO）、同步对象（Fence）以及 ImGui 集成的参考范例。

## 2. 使用说明

### 依赖环境
- **操作系统**: Windows (推荐), Linux, macOS
- **编译器**: 支持 C++17 (MSVC, MinGW, GCC, Clang)
- **构建工具**: CMake 3.15+
- **第三方库**:
    - **GLFW**: 窗口管理与输入 (通过 CMake 自动拉取或内置)
    - **GLAD**: OpenGL 函数加载 (源码集成)
    - **ImGui**: 立即模式 GUI 库 (源码集成)
    - **GLM**: 数学库 (通过 CMake 自动拉取或内置)

### 编译步骤

1. **环境准备**: 确保已安装 CMake 和 C++ 编译器。
2. **构建项目**:
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```
   *注意：推荐直接使用vscode中cmake插件自带的底部状态栏的build

### 运行操作

1.  运行生成的可执行文件 `build/Release/OffScreenRender.exe`。
2.  **控制面板功能**:
    *   **单/多线程切换**: 点击单选按钮实时切换渲染架构。
    *   **主线程负载 (CPU)**: 拖动滑块通过空转循环占用CPU算力模拟主线程极其繁重的逻辑计算。
    *   **渲染线程负载 (Render)**: 拖动滑块通过 Sleep 耗时模拟驱动或GPU进行足够繁重的渲染任务。
3.  **测试流程建议**:
    *   **步骤 1**: 在“单线程模式”拉高主线程负载直到 FPS 降至 30 左右。
    *   **步骤 2**: 拉高渲染负载直到 FPS 进一步降低，立方体渲染画面明显卡顿。
    *   **步骤 3**: 保持负载不变，切换到“多线程模式”。
    *   **观察结果**: FPS 显著回升（因为 CPU 计算与 GPU 等待并行了），且 UI 操作通常会比单线程模式更跟手。

## 3. 项目结构

### 目录结构
```text
OffScreenRender/
├── src/                    # 核心源代码
│   ├── main.cpp            # 主程序入口，包含主循环、UI绘制、事件处理
│   ├── Worker.cpp/.h       # 渲染工作线程类，负责后台 OpenGL 渲染
│   ├── Renderer.cpp/.h     # 单线程模式下的简易渲染器
│   ├── ScreenRenderer.cpp  # 负责将 FBO 纹理绘制到屏幕的后处理渲染器
│   ├── Scene.cpp/.h        # 具体的 3D 场景绘制（立方体 + 负载模拟）
│   ├── Framebuffer.cpp     # 帧缓冲区对象 (FBO) 封装
│   └── Shader.h            # GLSL 着色器加载工具
├── shaders/                # GLSL 着色器文件
│   ├── scene.vert/frag     # 3D 场景着色器
│   └── screen.vert/frag    # 屏幕四边形/后处理着色器
├── extern/                 # 第三方库源码 (ImGui, GLAD, GLFW 等)
├── CMakeLists.txt          # CMake 构建脚本
└── README.md               # 项目文档
```

### 控制逻辑对应
*   **程序入口与 UI**: `src/main.cpp`
    *   包含了 `main` 函数。
    *   负责 `ImGui` 的绘制（控制面板）。
    *   控制 `Single` vs `Multi` 模式的切换逻辑。
    *   在主循环中执行 `DoHeavyWork` 模拟 CPU 负载。
*   **后台渲染**: `src/Worker.cpp`
    *   管理第二个 OpenGL 上下文。
    *   运行独立的渲染线程循环 `ThreadMain`。
    *   使用双缓冲策略渲染到 FBO。
*   **模拟负载实现**:
    *   `src/main.cpp` -> `DoHeavyWork()`: 循环执行 `sqrt` 消耗 CPU。
    *   `src/Scene.cpp` -> `Draw()`: 使用 `std::this_thread::sleep_for` 消耗时间，模拟 GPU 瓶颈。

## 4. 核心原理

### 架构对比
    **项目概述**

    - **名称**: OffScreenRender — 多线程离屏渲染演示（C++ / OpenGL / ImGui）
    - **目的**: 演示将渲染剥离到后台线程（Worker）与主逻辑/UI 并行执行，从而改善 UI 响应和整体帧率的效果。

    **快速开始**

    - **依赖**: CMake 3.15+、支持 C++17 的编译器。第三方库（GLFW、GLAD、GLM、ImGui）已包含在 [extern/](extern/)。
    - **构建 (命令行)**:

      ```bash
      mkdir build
      cd build
      cmake -S .. -B .
      cmake --build . --config Debug
      ```

    - **使用 Visual Studio**: 打开 [build/OffScreenRender.sln](build/OffScreenRender.sln)，选择 Debug/Release 配置并生成。
    - **运行**: 生成后在 Windows 上通常可执行文件位于 [build/Debug/OffScreenRender.exe](build/Debug/OffScreenRender.exe)（或 Release 配置的相应目录）。

    **主要功能**

    - **单/多线程切换**: 通过 UI 切换主线程绘制与后台 Worker 绘制。
    - **负载模拟**: 可调节主线程（CPU）与渲染线程（Render）负载，用于比较两种架构下的表现。
    - **离屏渲染 + 同步**: Worker 将场景渲染到 FBO，主线程通过上下文共享和同步机制读取并显示纹理。

    **代码结构（要点）**

    - **入口与 UI**: [src/main.cpp](src/main.cpp) — 主循环、ImGui 控制面板、模式切换与负载模拟。
    - **Worker 线程**: [src/Worker.cpp](src/Worker.cpp) / [src/Worker.h](src/Worker.h) — 后台渲染循环、上下文管理、同步（fence/atomic）。
    - **场景与渲染**: [src/Scene.cpp](src/Scene.cpp) / [src/Scene.h](src/Scene.h) 和 [src/Renderer.cpp](src/Renderer.cpp) / [src/Renderer.h](src/Renderer.h)。
    - **Framebuffer / 屏幕显示**: [src/Framebuffer.cpp](src/Framebuffer.cpp) / [src/ScreenRenderer.cpp](src/ScreenRenderer.cpp)。
    - **着色器**: GLSL 文件位于 [shaders/](shaders/)（scene.vert/frag, screen.vert/frag）。

    **构建/调试建议**

    - 在 Windows 上推荐使用 Visual Studio 打开已有的 solution（[build/OffScreenRender.sln](build/OffScreenRender.sln)）。
    - 若通过命令行构建，指定 `--config Debug` 或 `--config Release` 以选择构建类型。
    - 若遇到 OpenGL 函数加载或上下文问题，确保系统驱动支持所需的 OpenGL 版本并使用内置的 GLAD（位于 [extern/glad/](extern/glad/)）。

    **运行与测试流程建议**

    - 启动程序后在 UI 中切换到“单线程”模式，并逐步增加“主线程负载”滑块，观察 FPS 和 UI 响应。
    - 在同一负载下切换到“多线程”模式，期待看到 UI 更流畅且总体帧率改善。

    **代码定位与扩展点**

    - 修改 UI 或负载模拟: 编辑 [src/main.cpp](src/main.cpp)。
    - 更改场景或渲染逻辑: 编辑 [src/Scene.cpp](src/Scene.cpp) 和 [src/Renderer.cpp](src/Renderer.cpp)。
    - 调整同步或双缓冲实现: 查看 [src/Worker.cpp](src/Worker.cpp) 与 [src/Framebuffer.cpp](src/Framebuffer.cpp)。

    **故障排查小贴士**

    - 程序无法启动/崩溃: 确认构建配置（Debug/Release）与可执行路径，并检查运行时控制台输出。
    - 黑屏但程序未崩溃: 检查 shaders 是否被正确加载（路径为 [shaders/](shaders/)），并确认驱动支持的 OpenGL 版本。
    - 纹理/渲染不同步: 检查 fence 使用与上下文共享是否正确（相关代码在 [src/Worker.cpp](src/Worker.cpp)）。

    **许可与致谢**

    - 本项目为学习/演示用途。所用第三方库遵循各自开源许可（GLFW/GLM/ImGui/GLAD）。
