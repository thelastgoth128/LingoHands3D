#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>
#include <vector>
#include <string>
#include "model.h"

class SimpleAnimator {
private:
    std::map<std::string, glm::mat4> manualTransforms;
    std::vector<glm::mat4> finalBoneMatrices;
    Model* model;
    
public:
    SimpleAnimator(Model* m) : model(m) {
        finalBoneMatrices.resize(100, glm::mat4(1.0f));
    }
    
    void SetBoneRotation(const std::string& boneName, glm::quat rotation) {
    // Directly convert quaternion to rotation matrix
    glm::mat4 rotMatrix = glm::mat4_cast(rotation);
    manualTransforms[boneName] = rotMatrix;

    // Apply scaled quaternion rotations to connected bones
    if (boneName == "RightArm") {
        glm::quat forearmRot = glm::angleAxis(glm::angle(rotation) * 0.3f, glm::axis(rotation));
        manualTransforms["RightForeArm"] = glm::mat4_cast(forearmRot);

        glm::quat handRot = glm::angleAxis(glm::angle(rotation) * 0.1f, glm::axis(rotation));
        manualTransforms["RightHand"] = glm::mat4_cast(handRot);
    }

    if (boneName == "LeftArm") {
        glm::quat forearmRot = glm::angleAxis(glm::angle(rotation) * 0.3f, glm::axis(rotation));
        manualTransforms["LeftForeArm"] = glm::mat4_cast(forearmRot);

        glm::quat handRot = glm::angleAxis(glm::angle(rotation) * 0.1f, glm::axis(rotation));
        manualTransforms["LeftHand"] = glm::mat4_cast(handRot);
    }

    std::cout << "[Animator] Set rotation for bone: " << boneName << "\n";
}

    
    void ResetAllBones() {
        manualTransforms.clear();
        // Reset to identity matrices
        std::fill(finalBoneMatrices.begin(), finalBoneMatrices.end(), glm::mat4(1.0f));
        std::cout << "[Animator] Reset all bones\n";
    }
    
    void UpdateAnimation(float deltaTime) {
        auto& boneMap = model->GetBoneInfoMap();
        
        // Initialize all bone matrices to identity
        for (int i = 0; i < finalBoneMatrices.size(); i++) {
            finalBoneMatrices[i] = glm::mat4(1.0f);
        }
        
        // Apply manual transforms - use the ALTERNATIVE approach that worked
        for (auto& [boneName, transform] : manualTransforms) {
            if (boneMap.find(boneName) != boneMap.end()) {
                int boneID = boneMap[boneName].id;
                if (boneID >= 0 && boneID < finalBoneMatrices.size()) {
                    // Use the alternative matrix calculation that worked
                    glm::mat4 invOffset = glm::inverse(boneMap[boneName].offset);
                    finalBoneMatrices[boneID] = boneMap[boneName].offset * transform * invOffset;
                }
            }
        }
        
        // For bones without manual transforms, set to identity (bind pose)
        for (auto& [boneName, boneInfo] : boneMap) {
            int boneID = boneInfo.id;
            if (boneID >= 0 && boneID < finalBoneMatrices.size()) {
                if (manualTransforms.find(boneName) == manualTransforms.end()) {
                    finalBoneMatrices[boneID] = glm::mat4(1.0f); // Identity for bind pose
                }
            }
        }
    }
    
    std::vector<glm::mat4> GetFinalBoneMatrices() {
        return finalBoneMatrices;
    }
    
    void PrintAvailableBones() {
        auto& boneMap = model->GetBoneInfoMap();
        std::cout << "[Animator] Available bones:\n";
        for (auto& [name, info] : boneMap) {
            std::cout << "  - " << name << " (ID: " << info.id << ")\n";
        }
    }
};