#version 330 core
layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_uv;

uniform float u_near;
uniform float u_far;

out vec2 v_uv;

void main() {
    v_uv = a_uv;
    gl_Position = vec4(a_pos, 0.0, 1.0);
}