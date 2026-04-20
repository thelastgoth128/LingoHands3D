
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/scene.h>

struct KeyPosition {
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation {
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale {
    glm::vec3 scale;
    float timeStamp;
};

class Bone {
private:
    std::vector<KeyPosition> m_Positions;
    std::vector<KeyRotation> m_Rotations;
    std::vector<KeyScale> m_Scales;
    int m_NumPositions;
    int m_NumRotations;
    int m_NumScalings;

    glm::mat4 m_LocalTransform;
    std::string m_Name;
    int m_ID;

public:
    Bone(const std::string& name, int ID, const aiNodeAnim* channel)
        : m_Name(name), m_ID(ID), m_LocalTransform(1.0f)
    {
        m_NumPositions = channel->mNumPositionKeys;
        for (int i = 0; i < m_NumPositions; ++i) {
            KeyPosition kp;
            kp.position = glm::vec3(
                channel->mPositionKeys[i].mValue.x,
                channel->mPositionKeys[i].mValue.y,
                channel->mPositionKeys[i].mValue.z
            );
            kp.timeStamp = static_cast<float>(channel->mPositionKeys[i].mTime);
            m_Positions.push_back(kp);
        }

        m_NumRotations = channel->mNumRotationKeys;
        for (int i = 0; i < m_NumRotations; ++i) {
            KeyRotation kr;
            kr.orientation = glm::quat(
                channel->mRotationKeys[i].mValue.w,
                channel->mRotationKeys[i].mValue.x,
                channel->mRotationKeys[i].mValue.y,
                channel->mRotationKeys[i].mValue.z
            );
            kr.timeStamp = static_cast<float>(channel->mRotationKeys[i].mTime);
            m_Rotations.push_back(kr);
        }

        m_NumScalings = channel->mNumScalingKeys;
        for (int i = 0; i < m_NumScalings; ++i) {
            KeyScale ks;
            ks.scale = glm::vec3(
                channel->mScalingKeys[i].mValue.x,
                channel->mScalingKeys[i].mValue.y,
                channel->mScalingKeys[i].mValue.z
            );
            ks.timeStamp = static_cast<float>(channel->mScalingKeys[i].mTime);
            m_Scales.push_back(ks);
        }
    }

    void Update(float animationTime) {
        glm::mat4 translation = InterpolatePosition(animationTime);
        glm::mat4 rotation = InterpolateRotation(animationTime);
        glm::mat4 scale = InterpolateScaling(animationTime);
        m_LocalTransform = translation * rotation * scale;
    }

    glm::mat4 GetLocalTransform() const { return m_LocalTransform; }
    std::string GetBoneName() const { return m_Name; }
    int GetBoneID() const { return m_ID; }

private:
    int GetPositionIndex(float time) const {
        for (int i = 0; i < m_NumPositions - 1; ++i) {
            if (time < m_Positions[i + 1].timeStamp)
                return i;
        }
        return 0;
    }

    int GetRotationIndex(float time) const {
        for (int i = 0; i < m_NumRotations - 1; ++i) {
            if (time < m_Rotations[i + 1].timeStamp)
                return i;
        }
        return 0;
    }

    int GetScaleIndex(float time) const {
        for (int i = 0; i < m_NumScalings - 1; ++i) {
            if (time < m_Scales[i + 1].timeStamp)
                return i;
        }
        return 0;
    }

    float GetScaleFactor(float lastTime, float nextTime, float currentTime) const {
        float duration = nextTime - lastTime;
        return duration == 0.0f ? 0.0f : (currentTime - lastTime) / duration;
    }

    glm::mat4 InterpolatePosition(float time) {
        if (m_NumPositions == 1)
            return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

        int index = GetPositionIndex(time);
        float factor = GetScaleFactor(m_Positions[index].timeStamp, m_Positions[index + 1].timeStamp, time);
        glm::vec3 interp = glm::mix(m_Positions[index].position, m_Positions[index + 1].position, factor);
        return glm::translate(glm::mat4(1.0f), interp);
    }

    glm::mat4 InterpolateRotation(float time) {
        if (m_NumRotations == 1)
            return glm::toMat4(glm::normalize(m_Rotations[0].orientation));

        int index = GetRotationIndex(time);
        float factor = GetScaleFactor(m_Rotations[index].timeStamp, m_Rotations[index + 1].timeStamp, time);
        glm::quat interp = glm::slerp(m_Rotations[index].orientation, m_Rotations[index + 1].orientation, factor);
        return glm::toMat4(glm::normalize(interp));
    }

    glm::mat4 InterpolateScaling(float time) {
        if (m_NumScalings == 1)
            return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

        int index = GetScaleIndex(time);
        float factor = GetScaleFactor(m_Scales[index].timeStamp, m_Scales[index + 1].timeStamp, time);
        glm::vec3 interp = glm::mix(m_Scales[index].scale, m_Scales[index + 1].scale, factor);
        return glm::scale(glm::mat4(1.0f), interp);
    }
};
