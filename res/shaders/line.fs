#version 330 core

flat in vec4 v_color;

uniform float u_far;
uniform float u_near;

uniform vec2 u_resolution;
uniform sampler2D u_depth_texture;
out vec4 o_color;

const vec3 LUMA = vec3(0.2126, 0.7152, 0.0722); // this is for grayscale conversion

float linearize_depth(float depth) {
    float z = depth * 2.0 - 1.0;
    return (2.0 * u_near * u_far) / (u_far + u_near - z * (u_far - u_near));
}

void main()
{   
    vec2 uv = gl_FragCoord.xy / u_resolution;

    float scene_depth_ndc = texture(u_depth_texture, uv).r;
    float frag_depth_ndc = gl_FragCoord.z;
    
    float scene_depth = linearize_depth(scene_depth_ndc);
    float frag_depth = linearize_depth(frag_depth_ndc);

    vec4 color = v_color;

    if (frag_depth > scene_depth + 0.01) {
        float gray = dot(color.rgb, LUMA);
        color.rgb = mix(color.rgb, vec3(gray), 0.7);
        color.a *= 0.6;
    }

    o_color = color;
}
