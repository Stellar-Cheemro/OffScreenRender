#include "ScreenRenderer.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

ScreenRenderer::ScreenRenderer() : quadVAO(0), quadVBO(0), screenShader(nullptr) {}

ScreenRenderer::~ScreenRenderer()
{
    // 添加空指针检查，避免重复释放
    if (screenShader)
    {
        delete screenShader;
        screenShader = nullptr;
    }

    // 检查VAO和VBO是否有效
    if (quadVAO != 0)
    {
        glDeleteVertexArrays(1, &quadVAO);
        quadVAO = 0;
    }
    if (quadVBO != 0)
    {
        glDeleteBuffers(1, &quadVBO);
        quadVBO = 0;
    }
}

void ScreenRenderer::Init()
{
    screenShader = new Shader("shaders/screen.vert", "shaders/screen.frag");
    screenShader->use();
    screenShader->setInt("screenTexture", 0);

    float quadVertices[] = {
        // 位置        // 纹理坐标
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f};

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
}

void ScreenRenderer::DrawTexture(unsigned int textureID)
{
    glDisable(GL_DEPTH_TEST);
    screenShader->use();
    glBindVertexArray(quadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}