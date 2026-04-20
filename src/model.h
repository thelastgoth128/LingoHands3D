#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/texture.h>

#include "shaders/shader.h"
#include "bone_info.h"        // This already defines BoneInfo
#include "mesh.h"
#include "texture_loader.h"
#include <map>

struct SkeletonNode {
    std::string name;
    glm::mat4 localBindTransform;
    std::vector<SkeletonNode> children;
};


using namespace std;

// REMOVE THIS - it's already in bone_info.h
// struct BoneInfo {
//    int id;
//    glm::mat4 offset;
// };

class Model
{
private:
    std::map<string, BoneInfo> m_BoneInfoMap;
    int m_BoneCounter = 0;
    SkeletonNode m_RootNode; // Add this to store the hierarchy

    void SetVertexBoneDataToDefault(Vertex& vertex) {
        for (int i = 0; i < MAX_BONE_WEIGHTS; i++) {  // Use MAX_BONE_WEIGHTS instead of AI_MAX_BONE_WEIGHTS
            vertex.m_BoneIDs[i] = -1;
            vertex.m_Weights[i] = 0.0f; 
        }
    }
    
public:
    auto& GetBoneInfoMap() {
        return m_BoneInfoMap;
    }
    
    int& GetBoneCount() {
        return m_BoneCounter;
    }

    const SkeletonNode& GetRootNode() const {
        return m_RootNode;
    }

    Model(const char *path) {
        loadModel(path);
    }
    
    void Draw(Shader &shader);

private:
    // model data
    vector<Texture> textures_loaded;
    vector<Mesh> meshes;
    string directory;
    
    void loadModel(string path);
    void processNode(aiNode *node, const aiScene *scene);
    void buildSkeletonHierarchy(SkeletonNode& dest, const aiNode* src);
    
    // FIX THIS LINE - add template parameter
    void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
    
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, const aiScene* scene);
    unsigned int createDefaultTexture();
};