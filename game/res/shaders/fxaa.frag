#version 330 core

uniform vec2 resolution;
uniform sampler2D screenTexture;

in vec2 vUV;

out vec4 FragColor;

const float FXAA_EDGE_THRESHOLD_MIN = 1.0 / 16.0;
const float FXAA_EDGE_THRESHOLD = 1.0 / 8.0;
const float FXAA_SUBPIX_TRIM = 1.0 / 8.0;
const float FXAA_SUBPIX_TRIM_SCALE = 1.0;
const float FXAA_SUBPIX_CAP = 7.0 / 8.0;

float luma(vec3 c)
{
    return dot(c, vec3(0.299, 0.587, 0.114));
}

void main()
{
    vec2 texel = 1.0 / vec2(1920, 1080);

    vec3 rgbM = texture(screenTexture, vUV).rgb;
    vec3 rgbN = texture(screenTexture, vUV + vec2(0.0, texel.y)).rgb;
    vec3 rgbS = texture(screenTexture, vUV - vec2(0.0, texel.y)).rgb;
    vec3 rgbE = texture(screenTexture, vUV + vec2(texel.x, 0.0)).rgb;
    vec3 rgbW = texture(screenTexture, vUV - vec2(texel.x, 0.0)).rgb;

    float lumaM = luma(rgbM);
    float lumaN = luma(rgbN);
    float lumaS = luma(rgbS);
    float lumaE = luma(rgbE);
    float lumaW = luma(rgbW);

    float lumaMin = min(lumaM, min(min(lumaN, lumaS), min(lumaE, lumaW)));

    float lumaMax = max(lumaM,  max(max(lumaN, lumaS), max(lumaE, lumaW)));

    float range = lumaMax - lumaMin;

    if(range < max(FXAA_EDGE_THRESHOLD_MIN, FXAA_EDGE_THRESHOLD * lumaMax)){
        FragColor = vec4(rgbM, 1.0);
        return;
    }

    float lumaL = (lumaN + lumaS + lumaE + lumaW) * 0.25;
    float rangeL = abs(lumaL - lumaM);

    float blendL = max(0.0, (rangeL / max(range, 1e-5)) - FXAA_SUBPIX_TRIM) * FXAA_SUBPIX_TRIM_SCALE;

    blendL = min(FXAA_SUBPIX_CAP, blendL);

    FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}