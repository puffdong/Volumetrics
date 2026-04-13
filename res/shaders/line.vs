#version 330 core
layout (location = 0) in float a_extrusion;
layout (location = 1) in vec3 a_start;
layout (location = 2) in vec3 a_end;
layout (location = 3) in vec4 a_color;

layout(std140) uniform b_camera_block {
	mat4 u_proj;
	mat4 u_view;
	mat4 u_proj_view;
	vec3 u_camera_pos;
	float padding_c;
};

flat out vec4 v_color;

void main()
{
    v_color = a_color;

    vec3 position = mix(a_start, a_end, a_extrusion);
    gl_Position = u_proj * u_view * vec4(position, 1.0);
}