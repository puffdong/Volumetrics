#version 330 core

layout(location = 0) out vec4 color;

flat in vec4 v_color;
flat in uint v_occupied;

void main()
{   
    if (v_occupied == 0u) discard; // cull empty voxels entirely
    color = v_color;
}