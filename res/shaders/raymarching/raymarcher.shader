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
    origin = (invprojview * vec4(aPos, -1.0, 1.0) * near_plane).xyz;
    ray = (invprojview * vec4(aPos * (far_plane - near_plane), far_plane + near_plane, far_plane - near_plane)).xyz;
}

#shader fragment
#version 330 core
layout(location = 0) out vec4 color;

in vec3 origin;
in vec3 ray;

uniform float time;
uniform vec3 sun_dir;
uniform sampler3D noise_texture;

uniform usampler3D u_voxels; // GL_R8UI
uniform ivec3 u_grid_dim; // (width, height, depth)
uniform vec3 u_grid_origin;
uniform float u_voxel_size; // world units per cell

uniform float near_plane;
uniform float far_plane;

// temporary
const float SPHERE_RADIUS = 15.0;
const vec3 SPHERE_POSITION = vec3(-5.0, -3.0, -10.0);
const vec3 SPHERE_COLOR = vec3(1.0);

// constants
const int MAX_STEPS = 96;
const float STEP_SIZE = 0.25;


const float MAX_DIST = 100.0;
const float MIN_DIST = 0.000001;
// const int MAX_STEPS = 16;

uint voxel_value_at(ivec3 cell) {
    if (any(lessThan(cell, ivec3(0))) || any(greaterThanEqual(cell, u_grid_dim))) return 0u;
    return texelFetch(u_voxels, cell, 0).r;
}

ivec3 world_to_cell(vec3 p) {
    vec3 local = (p - u_grid_origin) / u_voxel_size;
    return ivec3(floor(local));
}

float march_voxels(vec3 position) {
    ivec3 cell = world_to_cell(position);
    uint value = voxel_value_at(cell);
    if (value == 0u) {
        return 0.3;
    } else {
        return 0.0;
    }
}

float smin( float a, float b, float k )
{
    k *= 1.0;
    float r = exp2(-a/k) + exp2(-b/k);
    return -k*log2(r);
}

// Signed Distance Function for a sphere
float sdfSphere(vec3 position, vec3 center, float radius) {
    // center.x = center.x + 5 * sin(time);

    return length(position - center) - radius;
}

float sdfTorus(vec3 p, vec2 t)
{   
    return length(vec2(length(p.xz)-t.x, p.y) ) - t.y;
}

// Scene function - add more SDFs here to create complex scenes
float sceneSDF(vec3 position) {
    // float tmp = 1000;
    return march_voxels(position);
    // return sdfSphere(position, SPHERE_POSITION, SPHERE_RADIUS);
    // for (int i = 0; i < 5; i++) {
    //     if (i < num_spheres) {
    //         // tmp = smin(sdfSphere(position, sphere_positions[i], sphere_radiuses[i]), tmp, 2.0);
    //         tmp = smin(sdfSphere(position, sphere_positions[i], 5), tmp, 2.0);
    //     }
    // }

    // return tmp;
}

// Shading function to calculate color based on the distance to the scene
vec3 getNormal(vec3 position) {
    vec2 epsilon = vec2(0.001, 0.0);
    float dx = sceneSDF(position + vec3(epsilon.x, epsilon.y, epsilon.y)) - sceneSDF(position - vec3(epsilon.x, epsilon.y, epsilon.y));
    float dy = sceneSDF(position + vec3(epsilon.y, epsilon.x, epsilon.y)) - sceneSDF(position - vec3(epsilon.y, epsilon.x, epsilon.y));
    float dz = sceneSDF(position + vec3(epsilon.y, epsilon.y, epsilon.x)) - sceneSDF(position - vec3(epsilon.y, epsilon.y, epsilon.x));
    return normalize(vec3(dx, dy, dz));
}

vec4 do_raymarch(vec3 origin, vec3 dir) {
    float distance = 0.0;
    float collected_noise = 0.0;
    bool first_hit = true; 
    bool hit_something = false;
    vec3 hit_point;

    for (int i = 0; i < MAX_STEPS; i++) {
        vec3 pos = origin + dir * distance;
        float distToScene = sceneSDF(pos);
        
        if (distToScene < MIN_DIST) { // we are inside, aka "smoke"
            if (first_hit) {
                hit_something = true;
                first_hit = false;
                hit_point = pos;
            }

            collected_noise += texture(noise_texture, pos * 0.1).x * 0.25;
        }

        if (first_hit) {
            distance += distToScene;
        } else {
            distance += 0.01;
        }

        if (distance > MAX_DIST * 100) {
            break;
        }
        if (collected_noise > 1.0) {
            collected_noise = 1.0;
            break;
        }
        distance += 0.1;
    }
    vec4 result = vec4(0.0);

    if (hit_something) {
        vec3 normal = getNormal(hit_point);
        float lightIntensity = dot(normal, normalize(vec3(1.0, 1.0, 1.0))) * 0.5 + 0.5;
        result = vec4(1.0 * lightIntensity, 0.5 * lightIntensity, 0.2 * lightIntensity, collected_noise);
    }

    return result;
}

void main() {

    vec3 rayDir = normalize(ray);

    vec4 result = do_raymarch(origin, rayDir);

    color = result;
}