#include "texture_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



unsigned int TextureLoader::TextureFromFile(const char *path, const std::string &directory)
{   
    std::string filename;
    if (std::string(path).find(':') != std::string::npos) {
        filename = std::string(path);
    }else {
        filename = directory + '/' + std::string(path);
    }
    std::cout<<"filename: "<< filename<<endl;

    unsigned int textureID;
        std::cout<<"loading texture filename"<<endl;
    glGenTextures(1, &textureID);
    
     std::cout<<"loading gen textures"<<endl;


    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    
            std::cout<<"loading data"<< data <<endl;
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

            std::cout<<"loading data"<< data <<endl;
            
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);


        GLenum err = glGetError();
if (err != GL_NO_ERROR) {
    std::cerr << "[OpenGL] Texture upload error: " << err << std::endl;
}


        // Texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        

        stbi_image_free(data);
    }
    else
    {
        std::cerr << "Texture failed to load at path: " << filename << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int TextureLoader::TextureFromEmbedded(const aiTexture* texture) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    GLenum format = GL_RGB;
    int width, height, nrComponents;
    unsigned char* data = nullptr;

    if (texture->mHeight == 0) {
        // Compressed texture (e.g., PNG or JPG)
        data = stbi_load_from_memory(
            reinterpret_cast<unsigned char*>(texture->pcData),
            texture->mWidth,
            &width, &height, &nrComponents, 0);
    } else {
        // Raw texture data
        data = reinterpret_cast<unsigned char*>(texture->pcData);
        width = texture->mWidth;
        height = texture->mHeight;
        nrComponents = 4; // usually RGBA
    }

    if (data) {
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (texture->mHeight == 0) stbi_image_free(data);
    } else {
        std::cerr << "Failed to load embedded texture." << std::endl;
    }

    return textureID;
}

