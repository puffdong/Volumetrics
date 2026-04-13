#version 330 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texcoord;

layout(std140) uniform b_camera_block {
	mat4 u_view;
	mat4 u_proj;
	mat4 u_proj_view;
	vec3 u_camera_pos;
	float padding_c;
};


out vec3 v_dir;

void main()
{
	mat4 view = mat4(mat3(u_view));

	vec4 pos = u_proj * view * vec4(a_pos, 1.0);

	v_dir = a_pos;
	gl_Position = pos;
}