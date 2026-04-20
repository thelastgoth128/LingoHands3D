#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include "model.h"

struct BoneNode {
    std::string name;
    glm::quat localRotation = glm::quat(1, 0, 0, 0); // identity
    glm::mat4 bindLocalTransform = glm::mat4(1.0f); // original transform from model
    glm::mat4 globalTransform = glm::mat4(1.0f);
    BoneNode* parent = nullptr;
    std::vector<BoneNode*> children;

    BoneNode(const std::string& n) : name(n) {}
    

};

class SimpleAnimator {
private:
    std::map<std::string, BoneNode*> boneNodes;
    std::map<std::string, glm::quat> manualRotations;
    std::vector<glm::mat4> finalBoneMatrices;
    Model* model;
    BoneNode* rootBone = nullptr;

public:
    SimpleAnimator(Model* m) : model(m) {
        finalBoneMatrices.resize(100, glm::mat4(1.0f));
        buildHierarchyMap();
    }

    ~SimpleAnimator() {
        for (auto& [name, node] : boneNodes) {
            delete node;
        }
    }

    void SetBoneRotation(const std::string& boneName, const glm::quat& rotation) {
        manualRotations[boneName] = rotation;
        std::cout << "[Animator] Set quaternion rotation for bone: " << boneName << "\n";
    }

    void ResetAllBones() {
        manualRotations.clear();
        std::fill(finalBoneMatrices.begin(), finalBoneMatrices.end(), glm::mat4(1.0f));
        std::cout << "[Animator] Reset all bone rotations\n";
    }

    void UpdateAnimation(float deltaTime) {
        auto& boneMap = model->GetBoneInfoMap();

        for (auto& [name, node] : boneNodes) {
            node->localRotation = manualRotations.count(name) ? manualRotations[name] : glm::quat(1, 0, 0, 0);
        }

        if (rootBone) {
            updateGlobalTransform(rootBone, glm::mat4(1.0f));
        }

        for (auto& [name, info] : boneMap) {
            int id = info.id;
            if (id >= 0 && id < finalBoneMatrices.size()) {
                if (boneNodes.count(name)) {
                    finalBoneMatrices[id] = boneNodes[name]->globalTransform * info.offset;
                } else {
                    finalBoneMatrices[id] = glm::mat4(1.0f);
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

    void PrintHierarchy(BoneNode* node = nullptr, int depth = 0) {
        if (!node) node = rootBone;
        if (!node) return;

        std::string indent(depth * 2, ' ');
        std::cout << indent << "- " << node->name << "\n";
        for (BoneNode* child : node->children) {
            PrintHierarchy(child, depth + 1);
        }
    }

private:
 std::map<std::string, std::vector<std::string>> boneHierarchy;
    void buildHierarchyMap() {
        // Build parent-child relationships without creating complex structures
        boneHierarchy["RightShoulder"] = {"RightArm"};
        boneHierarchy["RightArm"] = {"RightForeArm"};
        boneHierarchy["RightForeArm"] = {"RightHand"};
        boneHierarchy["RightHand"] = {
            "RightHandThumb1", "RightHandIndex1", "RightHandMiddle1", 
            "RightHandRing1", "RightHandPinky1"
        };
        
        // Finger chains
        boneHierarchy["RightHandThumb1"] = {"RightHandThumb2"};
        boneHierarchy["RightHandThumb2"] = {"RightHandThumb3"};
        boneHierarchy["RightHandThumb3"] = {"RightHandThumb4"};
        
        boneHierarchy["RightHandIndex1"] = {"RightHandIndex2"};
        boneHierarchy["RightHandIndex2"] = {"RightHandIndex3"};
        boneHierarchy["RightHandIndex3"] = {"RightHandIndex4"};
        
        // Mirror for left side
        boneHierarchy["LeftShoulder"] = {"LeftArm"};
        boneHierarchy["LeftArm"] = {"LeftForeArm"};
        boneHierarchy["LeftForeArm"] = {"LeftHand"};
        boneHierarchy["LeftHand"] = {
            "LeftHandThumb1", "LeftHandIndex1", "LeftHandMiddle1", 
            "LeftHandRing1", "LeftHandPinky1"
        };
        
        boneHierarchy["LeftHandIndex1"] = {"LeftHandIndex2"};
        boneHierarchy["LeftHandIndex2"] = {"LeftHandIndex3"};
        boneHierarchy["LeftHandIndex3"] = {"LeftHandIndex4"};
        
        // Spine chain
        boneHierarchy["Hips"] = {"Spine"};
        boneHierarchy["Spine"] = {"Spine1"};
        boneHierarchy["Spine1"] = {"Spine2"};
        boneHierarchy["Spine2"] = {"Neck", "LeftShoulder", "RightShoulder"};
        boneHierarchy["Neck"] = {"Head"};
        boneHierarchy["Head"] = {"HeadTop_End", "LeftEye", "RightEye"};
        
        // Legs
        boneHierarchy["Hips"].insert(boneHierarchy["Hips"].end(), {"LeftUpLeg", "RightUpLeg"});
        boneHierarchy["LeftUpLeg"] = {"LeftLeg"};
        boneHierarchy["LeftLeg"] = {"LeftFoot"};
        boneHierarchy["LeftFoot"] = {"LeftToeBase"};
        boneHierarchy["LeftToeBase"] = {"LeftToe_End"};
        
        boneHierarchy["RightUpLeg"] = {"RightLeg"};
        boneHierarchy["RightLeg"] = {"RightFoot"};
        boneHierarchy["RightFoot"] = {"RightToeBase"};
        boneHierarchy["RightToeBase"] = {"RightToe_End"};
    }
   
    void updateGlobalTransform(BoneNode* node, const glm::mat4& parentTransform) {
        glm::mat4 localMatrix = node->bindLocalTransform * glm::toMat4(node->localRotation);
        node->globalTransform = parentTransform * localMatrix;

        for (BoneNode* child : node->children) {
            updateGlobalTransform(child, node->globalTransform);
        }
    }
};
