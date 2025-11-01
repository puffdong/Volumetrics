#version 330 core
in vec2 v_uv;
out vec4 o_color;

// Inputs
uniform sampler2D u_src_color;
uniform sampler2D u_volum_color;

// Controls
uniform int   u_volum_is_premultiplied = 1;
uniform float u_volum_opacity_mul      = 1.0;

void main() {
    vec3 scene_rgb = texture(u_src_color,   v_uv).rgb;

    vec4 volum_rgba = texture(u_volum_color, v_uv);
    float a = volum_rgba.a;
    vec3 out_rgb = volum_rgba.rgb + (1.0 - a) * scene_rgb;

    // out_rgb = clamp(out_rgb, 0.0, 1.0); // hdr no clamp, otherwise yes clamp
    o_color = vec4(out_rgb, 1.0);
}
