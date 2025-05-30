#shader vertex
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 invprojview;
uniform float near_plane;
uniform float far_plane;

out vec3 origin;
out vec3 ray;

void main() {
    gl_Position = vec4(aPos, 1.0);

    vec2 pos = vec2(aPos); // clear the aPos third argument, it ain't needed
    gl_Position = vec4(pos, 0.0, 1.0);
    origin = (invprojview * vec4(pos, -1.0, 1.0) * near_plane).xyz;
    ray = (invprojview * vec4(pos * (far_plane - near_plane), far_plane + near_plane, far_plane - near_plane)).xyz;

    // equivalent calculation:
    // ray = (invprojview * (vec4(pos, 1.0, 1.0) * far - vec4(pos, -1.0, 1.0) * near)).xyz
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

uniform float near_plane;
uniform float far_plane;

in vec3 origin;
in vec3 ray;

uniform sampler3D noise_texture;

uniform float time;

const float MAX_DIST = 100.0;
const float MIN_DIST = 0.000001;
const int MAX_STEPS = 128;

uniform vec3 sphere_positions[5];
uniform vec3 sphere_colors[5];
uniform float sphere_radiuses[5];
uniform int num_spheres;

float smin( float a, float b, float k )
{
    k *= 1.0;
    float r = exp2(-a/k) + exp2(-b/k);
    return -k*log2(r);
};

// Signed Distance Function for a sphere
float sdfSphere(vec3 position, vec3 center, float radius) {
    // center.x = center.x + 5 * sin(time);

    return length(position - center) - radius;
};

float sdfTorus(vec3 p, vec2 t)
{   
    return length(vec2(length(p.xz)-t.x, p.y) ) - t.y;
};

// Scene function - add more SDFs here to create complex scenes
float sceneSDF(vec3 position) {
    float tmp = 1000;
    for (int i = 0; i < 5; i++) {
        if (i < num_spheres) {
            // tmp = smin(sdfSphere(position, sphere_positions[i], sphere_radiuses[i]), tmp, 2.0);
            tmp = smin(sdfSphere(position, sphere_positions[i], 5), tmp, 2.0);
        }
    }

    return tmp;
};

// Shading function to calculate color based on the distance to the scene
vec3 getNormal(vec3 position) {
    vec2 epsilon = vec2(0.001, 0.0);
    float dx = sceneSDF(position + vec3(epsilon.x, epsilon.y, epsilon.y)) - sceneSDF(position - vec3(epsilon.x, epsilon.y, epsilon.y));
    float dy = sceneSDF(position + vec3(epsilon.y, epsilon.x, epsilon.y)) - sceneSDF(position - vec3(epsilon.y, epsilon.x, epsilon.y));
    float dz = sceneSDF(position + vec3(epsilon.y, epsilon.y, epsilon.x)) - sceneSDF(position - vec3(epsilon.y, epsilon.y, epsilon.x));
    return normalize(vec3(dx, dy, dz));
};

vec4 volumetric_march(vec3 origin, vec3 dir) {
    float distance = 0.0;
    float collected_noise;
    bool first_hit = true;
    vec3 hit_point;

    for (int i = 0; i < MAX_STEPS; i++) {
        vec3 pos = origin + dir * distance;
        float distToScene = sceneSDF(pos);
        
        if (distToScene < MIN_DIST) { // we are inside, aka "smoke"
            if (first_hit) {
                first_hit = false;
                pos = hit_point;
            }

            collected_noise += texture(noise_texture, pos * 0.1).x * 0.019;
        }

        if (first_hit) {
            distance += distToScene;
        } else {
            distance += 0.1;
        }

        if (distance > MAX_DIST * 100) {
            break;
        }
        if (collected_noise > 1.0) {
            collected_noise = 1.0;
            break;
        }
    }

    vec3 normal = getNormal(hit_point);
    float lightIntensity = dot(normal, normalize(vec3(1.0, 1.0, 1.0))) * 0.5 + 0.5;
    vec4 result = vec4(1.0 * lightIntensity, 0.5 * lightIntensity, 0.2 * lightIntensity, collected_noise);
    // vec4 result = vec4(1.0, 0.5, 0.2, collected_noise);

    return result;
};

void main() {

    vec3 rayDir = normalize(ray);

    vec4 result = volumetric_march(origin, rayDir);

    color = result;
};