#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "shaders/shader.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include "model.h"

using namespace std;

class Window {
    public:
        void framebuffer_size_callback(GLFWwindow* window, int width, int height);
        void processInput(GLFWwindow* window);
        void mouse_callback(GLFWwindow* window, double xpos, double ypos);
        void launch_glfw_renderer();

    private:
        glm::vec3 cameraPos;
        glm::vec3 cameraFront;
        glm::vec3 cameraUp;

        float deltaTime;
        float lastFrame;
        float lastX;
        float yaw;
        float pitch;
        float Zoom;
        bool firstMouse;
};

