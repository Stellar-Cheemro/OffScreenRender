#pragma once

#include <glad/glad.h>
#include "Shader.h"

class ScreenRenderer
{
public:
    ScreenRenderer();
    ~ScreenRenderer();
    void Init();
    void DrawTexture(unsigned int textureID);

private:
    unsigned int quadVAO, quadVBO;
    Shader *screenShader;
};
