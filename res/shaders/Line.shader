#shader vertex
#version 330 core
layout (location = 0) in float a_extrusion;
layout (location = 1) in vec3 a_start;
layout (location = 2) in vec3 a_end;
layout (location = 3) in vec4 a_color;

uniform mat4 projection;
uniform mat4 view;

// flat out int instance_id;
flat out vec4 v_color;

void main()
{
    // instance_id = gl_InstanceID;
    v_color = a_color;

    vec3 position = mix(a_start, a_end, a_extrusion);
    gl_Position = projection * view * vec4(position, 1.0);
}

#shader fragment
#version 330 core

uniform vec2 u_resolution;
uniform sampler2D u_depth_texture;

flat in vec4 v_color;

out vec4 o_color;

const vec3 LUMA = vec3(0.2126, 0.7152, 0.0722);

void main()
{   
    vec2 uv = gl_FragCoord.xy / u_resolution;

    float scene_depth = texture(u_depth_texture, uv).r;
    float frag_depth = gl_FragCoord.z;

    vec4 color = v_color;

    if (frag_depth > scene_depth + 0.00001) {
    float gray = dot(color.rgb, LUMA);
    color.rgb = mix(color.rgb, vec3(gray), 0.7);
    color.a *= 0.6;
    }

    o_color = color;
}
