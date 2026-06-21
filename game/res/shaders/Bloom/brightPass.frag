#version 330 core

in vec2 vUV;

uniform sampler2D screenTexture;
uniform float threshold;

out vec4 FragColor;

void main()
{
    vec3 color = texture(screenTexture, vUV).rgb;

    float brightness =
        dot(color, vec3(0.2126,0.7152,0.0722));

    FragColor =
        brightness > threshold
        ? vec4(1,1,1,1)
        : vec4(0,0,0,1);
}