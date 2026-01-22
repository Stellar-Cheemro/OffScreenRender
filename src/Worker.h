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

    unsigned int GetTextureID() const;
    // Wait for GPU work to finish and return the latest front texture ID
    unsigned int GetReadyTexture();
    // Try to get a ready texture without blocking; returns 0 if none ready
    unsigned int TryGetReadyTexture();

private:
    void ThreadMain();

    GLFWwindow *shareWindow;
    GLFWwindow *workerWindow;
    int width, height;

    // double-buffered FBOs
    Framebuffer *fboA;
    Framebuffer *fboB;
    Framebuffer *frontFbo;
    Framebuffer *backFbo;
    Scene *scene;

    std::thread workerThread;
    std::atomic<bool> running;
    // synchronization primitives exposed to main thread
    std::atomic<unsigned int> frontTexture;
    std::atomic<GLsync> latestFence;
};
