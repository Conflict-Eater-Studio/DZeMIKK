#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform vec2 uInverseScreenSize;

// FXAA parameters
uniform float uSpanMax;       // Default: 8.0
uniform float uReduceMul;     // Default: 1.0 / 8.0
uniform float uReduceMin;     // Default: 1.0 / 128.0

void main()
{
    vec2 texelSize = uInverseScreenSize;

    // Sample center and neighbors
    vec3 rgbNW = texture(screenTexture, vUV + vec2(-1.0, -1.0) * texelSize).rgb;
    vec3 rgbNE = texture(screenTexture, vUV + vec2(1.0, -1.0) * texelSize).rgb;
    vec3 rgbSW = texture(screenTexture, vUV + vec2(-1.0, 1.0) * texelSize).rgb;
    vec3 rgbSE = texture(screenTexture, vUV + vec2(1.0, 1.0) * texelSize).rgb;
    vec3 rgbM  = texture(screenTexture, vUV).rgb;

    // Luma coefficients (standard Rec. 709)
    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);

    // Find min and max luma in the local neighborhood
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    // Edge direction calculation
    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * uReduceMul), uReduceMin);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2(uSpanMax, uSpanMax),
          max(vec2(-uSpanMax, -uSpanMax),
          dir * rcpDirMin)) * texelSize;

    // Sample along the edge direction
    vec3 rgbA = 0.5 * (
        texture(screenTexture, vUV + dir * (1.0/3.0 - 0.5)).rgb +
        texture(screenTexture, vUV + dir * (2.0/3.0 - 0.5)).rgb);
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(screenTexture, vUV + dir * (0.0/3.0 - 0.5)).rgb +
        texture(screenTexture, vUV + dir * (3.0/3.0 - 0.5)).rgb);

    float lumaB = dot(rgbB, luma);

    // Fallback to rgbA if contrast is too high to prevent artifacts
    if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
        FragColor = vec4(rgbA, 1.0);
    } else {
        FragColor = vec4(rgbB, 1.0);
    }
}