// model.cpp - IMPLEMENTATION FILE (no #pragma once)

#include "model.h"  // Include your header

void Model::Draw(Shader &shader) {
    for(unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shader);
}

void Model::loadModel(string path) {
    std::cout << "[Model] Loading model from: " << path << std::endl;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
    
    // Build the skeleton hierarchy after the meshes are loaded
    buildSkeletonHierarchy(m_RootNode, scene->mRootNode);
    
    std::cout << "[Model] Finished loading.\n";
}

void Model::buildSkeletonHierarchy(SkeletonNode& dest, const aiNode* src) {
    if (!src) return;
    
    dest.name = src->mName.C_Str();
    
    // Extract local transformation matrix from aiNode
    aiMatrix4x4 t = src->mTransformation;
    dest.localBindTransform = glm::mat4(
        t.a1, t.b1, t.c1, t.d1,
        t.a2, t.b2, t.c2, t.d2,
        t.a3, t.b3, t.c3, t.d3,
        t.a4, t.b4, t.c4, t.d4
    );
    
    for (unsigned int i = 0; i < src->mNumChildren; ++i) {
        SkeletonNode childNode;
        buildSkeletonHierarchy(childNode, src->mChildren[i]);
        dest.children.push_back(childNode);
    }
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i)
        processNode(node->mChildren[i], scene);
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex{};
        vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        vertex.TexCoords = mesh->mTextureCoords[0] ?
            glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) :
            glm::vec2(0.0f, 0.0f);

        // Initialize bone data
        SetVertexBoneDataToDefault(vertex);
        vertices.push_back(vertex);
    }

    // Process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    // Process materials
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        auto diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        auto specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", scene);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        if (textures.empty()) {
            Texture fallback;
            fallback.id = TextureLoader::TextureFromFile("fallback.png", directory);
            fallback.type = "texture_diffuse";
            fallback.path = "fallback.png";
            textures.push_back(fallback);
            textures_loaded.push_back(fallback);
        }
    }

    ExtractBoneWeightForVertices(vertices, mesh, scene);
    return Mesh(vertices, indices, textures);
}

void Model::ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene) {
    for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
        int boneID = -1;

        if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end()) {
            BoneInfo newBoneInfo;
            newBoneInfo.id = m_BoneCounter;
            
            // Convert matrix
            aiMatrix4x4 offset = mesh->mBones[boneIndex]->mOffsetMatrix;
            newBoneInfo.offset = glm::mat4(
                offset.a1, offset.b1, offset.c1, offset.d1,
                offset.a2, offset.b2, offset.c2, offset.d2,
                offset.a3, offset.b3, offset.c3, offset.d3,
                offset.a4, offset.b4, offset.c4, offset.d4
            );
            
            m_BoneInfoMap[boneName] = newBoneInfo;
            boneID = m_BoneCounter++;
        } else {
            boneID = m_BoneInfoMap[boneName].id;
        }

        assert(boneID != -1);

        for (unsigned int weightIndex = 0; weightIndex < mesh->mBones[boneIndex]->mNumWeights; ++weightIndex) {
            int vertexID = mesh->mBones[boneIndex]->mWeights[weightIndex].mVertexId;
            float weight = mesh->mBones[boneIndex]->mWeights[weightIndex].mWeight;
            assert(vertexID < vertices.size());
            
            // Bone ID validation against the current shader limit
            if (boneID >= 200) {
                static bool warned = false;
                if (!warned) {
                    std::cerr << "WARNING::MODEL::BONE_ID_LIMIT_EXCEEDED: Bone ID " << boneID 
                              << " exceeds shader limit of 200. Vertices will stay at bind pose.\n";
                    warned = true;
                }
                continue;
            }

            // Find empty slot and assign bone data
            for (int i = 0; i < MAX_BONE_WEIGHTS; ++i) {
                if (vertices[vertexID].m_BoneIDs[i] < 0) {
                    vertices[vertexID].m_BoneIDs[i] = boneID;
                    vertices[vertexID].m_Weights[i] = weight;
                    break;
                }
            }
        }
    }

    // Weight Normalization: Ensure all vertex weights sum up to 1.0
    for (size_t i = 0; i < vertices.size(); i++) {
        float totalWeight = 0.0f;
        for (int j = 0; j < MAX_BONE_WEIGHTS; j++) {
            if (vertices[i].m_BoneIDs[j] != -1) {
                totalWeight += vertices[i].m_Weights[j];
            }
        }

        if (totalWeight > 0.0f && std::abs(totalWeight - 1.0f) > 0.0001f) {
            for (int j = 0; j < MAX_BONE_WEIGHTS; j++) {
                if (vertices[i].m_BoneIDs[j] != -1) {
                    vertices[i].m_Weights[j] /= totalWeight;
                }
            }
        }
    }
}
std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, const aiScene* scene) {
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
        aiString str;
        mat->GetTexture(type, i, &str);

        // Check if already loaded
        bool skip = false;
        for (auto& loaded : textures_loaded) {
            if (std::strcmp(loaded.path.c_str(), str.C_Str()) == 0) {
                textures.push_back(loaded);
                skip = true;
                break;
            }
        }

        if (!skip) {
            Texture texture;
            
            // First, try to load embedded texture
            const aiTexture* embeddedTex = scene->GetEmbeddedTexture(str.C_Str());
            
            if (embeddedTex) {
                std::cout << "[Texture] Loading embedded texture: " << str.C_Str() << "\n";
                texture.id = TextureLoader::TextureFromEmbedded(embeddedTex);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);
            } else {
                // Try to load from file system
                std::cout << "[Texture] Trying to load external texture: " << str.C_Str() << "\n";
                try {
                    texture.id = TextureLoader::TextureFromFile(str.C_Str(), directory);
                    texture.type = typeName;
                    texture.path = str.C_Str();
                    textures.push_back(texture);
                    textures_loaded.push_back(texture);
                } catch (...) {
                    std::cout << "[Texture] Failed to load texture: " << str.C_Str() << "\n";
                    // Don't add failed textures to the list
                }
            }
        }
    }

    // ONLY add fallback if NO textures were loaded at all
    if (textures.empty()) {
        std::cout << "[Texture] No textures loaded, creating fallback...\n";
        
        // Create a simple colored texture instead of loading from file
        Texture fallback;
        fallback.id = createDefaultTexture(); // We'll implement this
        fallback.type = typeName;
        fallback.path = "default_texture";
        textures.push_back(fallback);
        textures_loaded.push_back(fallback);
    }

    return textures;
}


// Add this helper function to create a default texture
unsigned int Model::createDefaultTexture() {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Create a simple white texture
    unsigned char data[] = {255, 255, 255, 255}; // White pixel
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    std::cout << "[Texture] Created default white texture\n";
    return textureID;
}