#version 330 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_tex_coord;

out vec3 v_pos;
out vec3 v_normal;
out vec2 v_tex_coord;
out vec3 v_frag_pos;

uniform mat4 u_proj;
uniform mat4 u_view;
uniform mat4 u_model;
uniform mat4 u_mvp;

void main()
{
	v_pos = a_pos;
	v_normal = a_normal;
	v_tex_coord = a_tex_coord;
	v_frag_pos = vec3(u_model * vec4(a_pos, 1.0));
	gl_Position = u_mvp * vec4(a_pos, 1.0f);
}