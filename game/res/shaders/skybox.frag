    #version 460 core
    in vec3 TexCoords;
    out vec4 FragColor;

    uniform samplerCube skybox;
    uniform vec3 color;
    uniform int mode;

    void main() {
        if (mode == 1)
            FragColor = texture(skybox, TexCoords);
        else
            FragColor = vec4(color, 1.0);
    }