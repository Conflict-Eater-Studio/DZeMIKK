    #version 330 core
    out vec4 FragColor;

    in vec3 FragPos;
    in vec3 Normal;

    uniform vec3 lightDir;
    uniform vec3 lightColor;

    void main() {
        vec3 norm = normalize(Normal);
        vec3 lightDirNorm = normalize(lightDir - FragPos);
        float diff = max(dot(norm, lightDirNorm), 0.0);
        vec3 diffuse = diff * lightColor;
        FragColor = vec4(diffuse * vec3(0.2,0.5,1.0), 1.0); // niebieska kostka
    }