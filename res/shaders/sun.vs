#version 330 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texcoord;

uniform mat4 u_mvp;

out vec2 v_texcoord;

void main()
{
	v_texcoord = a_texcoord;
	gl_Position = u_mvp * vec4(a_pos, 1.0f);
}

