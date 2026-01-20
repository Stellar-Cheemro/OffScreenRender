#pragma once

#include <glad/glad.h>
#include <iostream>

class Framebuffer {
public:
    Framebuffer(int width, int height);
    ~Framebuffer();

    void Bind();
    void Unbind();
    unsigned int GetTextureID() const { return textureColorBuffer; }

private:
    unsigned int fbo;
    unsigned int textureColorBuffer;
    unsigned int rbo;
    int width, height;
};
