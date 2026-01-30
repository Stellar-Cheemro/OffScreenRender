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
1.  **单线程模式 (Serial Execution)**:
    *   `Frame Time = Logic Time + Render Time`
    *   逻辑更新和渲染绘制在同一个线程按顺序执行。任何一方变慢都会直接拖慢帧率。
    
2.  **多线程模式 (Parallel Execution)**:
    *   `Frame Time ≈ Max(Logic Time, Render Time) + Overhead`
    *   **Main Thread**: 处理窗口消息、UI Interaction、逻辑更新。
    *   **Worker Thread**: 专职处理 OpenGL 绘制命令。
    *   两者并行工作，极大提升了硬件利用率。

### 关键技术点

1.  **OpenGL 上下文共享 (Context Sharing)**:
    Worker 线程创建窗口时通过 `glfwCreateWindow` 的最后一个参数共享主窗口的上下文资源（纹理、Buffer），使得 Worker 绘制的纹理可以直接被主线程读取和显示。

2.  **双缓冲 + 帧缓冲区 (FBO)**:
    Worker 不直接绘制到屏幕（Back Buffer），而是绘制到自定义的 FBO 纹理中。使用了双缓冲机制（Front/Back FBO），Worker 绘制 Back FBO，完成后交换给 Front，主线程只读取 Front FBO 进行上屏。

3.  **同步机制 (Synchronization)**:
    *   **GL Fence (`glFenceSync`)**: 为了防止主线程在 Worker 还没画完时就去读取纹理（导致画面撕裂或错乱），使用 OpenGL 同步栅栏。Worker 提交绘制后插入 Fence，主线程在绘制前检查 Fence 是否完成 (`glWaitSync` / `glClientWaitSync`)。
    *   **原子变量 (`std::atomic`)**: 线程间通信（如传递纹理ID、停止标志）使用 C++ 原子变量确保线程安全。

4.  **纹理上屏 (Texture Blit)**:
    主线程实际上不进行复杂的场景绘制，它唯一的渲染任务是画一个全屏的四边形，并将 Worker 产出的纹理贴上去。这由 `ScreenRenderer` 类完成。
