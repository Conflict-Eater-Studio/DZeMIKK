#version 330 core

in vec2 vUV;

uniform sampler2D image;
uniform bool horizontal;

out vec4 FragColor;

void main()
{
    vec2 texel = 1.5 / vec2(textureSize(image, 0)); 

    float weights[9] = float[](
        0.227027,
        0.1945946,
        0.1216216,
        0.054054,
        0.016216,
        0.054054,
        0.1216216,
        0.1945946,
        0.227027
    );

    vec3 result = texture(image, vUV).rgb * weights[4];

    for(int i = 1; i < 5; i++)
    {
        if(horizontal)
        {
            vec2 offset = vec2(texel.x * i, 0.0);

            result += texture(image, vUV + offset).rgb * weights[4 + i];
            result += texture(image, vUV - offset).rgb * weights[4 - i];
        }
        else
        {
            vec2 offset = vec2(0.0, texel.y * i);

            result += texture(image, vUV + offset).rgb * weights[4 + i];
            result += texture(image, vUV - offset).rgb * weights[4 - i];
        }
    }

    FragColor = vec4(result, 1.0);
}