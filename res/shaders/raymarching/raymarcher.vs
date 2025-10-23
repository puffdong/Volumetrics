#version 330 core
layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_uv;

uniform mat4 u_invprojview;
uniform float u_near_plane;
uniform float u_far_plane;
uniform vec3 u_camera_pos;

out vec2 v_uv;
out vec3 v_origin;
out vec3 v_ray;

void main() {
    v_uv = a_uv;
    gl_Position = vec4(a_pos, 0.0, 1.0);
    v_origin = (u_invprojview * vec4(a_pos, -1.0, 1.0) * u_near_plane).xyz;
    // v_origin = u_camera_pos; // seems quite unessecary to do the above, right? gotta calc it by hand cuz I just trust the above like my life depends on it
    v_ray = (u_invprojview * vec4(a_pos * (u_far_plane - u_near_plane), u_far_plane + u_near_plane, u_far_plane - u_near_plane)).xyz;
}