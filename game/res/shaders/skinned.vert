#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;
layout(location = 5) in ivec4 aBoneIDs;
layout(location = 6) in vec4 aWeights;

uniform mat4 model;
uniform mat4 u_Bones[250];

layout(std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out mat3 TBN;

void main()
{
    mat4 skin =
          aWeights.x * u_Bones[aBoneIDs.x]
        + aWeights.y * u_Bones[aBoneIDs.y]
        + aWeights.z * u_Bones[aBoneIDs.z]
        + aWeights.w * u_Bones[aBoneIDs.w];

    vec4 skinnedPos = skin * vec4(aPos, 1.0);

    vec4 worldPos = model * skinnedPos;
    FragPos = worldPos.xyz;

    mat3 normalMatrix =
        transpose(
            inverse(
                mat3(model * skin)));

    vec3 N =
        normalize(
            normalMatrix * aNormal);

    vec3 T =
        normalize(
            normalMatrix * aTangent);

    // Gram-Schmidt
    T = normalize(T - dot(T, N) * N);

    vec3 B = cross(N, T);

    Normal = N;
    TexCoord = aTexCoord;
    TBN = mat3(T, B, N);

    gl_Position =
        projection *
        view *
        worldPos;
}