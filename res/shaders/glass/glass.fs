#version 330 core
layout(location = 0) out vec4 o_color;

in vec2 v_uv;

uniform sampler2D u_src_color; // framebuffer sampling :)
uniform sampler2D u_depth_texture;
uniform vec2 u_glass_pane_position;
uniform float u_radius;

uniform float u_far;
uniform float u_near;

uniform vec2 u_resolution;
uniform vec2 u_mouse_pos;

float linearize_depth(float d,float z_near,float z_far)
{
    return z_near * z_far / (z_far + d * (z_near - z_far));
}

vec2 norm_pos_to_screen_pos(vec2 norm_pos) {
    return vec2(norm_pos);
}

void main() {
    
    vec2 uv = vec2(v_uv.x - 0.5, v_uv.y - 0.5); // centered, 
    // vec2 screen_pos = vec2(v_uv.x * u_resolution.x , (1 - v_uv.y) * u_resolution.y);

    float depth_sample = texture(u_depth_texture, v_uv).r;
    o_color = vec4(uv.x, uv.y, depth_sample, 1.0);
    
    o_color = texture(u_src_color, v_uv);
    // o_color = mainImage();
}