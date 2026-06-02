#version 330 core

uniform vec2 resolution;
uniform vec3 color;
uniform sampler2D screenTexture;

in vec2 vUV;

out vec4 FragColor;

float FXAA_EDGE_THRESHOLD_MIN = 0.0;
float FXAA_EDGE_THRESHOLD = 1.0 / 16.0;

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

     FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}