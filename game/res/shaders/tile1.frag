#version 460 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D diffuseTexture;
uniform bool useTexture;

uniform vec3 objectColor;
uniform vec3 viewPos;

uniform float shininess;
uniform float specularStrength;

#define MAX_DIR_LIGHTS 5000
#define MAX_POINT_LIGHTS 5000
#define MAX_SPOT_LIGHTS 5000

struct DirectionalLight
{
    vec4 direction;
    vec4 color; 
};

struct PointLight
{
    vec4 position;
    vec4 color;
    vec4 params; // x = range (optional)
};

struct SpotLight
{
    vec4 position;
    vec4 direction;
    vec4 color;
    vec4 params; // x = range, y = inner, z = outer
};

layout(std430, binding = 0) buffer LightBuffer
{
    int dirLightCount;
    int pointLightCount;
    int spotLightCount;
    int padding;

    DirectionalLight dirLights[MAX_DIR_LIGHTS];
    PointLight pointLights[MAX_POINT_LIGHTS];
    SpotLight spotLights[MAX_SPOT_LIGHTS];
};

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 diffuseLighting = vec3(0.0);
    vec3 specularLighting = vec3(0.0);

    for (int i = 0; i < dirLightCount; i++)
    {
        vec3 L = normalize(-dirLights[i].direction.xyz);

        float diff = max(dot(norm, L), 0.0);

        vec3 lightColor = dirLights[i].color.rgb;
        float intensity = dirLights[i].color.a;

        diffuseLighting += diff * lightColor * intensity;

        vec3 halfwayDir = normalize(L + viewDir);

        float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);

        specularLighting += spec * specularStrength * lightColor * intensity;
    }

    for (int i = 0; i < pointLightCount; i++)
    {
        vec3 L = pointLights[i].position.xyz - FragPos;

        float dist = length(L);
        L = normalize(L);

        float atten = 1.0 / (dist * dist);

        vec3 lightColor = pointLights[i].color.rgb;
        float intensity = pointLights[i].color.a;

        float diff = max(dot(norm, L), 0.0);

        diffuseLighting += diff * lightColor * intensity * atten;

        vec3 halfwayDir = normalize(L + viewDir);

        float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);

        specularLighting += spec * specularStrength * lightColor * intensity * atten;
    }

    for (int i = 0; i < spotLightCount; i++)
    {
        vec3 L = normalize(spotLights[i].position.xyz - FragPos);

        float theta = dot(L, normalize(-spotLights[i].direction.xyz));

        float spot = clamp(
            (theta - spotLights[i].params.z) /
            (spotLights[i].params.y - spotLights[i].params.z),
            0.0,
            1.0
        );

        vec3 lightColor = spotLights[i].color.rgb;
        float intensity = spotLights[i].color.a;

        float diff = max(dot(norm, L), 0.0);

        diffuseLighting += diff * lightColor * intensity * spot;

        vec3 halfwayDir = normalize(L + viewDir);

        float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);

        specularLighting += spec * specularStrength * lightColor * intensity * spot;
    }

    vec3 baseColor;

    if (useTexture)
    {
        vec3 texColor = texture(diffuseTexture, TexCoord).rgb;

        if (objectColor == vec3(1.0))
            baseColor = texColor;
        else
            baseColor = texColor * objectColor;
    }
    else
    {
        baseColor = objectColor;
    }

    vec3 ambient = baseColor * 0.05;

    vec3 finalColor =
        ambient +
        diffuseLighting * baseColor +
        specularLighting;

    FragColor = vec4(finalColor, 1.0);
}