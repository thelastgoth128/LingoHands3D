#include "glad/glad.h"
#include "GLFW/glfw3.h"

// TCP Socket imports (Must be before Windows.h or other heavy headers if possible)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601 // Windows 7 or higher
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#pragma comment(lib, "ws2_32.lib")

#include "shaders/shader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include "model.h"
#include "SimpleAnimator.h"

// ── Animation System ─────────────────────────────────────────────────────────
const std::string ANIM_DIR = "C:/Users/CYRUS JUMBE THINDWA/Documents/Cyrus/Projects/LingoHands/Avatar/src/resources/animations/";

// Maps bone name → (X, Y, Z) euler rotation in degrees
using AnimPose = std::map<std::string, glm::vec3>;

// Cache so we only read each file once
std::map<std::string, AnimPose> animCache;

// Load a sign's pose from disk (returns empty pose on failure)
AnimPose loadAnimation(const std::string& signName) {
    if (animCache.count(signName)) return animCache[signName];

    AnimPose pose;
    std::string path = ANIM_DIR + signName + ".txt";
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "[Anim] No animation file found for '" << signName << "' (" << path << ")\n";
        return pose;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;  // skip comments/blanks
        std::istringstream ss(line);
        std::string bone;
        float x = 0, y = 0, z = 0;
        if (ss >> bone >> x >> y >> z) {
            pose[bone] = glm::vec3(x, y, z);
        }
    }
    std::cout << "[Anim] Loaded '" << signName << "' – " << pose.size() << " bone(s)\n";
    animCache[signName] = pose;
    return pose;
}

// Apply a loaded pose to the animator
void applyPose(SimpleAnimator* anim, const AnimPose& pose) {
    for (const auto& [bone, euler] : pose) {
        glm::quat q = glm::quat(glm::radians(euler));
        anim->SetBoneRotation(bone, q);
    }
}

std::string current_sign = "idle";
std::string prev_sign    = "";

