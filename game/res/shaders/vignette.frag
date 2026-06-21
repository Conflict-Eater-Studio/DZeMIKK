#version 330 core

in vec2 vUV;

out vec4 FragColor;

uniform sampler2D screenTexture;

void main()
{
    vec3 color = texture(screenTexture, vUV).rgb;

    vec2 uv = vUV * 2.0 - 1.0;
    float dist = dot(uv, uv);

    float vignette = smoothstep(2.0, 0.3, dist);

    color *= vignette;

    color = mix(vec3(0.02), color, 1.0);

    FragColor = vec4(color, 1.0);
}