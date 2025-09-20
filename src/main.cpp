#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "shaders/shader.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include "model.h"
#include "SimpleAnimator.h"  // Use the simple animator instead

// Global variables
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Animation variables
SimpleAnimator* animator = nullptr;

// Key state tracking to prevent repeated actions
bool keyStates[512] = {false};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

bool isKeyJustPressed(int key,GLFWwindow* window) {
    bool currentState = glfwGetKey(window, key) == GLFW_PRESS;
    bool wasPressed = keyStates[key];
    keyStates[key] = currentState;
    return currentState && !wasPressed;
}

bool keyState[512] = {false};
bool prevKeyStates[512] = {false};

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // Update key states
    for (int i = 0; i < 512; i++) {
        prevKeyStates[i] = keyState[i];
        keyState[i] = (glfwGetKey(window, i) == GLFW_PRESS);
    }

    // Helper function for single press detection
    auto isKeyJustPressed = [](int key) {
        return keyState[key] && !prevKeyStates[key];
    };

    // Reset all bones (single press)
    if (isKeyJustPressed(GLFW_KEY_R)) {
        if (animator) {
            animator->ResetAllBones();
            std::cout << "[Input] Reset all bones\n";
        }
    }

    // Print available bones (single press) 
    if (isKeyJustPressed(GLFW_KEY_P)) {
        if (animator) {
            animator->PrintAvailableBones();
        }
    }

    // Rotate neck (single press)
    if (isKeyJustPressed(GLFW_KEY_N)) {
        if (animator) {
            animator->SetBoneRotation("Neck", glm::vec3(0, 30.0f, 0));
            std::cout << "[Input] Rotated neck\n";
        }
    }

    // Rotate head (single press)
    if (isKeyJustPressed(GLFW_KEY_H)) {
        if (animator) {
            animator->SetBoneRotation("Head", glm::vec3(-30.0f, 0.0f, 0.0f));
            std::cout << "[Input] Rotated head\n";
        }
    }

    // Bend finger (single press)
    if (isKeyJustPressed(GLFW_KEY_F)) {
        if (animator) {
            animator->SetBoneRotation("LeftHandIndex2", glm::vec3(45.0f, 0, 0));
            std::cout << "[Input] Bent finger\n";
        }
    }

    // NEW: Right hand up (single press)
    if (isKeyJustPressed(GLFW_KEY_U)) {
        if (animator) {
            // Raise right arm up (rotate around X-axis)
            animator->SetBoneRotation("RightArm", glm::vec3(-90.0f, 0.0f, 0.0f));
            // Also rotate the shoulder slightly for more natural movement
            animator->SetBoneRotation("RightShoulder", glm::vec3(0.0f, 0.0f, 15.0f));
            std::cout << "[Input] Right hand raised up\n";
        }
    }

    // NEW: Right hand down (single press)
    if (isKeyJustPressed(GLFW_KEY_J)) {
        if (animator) {
            // Lower right arm down (rotate around X-axis in opposite direction)
            animator->SetBoneRotation("RightArm", glm::vec3(30.0f, 0.0f, 0.0f));
            // Reset shoulder
            animator->SetBoneRotation("RightShoulder", glm::vec3(0.0f, 0.0f, -10.0f));
            std::cout << "[Input] Right hand lowered down\n";
        }
    }

    // NEW: Right hand wave (continuous - hold key)
   if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    if (animator) {
        float time = glfwGetTime();
        float primaryWave = sin(time * 3.0f) * 45.0f; // Main wave motion
        float secondaryWave = sin(time * 3.0f + 0.5f) * 15.0f; // Offset wave for naturalness
        
        // Move the entire arm chain for natural wave
        animator->SetBoneRotation("RightShoulder", glm::vec3(primaryWave * 0.2f, 0.0f, secondaryWave * 0.3f));
        animator->SetBoneRotation("RightArm", glm::vec3(primaryWave - 30.0f, 0.0f, 0.0f));
        animator->SetBoneRotation("RightForeArm", glm::vec3(primaryWave * 0.4f, 0.0f, sin(time * 3.0f) * 10.0f));
        animator->SetBoneRotation("RightHand", glm::vec3(primaryWave * 0.2f, secondaryWave * 0.5f, 0.0f));
        
        // Add subtle finger movement for more life-like wave
        float fingerWave = sin(time * 4.0f) * 15.0f;
        animator->SetBoneRotation("RightHandIndex1", glm::vec3(fingerWave * 0.3f, 0.0f, 0.0f));
        animator->SetBoneRotation("RightHandMiddle1", glm::vec3(fingerWave * 0.4f, 0.0f, 0.0f));
        animator->SetBoneRotation("RightHandRing1", glm::vec3(fingerWave * 0.3f, 0.0f, 0.0f));
        animator->SetBoneRotation("RightHandPinky1", glm::vec3(fingerWave * 0.2f, 0.0f, 0.0f));
    }
}

    // Continuous animation (hold key) - updated with right hand
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (animator) {
            float time = glfwGetTime();
            float waveNeck = sin(time * 2.0f) * 15.0f;
            float waveFinger = sin(time * 3.0f) * 30.0f + 30.0f;
            float waveRightArm = sin(time * 1.5f) * 60.0f; // Slower arm movement
            
            animator->SetBoneRotation("Neck", glm::vec3(0, waveNeck, 0));
            animator->SetBoneRotation("LeftHandIndex2", glm::vec3(waveFinger, 0, 0));
            animator->SetBoneRotation("Head", glm::vec3(sin(time) * 10.0f, 0, 0));
            
            // Add right arm to the continuous animation
            animator->SetBoneRotation("RightArm", glm::vec3(waveRightArm - 30.0f, 0, 0));
        }
    }

    // Camera controls (keep existing code)
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    const float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPos += cameraSpeed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPos -= cameraSpeed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
}

