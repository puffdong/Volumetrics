#shader vertex
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;

uniform mat4 invprojview;
uniform float near_plane;
uniform float far_plane;

out vec3 origin;
out vec3 ray;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);

    // Keep your existing reconstruction style
    origin = (invprojview * vec4(aPos, -1.0, 1.0) * near_plane).xyz;
    ray    = (invprojview * vec4(aPos * (far_plane - near_plane), far_plane + near_plane, far_plane - near_plane)).xyz;
}

#shader fragment
#version 330 core
layout(location = 0) out vec4 color;

in vec3 origin;
in vec3 ray;

uniform sampler3D noise_texture;
uniform float time;
uniform vec3 sun_dir;

uniform vec3  sphere_positions[5];
uniform vec3  sphere_colors[5];
uniform float sphere_radiuses[5];
uniform int   num_spheres;

const int MAX_STEPS = 96;
const float STEP_SIZE = 0.45;

const float SIGMA_T           = 1.1;    // extinction multiplier

const float NOISE_FREQ        = 0.10;   // 3D noise scale (higher = finer)
const float NOISE_THRESH      = 0.47;   // threshold for “puffs” (lower = fuller)
const float NOISE_BAND        = 0.20;   // softness around threshold
const float EDGE_SOFTNESS2    = 0.10;   // fade width near edge (in r^2 space)


const vec3  SUN_COLOR         = vec3(1.0, 0.98, 0.92);
const float AMBIENT           = 0.08;   // floor light

const float WRAP_LIGHT        = 0.35;   // 0=Lambert, ~0.3-0.5 wraps around edges

const float LINING_EDGE_START = 0.88;
const float LINING_EDGE_END   = 0.995;
const float LINING_GAIN       = 0.9;

// Super-cheap 2-tap shadow
const float SHADOW_STEP       = 1.4;
const float SHADOW_STRENGTH   = 1.35;

bool raySphere(vec3 ro, vec3 rd, vec3 c, float r, out float t0, out float t1) {
    vec3  oc = ro - c;
    float b  = dot(oc, rd);
    float c2 = dot(oc, oc) - r*r;
    float h  = b*b - c2;
    if (h < 0.0) { t0 = 1.0; t1 = -1.0; return false; }
    float s = sqrt(h);
    t0 = -b - s; t1 = -b + s;
    return true;
}

bool globalInterval(vec3 ro, vec3 rd, out float tBegin, out float tEnd) {
    bool any = false;
    float tMin = 1e30, tMax = -1e30;
    for (int i = 0; i < 5; ++i) {
        if (i >= num_spheres) break;
        float t0, t1;
        if (!raySphere(ro, rd, sphere_positions[i], sphere_radiuses[i], t0, t1)) continue;
        if (t1 <= 0.0) continue;
        t0 = max(t0, 0.0);
        tMin = min(tMin, t0);
        tMax = max(tMax, t1);
        any = true;
    }
    tBegin = tMin; tEnd = tMax;
    return any && (tEnd > tBegin);
}

void samplePuffs(vec3 p, out float dens, out vec3 tint, out vec3 sCenter, out float sRadius, out float x2Out) {
    float best = 0.0;
    vec3  col  = vec3(1.0);
    vec3  cSel = vec3(0.0);
    float rSel = 1.0;
    float x2Sel = 0.0;

    for (int i = 0; i < 5; ++i) {
        if (i >= num_spheres) break;

        vec3  c  = sphere_positions[i];
        float r  = sphere_radiuses[i];
        vec3  d  = p - c;
        float r2 = r * r;
        float d2 = dot(d, d);
        if (d2 >= r2) continue;

        float x2    = d2 / r2; // 0 center -> 1 edge
        float shell = 1.0 - smoothstep(1.0 - EDGE_SOFTNESS2, 1.0, x2);

        float n0   = texture(noise_texture, p * NOISE_FREQ).r;
        // float n1  = texture(noise_texture, p * 0.5).r;
        // float n2  = texture(noise_texture, p * 2).r;
        float occ = smoothstep(NOISE_THRESH - NOISE_BAND, NOISE_THRESH + NOISE_BAND, n0);
        //  occ += smoothstep(NOISE_THRESH - NOISE_BAND, NOISE_THRESH + NOISE_BAND, n1) * 0.3;
        // occ += smoothstep(NOISE_THRESH - NOISE_BAND, NOISE_THRESH + NOISE_BAND, n2) * 0.1;

        float thisD = shell * occ;
        if (thisD > best) {
            best  = thisD;
            col   = sphere_colors[i];
            cSel  = c;
            rSel  = r;
            x2Sel = x2;
        }
    }

    dens    = best;
    tint    = col;
    sCenter = cSel;
    sRadius = rSel;
    x2Out   = x2Sel;
}

