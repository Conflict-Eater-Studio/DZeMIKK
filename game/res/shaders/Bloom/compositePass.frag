#version 330 core

in vec2 vUV;

uniform sampler2D sceneTexture;
uniform sampler2D bloomTexture;

uniform float intensity;

out vec4 FragColor;

void main()
{
    vec3 scene =
        texture(sceneTexture,vUV).rgb;

    vec3 bloom =
        texture(bloomTexture,vUV).rgb;

    FragColor =
        vec4(scene + bloom * intensity,1.0);
}