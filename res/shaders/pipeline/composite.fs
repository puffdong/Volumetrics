#version 330 core
in vec2 v_uv;
out vec4 o_color;

uniform sampler2D u_src_color;    // scene (ping-pong source), bound to TU0
uniform sampler2D u_volum_color;  // volumetrics color, bound to TU1
uniform sampler2D u_scene_depth;  // kept for later (TU2) – unused for now

void main() {
    vec3 scene = texture(u_src_color,   v_uv).rgb;
    vec3 volum = texture(u_volum_color, v_uv).rgb;

    // Simple additive combine for v0; clamp to LDR range
    vec3 combined = clamp(scene + volum, 0.0, 1.0);

    o_color = vec4(combined, 1.0);
}