#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Renderer.h"
#include "Worker.h"
#include "ScreenRenderer.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

Renderer *globalSingleRenderer = nullptr;

int main(int argc, char** argv)
{
    // 默认设置
    bool useMultiThread = false;
    int cpuLoad = 0;
    int renderLoad = 0;

    // 解析命令行参数
    for (int i = 1; i < argc; ++i) if (std::string(argv[i]) == "--single") useMultiThread = false;
    for (int i = 1; i < argc; ++i) if (std::string(argv[i]) == "--multi") useMultiThread = true;

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
    // 禁用垂直同步以获得更高帧率
    glfwSwapInterval(0);

    // 4. 初始化 GLAD (加载OpenGL函数指针)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "初始化 GLAD 失败" << std::endl;
        return -1;
    }

    // 设置 ImGui 上下文
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    
    // 加载中文字体（微软雅黑）
    const char* fontPath = "c:\\Windows\\Fonts\\msyh.ttc";
    ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath, 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
    if (!font) {
        // 后备字体 (SimHei)
        fontPath = "c:\\Windows\\Fonts\\simhei.ttf";
        io.Fonts->AddFontFromFileTTF(fontPath, 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // 5. 初始化渲染系统
    // 单线程渲染器
    Renderer* singleRenderer = new Renderer(SCR_WIDTH, SCR_HEIGHT);
    singleRenderer->Init();
    globalSingleRenderer = singleRenderer; // 用于窗口调整大小回调

    // 多线程 Worker 和 屏幕渲染器
    Worker* worker = new Worker(window, SCR_WIDTH, SCR_HEIGHT);
    ScreenRenderer* screen = new ScreenRenderer();
    screen->Init();

    if (useMultiThread) {
        worker->Start();
    }

    // 6. 渲染循环（FPS/帧时统计）
    using clock = std::chrono::high_resolution_clock;
    auto lastReport = clock::now();
    int frames = 0;
    double accumFrameMs = 0.0;
    double lastFps = 0.0;
    double lastAvgMs = 0.0;

    // 辅助函数：模拟主线程 CPU 密集型任务
    auto DoHeavyWork = [](int units){
        if (units <= 0) return;
        volatile double acc = 0.0;
        int loops = units * 10000;
        for (int i = 0; i < loops; ++i)
        {
            acc += sqrt((double)(i % 100 + 1));
        }
    };
    
    // 记录上一帧的模式状态
    bool prevMultiThread = useMultiThread;

    // 纹理状态追踪
    unsigned int lastTex = 0;

    while (!glfwWindowShouldClose(window))
    {
        // 处理事件
        glfwPollEvents();

        // 开始 ImGui 帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 绘制 UI 窗口
        ImGui::SetNextWindowSize(ImVec2(1000, 250), ImGuiCond_FirstUseEver);
        ImGui::Begin(u8"性能测试控制面板");
        ImGui::Text(u8"平均帧率FPS: %.1f", lastFps);
        ImGui::Text(u8"平均每帧用时: %.3f ms", lastAvgMs);
        
        ImGui::Separator();
        
        bool modeChanged = false;
        if (ImGui::RadioButton(u8"单线程", !useMultiThread)) {
            useMultiThread = false;
            modeChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton(u8"多线程", useMultiThread)) {
            useMultiThread = true;
            modeChanged = true;
        }

        ImGui::Separator();
        ImGui::SliderInt(u8"主线程UI界面负载", &cpuLoad, 0, 1000);
        ImGui::SliderInt(u8"渲染线程负载", &renderLoad, 0, 1000);

        // ImGui::Separator();
        // ImGui::TextWrapped(
        //     u8"使用说明 (Instructions):\n"
        //     u8"1. 增加 [主线程负载] (CPU).\n"
        //     u8"2. 增加 [渲染负载] (Render).\n"
        //     u8"   (注: 为避免高端显卡导致测试失效，现已改为模拟耗时)\n"
        //     u8"3. 对比单/多线程模式 FPS.\n"
        //     u8"   - 单线程: 耗时 = 逻辑 + 渲染 (FPS低)\n"
        //     u8"   - 多线程: 耗时 = Max(逻辑, 渲染) (FPS高)\n"
        // );

        ImGui::End();

        // 处理模式切换
        if (prevMultiThread != useMultiThread) {
            if (useMultiThread) {
                worker->Start();
                // 切换到多线程模式时，重置上一帧纹理ID，防止使用非法纹理
                lastTex = 0; 
            } else {
                worker->Stop();
            }
            prevMultiThread = useMultiThread;
        }

        // 更新负载设置
        singleRenderer->SetSceneWorkload(renderLoad);
        worker->SetSceneWorkload(renderLoad);

        // 渲染主逻辑
        auto t0 = clock::now();
        
        // 模拟主线程 CPU 负载
        DoHeavyWork(cpuLoad);

        if (!useMultiThread) {
            // 单线程模式：直接在主线程渲染
            singleRenderer->Render();
        } else {
            // 多线程模式：获取 Worker 渲染好的纹理并上屏
            unsigned int tex = worker->TryGetReadyTexture();
            if (tex)
            {
                lastTex = tex;
                screen->DrawTexture(tex);
            }
            else if (lastTex != 0)
            {
                screen->DrawTexture(lastTex);
            }
            else 
            {
                // 若尚无纹理就绪，清屏
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
            }
        }
        
        // 渲染 ImGui UI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        
        auto t1 = clock::now();
        std::chrono::duration<double, std::milli> frameMs = t1 - t0;
        accumFrameMs += frameMs.count();
        frames++;

        // 计算 FPS
        auto now = clock::now();
        std::chrono::duration<double> elapsed = now - lastReport;
        if (elapsed.count() >= 1.0)
        {
            lastFps = frames / elapsed.count();
            lastAvgMs = (frames > 0) ? (accumFrameMs / frames) : 0.0;
            
            // 控制台输出
            // std::cout << "FPS: " << lastFps << "  Avg frame time: " << lastAvgMs << " ms" << std::endl;
            
            frames = 0;
            accumFrameMs = 0.0;
            lastReport = now;
        }
    }

    // 清理资源
    worker->Stop();
    delete worker;
    delete singleRenderer;
    delete screen;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

// 窗口大小改变回调函数
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    if (globalSingleRenderer)
    {
        globalSingleRenderer->Resize(width, height);
    }
}
