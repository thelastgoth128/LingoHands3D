#pragma once

#include <glm/glm.hpp>

#define MAX_BONE_INFLUENCE 4
#define MAX_BONE_WEIGHTS 4

struct BoneInfo {
    int id;
    glm::mat4 offset;
};

// If you want to keep vertex data separate, you can also create a separate Vertex header
// But if it's already in mesh.h, you can just include this file there