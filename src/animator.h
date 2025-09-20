#pragma once

#include "animation.h"
#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

class Animator {
public:
    Animator(Animation* animation) {
        m_CurrentTime = 0.0;
        m_CurrentAnimation = animation;
        
        m_FinalBoneMatrices.reserve(100);
        for (int i = 0; i < 100; i++)
            m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
    }

    void UpdateAnimation(float dt) {
        m_DeltaTime = dt;
        if (m_CurrentAnimation) {
            m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
            m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
            CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
        }
    }

    void PlayAnimation(Animation* pAnimation) {
        m_CurrentAnimation = pAnimation;
        m_CurrentTime = 0.0f;
    }

    void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform) {
        std::string nodeName = node->name;
        glm::mat4 nodeTransform = node->transformation;

        Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

        if (Bone) {
            Bone->Update(m_CurrentTime);
            nodeTransform = Bone->GetLocalTransform();
        }

        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
        if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
            int index = boneInfoMap[nodeName].id;
            glm::mat4 offset = boneInfoMap[nodeName].offset;
            m_FinalBoneMatrices[index] = globalTransformation * offset;
        }

        for (int i = 0; i < node->childrenCount; i++)
            CalculateBoneTransform(&node->children[i], globalTransformation);
    }

    std::vector<glm::mat4> GetFinalBoneMatrices() {
        return m_FinalBoneMatrices;
    }

    // Manual bone control methods
    void SetBoneRotation(const std::string& boneName, glm::vec3 rotation) {
        if (m_CurrentAnimation) {
            Bone* bone = m_CurrentAnimation->FindBone(boneName);
            if (bone) {
                // Create a custom transformation matrix
                glm::mat4 rotMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1, 0, 0));
                rotMatrix = glm::rotate(rotMatrix, glm::radians(rotation.y), glm::vec3(0, 1, 0));
                rotMatrix = glm::rotate(rotMatrix, glm::radians(rotation.z), glm::vec3(0, 0, 1));
                
                // Store manual transformations
                m_ManualTransforms[boneName] = rotMatrix;
            }
        }
    }

    void ResetBone(const std::string& boneName) {
        m_ManualTransforms.erase(boneName);
    }

    void ResetAllBones() {
        m_ManualTransforms.clear();
    }

private:
    std::vector<glm::mat4> m_FinalBoneMatrices;
    Animation* m_CurrentAnimation;
    float m_CurrentTime;
    float m_DeltaTime;
    
    // For manual bone control
    std::map<std::string, glm::mat4> m_ManualTransforms;
};