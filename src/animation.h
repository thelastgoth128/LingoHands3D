#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include "bone_info.h"
#include <functional>
#include <string>

struct AssimpNodeData {
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

class Bone {
public:
    Bone(const std::string& name, int ID, const aiNodeAnim* channel)
        : m_Name(name), m_ID(ID), m_LocalTransform(1.0f) {
        // Implementation here or in .cpp
    }

    void Update(float currentTime) {
        // Interpolation logic
    }

    glm::mat4 GetLocalTransform() { return m_LocalTransform; }
    std::string GetBoneName() const { return m_Name; }
    int GetBoneID() { return m_ID; }

private:
    std::string m_Name;
    int m_ID;
    glm::mat4 m_LocalTransform;
};

class Animation {
public:
    Animation() = default;
    Animation(const std::string& animationPath, Model* model) {
        // Load animation from file
    }

    Bone* FindBone(const std::string& name) {
        auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
            [&](const Bone& bone) {
                return bone.GetBoneName() == name;
            }
        );
        if (iter == m_Bones.end()) return nullptr;
        else return &(*iter);
    }

    float GetTicksPerSecond() { return m_TicksPerSecond; }
    float GetDuration() { return m_Duration; }
    const AssimpNodeData& GetRootNode() { return m_RootNode; }
    const std::map<std::string, BoneInfo>& GetBoneIDMap() { return m_BoneIDMap; }

private:
    float m_Duration;
    int m_TicksPerSecond;
    std::vector<Bone> m_Bones;
    AssimpNodeData m_RootNode;
    std::map<std::string, BoneInfo> m_BoneIDMap;
};