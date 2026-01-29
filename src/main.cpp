#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>

#include "Renderer.h"
#include "Worker.h"
#include "ScreenRenderer.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

Renderer *renderer = nullptr;

int main(int argc, char **argv)
{
    bool singleMode = false;
    int cpuLoad = 0; // arbitrary load units
    for (int i = 1; i < argc; ++i)
        if (std::string(argv[i]) == "--single")
            singleMode = true;
    for (int i = 1; i < argc; ++i)
    {
        std::string s = argv[i];
        if (s.rfind("--cpu-load=", 0) == 0)
        {
            try
            {
                cpuLoad = std::stoi(s.substr(11));
            }
            catch (...)
            {
                cpuLoad = 0;
            }
        }
        else if (s == "--cpu-load" && i + 1 < argc)
        {
            try
            {
                cpuLoad = std::stoi(argv[++i]);
            }
            catch (...)
            {
                cpuLoad = 0;
            }
        }
    }
    // 1. 初始化 GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 2. 创建窗口对象
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OffScreenRendering Demo", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "创建 GLFW 窗口失败" << std::endl;
        glfwTerminate();
        return -1;
    }
    // 3. 将窗口的上下文设置为当前线程的主上下文
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 4. Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    const GLubyte *glVersion = glGetString(GL_VERSION);
    if (glVersion)
    {
        std::cout << "GLAD initialized. OpenGL Version: " << reinterpret_cast<const char *>(glVersion) << std::endl;
    }
    else
    {
        std::cout << "GLAD initialized. OpenGL Version: NULL" << std::endl;
    }

    // 5. Initialize based on mode
    Renderer *singleRenderer = nullptr;
    Worker *worker = nullptr;
    ScreenRenderer *screen = nullptr;

    if (singleMode)
    {
        singleRenderer = new Renderer(SCR_WIDTH, SCR_HEIGHT);
        singleRenderer->Init();
        std::cout << "Single-thread renderer initialized." << std::endl;
    }
    else
    {
        std::cout << "Creating worker object..." << std::endl;
        worker = new Worker(window, SCR_WIDTH, SCR_HEIGHT);

        std::cout << "Starting worker thread..." << std::endl;
        worker->Start();

        std::cout << "Creating ScreenRenderer..." << std::endl;
        screen = new ScreenRenderer();
        std::cout << "Initializing ScreenRenderer..." << std::endl;
        screen->Init();

        std::cout << "Initialization complete. Starting render loop..." << std::endl;
    }

    // 6. 渲染循环（带 FPS/帧时统计）
    using clock = std::chrono::high_resolution_clock;
    auto lastReport = clock::now();
    int frames = 0;
    double accumFrameMs = 0.0;

    // helper: simulate CPU-heavy work on main thread
    auto DoHeavyWork = [](int units)
    {
        if (units <= 0)
            return;
        volatile double acc = 0.0;
        int loops = units * 10000;
        for (int i = 0; i < loops; ++i)
        {
            acc += sqrt((double)(i % 100 + 1));
        }
    };

    while (!glfwWindowShouldClose(window))
    {
        // 处理键盘输入
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // 计时并执行一帧：单线程直接渲染，多线程绘制 worker 的纹理
        auto t0 = clock::now();
        // simulate main-thread CPU work (UI, physics, etc.)
        DoHeavyWork(cpuLoad);
        if (singleMode)
        {
            singleRenderer->Render();
        }
        else
        {
            unsigned int tex = worker->TryGetReadyTexture();
            static unsigned int lastTex = 0;
            if (tex)
            {
                lastTex = tex;
                screen->DrawTexture(tex);
            }
            else if (lastTex)
            {
                // no new ready texture yet, reuse last one to avoid blocking
                screen->DrawTexture(lastTex);
            }
        }
        auto t1 = clock::now();

        std::chrono::duration<double, std::milli> frameMs = t1 - t0;
        accumFrameMs += frameMs.count();
        frames++;

        // 交换缓冲并检查事件
        glfwSwapBuffers(window);
        glfwPollEvents();

        // 每秒输出一次 FPS 和平均帧时
        auto now = clock::now();
        std::chrono::duration<double> elapsed = now - lastReport;
        if (elapsed.count() >= 1.0)
        {
            double avgMs = (frames > 0) ? (accumFrameMs / frames) : 0.0;
            std::cout << "FPS: " << frames << "  Avg frame time: " << avgMs << " ms" << std::endl;
            frames = 0;
            accumFrameMs = 0.0;
            lastReport = now;
        }
    }

    // 7. 清理资源
    if (singleMode)
    {
        delete singleRenderer;
        if (screen)
        {
            delete screen;
        }
    }
    else
    {
        worker->Stop();
        delete worker;
        delete screen;
    }
    glfwTerminate();
    return 0;
}

// 窗口大小改变回调函数
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    if (renderer)
    {
        renderer->Resize(width, height);
    }
}