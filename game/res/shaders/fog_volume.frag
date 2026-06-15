#version 460 core

in vec3 vLocalPos;
in vec3 vLocalCamPos;

out vec4 FragColor;

uniform sampler2D depthTexture;
uniform sampler2D noiseTexture;
uniform bool useNoiseTexture;

uniform mat4 model;
uniform mat4 invModel;
uniform mat4 invProjection;
uniform mat4 invView;

uniform vec3 viewPos;
uniform vec2 screenResolution;
uniform float time;
uniform vec4 fogColor;
uniform float density;
uniform float noiseScale;
uniform float windSpeed;
uniform float noiseContrast;
uniform float warpStrength;

// Ray structure
struct Ray {
    vec3 origin;
    vec3 dir;
};

// Ray-AABB intersection for unit box [-0.5, 0.5]
bool intersectAABB(Ray ray, vec3 boxMin, vec3 boxMax, out float tMin, out float tMax) {
    vec3 invDir = 1.0 / (ray.dir + vec3(1e-6));
    vec3 t0 = (boxMin - ray.origin) * invDir;
    vec3 t1 = (boxMax - ray.origin) * invDir;
    
    vec3 tmin3 = min(t0, t1);
    vec3 tmax3 = max(t0, t1);
    
    tMin = max(max(tmin3.x, tmin3.y), tmin3.z);
    tMax = min(min(tmax3.x, tmax3.y), tmax3.z);
    
    return tMin < tMax && tMax > 0.0;
}

// Reconstruct world-space position from depth buffer
vec3 getSceneWorldPos(vec2 texCoord) {
    float depth = texture(depthTexture, texCoord).r;
    vec4 clipPos = vec4(texCoord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos4 = invProjection * clipPos;
    vec3 vPos = viewPos4.xyz / viewPos4.w;
    vec4 wPos = invView * vec4(vPos, 1.0);
    return wPos.xyz;
}

// Triplanar texture mapping fallback
float triplanarNoise(vec3 p) {
    float xVal = texture(noiseTexture, p.yz).r;
    float yVal = texture(noiseTexture, p.xz).r;
    float zVal = texture(noiseTexture, p.xy).r;
    return (xVal + yVal + zVal) * 0.333333;
}

// Procedural 3D Noise Fallback
float hash(vec3 p) {
    p = fract(p * 0.3183099 + vec3(0.1, 0.1, 0.1));
    p *= 17.0;
    return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

float proceduralNoise(vec3 x) {
    vec3 i = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);
    
    return mix(mix(mix(hash(i + vec3(0.0, 0.0, 0.0)), hash(i + vec3(1.0, 0.0, 0.0)), f.x),
                   mix(hash(i + vec3(0.0, 1.0, 0.0)), hash(i + vec3(1.0, 1.0, 0.0)), f.x), f.y),
               mix(mix(hash(i + vec3(0.0, 0.0, 1.0)), hash(i + vec3(1.0, 0.0, 1.0)), f.x),
                   mix(hash(i + vec3(0.0, 1.0, 1.0)), hash(i + vec3(1.0, 1.0, 1.0)), f.x), f.y), f.z);
}

// Fast 2-octave FBM for organic cloud structure
float fbm(vec3 p) {
    float v = 0.0;
    v += 0.60 * proceduralNoise(p);
    v += 0.40 * proceduralNoise(p * 2.03);
    return v;
}

// Domain-warped smoke noise in world space
float getNoiseVal(vec3 p, float tRaw) {
    float t = tRaw * windSpeed;
    vec3 pos = p * noiseScale;
    
    // Warp coordinates over time (creates turbulent swirl)
    vec3 q = vec3(
        fbm(pos + vec3(t * 0.20, t * 0.10, 0.0)),
        fbm(pos + vec3(0.0, t * 0.15, t * 0.22)),
        fbm(pos + vec3(t * -0.08, 0.0, t * 0.12))
    );
    
    // Double warp / final sample
    vec3 r = pos + q * warpStrength + vec3(t * 0.30, t * 0.12, t * -0.20);
    
    float noise = fbm(r);
    
    // Interpolate contrast threshold
    float edgeLow = clamp(0.5 - (1.0 - noiseContrast) * 0.5, 0.0, 0.45);
    float edgeHigh = clamp(0.5 + (1.0 - noiseContrast) * 0.5, 0.55, 1.0);
    
    return smoothstep(edgeLow, edgeHigh, noise);
}

void main() {
    vec2 texCoord = gl_FragCoord.xy / screenResolution;
    
    // Setup camera ray in local space
    Ray ray;
    ray.origin = vLocalCamPos;
    ray.dir = normalize(vLocalPos - vLocalCamPos);
    
    // Intersect ray with unit box [-0.5, 0.5]
    float tMin, tMax;
    vec3 boxMin = vec3(-0.5);
    vec3 boxMax = vec3(0.5);
    if (!intersectAABB(ray, boxMin, boxMax, tMin, tMax)) {
        discard;
    }
    
    // Entry point: if camera is inside, start at 0
    float tEntry = max(0.0, tMin);
    
    // Exit point: initially end of box
    float tExit = tMax;
    
    // Account for scene geometry depth occlusion
    vec3 sceneWorldPos = getSceneWorldPos(texCoord);
    vec3 sceneLocalPos = vec3(invModel * vec4(sceneWorldPos, 1.0));
    
    // Projection of scene local position on camera ray
    float tScene = dot(sceneLocalPos - vLocalCamPos, ray.dir);
    
    // Clamp exit distance to the scene depth
    tExit = min(tExit, tScene);
    
    if (tExit <= tEntry) {
        discard;
    }
    
    // Transform entry and exit points to world space
    vec3 localEntryPos = ray.origin + ray.dir * tEntry;
    vec3 localExitPos = ray.origin + ray.dir * tExit;
    
    vec3 worldEntryPos = vec3(model * vec4(localEntryPos, 1.0));
    vec3 worldExitPos = vec3(model * vec4(localExitPos, 1.0));
    
    float worldDistInside = length(worldExitPos - worldEntryPos);
    
    // Perform quick and optimized raymarching in world space
    const int STEPS = 8;
    float stepSize = worldDistInside / float(STEPS);
    vec3 stepVec = normalize(worldExitPos - worldEntryPos) * stepSize;
    
    float accumulation = 0.0;
    vec3 currentWorldPos = worldEntryPos + stepVec * 0.5;
    
    for (int i = 0; i < STEPS; ++i) {
        float noise = getNoiseVal(currentWorldPos, time);
        // Base density + noise details
        accumulation += (0.15 + 0.85 * noise) * stepSize;
        currentWorldPos += stepVec;
    }
    
    // Calculate transmittance (density scaled by 0.1 for better slider range)
    float transmittance = exp(-accumulation * (density * 0.1));
    float alpha = 1.0 - transmittance;
    
    // Calculate edge fading based on the midpoint of the ray inside the box
    vec3 avgLocalPos = ray.origin + ray.dir * (tEntry + tExit) * 0.5;
    vec3 centerDist = abs(avgLocalPos);
    float edgeFadeX = smoothstep(0.5, 0.42, centerDist.x);
    float edgeFadeY = smoothstep(0.5, 0.42, centerDist.y);
    float edgeFadeZ = smoothstep(0.5, 0.42, centerDist.z);
    float edgeFade = edgeFadeX * edgeFadeY * edgeFadeZ;
    
    alpha *= edgeFade;
    
    if (alpha < 0.005) {
        discard;
    }
    
    FragColor = vec4(fogColor.rgb, alpha * fogColor.a);
}
