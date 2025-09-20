#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in ivec4 aBoneIDs;   // Bone indices
layout (location = 4) in vec4 aWeights;    // Bone weights

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const int MAX_BONES = 100;
uniform mat4 bones[MAX_BONES];

void main()
{
    // Blend bone transforms
    mat4 boneTransform =
        bones[aBoneIDs[0]] * aWeights[0] +
        bones[aBoneIDs[1]] * aWeights[1] +
        bones[aBoneIDs[2]] * aWeights[2] +
        bones[aBoneIDs[3]] * aWeights[3];

    vec4 transformedPos = boneTransform * vec4(aPos, 1.0);
    FragPos = vec3(model * transformedPos);
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;

    gl_Position = projection * view * model * transformedPos;
}
