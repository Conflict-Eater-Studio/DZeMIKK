    #version 330 core
    in vec2 TexCoords;
    out vec4 FragColor;

    uniform sampler2D spriteTexture;
    uniform vec4 spriteColor; // RGBA

    void main()
    {
        vec4 texColor = texture(spriteTexture, TexCoords);
        FragColor = vec4(spriteColor.rgb, spriteColor.a * texColor.a);
    }