// Keep your existing mouse callback
float lastX = 400, lastY = 300;
float yaw = -90.0f, pitch = 0.0f;
bool firstMouse = true;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f) pitch = 89.0f;
    if(pitch < -89.0f) pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

float Zoom = 45.0f;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f) Zoom = 1.0f;
    if (Zoom > 45.0f) Zoom = 45.0f;
}

int main() {
    // GLFW initialization
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Animated Model", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Load shaders and model
    Shader ourShader("C:/Users/HP/OneDrive/Documentos/Cyrus/Projects/LingoHands/Model/src/shaders/vertex.vs", "C:/Users/HP/OneDrive/Documentos/Cyrus/Projects/LingoHands/Model/src/shaders/fragment.fss");
    Model ourModel("C:\\Users\\HP\\OneDrive\\Documentos\\Cyrus\\Projects\\LingoHands\\Model\\src\\models\\684c0f35-ba0b-48e0-ab19-db572ea748d3.glb");

    // Debug: Check if bones were loaded
std::cout << "[Debug] Model loaded. Checking bones...\n";
std::cout << "[Debug] Bone count: " << ourModel.GetBoneCount() << "\n";
auto& boneMap = ourModel.GetBoneInfoMap();
std::cout << "[Debug] Bone map size: " << boneMap.size() << "\n";

if (boneMap.empty()) {
    std::cout << "[Debug] WARNING: No bones found in model!\n";
    std::cout << "[Debug] This could mean:\n";
    std::cout << "[Debug]   - The model has no skeleton\n";
    std::cout << "[Debug]   - Bone loading code isn't working\n";
    std::cout << "[Debug]   - Wrong model file\n";
} else {
    std::cout << "[Debug] Bones found:\n";
    for (auto& [name, info] : boneMap) {
        std::cout << "[Debug]   - " << name << " (ID: " << info.id << ")\n";
    }
}


    // Create the simple animator
    animator = new SimpleAnimator(&ourModel);

  // Replace your controls message in main() with this:

    std::cout << "[Main] Controls:\n";
    std::cout << "  R - Reset all bones\n";
    std::cout << "  N - Rotate neck\n";
    std::cout << "  H - Rotate head\n";
    std::cout << "  F - Bend finger\n";
    std::cout << "  U - Raise right hand UP\n";
    std::cout << "  J - Lower right hand DOWN\n";
    std::cout << "  Q - Right hand wave (hold)\n";
    std::cout << "  P - Print available bones\n";
    std::cout << "  SPACE - Continuous full animation\n";
    std::cout << "  WASD - Move camera\n";
    std::cout << "  ESC - Exit\n";

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update animation
        if (animator) {
            animator->UpdateAnimation(deltaTime);
        }

        // Set up matrices
        glm::mat4 projection = glm::perspective(glm::radians(Zoom), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 model = glm::mat4(1.0f);

        // Use shader
        ourShader.use();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        ourShader.setMat4("model", model);

        // Upload bone matrices
        if (animator) {
            auto transforms = animator->GetFinalBoneMatrices();
            for (unsigned int i = 0; i < transforms.size(); ++i) {
                ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
            }
        }

        // Draw model
        ourModel.Draw(ourShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    delete animator;
    glfwTerminate();
    return 0;
}