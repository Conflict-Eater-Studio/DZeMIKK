#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D screenTexture;

void main()
{
    vec3 color = texture(screenTexture, vUV).rgb;

    float gray = dot(color, vec3(0.2126, 0.7152, 0.0722));

    float strength = 0.5; // 1 = full grayscale, 0 = no effect

    vec3 result = mix(color, vec3(gray), strength);

    FragColor = vec4(result, 1.0);
}