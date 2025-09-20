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
    
    void SetBoneRotation(const std::string& boneName, glm::vec3 rotation) {
        glm::mat4 rotMatrix = glm::mat4(1.0f);
        rotMatrix = glm::rotate(rotMatrix, glm::radians(rotation.x), glm::vec3(1, 0, 0));
        rotMatrix = glm::rotate(rotMatrix, glm::radians(rotation.y), glm::vec3(0, 1, 0));
        rotMatrix = glm::rotate(rotMatrix, glm::radians(rotation.z), glm::vec3(0, 0, 1));
        
        manualTransforms[boneName] = rotMatrix;
        
        // When rotating arm, also apply some transform to connected bones
        if (boneName == "RightArm") {
            // Apply a portion of the arm rotation to the forearm for more natural movement
            glm::mat4 forearmMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x * 0.3f), glm::vec3(1, 0, 0));
            manualTransforms["RightForeArm"] = forearmMatrix;
            
            // Apply smaller rotation to hand
            glm::mat4 handMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x * 0.1f), glm::vec3(1, 0, 0));
            manualTransforms["RightHand"] = handMatrix;
        }
        
        // Similar for left arm
        if (boneName == "LeftArm") {
            glm::mat4 forearmMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x * 0.3f), glm::vec3(1, 0, 0));
            manualTransforms["LeftForeArm"] = forearmMatrix;
            
            glm::mat4 handMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x * 0.1f), glm::vec3(1, 0, 0));
            manualTransforms["LeftHand"] = handMatrix;
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