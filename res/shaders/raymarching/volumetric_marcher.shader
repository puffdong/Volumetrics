#shader vertex
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;

uniform mat4 invprojview;
uniform float near_plane;
uniform float far_plane;

out vec3 vOrigin;
out vec3 vRay;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);

    // Keep your existing reconstruction style
    vOrigin = (invprojview * vec4(aPos, -1.0, 1.0) * near_plane).xyz;
    vRay    = (invprojview * vec4(aPos * (far_plane - near_plane), far_plane + near_plane, far_plane - near_plane)).xyz;
}

#shader fragment
#version 330 core
layout(location = 0) out vec4 color;

in vec3 vOrigin;
in vec3 vRay;

uniform sampler3D noise_texture;
uniform float time;

// =================== FAST KNOBS ===================
#define STEP_MAX            24          // hard cap of fine steps
#define STEP_SKIP_SCALE     2.75        // how aggressively we skip in empty space (larger = faster)
#define SHADOW_STRIDE       4           // recompute shadow every N fine steps
#define LIGHT_PROBE_DIST    3.0         // single-tap shadow probe distance
#define DENSITY_MULT        1.15        // extinction scale
#define COVERAGE            0.43        // threshold before detail
#define PHASE_G             0.65
#define AMBIENT             0.07
// =================================================

const float CLOUD_RADIUS     = 25.0;
const vec3  CLOUD_CENTER     = vec3(0.0);
const float BASE_HEIGHT      = -10.0;
const float TOP_HEIGHT       = +12.0;

const vec3  SUN_DIR          = normalize(vec3(0.7, 0.5, 0.2));
const vec3  SUN_COLOR        = vec3(1.0, 0.98, 0.92);

// Small helpers
float remap(float x, float a, float b, float c, float d) {
    return clamp((x - a) / max(b - a, 1e-6), 0.0, 1.0) * (d - c) + c;
}

