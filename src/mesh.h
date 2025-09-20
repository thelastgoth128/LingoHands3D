#pragma once
#include <glm/glm.hpp>
#include "shaders/shader.h"
#include <vector>
#include <string>
#include "bone_info.h"
#define MAX_BONE_INFLUENCE 4


struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;

    //tanget
    glm::vec3 Tangent;
    //bitangent
    glm::vec3 Bitangent;

    //bone indexe which will influe this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];

};

struct Texture{
    unsigned int id;
    string type;
    string path;
};

class Mesh {
    public:
        //mesh data
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture>  textures;

        Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);
        void Draw(Shader &shader);

    private:
        //render data
        unsigned int VAO, VBO, EBO;
        void setupMesh();
};