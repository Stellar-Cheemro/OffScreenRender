#pragma once

#include <glad/glad.h>
#include <vector>

class Scene {
public:
    Scene();
    ~Scene();
    void Draw();
    void SetWorkload(int load);
    void SetSimulateWorkload(bool simulate) { simulateWorkload = simulate; }

private:
    unsigned int cubeVAO, cubeVBO;
    int workload = 0;
    bool simulateWorkload = true;
};
