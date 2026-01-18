#include "VoxelGrid.hpp"
#include <iostream>

VoxelGrid::VoxelGrid(int w, int h, int d, uint8_t init_value, float cell_size, glm::vec3 pos, glm::vec3 scale)
    : width(w), height(h), depth(d), cell_size(cell_size), num_occupied_voxels(0), position(pos), scale(scale)
{   
    num_voxels = h * w * d;
    voxels = std::vector<uint8_t>(num_voxels, static_cast<uint8_t>(init_value));
    
    turn_on_corner_visualization();

    add_cube(glm::ivec3(5, 5, 5), 10, 2, 10, 1);
    add_cube(glm::ivec3(6, 6, 6), 8, 2, 8, 1);
    add_cube(glm::ivec3(7, 7, 7), 6, 2, 6, 1);
    add_cube(glm::ivec3(8, 8, 8), 4, 6, 4, 1);
}

void VoxelGrid::init(ResourceManager& resources) {
    r_shader = resources.load_shader("res:://shaders/VoxelShaders/VoxelDebug.vs", "res:://shaders/VoxelShaders/VoxelDebug.fs");
    Res::Model r_model = resources.load_model("res://models/VoxelModels/defaultCube.obj");
    cube_model = resources.get_model_gpu_data(r_model.id);
    init_instance_buffer();
    create_voxel_texture();
}

void VoxelGrid::tick(float delta) {
    if (changed) {
        re_init_instance_buffer();
        changed = false;
    }
}

void VoxelGrid::enqueue(Renderer& renderer, ResourceManager& resources) {
    // if (!_visible) return; // this was moved to execute_pipeline!

    if (auto shader = resources.get_shader(r_shader.id)) {
        (*shader)->hot_reload_if_changed();
        (*shader)->bind();
        (*shader)->set_uniform_mat4("u_proj", renderer.get_proj());
        (*shader)->set_uniform_mat4("u_view", renderer.get_view());
        (*shader)->set_uniform_ivec3("u_grid_dim", glm::ivec3(width, height, depth));
        (*shader)->set_uniform_vec3("u_grid_origin", position);
        (*shader)->set_uniform_float("u_voxel_size", cell_size);

        TextureBinding bind{ voxel_tex, GL_TEXTURE_3D, 5, "u_voxels" };

        RenderCommand cmd{};
        cmd.vao        = cube_model.vao;
        cmd.draw_type   = DrawType::ElementsInstanced;
        cmd.primitive = GL_TRIANGLES;
        cmd.count      = cube_model.index_count;
        cmd.instance_count = num_occupied_voxels;
        cmd.shader     = (*shader);
        // cmd.state.depth_test = true;
        // cmd.state.depth_write = true;
        cmd.textures.push_back(bind);

        renderer.submit(RenderPass::RaymarchBounds, cmd);
    } else {
        std::cout << "something fricked up when displaying the voxelgrid" << std::endl;
    }
}

void VoxelGrid::init_instance_buffer() {
    num_occupied_voxels = 0; // reset this, we are recounting them!

    std::vector<glm::ivec3> instance_grid_indexes;

    for (int w = 0; w < width; w++) {
        for (int h = 0; h < height; h++) {
            for (int d = 0; d < depth; d++) {
                if (get_voxel_value(w, h, d) != 0u) {
                    instance_grid_indexes.push_back(glm::ivec3(w, h, d));
                    num_occupied_voxels += 1;
                }
            }
        }
    }

    std::cout << instance_grid_indexes.size() << " <--- VOXEL GRID INSTANCE COUNT!" << std::endl;

    GLuint vao = cube_model.vao;
    glBindVertexArray(vao);

    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instance_grid_indexes.size() * sizeof(glm::ivec3), instance_grid_indexes.data(), GL_STATIC_DRAW);

    GLsizei ivec3_size = sizeof(glm::ivec3);

    glEnableVertexAttribArray(3); // pos 3! because the cube got its own lil pool of info already :)
    glVertexAttribIPointer(3, 3, GL_INT, ivec3_size, (void*) 0);
    glVertexAttribDivisor(3, 1); 

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void VoxelGrid::delete_instance_buffer() {
    if (instanceVBO != 0) {
        glDeleteBuffers(1, &instanceVBO);
    }
}

void VoxelGrid::create_voxel_texture() {
    if (!voxel_tex) glGenTextures(1, &voxel_tex);

    glBindTexture(GL_TEXTURE_3D, voxel_tex);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // void glTexImage3D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, void *data );
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, width, height, depth, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, voxels.data());
    glBindTexture(GL_TEXTURE_3D, 0);
}

void VoxelGrid::update_voxel_data() {
    glBindTexture(GL_TEXTURE_3D, voxel_tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth,
                    GL_RED_INTEGER, GL_UNSIGNED_BYTE, voxels.data());
    glBindTexture(GL_TEXTURE_3D, 0);
}

void VoxelGrid::re_init_instance_buffer() {
    delete_instance_buffer();
    init_instance_buffer();
}

VoxelGrid::~VoxelGrid() {
    if (instanceVBO != 0) {
        glDeleteBuffers(1, &instanceVBO);
    }
}

uint8_t VoxelGrid::get_voxel_value(int x, int y, int z) {
    int index = height * width * z + width * y + x; // myeeee, math :)
    if (index >= 0 && index < num_voxels) {
        return voxels[index];
    }
    return 0;
}

void VoxelGrid::set_voxel_value(int x, int y, int z, uint8_t value) {
    int index = height * width * z + width * y + x;
    if (index >= 0 && index < num_voxels) {
        voxels[index] = static_cast<uint8_t>(value); // making sure its a byte :o u can never be too sure, or?
    }
    changed = true;
}

void VoxelGrid::turn_on_corner_visualization() {
    set_voxel_value(0, 0, 0, 1); // setting the corners to true to visualize them! 
    set_voxel_value(width - 1, height - 1, depth - 1, 1);
    set_voxel_value(0, height - 1, 0, 1);
    set_voxel_value(0, 0, depth - 1, 1);
    set_voxel_value(width - 1, 0, 0, 1);
    set_voxel_value(width - 1, height - 1, 0, 1);
    set_voxel_value(width - 1, 0, depth - 1, 1);
    set_voxel_value(0, height - 1, depth - 1, 1);
}

glm::vec3 VoxelGrid::get_voxel_world_pos(int x, int y, int z) {
    glm::vec3 local_pos = glm::vec3(x, y, z) * cell_size;
    return position + local_pos; 
}

void VoxelGrid::add_cube(glm::ivec3 position, int width, int height, int depth, uint8_t value) {
    for (int w = 0; w < width; w++) {
        for (int h = 0; h < height; h++) {
           for (int d = 0; d < depth; d++) {
               set_voxel_value(position.x + w, position.y + h, position.z + d, value);
           }
        }
    }
}