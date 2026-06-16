#version 460 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in mat3 TBN;

uniform vec3 viewPos;

// Material

uniform vec3 albedoColor;
uniform float metallic;
uniform float roughness;
uniform float ao;
uniform vec3 emissiveColor;
uniform float emissiveStrength;

uniform sampler2D albedoMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform sampler2D normalMap;
uniform sampler2D emissiveMap;

uniform bool hasAlbedoMap;
uniform bool hasMetallicMap;
uniform bool hasRoughnessMap;
uniform bool hasAOMap;
uniform bool hasNormalMap;
uniform bool hasEmissiveMap;

// Lights

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
    vec4 params;
};

struct SpotLight
{
    vec4 position;
    vec4 direction;
    vec4 color;
    vec4 params;
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

const float PI = 3.14159265359;

// ======================================================
// PBR Helpers
// ======================================================

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;

    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;

    float denom =
        (NdotH2 * (a2 - 1.0) + 1.0);

    denom = PI * denom * denom;

    return num / max(denom, 0.000001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float num = NdotV;

    float denom =
        NdotV * (1.0 - k) + k;

    return num / max(denom, 0.000001);
}

float GeometrySmith(
    vec3 N,
    vec3 V,
    vec3 L,
    float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float ggx1 =
        GeometrySchlickGGX(NdotV, roughness);

    float ggx2 =
        GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return
        F0 +
        (1.0 - F0) *
        pow(1.0 - cosTheta, 5.0);
}

// ======================================================

void AccumulateLight(
    vec3 L,
    vec3 radiance,
    vec3 N,
    vec3 V,
    vec3 albedo,
    float metallicValue,
    float roughnessValue,
    vec3 F0,
    inout vec3 Lo)
{
    vec3 H = normalize(V + L);

    float NDF =
        DistributionGGX(
            N,
            H,
            roughnessValue);

    float G =
        GeometrySmith(
            N,
            V,
            L,
            roughnessValue);

    vec3 F =
        FresnelSchlick(
            max(dot(H, V), 0.0),
            F0);

    vec3 numerator =
        NDF * G * F;

    float denominator =
        4.0 *
        max(dot(N, V), 0.0) *
        max(dot(N, L), 0.0);

    vec3 specular =
        numerator /
        max(denominator, 0.001);

    vec3 kS = F;

    vec3 kD =
        vec3(1.0) - kS;

    kD *=
        (1.0 - metallicValue);

    float NdotL =
        max(dot(N, L), 0.0);

    Lo +=
        (
            kD * albedo / PI +
            specular
        )
        *
        radiance
        *
        NdotL;
}

void main()
{
    vec3 N;

    if (hasNormalMap)
    {
        vec3 tangentNormal =
            texture(normalMap, TexCoord).rgb;

        tangentNormal =
            tangentNormal * 2.0 - 1.0;

        N =
            normalize(
                TBN *
                tangentNormal);
    }
    else
    {
        N = normalize(Normal);
    }

    vec3 V = normalize(viewPos - FragPos);

    // ======================================================
    // Material
    // ======================================================

    vec3 albedo = albedoColor;

    if (hasAlbedoMap)
    {
        albedo *=
            texture(albedoMap, TexCoord).rgb;
    }

    float metallicValue =
        hasMetallicMap
        ? texture(metallicMap, TexCoord).r
        : metallic;

    float roughnessValue =
        hasRoughnessMap
        ? texture(roughnessMap, TexCoord).r
        : roughness;

    float aoValue =
        hasAOMap
        ? texture(aoMap, TexCoord).r
        : ao;

    vec3 emissive = emissiveColor;

    if (hasEmissiveMap)
    {
        emissive *=
            texture(emissiveMap, TexCoord).rgb;
    }

    emissive *= emissiveStrength;

    roughnessValue =
        clamp(
            roughnessValue,
            0.04,
            1.0);

    vec3 F0 = vec3(0.04);

    F0 =
        mix(
            F0,
            albedo,
            metallicValue);

    vec3 Lo = vec3(0.0);

    // ======================================================
    // Directional Lights
    // ======================================================

    for (int i = 0; i < dirLightCount; i++)
    {
        vec3 L =
            normalize(
                -dirLights[i].direction.xyz);

        vec3 radiance =
            dirLights[i].color.rgb *
            dirLights[i].color.a;

        AccumulateLight(
            L,
            radiance,
            N,
            V,
            albedo,
            metallicValue,
            roughnessValue,
            F0,
            Lo);
    }

    // ======================================================
    // Point Lights
    // ======================================================

    for (int i = 0; i < pointLightCount; i++)
    {
        vec3 lightVec = pointLights[i].position.xyz - FragPos;

        float distance = length(lightVec);
        vec3 L = lightVec / max(distance, 0.0001);

        float attenuation = 1.0 / max(distance * distance, 0.01);

        vec3 radiance =
            pointLights[i].color.rgb *
            pointLights[i].color.a *
            attenuation;

        AccumulateLight(
            L,
            radiance,
            N,
            V,
            albedo,
            metallicValue,
            roughnessValue,
            F0,
            Lo);
    }

    // ======================================================
    // Spot Lights
    // ======================================================

    for (int i = 0; i < spotLightCount; i++)
    {
        vec3 lightVec =
            spotLights[i].position.xyz -
            FragPos;

        vec3 L =
            normalize(lightVec);

        float theta =
            dot(
                L,
                normalize(
                    -spotLights[i].direction.xyz));

        float spot =
            clamp(
                (theta - spotLights[i].params.z) /
                (spotLights[i].params.y - spotLights[i].params.z),
                0.0,
                1.0);

        vec3 radiance =
            spotLights[i].color.rgb *
            spotLights[i].color.a *
            spot;

        AccumulateLight(
            L,
            radiance,
            N,
            V,
            albedo,
            metallicValue,
            roughnessValue,
            F0,
            Lo);
    }

    // ======================================================
    // Ambient
    // ======================================================

    vec3 ambient =
        vec3(0.03) *
        albedo *
        aoValue;

    vec3 color =
        ambient +
        Lo +
        emissive;

    // Reinhard tonemap

    color =
        color /
        (color + vec3(1.0));

    // Gamma correction

    color =
        pow(
            color,
            vec3(1.0 / 2.2));

    FragColor =
        vec4(color, 1.0);
}