#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "Renderer.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Renderer* renderer = nullptr;

int main() {
    // 1. 初始化 GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 2. 创建窗口对象
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OffScreenRendering Demo", NULL, NULL);
    if (window == NULL) {
        std::cout << "创建 GLFW 窗口失败" << std::endl;
        glfwTerminate();
        return -1;
    }
    // 3. 将窗口的上下文设置为当前线程的主上下文
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 4. 初始化 GLAD（必须在有上下文的线程中调用）
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "初始化 GLAD 失败" << std::endl;
        return -1;
    }

    std::cout << "GLAD 已初始化。OpenGL 版本：" << glGetString(GL_VERSION) << std::endl;

    // 5. 创建并初始化渲染器（包含 Shader 编译、FBO 创建、模型加载）
    renderer = new Renderer(SCR_WIDTH, SCR_HEIGHT);
    
    std::cout << "正在初始化渲染器..." << std::endl;
    renderer->Init();
    std::cout << "渲染器已初始化。开始循环..." << std::endl;

    // 6. 渲染循环
    while (!glfwWindowShouldClose(window)) {
        // 处理键盘输入
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // 执行一帧渲染
        renderer->Render();

        // 交换缓冲并检查事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 7. 清理资源
    delete renderer;
    glfwTerminate();
    return 0;
}

// 窗口大小改变回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if (renderer) {
        renderer->Resize(width, height);
    }
}
