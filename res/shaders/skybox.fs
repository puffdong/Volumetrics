#version 330 core
layout(location = 0) out vec4 o_color;

in vec3 v_dir;

uniform samplerCube u_texture;

void main()
{
	o_color = texture(u_texture, normalize(v_dir));
}