float twoStepShadow(vec3 p) {
    float T = 1.0;
    vec3 dir = sun_dir;
    for (int k = 0; k < 2; ++k) {
        p += dir * SHADOW_STEP;
        float d; vec3 dummyC; vec3 c; float r; float x2;
        samplePuffs(p, d, dummyC, c, r, x2);
        float sigma = d * SIGMA_T * SHADOW_STRENGTH;
        T *= exp(-sigma * SHADOW_STEP);
        if (T < 0.05) break;
    }
    return T;
}

// do some of this, some of that, get back a color
// vec4 raymarch(vec3 ray_origin, vec3 ray_direction) {
//     // early exit strategy    
//     float t = 0.0;
//     float T = 1.0;

//     vec3 color = vec3(0.0);
//     float alpha = 0.0f;

//     for (int i = 0; i < MAX_STEPS; ++i) {
//         // do some early breaking

//         vec3 point = ray_origin + ray_direction * t;
        
//         float dens; vec3 tint; vec3 c; float r; float x2;
//         samplePuffs(p, dens, tint, c, r, x2);

//         if (dens > 0.0005) {
//             // Extinction over this slice
//             float sigma = dens * SIGMA_T;
//             float atten = exp(-sigma * STEP_SIZE);
//             float absorb = 1.0 - atten;

//             // Lighting (cheap but nicer):
//             // - wrap diffuse using analytic sphere normal
//             // - two-tap shadow
//             // - silver-lining accent near edge
//             vec3  nSphere = normalize(p - c);
//             float ndotl   = dot(nSphere, sun_dir);
//             float wrap    = clamp((ndotl + WRAP_LIGHT) / (1.0 + WRAP_LIGHT), 0.0, 1.0);

//             float shadowT = twoStepShadow(p);

//             float edge    = smoothstep(LINING_EDGE_START, LINING_EDGE_END, x2);
//             float lining  = edge * clamp(dot(rd, sun_dir), 0.0, 1.0); // stronger on sun-facing rim

//             float light   = AMBIENT + shadowT * (wrap + LINING_GAIN * lining);
//             vec3  Li      = SUN_COLOR * light;

//             color += T * (tint * Li) * absorb;
//             T   *= atten;

//             t += STEP_SIZE;
//         }
//     }

//     float alpha = clamp(1.0 - T, 0.0, 1.0);

//     return vec4(color, alpha);
// }

void main() {
    vec3 ro = origin;
    vec3 rd = normalize(ray);

    float t0, t1;
    if (!globalInterval(ro, rd, t0, t1)) {
        color = vec4(0.0);
        return;
    }

    float span     = max(t1 - t0, 0.0);
    int   maxIters = min(MAX_STEPS, int(ceil(span / STEP_SIZE)) + 1);

    float t = t0;
    float T = 1.0;        // view transmittance
    vec3  rgb = vec3(0.0);

    for (int i = 0; i < maxIters; ++i) {
        if (T < 0.01 || t > t1) break;

        vec3 p = ro + rd * t;

        float dens; vec3 tint; vec3 c; float r; float x2;
        samplePuffs(p, dens, tint, c, r, x2);

        if (dens > 0.0005) {
            // Extinction over this slice
            float sigma = dens * SIGMA_T;
            float atten = exp(-sigma * STEP_SIZE);
            float absorb = 1.0 - atten;

            // Lighting (cheap but nicer):
            // - wrap diffuse using analytic sphere normal
            // - two-tap shadow
            // - silver-lining accent near edge
            vec3  nSphere = normalize(p - c);
            float ndotl   = dot(nSphere, sun_dir);
            float wrap    = clamp((ndotl + WRAP_LIGHT) / (1.0 + WRAP_LIGHT), 0.0, 1.0);

            float shadowT = twoStepShadow(p);

            float edge    = smoothstep(LINING_EDGE_START, LINING_EDGE_END, x2);
            float lining  = edge * clamp(dot(rd, sun_dir), 0.0, 1.0); // stronger on sun-facing rim

            float light   = AMBIENT + shadowT * (wrap + LINING_GAIN * lining);
            vec3  Li      = SUN_COLOR * light;

            rgb += T * (tint * Li) * absorb;
            T   *= atten;
        }

        t += STEP_SIZE;
    }

    float alpha = clamp(1.0 - T, 0.0, 1.0);
    color = vec4(rgb, alpha);
}
