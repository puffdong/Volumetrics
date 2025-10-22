#version 330 core
in vec2 v_uv;
out vec4 o_color;

uniform sampler2D u_src_color; // generic “source” texture, bound to TU0

void main() {
    o_color = texture(u_src_color, v_uv);
}
