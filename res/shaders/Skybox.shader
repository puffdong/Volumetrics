#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;      // Vertex position
layout(location = 1) in vec3 aNormal;   // Vertex normal
layout(location = 2) in vec2 aTexCoord; // Vertex texture coordinates

out vec2 v_texCoord;

uniform mat4 u_MVP;

void main()
{
	v_texCoord = aTexCoord;
	gl_Position = u_MVP * vec4(aPos, 1.0f);
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 v_texCoord;

uniform sampler2D u_Texture;

void main()
{
	color = texture(u_Texture, v_texCoord) * vec4(1.0, 1.0, 1.0, 1.0);
};