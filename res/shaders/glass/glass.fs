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
uniform vec3 u_mouse_pos;

vec2 uv_to_texture(vec2 uv) { return (uv * vec2(1.0 / (u_resolution.x / u_resolution.y), 1.0) / 2) + 0.5; }

void main() {
    vec2 uv = (2.0 * gl_FragCoord.xy-u_resolution.xy) / u_resolution.y;
    
    float circle_radius = 0.45;
    vec2 pos = vec2(0.0);
    
    float center_distance = distance(uv, pos);
    
    vec3 background_color = texture(u_src_color, v_uv).xyz;
    
    float refraction = pow(center_distance / circle_radius, 7.0) * 0.35;
    vec2 offset = normalize(pos - uv) * refraction; // how we offset our sampling for to get refractions
    vec2 dispersion = offset * 0.05; // added offset per color channel for that extra pzzazzzzz

    float red = texture(u_src_color, uv_to_texture((uv + offset - dispersion) * 0.80)).r;
    float green = texture(u_src_color, uv_to_texture((uv + offset - dispersion * 0.5) * 0.80)).g;
    float blue = texture(u_src_color, uv_to_texture((uv + offset + dispersion) * 0.80)).b;
    
    vec3 glass_color = vec3(red, green, blue);
    
    float edge = fwidth(center_distance);
    float alpha = 1.0 - smoothstep(circle_radius - edge, circle_radius + edge, center_distance); // interpolates alpha value at the edge

    vec3 color = mix(background_color, glass_color, alpha);
    o_color = vec4(color, 1.0);

}
