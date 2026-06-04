#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D screenTexture;

uniform float uExposure;       // Default: 0.0 (multiplier: 2^uExposure)
uniform float uContrast;       // Default: 1.0
uniform float uSaturation;     // Default: 1.0
uniform float uTemperature;    // Default: 0.0
uniform float uTint;           // Default: 0.0
uniform vec3 uColorFilter;     // Default: vec3(1.0, 1.0, 1.0)

const vec3 LumaCoeff = vec3(0.2126, 0.7152, 0.0722);

void main()
{
    vec3 color = texture(screenTexture, vUV).rgb;

    // 1. Exposure
    color *= pow(2.0, uExposure);

    // 2. Contrast
    color = (color - vec3(0.5)) * uContrast + vec3(0.5);
    color = max(color, vec3(0.0));

    // 3. Saturation
    float luma = dot(color, LumaCoeff);
    color = mix(vec3(luma), color, uSaturation);
    color = max(color, vec3(0.0));

    // 4. Temperature & Tint
    vec3 warmCool = vec3(uTemperature * 0.05, 0.0, -uTemperature * 0.05);
    vec3 greenMagenta = vec3(-uTint * 0.02, uTint * 0.04, -uTint * 0.02);
    color += warmCool + greenMagenta;
    color = max(color, vec3(0.0));

    // 5. Color Filter
    color *= uColorFilter;

    FragColor = vec4(color, 1.0);
}