void socket_listener() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cout << "[Socket] WSAStartup failed\n";
        return;
    }

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET) {
        std::cout << "[Socket] Failed to create socket\n";
        WSACleanup();
        return;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(65432);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cout << "[Socket] Bind failed\n";
        closesocket(server);
        WSACleanup();
        return;
    }

    listen(server, 1);
    std::cout << "[Socket] Listening on port 65432 for Python bridge...\n";

    SOCKET client = accept(server, nullptr, nullptr);
    if (client == INVALID_SOCKET) {
        std::cout << "[Socket] Accept failed\n";
        closesocket(server);
        WSACleanup();
        return;
    }
    std::cout << "[Socket] Python script connected!\n";

    char buffer[64];
    std::string data_buffer_str = "";
    while (true) {
        int bytes = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) break;
        buffer[bytes] = '\0';
        data_buffer_str += buffer;
        
        size_t pos;
        while ((pos = data_buffer_str.find('\n')) != std::string::npos) {
            current_sign = data_buffer_str.substr(0, pos);
            data_buffer_str.erase(0, pos + 1);
            
            // Trim any carriage returns
            if (!current_sign.empty() && current_sign.back() == '\r') {
                current_sign.pop_back();
            }
            std::cout << "[Socket] Received sign: " << current_sign << "\n";
        }
    }

    closesocket(client);
    closesocket(server);
    WSACleanup();
    std::cout << "[Socket] Listener thread exiting\n";
}

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
            animator->SetBoneRotation("Neck", glm::quat(glm::radians(glm::vec3(0, 30.0f, 0))));
            std::cout << "[Input] Rotated neck\n";
        }
    }

    // Rotate head (single press)
    if (isKeyJustPressed(GLFW_KEY_H)) {
        if (animator) {
            animator->SetBoneRotation("Head", glm::quat(glm::radians(glm::vec3(0.0f, 30.0f, 0.0f))));
            std::cout << "[Input] Rotated head\n";
        }
    }

    // Bend finger (single press)
    if (isKeyJustPressed(GLFW_KEY_F)) {
        if (animator) {
            animator->SetBoneRotation("LeftHandIndex2", glm::quat(glm::radians(glm::vec3(45.0f, 0, 0))));
            std::cout << "[Input] Bent finger\n";
        }
    }

    if (isKeyJustPressed(GLFW_KEY_J)) {
        if (animator) {
            // Reset to natural/neutral position (no rotation)
            animator->SetBoneRotation("RightArm", glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
            animator->SetBoneRotation("RightShoulder", glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
            animator->SetBoneRotation("RightForeArm", glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
            animator->SetBoneRotation("RightHand", glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
            std::cout << "[Input] Right hand reset to neutral (down) position\n";
        }
    }

    // Keep your U key the same - it raises from the natural position
    if (isKeyJustPressed(GLFW_KEY_U)) {
    if (animator) {
        // Raise the right arm vertically
        glm::quat raiseArm = glm::angleAxis(glm::radians(50.0f), glm::vec3(0, -1, 0));
        // animator->SetBoneRotation("RightHand", raiseArm);
        animator->SetBoneRotation("RightArm",raiseArm);


        std::cout << "[Input] Right hand raised up from neutral position\n";
    }
}



    // NEW: Right hand wave (continuous - hold key)
   if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    if (animator) {
        float time = glfwGetTime();
        float primaryWave = sin(time * 3.0f) * 45.0f; // Main wave motion
        float secondaryWave = sin(time * 3.0f + 0.5f) * 15.0f; // Offset wave for naturalness
        
        // Move the entire arm chain for natural wave using Quaternions
        glm::quat qShoulder = glm::quat(glm::radians(glm::vec3(primaryWave * 0.2f, 0.0f, secondaryWave * 0.3f)));
        glm::quat qArm = glm::quat(glm::radians(glm::vec3(primaryWave - 30.0f, 0.0f, 0.0f)));
        glm::quat qForeArm = glm::quat(glm::radians(glm::vec3(primaryWave * 0.4f, 0.0f, sin(time * 3.0f) * 10.0f)));
        glm::quat qHand = glm::quat(glm::radians(glm::vec3(primaryWave * 0.2f, secondaryWave * 0.5f, 0.0f)));
        
        animator->SetBoneRotation("RightShoulder", qShoulder);
        animator->SetBoneRotation("RightArm", qArm);
        animator->SetBoneRotation("RightForeArm", qForeArm);
        animator->SetBoneRotation("RightHand", qHand);
        
        // Add subtle finger movement for more life-like wave
        float fingerWave = sin(time * 4.0f) * 15.0f;
        glm::quat qFinger1 = glm::quat(glm::radians(glm::vec3(fingerWave * 0.3f, 0.0f, 0.0f)));
        glm::quat qFinger2 = glm::quat(glm::radians(glm::vec3(fingerWave * 0.4f, 0.0f, 0.0f)));
        glm::quat qFinger3 = glm::quat(glm::radians(glm::vec3(fingerWave * 0.3f, 0.0f, 0.0f)));
        glm::quat qFinger4 = glm::quat(glm::radians(glm::vec3(fingerWave * 0.2f, 0.0f, 0.0f)));

        animator->SetBoneRotation("RightHandIndex1", qFinger1);
        animator->SetBoneRotation("RightHandMiddle1", qFinger2);
        animator->SetBoneRotation("RightHandRing1", qFinger3);
        animator->SetBoneRotation("RightHandPinky1", qFinger4);
    }
}

    // Continuous animation (hold key) - updated with right hand
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (animator) {
            float time = glfwGetTime();
            float waveNeck = sin(time * 2.0f) * 15.0f;
            float waveFinger = sin(time * 3.0f) * 30.0f + 30.0f;
            float waveRightArm = sin(time * 1.5f) * 60.0f; // Slower arm movement
            
            animator->SetBoneRotation("Neck", glm::quat(glm::radians(glm::vec3(0, waveNeck, 0))));
            animator->SetBoneRotation("LeftHandIndex2", glm::quat(glm::radians(glm::vec3(waveFinger, 0, 0))));
            animator->SetBoneRotation("Head", glm::quat(glm::radians(glm::vec3(sin(time) * 10.0f, 0, 0))));
            
            // Add right arm to the continuous animation
            animator->SetBoneRotation("RightArm", glm::quat(glm::radians(glm::vec3(waveRightArm - 30.0f, 0, 0))));
        }
    }

    // NEW: Both arms forward and elbows bending back and forth (continuous - hold E key)
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        if (animator) {
            float time = glfwGetTime();
            
            // Left arm: Seems fine, keep it
            glm::quat qShoulderLeft = glm::angleAxis(glm::radians(-20.0f), glm::vec3(0, 1, 0));
            glm::quat qArmForwardLeft = glm::angleAxis(glm::radians(-70.0f), glm::vec3(0, 1, 0));
            
            // Right arm: Distortion reported. Try rotating ONLY the Arm and use a different angle.
            // Often Right/Left axes are flipped in rigs. 
            glm::quat qArmForwardRight = glm::angleAxis(glm::radians(80.0f), glm::vec3(0, 1, 0));

            // Elbow bending motion (Forearms)
            float elbowBendAngle = ((sin(time * 3.0f) + 1.0f) / 2.0f) * 110.0f; 
            
            glm::quat qElbowBendRight = glm::angleAxis(glm::radians(elbowBendAngle), glm::vec3(1, 0, 0));
            glm::quat qElbowBendLeft = glm::angleAxis(glm::radians(elbowBendAngle), glm::vec3(1, 0, 0));

            // Apply rotations
            animator->SetBoneRotation("RightArm", qArmForwardRight);
            animator->SetBoneRotation("RightForeArm", qElbowBendRight);

            animator->SetBoneRotation("LeftShoulder", qShoulderLeft);
            animator->SetBoneRotation("LeftArm", qArmForwardLeft);
            animator->SetBoneRotation("LeftForeArm", qElbowBendLeft);
        }
    }

    // NEW: Flex all fingers (continuous - hold G key)
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        if (animator) {
            float time = glfwGetTime();
            float flexAngle = ((sin(time * 2.0f) + 1.0f) / 2.0f) * 75.0f; // Flex back and forth
            glm::quat qFlex = glm::angleAxis(glm::radians(flexAngle), glm::vec3(1, 0, 0));

            std::string fingers[] = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
            std::string sides[] = {"LeftHand", "RightHand"};

            for (const auto& side : sides) {
                for (const auto& finger : fingers) {
                    for (int i = 1; i <= 3; ++i) {
                        std::string boneName = side + finger + std::to_string(i);
                        animator->SetBoneRotation(boneName, qFlex);
                    }
                }
            }
        }
    }

    // NEW: ASL Hello Salute (continuous - hold X key)
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        if (animator) {
            float time = glfwGetTime();
            
            // Raise Right Arm up to the side
            glm::quat qArmUP = glm::angleAxis(glm::radians(-80.0f), glm::vec3(0, 0, 1)); 
            
            // Bend Forearm in towards the head
            glm::quat qForearmIn = glm::angleAxis(glm::radians(100.0f), glm::vec3(1, 0, 0));
            
            // Hand waving salute motion
            float wave = sin(time * 5.0f) * 15.0f;
            glm::quat qHandSalute = glm::angleAxis(glm::radians(wave), glm::vec3(0, 1, 0));

            // Apply rotations
            animator->SetBoneRotation("RightArm", qArmUP);
            animator->SetBoneRotation("RightForeArm", qForearmIn);
            animator->SetBoneRotation("RightHand", qHandSalute);

            // Flatten fingers for the open palm salute
            glm::quat qFlat = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            std::string fingers[] = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
            for (const auto& finger : fingers) {
                for (int i = 1; i <= 3; ++i) {
                    animator->SetBoneRotation("RightHand" + finger + std::to_string(i), qFlat);
                }
            }
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
    Shader ourShader("C:/Users/CYRUS JUMBE THINDWA/Documents/Cyrus/Projects/LingoHands/Avatar/src/shaders/vertex.vs", "C:/Users/CYRUS JUMBE THINDWA/Documents/Cyrus/Projects/LingoHands/Avatar/src/shaders/fragment.fss");
    Model ourModel("C:/Users/CYRUS JUMBE THINDWA/Documents/Cyrus/Projects/LingoHands/Avatar/src/models/684c0f35-ba0b-48e0-ab19-db572ea748d3.glb");

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
    std::cout << "  E - Arms forward, elbows bending (hold)\n";
    std::cout << "  G - Flex all fingers (hold)\n";
    std::cout << "  X - 'Hello' sign (hold)\n";
    std::cout << "  P - Print available bones\n";
    std::cout << "  SPACE - Continuous full animation\n";
    std::cout << "  WASD - Move camera\n";
    std::cout << "  ESC - Exit\n";


    glEnable(GL_DEPTH_TEST);

    // Start TCP Listener Thread
    std::thread(socket_listener).detach();

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update animation
        if (animator) {
            animator->UpdateAnimation(deltaTime);

            if (current_sign != prev_sign) {
                std::cout << "[Anim] Sign changed: '" << prev_sign << "' -> '" << current_sign << "'\n";
                prev_sign = current_sign;
            }

            if (current_sign == "idle") {
                // Reset all to bind pose
                animator->ResetAllBones();
            } else {
                // --- DATA-DRIVEN: load from file, e.g. resources/animations/hello.txt ---
                AnimPose pose = loadAnimation(current_sign);
                if (!pose.empty()) {
                    applyPose(animator, pose);
                } else {
                    // Fallback: unknown sign – just stay at bind pose
                    animator->ResetAllBones();
                }
            }
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