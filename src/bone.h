#pragma once
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Bone {
    std::string name;
    glm::mat4 offsetMatrix;
    glm::mat4 localTransform = glm::mat4(1.0f);
    glm::mat4 finalTransform = glm::mat4(1.0f);
    int parentIndex = -1;
};