#shader vertex
#version 330 core
layout(location = 0) in vec3 a_pos;      // Vertex position
layout(location = 1) in vec3 a_normal;   // Vertex normal
layout(location = 2) in vec2 a_texcoord; // Vertex texture coordinates

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

#shader fragment
#version 330 core

layout(location = 0) out vec4 o_color;

in vec3 v_dir;

uniform samplerCube u_texture;

void main()
{
	o_color = texture(u_texture, normalize(v_dir));
}