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
    // create an invisible window sharing resources with the main window
    // This must be done on the main thread
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    workerWindow = glfwCreateWindow(1, 1, "worker", NULL, shareWindow);
    if (!workerWindow)
    {
        std::cerr << "Failed to create worker window/context" << std::endl;
    }
}

Worker::~Worker()
{
    Stop();
}

void Worker::Start()
{
    running = true;
    workerThread = std::thread(&Worker::ThreadMain, this);
}

void Worker::Stop()
{
    running = false;
    if (workerThread.joinable())
        workerThread.join();
    if (workerWindow)
    {
        glfwMakeContextCurrent(nullptr);
        glfwDestroyWindow(workerWindow);
        workerWindow = nullptr;
    }
    // 先处理fence，避免在释放其他资源时访问
    GLsync fence = latestFence.exchange(nullptr);
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
    // wait on latest fence if present, then return front texture id
    GLsync fence = latestFence.exchange(nullptr);
    if (fence)
    {
        // client wait for GPU to finish writing
        GLenum waitRes = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000);
        (void)waitRes;
        glDeleteSync(fence);
    }
    return frontTexture.load();
}

unsigned int Worker::TryGetReadyTexture()
{
    // Non-blocking check: return 0 if the latest fence is not yet signaled.
    GLsync fence = latestFence.load();
    if (!fence)
    {
        // no outstanding fence, texture is ready
        return frontTexture.load();
    }

    GLenum waitRes = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
    if (waitRes == GL_ALREADY_SIGNALED || waitRes == GL_CONDITION_SATISFIED)
    {
        // try to consume the fence pointer (ensure we delete it only once)
        GLsync expected = fence;
        if (latestFence.compare_exchange_strong(expected, nullptr))
        {
            glDeleteSync(fence);
        }
        return frontTexture.load();
    }

    // not ready yet
    return 0;
}

void Worker::ThreadMain()
{
    if (!workerWindow)
    {
        running = false;
        return;
    }

    // make context current in this thread
    glfwMakeContextCurrent(workerWindow);

    // initialize GL resources for offscreen rendering (double FBO)
    fboA = new Framebuffer(width, height);
    fboB = new Framebuffer(width, height);
    frontFbo = fboA;
    backFbo = fboB;
    frontTexture.store(frontFbo->GetTextureID());
    latestFence.store(nullptr);

    scene = new Scene();
    Shader sceneShader("shaders/scene.vert", "shaders/scene.frag");

    // simple render loop with double-buffer and fence sync
    using clock = std::chrono::high_resolution_clock;
    while (running)
    {
        // render to back FBO
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

        // ensure commands are submitted and fence the GPU work
        glFlush();
        GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        // publish fence and new front texture id
        latestFence.store(fence);

        // swap front/back and publish new front texture id for main thread
        std::swap(frontFbo, backFbo);
        frontTexture.store(frontFbo->GetTextureID());

        // small sleep to avoid busy loop
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    // cleanup done in Stop()
}