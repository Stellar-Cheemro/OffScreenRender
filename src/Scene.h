#pragma once

#include <glad/glad.h>
#include <vector>

class Scene {
public:
    Scene();
    ~Scene();
    void Draw();

private:
    unsigned int cubeVAO, cubeVBO;
};
