#version 330 core
layout(location = 0) out vec4 o_color;

in vec2 v_uv;
in vec3 v_origin;
in vec3 v_ray;

struct Light {
    vec4 position_radius; // position, radius
    vec4 color_intensity; // color, intensity
    vec4 misc;            // volumetric_intensity, type (0 = point, 1 = directional), padding padding
};

layout(std140) uniform b_light_block {
    Light u_lights[16];
};

uniform int u_light_count;

uniform float time;
uniform vec3 u_sun_dir;
uniform vec3 u_sun_color;
uniform vec3 u_camera_pos;

// textures
uniform sampler2D u_scene_depth;
uniform sampler2D u_raymarch_depth;
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
uniform float u_hit_step_size; // unused
uniform float u_light_step_size;
uniform float u_max_distance;
uniform float u_min_distance;
uniform vec3 u_base_color;
uniform float u_absorption_coefficient;
uniform float u_scattering_coefficient;
uniform float u_extincion_coefficient;
uniform float u_anisotropy;
uniform float u_sun_intensity;

float linearize_depth(float depth) {
    float z = depth * 2.0 - 1.0; // back to NDC
    return (2.0 * u_near_plane * u_far_plane) / (u_far_plane + u_near_plane - z * (u_far_plane - u_near_plane));
}

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

// g = anisotropy.
// g = 0 -> isotropic (Rayleigh-esque)
// g = 0.5 -> forward scattering (Smoke/Clouds)
// g = 0.9 -> strong "Silver Lining"
float henyey_greenstein(float cos_theta, float g) {
    float g2 = g * g;
    float num = 1.0 - g2;
    float denom = 1.0 + g2 - 2.0 * g * cos_theta;
    
    // The power of 1.5 is computationally expensive, sometimes folks use approximations, 
    // but keep it for now for accuracy.
    return (1.0 / (4.0 * 3.14159)) * num / pow(denom, 1.5);
}

float sample_density(vec3 sample_pos) {
    float noise = texture(u_noise_texture, (sample_pos) * 0.05).r;
    return noise;
}

float do_light_marching(vec3 origin, vec3 direction) {
    return 0.0;
}

vec4 do_raymarch(vec3 ray_origin, vec3 ray_direction, float scene_depth, float raymarch_depth) {    
    float start_dist = linearize_depth(raymarch_depth);
    float scene_dist = linearize_depth(scene_depth);
    
    float distance_traveled = start_dist;

    vec3 total_light = vec3(0.0);
    float transmittance = 1.0;

    vec3 sun_vec = -normalize(u_sun_dir);

    float cos_theta = dot(sun_vec, ray_direction);
    float phase = henyey_greenstein(cos_theta, u_anisotropy);

    // vec3 col = u_base_color;
    vec3 col = vec3(0.0);
    float alpha = 1.0f;
    float collected_density = 0.0;
    float thickness = 0.0f;

    Light point_light = u_lights[0]; // trial run, lets go

    // Unpack the light data once
    // vec3  light_pos         = point_light.position_radius.xyz;
    // float light_radius      = point_light.position_radius.w;
    // vec3  light_color       = point_light.color_intensity.xyz;
    // float light_intensity   = point_light.color_intensity.w;
    // float light_volumetric  = point_light.misc.x;
    // float light_type        = point_light.misc.y; // 0 = point, 1 = directional (unused for now)
    
    for (int i = 0; i < u_max_steps; ++i) {
        
        vec3 sample_pos = ray_origin + ray_direction * distance_traveled;
        uint v = get_voxel(sample_pos); // snaps to closest voxel from vec3(float)

        if (v != 0u) {
            float density = sample_density(sample_pos);

            collected_density += density * u_step_size;// * 0.5 * 0.9;
            thickness += density * u_step_size;
            alpha = exp(-thickness * collected_density * u_extincion_coefficient);

            vec3 light_sample_pos = sample_pos;
            float light = 0.0f;
            vec3 light_result_color = vec3(0.0);

            light += density * 1.0;
            for (int j = 0; j < u_max_light_steps; ++j) {
                light_sample_pos = light_sample_pos + normalize(u_sun_dir) * u_light_step_size;
                uint w = get_voxel(light_sample_pos);
                if (w != 0u) {
                    light += sample_density(light_sample_pos);
                }
                // light_sample_pos = light_sample_pos + normalize(point_light.position_radius.xyz - light_sample_pos) * u_light_step_size;    
                // uint w = get_voxel(light_sample_pos);
                // if (w != 0u) {
                //     light += sample_density;
                //     light_result_color += light_color * light;
                // }
            }

            vec3 light_attenuation = exp(-(light / vec3(1.0) * u_extincion_coefficient * 1.0));
            col += vec3(1.0) * light_attenuation * alpha * phase * u_scattering_coefficient * 1.0 * density;
            // col += vec3(1.0) * light_result_color * light_attenuation * alpha * phase * u_scattering_coefficient * 1.0 * sample_density;

            if (collected_density >= 1.0) {
                collected_density = 1.0;
                break;
            }
        }

        if (distance_traveled > scene_dist) break; // hmm maybe this can be used to determine max steps dynamically?
        distance_traveled += u_step_size;
    }

    return vec4(col, clamp(collected_density, 0.0, 1.0));
}

void main() {
    float scene_depth = texture(u_scene_depth, v_uv).r;
    float raymarch_depth = texture(u_raymarch_depth, v_uv).r;

    // check for if marching is needed
    if (raymarch_depth >= scene_depth) {
        o_color = vec4(0.0); // behind scene, early out
        return;
    }

    if (raymarch_depth >= 1.0) {
        o_color = vec4(0.0);
        return;
    }
    // if all clear, then we march!

    vec3 ray_direction = normalize(v_ray);
    vec4 result = do_raymarch(v_origin, ray_direction, scene_depth, raymarch_depth);

    // VISUALIZE RAY ORIGIN
    // vec4 result = vec4(normalize(v_origin), 1.0);

    // VISUALIZE RAY DIRECTION
    // vec4 result = vec4(normalize(ray_direction), 1.0);
    
    o_color = result;
}