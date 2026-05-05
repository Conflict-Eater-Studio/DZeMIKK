    #version 330 core
    out vec4 FragColor;

    in vec3 FragPos;
    in vec3 Normal;

    uniform vec3 lightDir;      
    uniform vec3 lightColor;    
    uniform vec3 objectColor; 
    uniform float lightIntensity;

    void main() {
        // Lambert: max(dot(N,L),0)
        vec3 norm = normalize(Normal);
        vec3 lightDirNorm = normalize(lightDir - FragPos); 
        float diff = max(dot(norm, lightDirNorm), 0.0);

        vec3 diffuse = diff * lightColor * lightIntensity;

        vec3 result = diffuse * objectColor;
        FragColor = vec4(result, 1.0);
    }