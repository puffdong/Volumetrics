#version 330 core
layout (location = 0) in float a_extrusion;
layout (location = 1) in vec3 a_start;
layout (location = 2) in vec3 a_end;
layout (location = 3) in vec4 a_color;

uniform mat4 u_projection;
uniform mat4 u_view;

flat out vec4 v_color;

void main()
{
    v_color = a_color;

    vec3 position = mix(a_start, a_end, a_extrusion);
    gl_Position = u_projection * u_view * vec4(position, 1.0);
}