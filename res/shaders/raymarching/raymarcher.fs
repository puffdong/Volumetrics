#version 330 core
layout(location = 0) out vec4 o_color;

in vec2 v_uv;
in vec3 v_origin;
in vec3 v_ray;

// lights
struct Light {
    vec4 position_radius; // position, radius
    vec4 color_intensity; // color, intensity
    vec4 misc;            // volumetric_multiplier, type (0 = point, 1 = directional), padding padding
};

layout(std140) uniform b_light_block {
    Light u_lights[16];
};

uniform int u_light_count;

uniform vec3 u_sun_dir;
uniform vec4 u_sun_color; // .w = intensity 
uniform mat4 u_light_space_matrix;
uniform sampler2D u_shadow_map;

// standard uniforms
uniform float u_time;
uniform vec2 u_resolution;
uniform vec3 u_camera_pos;
uniform mat4 u_invprojview;

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
uniform float u_sun_intensity_multiplier;

uint voxel_value_at(ivec3 cell) { // removing the boundary check increases performance
    return texelFetch(u_voxels, cell, 0).r; // opengl takes care of clamping out-of-bounds
}

float get_voxel_value_float(ivec3 cell) {
    return float(texelFetch(u_voxels, cell, 0).r) / 255.0;
}

ivec3 world_to_cell(vec3 p) {
    vec3 local = (p - u_grid_origin + vec3(u_cell_size / 2)) / u_cell_size;
    return ivec3(floor(local));
}

