#version 330 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texcoord;

out vec2 v_texcoord;

uniform mat4 u_mvp;

uniform mat4 u_view;
uniform mat4 u_proj;

out vec3 v_dir;

void main()
{
	mat4 view = mat4(mat3(u_view));

	vec4 pos = u_proj * view * vec4(a_pos, 1.0);

	v_texcoord = a_texcoord;
	gl_Position = pos;
	v_dir = a_pos;
}