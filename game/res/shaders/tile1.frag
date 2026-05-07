#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 objectColor;
uniform vec3 viewPos;

uniform float shininess;
uniform float specularStrength;

uniform int dirLightCount;
uniform int pointLightCount;
uniform int spotLightCount;

#define MAX_DIR_LIGHTS 8
#define MAX_POINT_LIGHTS 16
#define MAX_SPOT_LIGHTS 8

uniform vec3 dirDirection[MAX_DIR_LIGHTS];
uniform vec3 dirColor[MAX_DIR_LIGHTS];
uniform float dirIntensity[MAX_DIR_LIGHTS];

uniform vec3 pointPos[MAX_POINT_LIGHTS];
uniform vec3 pointColor[MAX_POINT_LIGHTS];
uniform float pointIntensity[MAX_POINT_LIGHTS];
uniform float pointRange[MAX_POINT_LIGHTS];

uniform vec3 spotPos[MAX_SPOT_LIGHTS];
uniform vec3 spotDir[MAX_SPOT_LIGHTS];
uniform vec3 spotColor[MAX_SPOT_LIGHTS];
uniform float spotIntensity[MAX_SPOT_LIGHTS];
uniform float spotInner[MAX_SPOT_LIGHTS];
uniform float spotOuter[MAX_SPOT_LIGHTS];

void main() {

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 diffuseLighting = vec3(0.0);
    vec3 specularLighting = vec3(0.0);

    // =========================================================
    // Directional
    // =========================================================

    for (int i = 0; i < dirLightCount; i++) {

        vec3 L = normalize(-dirDirection[i]);

        // Diffuse
        float diff = max(dot(norm, L), 0.0);

        diffuseLighting +=
            diff *
            dirColor[i] *
            dirIntensity[i];

        // Blinn-Phong Specular
        vec3 halfwayDir = normalize(L + viewDir);

        float spec =
            pow(max(dot(norm, halfwayDir), 0.0), shininess);

        specularLighting +=
            spec *
            specularStrength *
            dirColor[i] *
            dirIntensity[i];
    }

    // =========================================================
    // Point
    // =========================================================

    for (int i = 0; i < pointLightCount; i++) {

        vec3 L = pointPos[i] - FragPos;

        float dist = length(L);

        L = normalize(L);

        float atten = 1.0 / (dist * dist);

        // Diffuse
        float diff = max(dot(norm, L), 0.0);

        diffuseLighting +=
            diff *
            pointColor[i] *
            pointIntensity[i] *
            atten;

        // Specular
        vec3 halfwayDir = normalize(L + viewDir);

        float spec =
            pow(max(dot(norm, halfwayDir), 0.0), shininess);

        specularLighting +=
            spec *
            specularStrength *
            pointColor[i] *
            pointIntensity[i] *
            atten;
    }

    // =========================================================
    // Spot
    // =========================================================

    for (int i = 0; i < spotLightCount; i++) {

        vec3 L = normalize(spotPos[i] - FragPos);

        float theta =
            dot(L, normalize(-spotDir[i]));

        float spot =
            clamp(
                (theta - spotOuter[i]) /
                (spotInner[i] - spotOuter[i]),
                0.0,
                1.0
            );

        float diff =
            max(dot(norm, L), 0.0);

        diffuseLighting +=
            diff *
            spotColor[i] *
            spotIntensity[i] *
            spot;

        // Specular
        vec3 halfwayDir =
            normalize(L + viewDir);

        float spec =
            pow(max(dot(norm, halfwayDir), 0.0), shininess);

        specularLighting +=
            spec *
            specularStrength *
            spotColor[i] *
            spotIntensity[i] *
            spot;
    }

    vec3 ambient = objectColor * 0.05;

    vec3 finalColor =
        ambient +
        diffuseLighting * objectColor +
        specularLighting;

    FragColor = vec4(finalColor, 1.0);
}