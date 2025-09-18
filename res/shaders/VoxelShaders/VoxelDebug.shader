#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in mat4 instanceMatrix;

uniform mat4 view;
uniform mat4 proj;
flat out vec4 v_color;

float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    vec4 worldPos = instanceMatrix * vec4(aPos, 1.0);
    vec4 viewPos = view * worldPos;

    float red = rand(vec2(worldPos.x, worldPos.y));
    float green = rand(vec2(worldPos.z, worldPos.w));
    float blue = rand(vec2(worldPos.y, worldPos.x));
    v_color = vec4(red, green, blue, 1.0);
    // exSurfacePos = vec3(viewPos); // Surface position in view space

    gl_Position = proj * viewPos;
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;
flat in vec4 v_color;

void main()
{
    color = v_color;
}