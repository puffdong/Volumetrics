#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;
uniform float woah;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}


#shader fragment
#version 330 core

out vec4 FragColor;
uniform float woah;

void main()
{
    FragColor = vec4(1.0, sin(woah), 0.0, 1.0);  // Red Color
}
