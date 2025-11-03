#version 330 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_tex_coord;
layout(location = 3) in ivec3 i_cell; // TODO: just supply it with an ivec4 and have .w be the value at that point!
// easy peasy solution to skip having to sample a big 3D texture every time

uniform mat4 u_view;
uniform mat4 u_proj;

uniform ivec3 u_grid_dim; // (width, height, depth)
uniform vec3 u_grid_origin;
uniform float u_voxel_size; // world units per cell

void main()
{
    vec3 cell_center = u_grid_origin + vec3(i_cell) * u_voxel_size;

    vec3 local_pos = a_pos * (u_voxel_size * 0.5);
    vec3 world_pos = cell_center + local_pos;

    gl_Position = u_proj * u_view * vec4(world_pos, 1.0);
}