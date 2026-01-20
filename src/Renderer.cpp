#include "Renderer.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Renderer::Renderer(int width, int height) : screenWidth(width), screenHeight(height) {
    sceneShader = nullptr;
    screenShader = nullptr;
    fbo = nullptr;
    scene = nullptr;
}

Renderer::~Renderer() {
    delete sceneShader;
    delete screenShader;
    delete fbo;
    delete scene;
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
}

void Renderer::Init() {
    glEnable(GL_DEPTH_TEST);

    // 加载并编译着色器：场景渲染 & 屏幕后处理
    sceneShader = new Shader("shaders/scene.vert", "shaders/scene.frag");
    screenShader = new Shader("shaders/screen.vert", "shaders/screen.frag");

    // 配置 Shader 纹理单元
    sceneShader->use();
    sceneShader->setInt("texture1", 0);

    screenShader->use();
    screenShader->setInt("screenTexture", 0);

    // 初始化 FBO、场景物体、全屏四边形
    fbo = new Framebuffer(screenWidth, screenHeight);
    scene = new Scene();
    InitQuad();
}

void Renderer::InitQuad() {
    float quadVertices[] = { // 标准化设备坐标中填充整个屏幕的四边形的顶点属性。
        // 位置       // 纹理坐标
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void Renderer::Render() {
    float time = (float)glfwGetTime();

    // --- 第一阶段：离屏渲染 ---
    // 绑定自定义 FBO，所有渲染结果写入其中的纹理附件，而非屏幕
    fbo->Bind();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // 深色背景清屏
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 渲染 3D 场景（旋转的立方体）
    sceneShader->use();
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

    model = glm::rotate(model, time, glm::vec3(0.5f, 1.0f, 0.0f));

    sceneShader->setMat4("view", view);
    sceneShader->setMat4("projection", projection);
    sceneShader->setMat4("model", model);

    scene->Draw();
    
    // 解绑 FBO，恢复默认帧缓冲区
    fbo->Unbind();

    // --- 第二阶段：屏幕后处理 ---
    // 可以在这里禁用深度测试，这对于绘制全屏四边形往往是好的
    glDisable(GL_DEPTH_TEST); 
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // 纯白背景清屏（实际上会被四边形覆盖）
    glClear(GL_COLOR_BUFFER_BIT);

    // 绘制全屏四边形，并将离屏渲染产生的纹理作为贴图输入
    screenShader->use();
    glBindVertexArray(quadVAO);
    glBindTexture(GL_TEXTURE_2D, fbo->GetTextureID()); // 使用 FBO 的颜色纹理
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::Resize(int width, int height) {
    screenWidth = width;
    screenHeight = height;
    glViewport(0, 0, width, height);
    
    // 如果需要，使用新尺寸重新创建 FBO，确保严格匹配
    delete fbo;
    fbo = new Framebuffer(width, height);
}
