#version 330 core

in vec2 vUV;

out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float grainIntensity = 0.5;
uniform float grainSize = 5;

float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}
void main()
{
    vec3 color = texture(screenTexture, vUV).rgb;

    vec2 grainUV = vUV / max(grainSize, 0.001);
    vec2 seed = grainUV * textureSize(screenTexture, 0);

    float noise = rand(seed) - 0.5;

    noise *= grainIntensity;
    color += noise;

    FragColor = vec4(color, 1.0);
}