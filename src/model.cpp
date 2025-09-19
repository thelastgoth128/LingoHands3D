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


vector<Texture> Model::loadMaterialTextures(aiMaterial *mat,aiTextureType type,string typeName, const aiScene* scene)
 {
std::cout<<"getting texture diffuse" << endl;
 vector<Texture>textures;
 for(unsigned int i=0; i < mat->GetTextureCount(type);i++)
 {
 aiString str;
 mat->GetTexture(type, i, &str);
 bool skip = false;
 for(unsigned int j = 0; j < textures_loaded.size();j++)
 {
 if(std::strcmp(textures_loaded[j].path.data(),
 str.C_Str())==0)
 {
 textures.push_back(textures_loaded[j]);
 skip=true;
 break;
 }
 }
 if(!skip)
 {//iftexturehasn’tbeenloadedalready,loadit
std::cout<<"Getting texture from embedded" << endl;
std::cout << "[TextureLoader] Attempting to load texture: " << str.C_Str() << " of type: " << typeName << std::endl;
}

 if (mat->GetTexture(type, i, &str) == AI_SUCCESS) {
   const aiTexture* embeddedTex = scene->GetEmbeddedTexture(str.C_Str());

   if (embeddedTex) {
    std::cout << "[TextureLoader] Embedded texture found: " << str.C_Str() << std::endl;
} else {
    std::cerr << "[TextureLoader] Embedded texture NOT found for: " << str.C_Str() << std::endl;
}

   if (embeddedTex) {
      std::cout << "[TextureLoader] Embedded texture found: " << str.C_Str() << std::endl;
        try {
            Texture texture;
            texture.id = TextureLoader::TextureFromEmbedded(embeddedTex);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);
            continue; // skip file-based loading
        } catch (...) {
            std::cerr << "[TextureLoader] Failed to load embedded texture: " << str.C_Str() << std::endl;
        }
   }
   std::cout << "[TextureLoader] Trying external texture: " << str.C_Str() << std::endl;
    try {
        Texture texture;
        texture.id = TextureLoader::TextureFromFile(str.C_Str(), directory);
        texture.type = typeName;
        texture.path = str.C_Str();
        textures.push_back(texture);
        textures_loaded.push_back(texture);
        std::cout << "Done loading texture" << endl;
    } catch (...) {
        std::cerr << "[TextureLoader] Failed to load external texture: " << str.C_Str() << std::endl;
    }
 }
 }
 if (textures.empty()) {
    std::cout << "[TextureLoader] No textures found. Injecting fallback.\n";
    Texture fallback;
    fallback.id = TextureLoader::TextureFromFile("C:/Users/HP/OneDrive/Documentos/Cyrus/Projects/model_loading/src/Textures/source/684c0f35-ba0b-48e0-ab19-db572ea748d3.glb", directory);
    fallback.type = typeName;
    fallback.path = "C:/Users/HP/OneDrive/Documentos/Cyrus/Projects/model_loading/src/Textures/source/684c0f35-ba0b-48e0-ab19-db572ea748d3.glb";
    textures.push_back(fallback);
    textures_loaded.push_back(fallback);
}

 return textures;
 }
