#pragma once

#include <glad/glad.h>
#include <vector>
#include "Shader.h"
#include "Framebuffer.h"
#include "Scene.h"

class Renderer
{
public:
    Renderer(int width, int height);
    ~Renderer();

    void Init();
    void Render();
    void Resize(int width, int height);

private:
    int screenWidth, screenHeight;
    unsigned int quadVAO, quadVBO;

    Shader *sceneShader;
    Shader *screenShader;
    Framebuffer *fbo;
    Scene *scene;

    void InitQuad();
    void RenderScene();
    void RenderScreen();
};
