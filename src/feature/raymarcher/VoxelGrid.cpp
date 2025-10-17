#include "VoxelGrid.hpp"
#include <iostream>
#include "core/space/Space.hpp"

VoxelGrid::VoxelGrid(int w, int h, int d, 
                     uint8_t init_value, float cell_size,
			         glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, Base* parent)
    : width(w), height(h), depth(d), cell_size(cell_size), Base(pos, rot, scale, parent) 
{   
    num_voxels = h * w * d;
    voxels = std::vector<uint8_t>(num_voxels, static_cast<uint8_t>(init_value));
    
    // edges
    set_voxel_value(0, 0, 0, 1);
    set_voxel_value(w - 1, h - 1, d - 1, 1);
    set_voxel_value(0, h - 1, 0, 1);
    set_voxel_value(0, 0, d - 1, 1);
    set_voxel_value(w - 1, 0, 0, 1);
    set_voxel_value(w - 1, h - 1, 0, 1);
    set_voxel_value(w - 1, 0, d - 1, 1);
    set_voxel_value(0, h - 1, d - 1, 1);

    // lil cube
    // for (int c1 = 0; c1 < 5; c1++) {
    //     for (int c2 = 0; c2 < 5; c2++) {
    //         for (int c3 = 0; c3 < 5; c3++) {
    //             set_voxel_value(6 + c1, 6 + c2, 6 + c3, 1 + c1);
    //         }
    //     }
    // }
    add_cube(glm::ivec3(5, 5, 5), 10, 2, 10, 1);
    add_cube(glm::ivec3(6, 6, 6), 8, 2, 8, 1);
    add_cube(glm::ivec3(7, 7, 7), 6, 2, 6, 1);
    add_cube(glm::ivec3(8, 8, 8), 4, 6, 4, 1);
    // add_cube(glm::ivec3(9, 9, 9), 2, 4, 2, 1);
    // add_cube(glm::ivec3(0, 0, 0), 10, 2, 10, 1);
    // add_cube(glm::ivec3(0, 0, 0), 10, 2, 10, 1);
    // for (int c1 = 0; c1 < 12; c1++) {
    //     for (int c2 = 0; c2 < 2; c2++) {
    //        for (int c3 = 0; c3 < 12; c3++) {
    //            set_voxel_value(3 + c1, 4 + c2, 3 + c3, 5 + c1);
    //        }
    //     }
    // }
}

void VoxelGrid::init(ResourceManager& resources, Space* space) {
    Base::init(resources, space);
    r_shader = resources.load_shader("res://shaders/VoxelShaders/VoxelDebug.shader");
    cube = new ModelObject(resources.get_full_path("res://models/VoxelModels/defaultCube.obj"));

    init_instance_buffer();
    create_voxel_texture();
}

void VoxelGrid::tick(float delta) {

}

void VoxelGrid::enqueue(Renderer& renderer, ResourceManager& resources) {
    if (auto shader = resources.get_shader(r_shader.id)) {
        (*shader)->hot_reload_if_changed();
        (*shader)->bind();
        (*shader)->set_uniform_mat4("u_proj", renderer.get_proj());
        (*shader)->set_uniform_mat4("u_view", renderer.get_view());
        (*shader)->set_uniform_ivec3("u_grid_dim", glm::ivec3(width, height, depth));
        (*shader)->set_uniform_vec3("u_grid_origin", position);
        (*shader)->set_uniform_float("u_voxel_size", cell_size);

        GLuint vao = cube->getVAO();             
        unsigned int index_count = cube->getIndexCount(); 

        TextureBinding bind{ voxel_tex, GL_TEXTURE_3D, 0, "u_voxels" };

        RenderCommand cmd{};
        cmd.vao        = vao;
        cmd.draw_type   = DrawType::ElementsInstanced;
        cmd.primitive = GL_TRIANGLES;
        cmd.count      = index_count;
        cmd.instance_count = num_voxels;
        cmd.shader     = (*shader);
        cmd.state.depth_test = true;
        cmd.state.depth_write = true;
        cmd.textures.push_back(bind);

        renderer.submit(RenderPass::Forward, cmd);
    } else {
        std::cout << "something fricked up when displaying the voxelgrid" << std::endl;
    }
}

void VoxelGrid::init_instance_buffer() {
    std::vector<glm::mat4> instance_model_matrices;
    instance_model_matrices.reserve(num_voxels);

    for (int current_h = 0; current_h < height; current_h++) {
        for (int current_d = 0; current_d < depth; current_d++) {
            for (int current_w = 0; current_w < width; current_w++) {
                instance_model_matrices.push_back(get_model_matrix(current_h, current_d, current_w));
            }
        }
    }

    GLuint vao = cube->getVAO();
    glBindVertexArray(vao);

    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instance_model_matrices.size() * sizeof(glm::mat4), instance_model_matrices.data(), GL_STATIC_DRAW);

    GLsizei vec4Size = sizeof(glm::vec4);
    GLsizei mat4Size = sizeof(glm::mat4);

    glEnableVertexAttribArray(3); // First 
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, mat4Size, (void*)0);
    glVertexAttribDivisor(3, 1);

    glEnableVertexAttribArray(4); // Second
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, mat4Size, (void*)(1 * vec4Size));
    glVertexAttribDivisor(4, 1);

    glEnableVertexAttribArray(5); // Third
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, mat4Size, (void*)(2 * vec4Size));
    glVertexAttribDivisor(5, 1);

    glEnableVertexAttribArray(6); // Fourth
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, mat4Size, (void*)(3 * vec4Size));
    glVertexAttribDivisor(6, 1);

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



VoxelGrid::~VoxelGrid() {
    delete cube;
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
}

glm::vec3 VoxelGrid::get_voxel_world_pos(int x, int y, int z) {
    glm::vec3 local_pos = glm::vec3(x, y, z) * cell_size;
    return position + local_pos; 
}

glm::mat4 VoxelGrid::get_model_matrix(int x, int y, int z) {
    glm::vec3 voxel_world_pos = get_voxel_world_pos(x, y, z);
    glm::mat4 m = glm::mat4(1.0f);
    m = glm::translate(m, voxel_world_pos);
    m = glm::scale(m, glm::vec3(cell_size * 0.5f)); 
    return glm::scale(glm::translate(glm::mat4(1.f), voxel_world_pos), glm::vec3(this->cell_size * 0.5f));
}

void VoxelGrid::add_cube(glm::ivec3 position, int width, int height, int depth, uint8_t value) {
    for (int w = 0; w < width; w++) {
        for (int h = 0; h < height; h++) {
           for (int d = 0; d < depth; d++) {
               set_voxel_value(position.x + w, position.y + h, position.y + d, value);
           }
        }
    }
}