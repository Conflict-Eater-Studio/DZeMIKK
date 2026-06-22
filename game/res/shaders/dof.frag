#version 330 core

in vec2 vUV;

out vec4 FragColor;

uniform sampler2D screenTexture;
uniform sampler2D uDepth;

uniform float uNearPlane;
uniform float uFarPlane;

uniform float uFocusDistance;
uniform float uFocusRange;
uniform float uMaxBlur;

// --------------------------------------------

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;

    return (2.0 * uNearPlane * uFarPlane) /
           (uFarPlane + uNearPlane -
            z * (uFarPlane - uNearPlane));
}

// --------------------------------------------

const vec2 offsets[8] = vec2[](
    vec2(-1.0,  0.0),
    vec2( 1.0,  0.0),
    vec2( 0.0, -1.0),
    vec2( 0.0,  1.0),

    vec2(-0.707, -0.707),
    vec2( 0.707, -0.707),
    vec2(-0.707,  0.707),
    vec2( 0.707,  0.707)
);

// --------------------------------------------

void main()
{
    float depth = texture(uDepth, vUV).r;

    float linearDepth = LinearizeDepth(depth);

    float coc =
        abs(linearDepth - uFocusDistance) /
        uFocusRange;

    coc = clamp(coc, 0.0, 1.0);

    vec2 texelSize =
        1.0 / vec2(textureSize(screenTexture, 0));

    vec3 color = texture(screenTexture, vUV).rgb;
    vec3 blurred = color;

    if (coc > 0.001)
    {
        blurred = vec3(0.0);

        for (int i = 0; i < 8; i++)
        {
            vec2 offset =
                offsets[i] *
                texelSize *
                uMaxBlur *
                coc;

            blurred +=
                texture(screenTexture, vUV + offset).rgb;
        }

        blurred /= 8.0;
    }

    vec3 finalColor =
        mix(color, blurred, coc);

    FragColor = vec4(finalColor, 1.0);
}