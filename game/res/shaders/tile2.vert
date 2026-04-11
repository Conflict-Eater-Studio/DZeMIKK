    #version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec3 aNormal;

    // Atrybut instancji: model mat4 (4 wektory)
    layout(location = 2) in vec4 aModelRow0;
    layout(location = 3) in vec4 aModelRow1;
    layout(location = 4) in vec4 aModelRow2;
    layout(location = 5) in vec4 aModelRow3;

    layout (std140) uniform Matrices
    {
        mat4 projection;
        mat4 view;
    };

    out vec3 FragPos;
    out vec3 Normal;

    void main() {
        mat4 model = mat4(aModelRow0, aModelRow1, aModelRow2, aModelRow3);
        FragPos = vec3(model * vec4(aPos,1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;
        gl_Position = projection * view * vec4(FragPos,1.0);
    }