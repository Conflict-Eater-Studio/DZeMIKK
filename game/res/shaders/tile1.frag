    #version 330 core
    out vec4 FragColor;

    in vec3 FragPos;
    in vec3 Normal;

    uniform vec3 lightDir;      
    uniform vec3 lightColor;    
    uniform vec3 objectColor; 
    uniform vec4 meshColor;

    void main() {
        // Lambert: max(dot(N,L),0)
        vec3 norm = normalize(Normal);
        vec3 lightDirNorm = normalize(-lightDir); 
        float diff = max(dot(norm, lightDirNorm), 0.0);

        vec3 diffuse = diff * lightColor;

        vec3 result = diffuse * objectColor * meshColor.rgb;
        FragColor = vec4(result, meshColor.a);
    }