float hash12(vec2 p) {
    vec3 p3  = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float phaseHG(float cosTheta, float g) {
    float g2 = g*g;
    return (1.0 - g2) / (4.0 * 3.14159265 * pow(1.0 + g2 - 2.0*g*cosTheta, 1.5));
}

// Height shaping for flat bottoms/soft tops (cheap)
float heightShape(float y) {
    float h = remap(y, BASE_HEIGHT, TOP_HEIGHT, 0.0, 1.0);
    float base = smoothstep(0.02, 0.32, h);
    float top  = 1.0 - smoothstep(0.75, 1.0, h);
    return clamp(base * (0.55 + 0.45*top), 0.0, 1.0);
}

// ---------- NOISE: macro vs detail ----------
// macro: 2 octaves, low LOD, wide frequency — cheap & cache-friendly
float macroNoise(vec3 p) {
    // Force lower LOD to avoid thrashing cache; tune lods to your texture size
    float n = 0.0, w = 0.7;
    n += textureLod(noise_texture, p * 0.035, 2.5).r * w; w *= 0.55;
    n += textureLod(noise_texture, p * 0.070, 3.5).r * w;
    return clamp(n * 1.8, 0.0, 1.0);
}

// detail: 3 octaves, default LOD (let GPU pick)
float detailNoise(vec3 p) {
    float n = 0.0, w = 0.6;
    n += texture(noise_texture, p * 0.14).r * w;  w *= 0.52;
    n += texture(noise_texture, p * 0.28).r * w;  w *= 0.50;
    n += texture(noise_texture, p * 0.56).r * w;
    return clamp(n * 1.9, 0.0, 1.0);
}

// Sphere hit: returns t0,t1 (if miss, t0>t1)
vec2 raySphere(vec3 ro, vec3 rd, vec3 c, float r) {
    vec3 oc = ro - c;
    float b = dot(oc, rd);
    float c2 = dot(oc, oc) - r*r;
    float h = b*b - c2;
    if (h < 0.0) return vec2(1.0, -1.0);
    h = sqrt(h);
    return vec2(-b - h, -b + h);
}

// Very cheap single-tap "shadow": sample macro density ahead toward sun
float lightTransmittanceProbe(vec3 p) {
    vec3 q = p + SUN_DIR * LIGHT_PROBE_DIST;
    float hShape = heightShape(q.y);
    if (hShape <= 0.0) return 1.0;
    float shell   = clamp(1.0 - smoothstep(0.0, 2.5, max(0.0, length(q - CLOUD_CENTER) - CLOUD_RADIUS)), 0.0, 1.0);
    if (shell <= 0.0) return 1.0;
    float macro = macroNoise(q);
    float dMacro = clamp((macro * hShape * shell) - COVERAGE, 0.0, 1.0);
    float sigma_t = dMacro * DENSITY_MULT * 1.35;
    return exp(-sigma_t * LIGHT_PROBE_DIST);
}

void main() {
    vec3 ro = vOrigin;
    vec3 rd = normalize(vRay);

    // Ray-sphere bound
    vec2 hit = raySphere(ro, rd, CLOUD_CENTER, CLOUD_RADIUS);
    if (hit.x > hit.y) { color = vec4(0.0); return; }

    float t0 = max(hit.x, 0.0);
    float t1 = hit.y;
    float segLen = max(t1 - t0, 0.0);
    if (segLen <= 0.0) { color = vec4(0.0); return; }

    // Base step from segment length (bigger by default; we’ll refine adaptively)
    float dtBase = clamp(segLen / 36.0, 0.35, 0.85);

    // Dither start to reduce banding
    float jitter = hash12(gl_FragCoord.xy + time);
    float t = t0 + dtBase * jitter;

    float T = 1.0;                 // transmittance
    vec3  L = vec3(0.0);           // accumulated radiance
    float cachedShadow = 1.0;      // amortized shadow
    int   stepSinceShadow = SHADOW_STRIDE; // force compute on first hit

    // March with empty-space skipping
    for (int i = 0; i < STEP_MAX; ++i) {
        if (t > t1 || T < 0.01) break;

        vec3 p = ro + rd * t;

        // Macro density (cheap) for skip/enter decisions
        float hShape = heightShape(p.y);
        if (hShape <= 0.0) { t += dtBase * STEP_SKIP_SCALE; continue; }

        float toCenter = length(p - CLOUD_CENTER);
        float shell = clamp(1.0 - smoothstep(0.0, 2.5, max(0.0, toCenter - CLOUD_RADIUS)), 0.0, 1.0);
        if (shell <= 0.0) { t += dtBase * STEP_SKIP_SCALE; continue; }

        float macro = macroNoise(p);
        float dMacro = clamp((macro * hShape * shell) - COVERAGE, 0.0, 1.0);

        // If macro says "almost empty", take a big skip
        if (dMacro < 0.02) { t += dtBase * STEP_SKIP_SCALE; continue; }

        // Adaptive step: denser => smaller step, sparser => bigger
        float dt = mix(dtBase * STEP_SKIP_SCALE, dtBase * 0.5, clamp(dMacro * 1.7, 0.0, 1.0));

        // Only now do detail sampling (more expensive)
        float dDetail = detailNoise(p);
        float d = clamp(mix(dMacro, dDetail, 0.65), 0.0, 1.0);
        if (d < 0.001) { t += dt; continue; }

        // Beer-Lambert over this segment
        float sigma_t = d * DENSITY_MULT;
        float atten   = exp(-sigma_t * dt);
        float absorb  = 1.0 - atten;

        // Amortized lighting: refresh shadow every SHADOW_STRIDE fine steps
        if (stepSinceShadow >= SHADOW_STRIDE) {
            cachedShadow = lightTransmittanceProbe(p);
            stepSinceShadow = 0;
        }
        stepSinceShadow++;

        float mu = dot(rd, SUN_DIR);
        float phase = phaseHG(mu, PHASE_G);

        vec3 Li = SUN_COLOR * (AMBIENT + cachedShadow);
        vec3 scatter = Li * phase;

        L += T * scatter * absorb;  // accumulate single scattering
        T *= atten;                 // update transmittance

        t += dt;
    }

    float alpha = clamp(1.0 - T, 0.0, 1.0);
    vec3  outRGB = L / (1.0 + L);  // simple Reinhard

    color = vec4(outRGB, alpha);
}
