    #version 460 core

    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec3 aNormal;
    layout(location = 2) in vec2 aUV;

    out vec2 TexCoords;
    out vec3 Normal;

    uniform mat4 model;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * model * vec4(aPos, 1.0);
        TexCoords = aUV;
        Normal = mat3(transpose(inverse(model))) * aNormal; 
    }