bool is_inside_grid(vec3 world_pos) {
    ivec3 cell = world_to_cell(world_pos);
    return !any(lessThan(cell, ivec3(0))) && !any(greaterThanEqual(cell, u_grid_dim));
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

float henyey_greenstein(float cos_theta, float g) {
    float g2 = g * g;
    float num = 1.0 - g2;
    float denom = 1.0 + g2 - 2.0 * g * cos_theta;
    
    // The power of 1.5 is computationally expensive, sometimes folks use approximations, 
    // but keep it for now for accuracy.
    return (1.0 / (4.0 * 3.14159)) * num / pow(denom, 1.5);
}

float sample_density(vec3 sample_pos, uint voxel_value) {
    vec3 wind_direction = normalize(vec3(1.0, 0.0, 1.0));
    vec3 offset = wind_direction * u_time * 0.1; // animate noise with time and wind direction
    float noise = texture(u_noise_texture, (sample_pos * 0.05) + offset).r;
    noise = smoothstep(0.1, 0.8, noise); // below 0.1 -> 0.0, above 0.8 -> 1.0
    
    return noise * (float(voxel_value) / 255.0);
}

float check_sun_shadow(vec3 world_pos) {
    vec4 pos_ls = u_light_space_matrix * vec4(world_pos, 1.0);
    vec3 proj_coords = pos_ls.xyz / pos_ls.w;
    proj_coords = proj_coords * 0.5 + 0.5;
    
    if (proj_coords.z > 1.0 || any(lessThan(proj_coords.xy, vec2(0)))) {
        return 1.0;
    }
    
    float bias = 0.0015;
    float depth = texture(u_shadow_map, proj_coords.xy).r;
    return proj_coords.z - bias <= depth ? 1.0 : 0.0;
}

float do_sunlight_march(vec3 start_pos, vec3 sun_dir) {
    if (check_sun_shadow(start_pos) < 0.5) {
        return 0.0;
    }

    float distance_traveled = 0.0;
    float optical_depth = 0.0;

    float sigma_t = u_scattering_coefficient + u_absorption_coefficient;

    for (int i = 0; i < u_max_light_steps; ++i) {
        vec3 ray_pos = start_pos + sun_dir * distance_traveled;

        uint v = get_voxel(ray_pos);
        if (v != 0u) {
            float density = sample_density(ray_pos, v);
            optical_depth += density * sigma_t * u_light_step_size;
        } 
        distance_traveled += u_light_step_size;    
    }

    return exp(-optical_depth);
}

vec4 do_raymarch(vec3 ray_origin, vec3 ray_direction, float start_distance, float scene_distance) {
    float distance_traveled = start_distance + 0.01; // added offset to ensure we start inside a voxel

    float transmittance = 1.0;
    vec3 light_energy = vec3(0.0);

    float sigma_s = u_scattering_coefficient;
    float sigma_t = u_scattering_coefficient + u_absorption_coefficient;

    vec3 sun_vec = normalize(u_sun_dir);
    float cos_theta = dot(sun_vec, ray_direction);
    float phase = henyey_greenstein(cos_theta, u_anisotropy);

    vec3 ambient_light = u_base_color; // treated as light hitting from all directions
    vec3 ambient_intensity = vec3(0.1); // todo: make this a uniform

    vec3 sun_color = u_sun_color.rgb;
    float sun_intensity = u_sun_color.w * u_sun_intensity_multiplier;

    Light point_light = u_lights[0];

    // unpack point lights
    vec3 point_light_pos = point_light.position_radius.xyz;
    vec3 point_light_color = point_light.color_intensity.xyz;
    float point_light_intensity = point_light.color_intensity.w;

    // float iteration = 0.0; // for debugging step count

    for (int i = 0; i < u_max_steps; ++i) {
        
        vec3 ray_pos = ray_origin + ray_direction * distance_traveled;
        if (!is_inside_grid(ray_pos)) {
            break; // outside of grid volume, stop marching
        }
        uint v = get_voxel(ray_pos); // snaps to closest voxel

        if (v != 0u) {
            float density = sample_density(ray_pos, v);

            // calculate sunlight contribution
            float sun_transmittance = do_sunlight_march(ray_pos, sun_vec);
            vec3 sun_light = sun_color * sun_intensity * sun_transmittance * phase;
            
            // calculate point light contribution
            // todo

            vec3 incoming_light = sun_light + ambient_light * ambient_intensity; // combine all light

            float step_attenuation = exp(-density * sigma_t * u_step_size);
            float integrated_light = (1.0 + step_attenuation) * 0.5;

            light_energy += transmittance * (density * sigma_s * incoming_light * u_step_size) * integrated_light; // Accumulate

            transmittance *= step_attenuation; // Attenuate

            if (transmittance < 0.01) {
                transmittance = 0.0;
                break;
            }
        }

        distance_traveled += u_step_size;
        if (distance_traveled > scene_distance) break;
        // iteration += 1.0;
    }

    return vec4(light_energy, 1.0 - transmittance);
    // return vec4(vec3(iteration / float(u_max_steps)), 1.0);
}

float linearize_depth(float depth) {
    float z = depth * 2.0 - 1.0; // back to NDC
    return (2.0 * u_near_plane * u_far_plane) / (u_far_plane + u_near_plane - z * (u_far_plane - u_near_plane));
}

vec3 reconstruct_world(vec2 uv, float depth) {
    vec4 ndc = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 w = u_invprojview * ndc;
    return w.xyz / w.w;
}

void main() {
    float scene_depth = texture(u_scene_depth, v_uv).r;
    float raymarch_depth = texture(u_raymarch_depth, v_uv).r;

    if (raymarch_depth >= scene_depth) {
        o_color = vec4(0.0); // behind scene, early out
        return;
    }

    if (raymarch_depth >= 1.0) {
        o_color = vec4(0.0);
        return;
    }

    vec3 scene_pos = reconstruct_world(v_uv, scene_depth);
    vec3 start_pos = reconstruct_world(v_uv, raymarch_depth);

    float start_dist = length(start_pos - v_origin);
    float scene_dist = length(scene_pos - v_origin);
    
    vec3 ray_direction = normalize(v_ray);
    vec4 result = do_raymarch(v_origin, ray_direction, start_dist, scene_dist);
    
    o_color = result;
}