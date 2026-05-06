#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 3) in ivec4 aBoneIDs;
layout(location = 4) in vec4 aWeights;

uniform mat4 model;
uniform mat4 u_Bones[250];

out vec3 FragPos;
out vec3 Normal;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

void main() {
    mat4 skin =
        aWeights.x * u_Bones[aBoneIDs.x] +
        aWeights.y * u_Bones[aBoneIDs.y] +
        aWeights.z * u_Bones[aBoneIDs.z] +
        aWeights.w * u_Bones[aBoneIDs.w];

    vec4 skinnedPos = skin * vec4(aPos, 1.0);

    mat3 skinNormal = mat3(skin);

    vec4 worldPos = model * skinnedPos;

    FragPos = worldPos.xyz;
    Normal = normalize(mat3(transpose(inverse(model * skin))) * aNormal);

    gl_Position = projection * view * worldPos;
}