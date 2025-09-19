#pragma once
#include <iostream>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/texture.h>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "GLFW/glfw3.h"
#include <iostream>

using namespace std;

class TextureLoader {
    public:
      static  unsigned int TextureFromFile(const char *path, const std::string &directory);
      static unsigned int TextureFromEmbedded(const aiTexture* texture);
};
