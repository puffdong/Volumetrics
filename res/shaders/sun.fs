#version 330 core
layout(location = 0) out vec4 o_color;

uniform float time;
uniform vec4 u_sun_color;

in vec2 v_texcoord;

void main()
{
	float dist = length(v_texcoord - vec2(0.5, 0.5));
	if (dist > 0.5) discard; // circle!

	vec4 color = u_sun_color;
	color.a *= smoothstep(0.5, 0.3, dist);
	o_color = color;
}