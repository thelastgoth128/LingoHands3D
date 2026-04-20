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
        finalBoneMatrices.resize(200, glm::mat4(1.0f));
    }
    
    void SetBoneRotation(const std::string& boneName, glm::quat rotation) {
        manualTransforms[boneName] = glm::mat4_cast(rotation);
    }

    
    void ResetAllBones() {
        manualTransforms.clear();
        std::fill(finalBoneMatrices.begin(), finalBoneMatrices.end(), glm::mat4(1.0f));
    }
    
    void UpdateAnimation(float deltaTime) {
        // Initialize all bone matrices to identity safely
        for (int i = 0; i < finalBoneMatrices.size(); i++) {
            finalBoneMatrices[i] = glm::mat4(1.0f);
        }
        
        // Calculate all transformations starting from the root of the hierarchy
        CalculateBoneTransform(model->GetRootNode(), glm::mat4(1.0f));
    }

private:
    void CalculateBoneTransform(const SkeletonNode& node, glm::mat4 parentTransform) {
        std::string boneName = node.name;
        glm::mat4 localTransform = node.localBindTransform;

        // Apply manual rotations if they exist
        if (manualTransforms.find(boneName) != manualTransforms.end()) {
            // Apply the local manual rotation on top of the base bind pose transform
            // (Assumes local pivot is at the bone origin)
            localTransform = localTransform * manualTransforms[boneName];
        }

        // Global transform of the current node
        glm::mat4 globalTransform = parentTransform * localTransform;

        // If this node is a mapped bone, compute its final matrix
        auto& boneMap = model->GetBoneInfoMap();
        if (boneMap.find(boneName) != boneMap.end()) {
            int boneID = boneMap[boneName].id;
            glm::mat4 offset = boneMap[boneName].offset;
            
            // Final Matrix = Global Transform * Inverse Bind Transform (Offset)
            if (boneID >= 0 && boneID < finalBoneMatrices.size()) {
                finalBoneMatrices[boneID] = globalTransform * offset;
            }
        }

        // Recursively compute the children
        for (const SkeletonNode& child : node.children) {
            CalculateBoneTransform(child, globalTransform);
        }
    }
public:
    
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