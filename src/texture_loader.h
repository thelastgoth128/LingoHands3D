#pragma once
#include <iostream>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/texture.h>
#include <glad/glad.h>
#include "GLFW/glfw3.h"

using namespace std;

class TextureLoader {
    public:
      static  unsigned int TextureFromFile(const char *path, const std::string &directory);
      static unsigned int TextureFromEmbedded(const aiTexture* texture);
};
