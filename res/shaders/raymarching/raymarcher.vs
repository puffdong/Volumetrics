#version 330 core
layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_uv;

uniform mat4 u_invprojview;
uniform vec3 u_camera_pos;

out vec2 v_uv;
out vec3 v_origin;
out vec3 v_ray;

void main() {
    v_uv = a_uv;
    gl_Position = vec4(a_pos, 0.0, 1.0);

    // Figure out the world-space point on the far plane per vertex.
    vec4 far_h  = u_invprojview * vec4(a_pos,  1.0, 1.0);
    vec3 far_w  = far_h.xyz / far_h.w;

    v_origin = u_camera_pos;
    v_ray    = far_w - v_origin;
}