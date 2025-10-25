#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;      // Vertex position
layout(location = 1) in vec3 aNormal;   // Vertex normal
layout(location = 2) in vec2 aTexCoord; // Vertex texture coordinates

// uniform vec3 CameraUp_worldspace;
// uniform vec3 CameraRight_worldspace;
// uniform vec2 BillboardSize;
// uniform vec3 particleCenter_worldspace;

uniform mat4 u_mvp;

void main()
{
	// vec3 vertexPosition_worldspace = particleCenter_worldspace + CameraRight_worldspace * squareVertices.x * BillboardSize.x + CameraUp_worldspace * squareVertices.y * BillboardSize.y;
	// vertexPosition_worldspace = particleCenter_worldspace;
	// Get the screen-space position of the particle's center
	// gl_Position = vec4(vertexPosition_worldspace, 1.0f);

	gl_Position = u_mvp * vec4(aPos, 1.0f);
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

void main()
{
	color = vec4(0.0, 0.0, 1.0, 1.0);
}