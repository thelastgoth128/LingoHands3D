#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>
#include <vector>
#include <string>
#include "model.h"

struct BoneNode {
    std::string name;
    glm::mat4 localTransform;
    glm::mat4 globalTransform;
    std::vector<BoneNode*> children;
    BoneNode* parent;
    
    BoneNode(const std::string& n) : name(n), localTransform(1.0f), globalTransform(1.0f), parent(nullptr) {}
};

class HierarchicalAnimator {
private:
    std::map<std::string, glm::mat4> manualTransforms;
    std::map<std::string, BoneNode*> boneNodes;
    std::vector<glm::mat4> finalBoneMatrices;
    Model* model;
    BoneNode* rootBone;
    
    void buildBoneHierarchy() {
        auto& boneMap = model->GetBoneInfoMap();
        
        // Create all bone nodes
        for (auto& [name, info] : boneMap) {
            boneNodes[name] = new BoneNode(name);
        }
        
        // Set up hierarchy (simplified - you might need to adjust based on your model)
        setupHierarchy();
        
        // Find root bone (usually Hips)
        rootBone = boneNodes.find("Hips") != boneNodes.end() ? boneNodes["Hips"] : nullptr;
    }
    
    void setupHierarchy() {
        // Define the bone hierarchy structure
        // This is a simplified version - you might need to adjust based on your specific model
        
        auto addChild = [this](const std::string& parent, const std::string& child) {
            if (boneNodes.find(parent) != boneNodes.end() && boneNodes.find(child) != boneNodes.end()) {
                boneNodes[parent]->children.push_back(boneNodes[child]);
                boneNodes[child]->parent = boneNodes[parent];
            }
        };
        
        // Spine hierarchy
        addChild("Hips", "Spine");
        addChild("Spine", "Spine1");
        addChild("Spine1", "Spine2");
        addChild("Spine2", "Neck");
        addChild("Neck", "Head");
        
        // Left arm hierarchy
        addChild("Spine2", "LeftShoulder");
        addChild("LeftShoulder", "LeftArm");
        addChild("LeftArm", "LeftForeArm");
        addChild("LeftForeArm", "LeftHand");
        
        // Right arm hierarchy
        addChild("Spine2", "RightShoulder");
        addChild("RightShoulder", "RightArm");
        addChild("RightArm", "RightForeArm");
        addChild("RightForeArm", "RightHand");
        
        // Add finger hierarchies
        addChild("RightHand", "RightHandThumb1");
        addChild("RightHandThumb1", "RightHandThumb2");
        addChild("RightHandThumb2", "RightHandThumb3");
        addChild("RightHandThumb3", "RightHandThumb4");
        
        addChild("RightHand", "RightHandIndex1");
        addChild("RightHandIndex1", "RightHandIndex2");
        addChild("RightHandIndex2", "RightHandIndex3");
        addChild("RightHandIndex3", "RightHandIndex4");
        
        // Left finger hierarchies
        addChild("LeftHand", "LeftHandIndex1");
        addChild("LeftHandIndex1", "LeftHandIndex2");
        addChild("LeftHandIndex2", "LeftHandIndex3");
        addChild("LeftHandIndex3", "LeftHandIndex4");
        
        // Leg hierarchies
        addChild("Hips", "LeftUpLeg");
        addChild("LeftUpLeg", "LeftLeg");
        addChild("LeftLeg", "LeftFoot");
        addChild("LeftFoot", "LeftToeBase");
        
        addChild("Hips", "RightUpLeg");
        addChild("RightUpLeg", "RightLeg");
        addChild("RightLeg", "RightFoot");
        addChild("RightFoot", "RightToeBase");
    }
    
    void updateGlobalTransforms(BoneNode* node, const glm::mat4& parentTransform) {
        if (!node) return;
        
        // Apply manual transform if it exists, otherwise use identity
        glm::mat4 localTransform = glm::mat4(1.0f);
        if (manualTransforms.find(node->name) != manualTransforms.end()) {
            localTransform = manualTransforms[node->name];
        }
        
        node->localTransform = localTransform;
        node->globalTransform = parentTransform * localTransform;
        
        // Update all children
        for (BoneNode* child : node->children) {
            updateGlobalTransforms(child, node->globalTransform);
        }
    }
    
public:
    HierarchicalAnimator(Model* m) : model(m), rootBone(nullptr) {
        finalBoneMatrices.resize(100, glm::mat4(1.0f));
        buildBoneHierarchy();
    }
    
    ~HierarchicalAnimator() {
        for (auto& [name, node] : boneNodes) {
            delete node;
        }
    }
    
    void SetBoneRotation(const std::string& boneName, glm::vec3 rotation) {
        glm::mat4 rotMatrix = glm::mat4(1.0f);
        rotMatrix = glm::rotate(rotMatrix, glm::radians(rotation.x), glm::vec3(1, 0, 0));
        rotMatrix = glm::rotate(rotMatrix, glm::radians(rotation.y), glm::vec3(0, 1, 0));
        rotMatrix = glm::rotate(rotMatrix, glm::radians(rotation.z), glm::vec3(0, 0, 1));
        
        manualTransforms[boneName] = rotMatrix;
        std::cout << "[Animator] Set rotation for bone: " << boneName << "\n";
    }
    
    void ResetAllBones() {
        manualTransforms.clear();
        std::fill(finalBoneMatrices.begin(), finalBoneMatrices.end(), glm::mat4(1.0f));
        std::cout << "[Animator] Reset all bones\n";
    }
    
    void UpdateAnimation(float deltaTime) {
        auto& boneMap = model->GetBoneInfoMap();
        
        // Initialize all matrices to identity
        std::fill(finalBoneMatrices.begin(), finalBoneMatrices.end(), glm::mat4(1.0f));
        
        // Update the hierarchy starting from root
        if (rootBone) {
            updateGlobalTransforms(rootBone, glm::mat4(1.0f));
        }
        
        // Calculate final bone matrices
        for (auto& [boneName, node] : boneNodes) {
            if (boneMap.find(boneName) != boneMap.end()) {
                int boneID = boneMap[boneName].id;
                if (boneID >= 0 && boneID < finalBoneMatrices.size()) {
                    // Apply the hierarchy transform with the offset matrix
                    finalBoneMatrices[boneID] = node->globalTransform * boneMap[boneName].offset;
                }
            }
        }
        
        // Handle bones not in hierarchy (set to bind pose)
        for (auto& [boneName, boneInfo] : boneMap) {
            int boneID = boneInfo.id;
            if (boneID >= 0 && boneID < finalBoneMatrices.size() && 
                boneNodes.find(boneName) == boneNodes.end()) {
                finalBoneMatrices[boneID] = boneInfo.offset;
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