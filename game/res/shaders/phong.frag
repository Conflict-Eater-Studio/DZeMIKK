#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightDir;      
uniform vec3 lightColor;    
uniform vec3 objectColor; 
uniform float lightIntensity;

void main()
{
    vec3 norm = normalize(Normal);

    vec3 lightDirNorm = normalize(lightDir-FragPos);

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    float diff = max(dot(norm, lightDirNorm), 0.0);
    vec3 diffuse = diff * lightColor * lightIntensity;

    vec3 viewDir = normalize( - FragPos);
    vec3 reflectDir = reflect(-lightDirNorm, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 4.0);
    vec3 specular = spec * lightColor * lightIntensity;

    vec3 result = (ambient + diffuse + specular) * objectColor;

    FragColor = vec4(result, 1.0);
}