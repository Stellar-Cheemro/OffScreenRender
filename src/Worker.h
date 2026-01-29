#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <thread>
#include <atomic>
#include "Framebuffer.h"
#include "Scene.h"
#include <glad/glad.h>

class Worker
{
public:
    Worker(GLFWwindow *shareWindow, int width, int height);
    ~Worker();

    void Start();
    void Stop();
    void SetSceneWorkload(int load) { targetWorkload.store(load); }

    unsigned int GetTextureID() const;
    // 等待 GPU 完成工作并返回最新的纹理 ID
    unsigned int GetReadyTexture();
    // 非阻塞获取就绪纹理；若未就绪返回 0
    unsigned int TryGetReadyTexture();

private:
    void ThreadMain();

    GLFWwindow *shareWindow;
    GLFWwindow *workerWindow;
    int width, height;

    // 双缓冲 FBO
    Framebuffer *fboA;
    Framebuffer *fboB;
    Framebuffer *frontFbo;
    Framebuffer *backFbo;
    Scene *scene;

    std::thread workerThread;
    std::atomic<bool> running;
    // 向主线程暴露的同步原语
    std::atomic<unsigned int> frontTexture;
    std::atomic<GLsync> latestFence;
    std::atomic<int> targetWorkload{0};
};
