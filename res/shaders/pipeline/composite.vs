#version 330 core
layout(location = 0) in vec2 a_pos;  // NDC positions from your fullscreen tri
layout(location = 1) in vec2 a_uv;   // precomputed UVs

out vec2 v_uv;

void main() {
    v_uv = a_uv;
    gl_Position = vec4(a_pos, 0.0, 1.0);
}
