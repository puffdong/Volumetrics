#version 330 core
in vec2 v_uv;
out vec4 o_color;

// Inputs
uniform float u_near;
uniform float u_far;

uniform sampler2D u_src_color;
uniform sampler2D u_volum_color;
uniform sampler2D u_scene_depth;
uniform sampler2D u_raymarch_depth;

// Controls
uniform int   u_volum_is_premultiplied = 1;
uniform float u_volum_opacity_mul = 1.0;

float linearize_depth(float depth) {
    float z = depth * 2.0 - 1.0; // back to NDC
    float near = 0.1;
    float far  = 512.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main() {
    float scene_depth = texture(u_scene_depth, v_uv).r;
    float raymarch_depth = texture(u_raymarch_depth, v_uv).r;

    if (raymarch_depth >= scene_depth) {
        o_color = texture(u_src_color, v_uv);
    } else {
        vec3 scene_rgb = texture(u_src_color,   v_uv).rgb;

        vec4 volum_rgba = texture(u_volum_color, v_uv);
        float a = volum_rgba.a;
        vec3 out_rgb = volum_rgba.rgb + (1.0 - a) * scene_rgb;

        // out_rgb = clamp(out_rgb, 0.0, 1.0); // Psst... hdr support would be cool to do!
        o_color = vec4(out_rgb, 1.0);
    }


}
