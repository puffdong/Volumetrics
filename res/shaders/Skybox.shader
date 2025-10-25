#shader vertex
#version 330 core
layout(location = 0) in vec3 a_pos;      // Vertex position
layout(location = 1) in vec3 a_normal;   // Vertex normal
layout(location = 2) in vec2 a_texcoord; // Vertex texture coordinates

out vec2 v_texcoord;

uniform mat4 u_mvp;

void main()
{
	v_texcoord = a_texcoord;
	gl_Position = u_mvp * vec4(a_pos, 1.0f);
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 v_texcoord;

uniform sampler2D u_texture;

void main()
{
	color = texture(u_texture, v_texcoord);
}