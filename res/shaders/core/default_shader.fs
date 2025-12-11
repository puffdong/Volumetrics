#version 330 core

layout(location = 0) out vec4 color;

struct Light {
    vec4 position_radius; // position, radius
    vec4 color_intensity; // color, intensity
    vec4 misc;            // volumetric_intensity, type (0 = point, 1 = directional), padding padding
};

layout(std140) uniform b_light_block {
    // Light u_lights[MAX_LIGHTS];
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
uniform vec3 u_sun_color;
uniform vec3 u_camera_pos;
uniform int u_light_count;

// uniform float textureScale;
// uniform float specularStrength;
// uniform float diffuseStrength;
// uniform int shininess;

const float diffuse_strength = 0.9;
const float specular_strength = 0.4;
const float shininess = 64.0;
const float texture_scale = 1.0;

void main()
{
    vec3 norm     = normalize(v_normal);
    vec3 view_dir = normalize(u_camera_pos - v_frag_pos);

    vec3 total_diffuse  = vec3(0.0);
    vec3 total_specular = vec3(0.0);

    for (int i = 0; i < u_light_count; ++i)
    {
        Light point_light = u_lights[i];
        
        vec3  light_pos         = point_light.position_radius.xyz;
        float light_radius      = point_light.position_radius.w;
        vec3  light_color       = point_light.color_intensity.xyz;
        float light_intensity   = point_light.color_intensity.w;
        float light_volumetric  = point_light.misc.x;
        float light_type        = point_light.misc.y; // 0 = point, 1 = directional (unused for now)

        vec3 light_dir   = normalize(light_pos - v_frag_pos);
        vec3 halfway_dir = normalize(light_dir + view_dir);

        float spec = pow(max(dot(norm, halfway_dir), 0.0), shininess);
        vec3 specular = light_color * spec;

        float diff = max(dot(norm, light_dir), 0.0);
        vec3 diffuse = diff * light_color;

        total_diffuse  += diffuse;
        total_specular += specular;
    }

    vec3 ambient = vec3(0.2, 0.2, 0.2);
    vec3 result  = (ambient + total_diffuse + total_specular) * vec3(0.9, 0.9, 0.9);

    color = vec4(result, 1.0);
}
