#version 330 core

layout(location = 0) out vec4 o_color;

struct Light {
    vec4 position_radius; // position, radius
    vec4 color_intensity; // color, intensity
    vec4 misc;            // volumetric_intensity, type (0 = point, 1 = directional), padding padding
};

layout(std140) uniform b_light_block {
    Light u_lights[16];
};

in vec3 v_pos;
in vec3 v_normal;
in vec2 v_tex_coord;
in vec3 v_frag_pos;

uniform mat4 u_model_matrix;
uniform mat4 u_world_matrix;
uniform mat4 u_mvp;
uniform sampler2D u_texture1;

uniform vec3 u_sun_dir;
uniform vec4 u_sun_color; // .w = intensity
uniform vec3 u_camera_pos;
uniform int u_light_count;

// light properties
const vec3 AMBIENT = vec3(0.0);

// material properties
const float MATERIAL_DIFFUSE = 1.0;
const float MATERIAL_SPECULAR = 0.6;
const float MATERIAL_SHININESS = 64.0;

void main()
{
    vec3 norm     = normalize(v_normal);
    vec3 view_dir = normalize(u_camera_pos - v_frag_pos);

    vec3 total_color = vec3(0.0);

    for (int i = 0; i < u_light_count; ++i)
    {
        Light light = u_lights[i];

        vec3  light_pos       = light.position_radius.xyz;
        float light_radius    = light.position_radius.w;
        vec3  light_color     = light.color_intensity.xyz;
        float light_intensity = light.color_intensity.w;
        float light_type      = light.misc.y; // 0 = point, 1 = directional (gotta do that)

        vec3 light_dir = normalize(light_pos - v_frag_pos);
        vec3 halfway_dir = normalize(light_dir + view_dir);
        float dist = length(light_pos - v_frag_pos);

        vec3 ambient = MATERIAL_DIFFUSE * AMBIENT;
        vec3 diffuse = light_color * MATERIAL_DIFFUSE * max(0.0, dot(norm, light_dir));
        vec3 specular = light_color * MATERIAL_SPECULAR * pow(max(0.0, dot(norm, halfway_dir)), MATERIAL_SHININESS);
        
        // attenuation is a cool word, what does it even mean
        float inv_sq  = 1.0 / max(dist * dist, 1e-4);
        float x = clamp(1.0 - dist / light_radius, 0.0, 1.0);
        float radius_fade = x * x;
        float attenuation = inv_sq * radius_fade;

        // float a = 1.0; float b = 1.0; float c = 1.0;
        // float attenuation = 1 / (a + b * dist + c * dist * dist)

        vec3 color = ambient + (diffuse + specular) * light_intensity * attenuation; 
        total_color += color;
    }

    vec3 result_color = total_color;
    o_color = vec4(result_color, 1.0);
}
