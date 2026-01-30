#include "Worker.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <chrono>

Worker::Worker(GLFWwindow *shareWindow, int width, int height)
    : shareWindow(shareWindow), workerWindow(nullptr), width(width), height(height),
      fboA(nullptr), fboB(nullptr), frontFbo(nullptr), backFbo(nullptr), scene(nullptr), running(false), frontTexture(0), latestFence(nullptr)
{
    // 创建一个不可见的窗口，与主窗口共享资源
    // 必须在主线程中完成
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    workerWindow = glfwCreateWindow(1, 1, "worker", NULL, shareWindow);
    if (!workerWindow)
    {
        std::cerr << "无法创建 Worker 窗口/上下文" << std::endl;
    }
}

Worker::~Worker()
{
    Stop();
}

void Worker::Start()
{
    // 如果窗口被销毁，重建它
    if (!workerWindow)
    {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        workerWindow = glfwCreateWindow(1, 1, "worker", NULL, shareWindow);
        if (!workerWindow)
        {
            std::cerr << "无法重建 Worker 窗口/上下文" << std::endl;
            return;
        }
    }
    running = true;
    workerThread = std::thread(&Worker::ThreadMain, this);
}

void Worker::Stop()
{
    running = false;
    if (workerThread.joinable())
        workerThread.join();

    // 线程会自行清理资源，这里只需销毁窗口
    // 不要解绑主线程的上下文
    if (workerWindow)
    {
        glfwDestroyWindow(workerWindow);
        workerWindow = nullptr;
    }

    // 清理遗留的栅欄对象
    GLsync fence = latestFence.load();
    if (fence)
    {
        // 确保在正确的上下文环境中操作GL资源
        if (workerWindow)
        {
            glfwMakeContextCurrent(workerWindow);
            glDeleteSync(fence);
            glfwMakeContextCurrent(nullptr);
        }
        else
        {
            // 如果上下文已经失效，直接删除同步对象
            glDeleteSync(fence);
        }
    }

    // 删除framebuffers和scene，检查空指针避免重复释放
    if (fboA)
    {
        delete fboA;
        fboA = nullptr;
    }
    if (fboB)
    {
        delete fboB;
        fboB = nullptr;
    }
    if (scene)
    {
        delete scene;
        scene = nullptr;
    }
}

unsigned int Worker::GetTextureID() const
{
    return frontTexture.load();
}

unsigned int Worker::GetReadyTexture()
{
    // 等待并获取最新的纹理
    GLsync fence = latestFence.exchange(nullptr);
    if (fence)
    {
        GLenum waitRes = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000);
        (void)waitRes;
        glDeleteSync(fence);
    }
    return frontTexture.load();
}

unsigned int Worker::TryGetReadyTexture()
{
    // 非阻塞检查：如果没有新栅欄，直接返回纹理（可能是旧的）
    GLsync fence = latestFence.load();
    if (!fence)
    {
        return frontTexture.load();
    }

    // 检查 GPU 命令是否完成
    GLenum waitRes = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
    if (waitRes == GL_ALREADY_SIGNALED || waitRes == GL_CONDITION_SATISFIED)
    {
        // 尝试获取所有权并删除栅欄
        GLsync expected = fence;
        if (latestFence.compare_exchange_strong(expected, nullptr))
        {
            glDeleteSync(fence);
        }
        return frontTexture.load();
    }

    // 未完成返回 0
    return 0;
}

void Worker::ThreadMain()
{
    if (!workerWindow)
    {
        running = false;
        return;
    }

    // 设置当前线程上下文
    glfwMakeContextCurrent(workerWindow);

    // 初始化双缓冲 FBO
    fboA = new Framebuffer(width, height);
    fboB = new Framebuffer(width, height);
    frontFbo = fboA;
    backFbo = fboB;
    frontTexture.store(frontFbo->GetTextureID());
    latestFence.store(nullptr);

    scene = new Scene();
    Shader sceneShader("shaders/scene.vert", "shaders/scene.frag");

    // 渲染循环
    using clock = std::chrono::high_resolution_clock;
    auto lastReport = clock::now();
    int frames = 0;

    while (running)
    {
        // 更新负载
        scene->SetWorkload(targetWorkload.load());

        // 渲染到后缓冲
        backFbo->Bind();
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        sceneShader.use();
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
        float time = (float)glfwGetTime();
        model = glm::rotate(model, time, glm::vec3(0.5f, 1.0f, 0.0f));
        sceneShader.setMat4("view", view);
        sceneShader.setMat4("projection", projection);
        sceneShader.setMat4("model", model);

        scene->Draw();
        backFbo->Unbind();

        // 提交命令并插入栅欄
        glFlush();
        GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        latestFence.store(fence);

        // 交换前后缓冲
        std::swap(frontFbo, backFbo);
        frontTexture.store(frontFbo->GetTextureID());

        // 计算渲染帧率
        frames++;
        auto now = clock::now();
        std::chrono::duration<double> elapsed = now - lastReport;
        if (elapsed.count() >= 1.0)
        {
            fps.store((double)frames / elapsed.count());
            frames = 0;
            lastReport = now;
        }

        // 无休眠全速运行
    }

    // 线程退出前资源清理
    delete fboA;
    delete fboB;
    delete scene;
    fboA = nullptr;
    fboB = nullptr;
    scene = nullptr;

    // 解绑上下文
    glfwMakeContextCurrent(nullptr);
}