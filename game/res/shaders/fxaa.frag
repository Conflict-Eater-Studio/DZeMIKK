#version 330 core

uniform vec2 resolution;
uniform bool showDifference;
uniform sampler2D screenTexture;

in vec2 vUV;

out vec4 FragColor;

uniform float FXAA_EDGE_THRESHOLD_MIN = 0.0;
uniform float FXAA_EDGE_THRESHOLD = 1.0 / 16.0;
uniform float FXAA_SUBPIX_TRIM = 0.0;
uniform float FXAA_SUBPIX_TRIM_SCALE = 1.0;
uniform float FXAA_SUBPIX_CAP = 1.0;

float luma(vec3 c)
{
    return dot(c, vec3(0.299, 0.587, 0.114));
}

void main()
{
    vec2 texel = 1.0 / resolution;

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

    vec2 dir;
    dir.x = -((lumaN + lumaS) - (lumaE + lumaW));
    dir.y =  ((lumaE + lumaW) - (lumaN + lumaS));

    float dirReduce = max((lumaN + lumaS + lumaE + lumaW) * 0.25 * 0.5, 1.0/128.0);

    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = clamp(dir * rcpDirMin, -8.0, 8.0) * texel;

    vec3 rgbA = 0.5 * (
        texture(screenTexture, vUV + dir * (1.0/6.0)).rgb +
        texture(screenTexture, vUV - dir * (1.0/6.0)).rgb
    );

    vec3 rgbB = rgbA * 0.5 +
        0.25 * (
        texture(screenTexture, vUV + dir * 0.5).rgb +
        texture(screenTexture, vUV - dir * 0.5).rgb
    );

    float lumaL = (lumaN + lumaS + lumaE + lumaW) * 0.25;
    float rangeL = abs(lumaL - lumaM);
    float blendL = max(0.0, (rangeL / max(range, 1e-5)) - FXAA_SUBPIX_TRIM) * FXAA_SUBPIX_TRIM_SCALE;
    blendL = min(FXAA_SUBPIX_CAP, blendL);

    vec3 finalColor = mix(rgbB, rgbA, blendL);

    if(showDifference){
        vec3 diff = abs(finalColor - rgbM) * 8.0;
        FragColor = vec4(clamp(diff, 0.0, 1.0), 1.0);
        return;
    }

    FragColor = vec4(finalColor, 1.0);
}