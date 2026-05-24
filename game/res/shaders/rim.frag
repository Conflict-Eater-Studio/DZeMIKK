#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform vec3 objectColor; 

void main()
{
    vec3 N = normalize(-Normal);
    vec3 V = normalize(viewPos - FragPos);

    float rim = 1.0 - max(dot(N, V), 0.0);

    rim = pow(rim, 1.5);

    vec3 baseColor = objectColor;
    vec3 rimColor  = vec3(1.0, 1.0, 1.0);

    vec3 color = baseColor + rim * rimColor;

    FragColor = vec4(color, 1.0);
}