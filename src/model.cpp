#include "model.h"



 void Model::Draw(Shader &shader)
 {
 for(unsigned int i = 0; i < meshes.size(); i++)
 meshes[i].Draw(shader);
}

 void Model::loadModel(string path)
 {
   std::cout << "[Model] Constructor called with path: " << path << std::endl;
 Assimp::Importer import;
 const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate |
 aiProcess_FlipUVs);

 if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||!scene->mRootNode) {
   cout << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
   return;
}

std::cout << "Animations: " << scene->mNumAnimations << std::endl;
std::cout << "Meshes: " << scene->mNumMeshes << std::endl;
std::cout << "Bones in first mesh: " << scene->mMeshes[0]->mNumBones << std::endl;

 directory = path.substr(0, path.find_last_of('/'));
 processNode(scene->mRootNode, scene);
  std::cout << "[Model] Constructor finished.\n";
}

 void Model::processNode(aiNode *node, const aiScene *scene)
 {
 // process all the node’s meshes (if any)
 for(unsigned int i = 0; i < node->mNumMeshes; i++)
 {
 aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
 meshes.push_back(processMesh(mesh, scene));
 }
 // then do the same for each of its children
 for(unsigned int i = 0; i < node->mNumChildren; i++)
 {
 processNode(node->mChildren[i], scene);
 }
 std::cout << "[Model] Finished processing node: " << node->mName.C_Str() << std::endl;


}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
 {
 vector<Vertex> vertices;
 vector<unsigned int> indices;
 vector<Texture> textures;
 for(unsigned int i = 0; i < mesh->mNumVertices; i++)
 {
 Vertex vertex;
 // process vertex positions, normals and texture coordinates
    glm::vec3 vector;
 vector.x = mesh->mVertices[i].x;
 vector.y = mesh->mVertices[i].y;
 vector.z = mesh->mVertices[i].z;
 vertex.Position = vector;

 vector.x = mesh->mNormals[i].x;
 vector.y = mesh->mNormals[i].y;
 vector.z = mesh->mNormals[i].z;
 vertex.Normal = vector;

 if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
 {
 glm::vec2 vec;
 vec.x = mesh->mTextureCoords[0][i].x;
 vec.y = mesh->mTextureCoords[0][i].y;
 vertex.TexCoords = vec;
 }
 else
 vertex.TexCoords = glm::vec2(0.0f, 0.0f);


 vertices.push_back(vertex);
 }


 // process indices
      for(unsigned int i = 0; i < mesh->mNumFaces; i++)
 {
 aiFace face = mesh->mFaces[i];
 for(unsigned int j = 0; j < face.mNumIndices; j++)
 indices.push_back(face.mIndices[j]);
 }

 std::cout<< "loading texture" << endl;
 // process material
 if(mesh->mMaterialIndex >= 0)
 {
   if(mesh->mMaterialIndex >= 0) {
      aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
       std::cout<< "loading texture_diffuse" << endl;
       std::cout << "[Material] Diffuse count: " << material->GetTextureCount(aiTextureType_DIFFUSE) << std::endl;

      vector<Texture> diffuseMaps = loadMaterialTextures(material,
      aiTextureType_DIFFUSE, "texture_diffuse",scene);
      textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
       std::cout<< "loading texture_specular" << endl;
       std::cout << "[Material] Specular count: " << material->GetTextureCount(aiTextureType_SPECULAR) << std::endl;
      vector<Texture> specularMaps = loadMaterialTextures(material,aiTextureType_SPECULAR, "texture_specular",scene);
      textures.insert(textures.end(), specularMaps.begin(),specularMaps.end());
   
      // Force a known texture if none were loaded
if (textures.empty()) {
    Texture forcedTexture;
    forcedTexture.id = TextureLoader::TextureFromFile("C:/Users/HP/OneDrive/Documentos/Cyrus/Projects/model_loading/src/Textures/source/684c0f35-ba0b-48e0-ab19-db572ea748d3.glb", directory); // or full path
    forcedTexture.type = "texture_diffuse1";
    forcedTexture.path = "C:/Users/HP/OneDrive/Documentos/Cyrus/Projects/model_loading/src/Textures/source/684c0f35-ba0b-48e0-ab19-db572ea748d3.glb";
    textures.push_back(forcedTexture);

    std::cout << "Injected fallback texture: " << forcedTexture.path << std::endl;
}

   }
 }
  std::cout << "[Model] Finished processing mesh with " << vertices.size() << " vertices.\n";

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
