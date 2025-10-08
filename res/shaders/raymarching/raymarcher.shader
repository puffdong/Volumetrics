#shader vertex
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;

uniform mat4 invprojview;
uniform float near_plane;
uniform float far_plane;

out vec3 v_origin;
out vec3 v_ray;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    v_origin = (invprojview * vec4(aPos, -1.0, 1.0) * near_plane).xyz;
    v_ray = (invprojview * vec4(aPos * (far_plane - near_plane), far_plane + near_plane, far_plane - near_plane)).xyz;
}

#shader fragment
#version 330 core
layout(location = 0) out vec4 color;

in vec3 v_origin;
in vec3 v_ray;

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
const int MAX_STEPS = 56;
const float STEP_SIZE = 0.5;
const float HIT_STEP_SIZE = 0.2; // a bit lower but myeee


const float MAX_DIST = 256.0;
const float MIN_DIST = 0.000001;
// const int MAX_STEPS = 16;

uint voxel_value_at(ivec3 cell) {
    if (any(lessThan(cell, ivec3(0))) || any(greaterThanEqual(cell, u_grid_dim))) return 0u;
    return texelFetch(u_voxels, cell, 0).r;
}

ivec3 world_to_cell(vec3 p) {
    vec3 local = (p - u_grid_origin + vec3(u_voxel_size / 2)) / u_voxel_size; // the added vec3(// HALF OF CELL_SIZE //) makes it line up with the debug view
    return ivec3(floor(local)); // yet it feels inherently wrong. prolly debug is the one that is off
}

float occupancy_at_world(vec3 p) {
    return float(voxel_value_at(world_to_cell(p)) > 0u);
}

uint get_voxel(vec3 pos) {
    return voxel_value_at(world_to_cell(pos));
}

vec4 do_raymarch(vec3 ray_origin, vec3 ray_direction) {
    float distance_traveled = 0.0f;
    int iteration = 0;
    
    vec3 col = vec3(0.0f);
    float alpha = 1.0f;
    float collected_noise = 0.0;

    bool hit_something = false;

    for (int i = 0; i < MAX_STEPS; ++i) {
        vec3 sample_pos = ray_origin + ray_direction * distance_traveled;
        
        // if (v == 0u) {
        //     distance_traveled += STEP_SIZE;
        //     iteration += 1;
        //     continue;
        // }


        float occ = occupancy_at_world(sample_pos);

        if (occ > 0.5) {
            if (!hit_something) {
                hit_something = true;
                distance_traveled -= STEP_SIZE; // take a step back and then increase step size later on to fill in the gaps
            }

            float noise = texture(noise_texture, (sample_pos) * 0.05).r;

            collected_noise += noise * HIT_STEP_SIZE * 0.5 * 0.9;

            if (collected_noise >= 1.0) {
                collected_noise = 1.0;
                break;
            }
        }

        if (hit_something) {
            distance_traveled += HIT_STEP_SIZE;
        } else {
        distance_traveled += STEP_SIZE;
        }
        if (distance_traveled > MAX_DIST) break;
    }

    vec4 result = vec4(0.0);
    if (hit_something) {
        vec3 base_color = vec3(1.0, 8.0, 8.0);
        result = vec4(base_color, clamp(collected_noise, 0.0, 1.0));
    }
    return result;
}

void main() {

    vec3 ray_direction = normalize(v_ray);
    vec4 result = do_raymarch(v_origin, ray_direction);

    color = result;
}