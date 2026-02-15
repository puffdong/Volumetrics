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
uniform mat4 u_light_space_matrix;

uniform sampler2D u_shadow_map;
uniform int u_light_count;

in vec3 v_pos;
in vec3 v_normal;
in vec2 v_tex_coord;
in vec3 v_frag_pos;
in vec4 v_frag_pos_ls; // light space position

uniform mat4 u_model_matrix;
uniform mat4 u_world_matrix;
uniform mat4 u_mvp;
uniform sampler2D u_texture1;

uniform vec3 u_sun_dir;
uniform vec4 u_sun_color; // .w = intensity
uniform vec3 u_camera_pos;
uniform bool u_is_selected;

// Material uniforms
uniform vec4 u_diffuse_color;   // rgb + diffuse strength
uniform vec4 u_specular_color;  // rgb + specular strength
uniform vec4 u_material_params; // x=shininess, y=metallic, z=roughness, w=padding

// Global ambient
const vec3 AMBIENT = vec3(0.03);

void main()
{
    vec3 norm     = normalize(v_normal);
    vec3 view_dir = normalize(u_camera_pos - v_frag_pos);
    
    // unpack material properties
    vec3 mat_diffuse_color = u_diffuse_color.rgb;
    float mat_diffuse = u_diffuse_color.a;
    vec3 mat_specular_color = u_specular_color.rgb;
    float mat_specular = u_specular_color.a;
    float mat_shininess = u_material_params.x;

    vec3 total_color = vec3(0.0);

    // sun lighting
    {
        vec3 sun_dir = normalize(u_sun_dir);
        vec3 sun_color = u_sun_color.rgb;
        float sun_intensity = u_sun_color.w;
        vec3 halfway_dir = normalize(sun_dir + view_dir);
        
        vec3 proj_coords = v_frag_pos_ls.xyz / v_frag_pos_ls.w;
        proj_coords = proj_coords * 0.5 + 0.5;
        float shadow = 0.0;
        if (proj_coords.z <= 1.0) {
            float bias = 0.0015; // 0.0065;
            float texel = 1.0 / float(textureSize(u_shadow_map, 0).x);
            float closest = texture(u_shadow_map, proj_coords.xy + texel).r;
            shadow += proj_coords.z - bias > closest ? 1.0 : 0.0;
        }
        float visibility = 1.0 - shadow;

        vec3 ambient = mat_diffuse * mat_diffuse_color * AMBIENT * sun_color;
        vec3 diffuse = mat_diffuse_color * mat_diffuse * max(0.0, dot(norm, sun_dir)) * sun_color;
        vec3 specular = mat_specular_color * mat_specular * pow(max(0.0, dot(norm, halfway_dir)), mat_shininess) * sun_color;
        
        total_color += ambient + (diffuse + specular) * sun_intensity * visibility;
    }

    // light loop
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

        vec3 diffuse = mat_diffuse_color * mat_diffuse * max(0.0, dot(norm, light_dir)) * light_color;
        vec3 specular = mat_specular_color * mat_specular * pow(max(0.0, dot(norm, halfway_dir)), mat_shininess) * light_color;
        
        // attenuation is a cool word, what does it even mean
        float inv_sq  = 1.0 / max(dist * dist, 1e-4);
        float x = clamp(1.0 - dist / light_radius, 0.0, 1.0);
        float radius_fade = x * x;
        float attenuation = inv_sq * radius_fade;

        // float a = 1.0; float b = 1.0; float c = 1.0;
        // float attenuation = 1 / (a + b * dist + c * dist * dist)

        vec3 color = (diffuse + specular) * light_intensity * attenuation; 
        total_color += color;
    }

    if (u_is_selected) {
        float rim = pow(1.0 - max(dot(norm, view_dir), 0.0), 2.0);
        vec3 rim_color = vec3(0.2, 0.7, 1.0);
        total_color += rim * rim_color;
    }

    vec3 result_color = total_color;
    o_color = vec4(result_color, 1.0);
}
