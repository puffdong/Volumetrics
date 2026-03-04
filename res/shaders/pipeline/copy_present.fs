#version 330 core
layout(location = 0) out vec4 o_color;
in vec2 v_uv;

uniform sampler2D u_src_color;

// make a function that turns log depth into linear depth, then use that to make the output color more visible
float log_to_linear(float log_depth, float near_plane, float far_plane) {
    float linear_depth = (pow(2.0, log_depth) - 1.0) * (far_plane - near_plane) + near_plane;
    return linear_depth;
}

void main() {
    o_color = texture(u_src_color, v_uv);
    // float value = texture(u_src_color, v_uv).r;
    // float linear_depth = log_to_linear(value, 1.0, 256.0);
    // float normalized_depth = (linear_depth - 100.0) / (256.0 - 50.0);
    // if (normalized_depth > 0.7) normalized_depth = 0.0;
    // o_color = vec4((normalized_depth), 0.0, 0.0, 1.0);
}
