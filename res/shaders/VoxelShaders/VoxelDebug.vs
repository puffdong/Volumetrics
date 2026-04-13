#version 330 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_tex_coord;
layout(location = 3) in ivec3 i_cell;

layout(std140) uniform b_camera_block {
	mat4 u_view;
	mat4 u_proj;
	mat4 u_proj_view;
	vec3 u_camera_pos;
	float padding_c;
};

layout(std140) uniform b_voxel_grid {
    vec3 u_grid_origin;
    float u_cell_size; // world units per cell
    ivec3 u_grid_dim; // width, height, depth
    float padding_b; // to make it 16 byte aligned
};

void main()
{
    vec3 cell_center = u_grid_origin + vec3(i_cell) * u_cell_size;

    vec3 local_pos = a_pos * (u_cell_size * 0.5);
    vec3 world_pos = cell_center + local_pos;

    gl_Position = u_proj * u_view * vec4(world_pos, 1.0);
}