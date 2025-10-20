#version 330 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_tex_coord;
layout(location = 3) in mat4 instance_matrix;

uniform mat4 u_view;
uniform mat4 u_proj;

flat out vec4 v_color;
flat out uint v_occupied;

uniform usampler3D u_voxels; // GL_R8UI
uniform ivec3 u_grid_dim; // (width, height, depth)
uniform vec3 u_grid_origin;
uniform float u_voxel_size; // world units per cell

float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

uint voxel_value_at(ivec3 c) {
    if (any(lessThan(c, ivec3(0))) || any(greaterThanEqual(c, u_grid_dim))) return 0u;
    return texelFetch(u_voxels, c, 0).r; // exact integer fetch; no filtering
}

ivec3 world_to_cell(vec3 p) {
    vec3 local = (p - u_grid_origin + vec3(u_voxel_size / 2)) / u_voxel_size;
    return ivec3(floor(local));
}

void main()
{
    vec3 instance_center = (instance_matrix * vec4(0.0, 0.0, 0.0, 1.0)).xyz;

    ivec3 cell = world_to_cell(instance_center); // get the cell
    uint voxel_value = voxel_value_at(cell);
    v_occupied = voxel_value; // of it 0 then it discarded in fraggie

    if (voxel_value == 0u) v_color = vec4(0.1, 1.0, 1.0, 1.0);
    if (voxel_value == 1u) v_color = vec4(0.25, 0.0, 0.0, 1.0);
    if (voxel_value == 2u) v_color = vec4(0.5, 0.0, 0.0, 1.0);
    if (voxel_value == 3u) v_color = vec4(0.75, 0.0, 0.0, 1.0);
    if (voxel_value == 4u) v_color = vec4(1.0, 0.0, 0.0, 1.0);
    if (voxel_value == 5u) v_color = vec4(1.0, 0.5, 0.0, 1.0);
    if (voxel_value == 6u) v_color = vec4(1.0, 1.0, 0.0, 1.0);

    vec4 world_pos = instance_matrix * vec4(a_pos, 1.0);
    gl_Position = u_proj * u_view * world_pos;

    // gl_Position = u_proj * view_pos;
}