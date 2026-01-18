#version 330 core
layout(location = 0) out vec4 o_color;
in vec2 v_uv;

uniform sampler2D u_src_color;

void main() {
    o_color = texture(u_src_color, v_uv);
}
