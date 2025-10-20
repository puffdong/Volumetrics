#version 330 core
layout(location = 0) out vec4 color;

in vec3 v_origin;
in vec3 v_ray;

uniform float time;
uniform vec3 u_sun_dir;
uniform vec3 u_sun_color;
uniform vec3 u_camera_pos;
uniform sampler3D u_noise_texture;

// voxels
uniform usampler3D u_voxels; // GL_R8UI
uniform ivec3 u_grid_dim; // (width, height, depth)
uniform vec3 u_grid_origin;
uniform float u_cell_size; // world units per cell

uniform float u_near_plane;
uniform float u_far_plane;

uniform int u_max_steps;
uniform float u_step_size;
uniform int u_max_light_steps;
uniform float u_hit_step_size;
uniform float u_light_step_size;
uniform float u_max_distance;
uniform float u_min_distance;
uniform vec3 u_base_color;
uniform float u_absorption_coefficient;
uniform float u_scattering_coefficient;
uniform float u_extincion_coefficient;


// constants
// marching
const int MAX_STEPS = 56;
const float STEP_SIZE = 0.5;
const float HIT_STEP_SIZE = 0.2; // a bit lower but myeee

const int MAX_LIGHT_STEPS = 16;
const float LIGHT_STEP_SIZE = 0.75;

// light calc params
const vec3 FOG_COLOR = vec3(0.05, 0.05, 0.05);
const float ABSORPTION_COEFF = 0.5f;
const float SCATTERING_COEFF = 0.5f;
const float EXTINCTION_COEFF = 0.25; // absorption * scattering

const float SHADOW_DENSITY = 1.0f;

const float MAX_DIST = 256.0;
const float MIN_DIST = 0.000001;
// const int MAX_STEPS = 16;

uint voxel_value_at(ivec3 cell) {
    if (any(lessThan(cell, ivec3(0))) || any(greaterThanEqual(cell, u_grid_dim))) return 0u;
    return texelFetch(u_voxels, cell, 0).r;
}

ivec3 world_to_cell(vec3 p) {
    vec3 local = (p - u_grid_origin + vec3(u_cell_size / 2)) / u_cell_size; // the added vec3(// HALF OF CELL_SIZE //) makes it line up with the debug view
    return ivec3(floor(local)); // yet it feels inherently wrong. prolly debug is the one that is off
}

float occupancy_at_world(vec3 p) {
    return float(voxel_value_at(world_to_cell(p)) > 0u);
}

uint get_voxel(vec3 pos) {
    return voxel_value_at(world_to_cell(pos));
}

float rayleigh(float cos_theta) {
    return (3.0f / (16.0f * 3.1415926538)) * (1 + cos_theta * cos_theta);
}

float sample_density(vec3 sample_pos) {
    float noise = texture(u_noise_texture, (sample_pos) * 0.05).r;
    return noise;
}

vec4 do_raymarch(vec3 ray_origin, vec3 ray_direction) {
    float distance_traveled = 0.0f;
    int iteration = 0;
    
    vec3 col = u_base_color;
    float alpha = 1.0f;
    float collected_density = 0.0;
    float thickness = 0.0f;

    bool hit_something = false;

    for (int i = 0; i < u_max_steps; ++i) {
        
        vec3 sample_pos = ray_origin + ray_direction * distance_traveled;
        uint v = get_voxel(sample_pos); // snaps to closest voxel from vec3(float)

        if (v != 0u) {
            if (!hit_something) {
                hit_something = true; // first hit
                distance_traveled -= u_step_size; // take a step back and then increase step size later on to fill in the gaps
            }
            // cool calcs below
            float cos_theta = dot(ray_direction, vec3(0.0, 1.0, 0.0));
            float phase = rayleigh(cos_theta);

            float sample_density = sample_density(sample_pos);

            collected_density += sample_density * u_hit_step_size;// * 0.5 * 0.9;
            thickness += sample_density * u_hit_step_size;
            alpha = exp(-thickness * collected_density * u_extincion_coefficient);

            vec3 light_pos = sample_pos;
            float light = 0.0f;
            light += sample_density * 1.0;
            for (int j = 0; j < u_max_light_steps; ++j) {
                light_pos = light_pos + normalize(u_sun_dir) * u_light_step_size;
                uint w = get_voxel(light_pos);
                if (w != 0u) {
                    light += sample_density;
                }
            }

            vec3 light_attenuation = exp(-(light / vec3(1.0) * u_extincion_coefficient * 1.0));
            col += vec3(1.0) * light_attenuation * alpha * phase * u_scattering_coefficient * 1.0 * sample_density;

            if (collected_density >= 1.0) {
                collected_density = 1.0;
                break;
            }
        }


        // end of iteration stuff
        if (distance_traveled > u_max_distance) break;
        if (hit_something) {
            distance_traveled += u_hit_step_size;
        } else {
        distance_traveled += u_step_size;
        }
    }

    vec4 result = vec4(0.0);
    if (hit_something) {
        vec3 base_color = vec3(1.0, 8.0, 8.0);
        result = vec4(u_base_color, clamp(collected_density, 0.0, 1.0));
    }
    return vec4(col, clamp(collected_density, 0.0, 1.0));
}

void main() {

    vec3 ray_direction = normalize(v_ray);
    vec4 result = do_raymarch(v_origin, ray_direction);

    color = result;
}