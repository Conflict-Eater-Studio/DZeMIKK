#version 460 core

in vec2 TexCoords;
in vec3 Normal;
out vec4 FragColor;

uniform sampler2D spriteTexture;
uniform vec4 spriteColor;
uniform bool useTexture;

void main()
{
    if (useTexture)
    {
        vec4 texColor = texture(spriteTexture, TexCoords);
        FragColor = texColor * spriteColor;
    }
    else
    {
        FragColor = spriteColor;
    }
}