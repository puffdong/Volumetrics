#version 330 core
in vec2 v_uv;
out vec4 o_color;

// Inputs
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

    // float vis_scene_depth = 1 - linearize_depth(scene_depth) / 512.0;
    // float vis_raymarch_depth = 1 - linearize_depth(raymarch_depth) / 512.0;

    // float combined = max(vis_scene_depth, vis_raymarch_depth);

    // o_color = vec4(vec3(vis_scene_depth), 1.0);
    // o_color = vec4(vec3(vis_raymarch_depth), 1.0);
    // o_color = vec4(vec3(combined), 1.0);
    // o_color = texture(u_volum_color, v_uv);

    if (raymarch_depth >= scene_depth) {
        // behind scene, just output scene color
        o_color = texture(u_src_color, v_uv);
    } else {
        vec3 scene_rgb = texture(u_src_color,   v_uv).rgb;

        vec4 volum_rgba = texture(u_volum_color, v_uv);
        float a = volum_rgba.a;
        vec3 out_rgb = volum_rgba.rgb + (1.0 - a) * scene_rgb;

        // out_rgb = clamp(out_rgb, 0.0, 1.0); // hdr no clamp, otherwise yes clamp
        o_color = vec4(out_rgb, 1.0);
    }


}
