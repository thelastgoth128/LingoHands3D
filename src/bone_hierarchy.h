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

            void PrintBoneHierarchy(BoneNode* node, int depth = 0) {
    if (!node) return;

    std::string indent(depth * 2, ' ');
    std::cout << indent << "- " << node->name << "\n";

    for (BoneNode* child : node->children) {
        PrintBoneHierarchy(child, depth + 1);
    }
}


void PrintHierarchy() {
    std::cout << "[Animator] Bone Hierarchy:\n";
    PrintBoneHierarchy(rootBone);
}
    
    void SetBoneRotation(const std::string& boneName, glm::quat rotation) {
    // Directly convert quaternion to rotation matrix
    glm::mat4 rotMatrix = glm::mat4_cast(rotation);
    manualTransforms[boneName] = rotMatrix;

    // Apply scaled quaternion rotations to connected bones
    if (boneName == "RightArm") {
        // glm::quat forearmRot = glm::angleAxis(glm::angle(rotation) * 0.3f, glm::axis(rotation));
        // manualTransforms["RightForeArm"] = glm::mat4_cast(forearmRot);

        glm::quat righthand = glm::angleAxis(glm::angle(rotation) * 0.3f, glm::axis(rotation));
        manualTransforms["RightHandIndex1"] = glm::mat4_cast(righthand);

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