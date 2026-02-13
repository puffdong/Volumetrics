#include "VoxelGrid.hpp"
#include <iostream>
#include <queue>

VoxelGrid::VoxelGrid(int w, int h, int d, uint8_t init_value, float cell_size, glm::vec3 pos, glm::vec3 scale)
    : width(w), height(h), depth(d), cell_size(cell_size), num_occupied_voxels(0), position(pos), scale(scale)
{   
    num_voxels = h * w * d;
    voxels = std::vector<uint8_t>(num_voxels, static_cast<uint8_t>(init_value));

    flood_fill(glm::ivec3(15, 15, 15), 8); // ensure the entire grid is filled with the init value, and num_occupied_voxels is correct
    // // chimney looking thing
    // add_cube(glm::ivec3(5, 5, 5), 10, 2, 10, 1);
    // add_cube(glm::ivec3(6, 6, 6), 8, 2, 8, 1);
    // add_cube(glm::ivec3(7, 7, 7), 6, 2, 6, 1);
    // add_cube(glm::ivec3(8, 8, 8), 4, 6, 4, 1);

    // // hollow tube along x axis
    // for (int x = 0; x < 30; x++) {
    //     for (int y = 3; y < 11; y++) {
    //         for (int z = 18; z < 26; z++) {
    //             if (y == 3 || y == 4 || y == 9 || y == 10|| z == 18 || z == 19 || z == 24 || z == 25) {
    //                 uint8_t value = x;
    //                 if (x > 15) value = 15;
    //                 set_voxel_value(x, y, z, value);
    //             }
    //         }
    //     }
    // }
    
    // Clear dirty flags after initial construction
    _voxels_changed = false;
    _instances_dirty = false;
}

void VoxelGrid::init(ResourceManager& resources) {
    shader = new Shader(resources.get_full_path("res:://shaders/VoxelShaders/VoxelDebug.vs"), resources.get_full_path("res:://shaders/VoxelShaders/VoxelDebug.fs"));
    selection_box_shader = new Shader(resources.get_full_path("res:://shaders/core/default_shader.vs"), resources.get_full_path("res:://shaders/core/default_shader.fs"));

    Res::Model r_model = resources.load_model("res://models/VoxelModels/defaultCube.obj");
    cube_model = resources.get_model_gpu_data(r_model.id);
    init_instance_buffer();
    create_voxel_texture();
}

void VoxelGrid::tick(float delta, glm::vec3 selection_ray_start, glm::vec3 selection_ray_dir, bool mouse_pointer_active, bool mouse_clicked) {
    if (_instances_dirty) {
        re_init_instance_buffer();
        _instances_dirty = false;
    }
    if (_voxels_changed) {
        update_voxel_data();
        _voxels_changed = false;
    }

    selection_box_tick(delta, selection_ray_start, selection_ray_dir, mouse_pointer_active, mouse_clicked);

}

void VoxelGrid::enqueue(Renderer& renderer, ResourceManager& resources, glm::vec3 camera_pos) {
    // if (!_visible) return; // this was moved to execute_pipeline!

    shader->hot_reload_if_changed();
    shader->bind();
    shader->set_uniform_mat4("u_proj", renderer.get_proj());
    shader->set_uniform_mat4("u_view", renderer.get_view());
    shader->set_uniform_ivec3("u_grid_dim", glm::ivec3(width, height, depth));
    shader->set_uniform_vec3("u_grid_origin", position);
    shader->set_uniform_float("u_voxel_size", cell_size);

    TextureBinding bind{ voxel_tex, GL_TEXTURE_3D, 5, "u_voxels" };

    RenderCommand cmd{};
    cmd.vao        = cube_model.vao;
    cmd.draw_type   = DrawType::ElementsInstanced;
    cmd.primitive = GL_TRIANGLES;
    cmd.count      = cube_model.index_count;
    cmd.instance_count = num_occupied_voxels;
    cmd.shader     = shader;
    cmd.attach_lights = true;
    cmd.textures.push_back(bind);

    renderer.submit(RenderPass::RaymarchBounds, cmd);

    if (render_selection_box) {
        // just render the cube at the selection pos!
        selection_box_shader->hot_reload_if_changed();
        selection_box_shader->bind();
        glm::mat4 mvp = renderer.get_proj() * renderer.get_view() * glm::translate(glm::mat4(1.f), selection_pos) * glm::scale(glm::mat4(1.f), glm::vec3(cell_size * 0.5f));
        selection_box_shader->set_uniform_mat4("u_mvp", mvp);
        selection_box_shader->set_uniform_mat4("u_model", glm::translate(glm::mat4(1.f), selection_pos) * glm::scale(glm::mat4(1.f), glm::vec3(cell_size * 0.5f)));
        selection_box_shader->set_uniform_mat4("u_proj", renderer.get_proj());
        selection_box_shader->set_uniform_mat4("u_view", renderer.get_view());
        selection_box_shader->set_uniform_vec3("u_camera_pos", camera_pos);
        selection_box_shader->set_uniform_vec3("u_sun_dir", glm::vec3(0.0, 1.0, 0.0)); // todo: get actual sun dir in shader
        selection_box_shader->set_uniform_vec3("u_sun_color", glm::vec3(1.0, 1.0, 1.0)); // todo: get actual sun color in shader

        RenderCommand cmd{};
        cmd.vao = cube_model.vao;
        cmd.draw_type = DrawType::Elements;
        cmd.count = cube_model.index_count;
        cmd.shader = selection_box_shader;
        cmd.attach_lights = true;

        renderer.submit(RenderPass::Forward, cmd);
    }

    if (_show_bounds) {
        enqueue_bounds_visualization(renderer); // todo
    }
}

void VoxelGrid::selection_box_tick(float delta, glm::vec3 selection_ray_start, glm::vec3 selection_ray_dir, bool mouse_pointer_active, bool mouse_clicked) {
    if (!selection_box_enabled) {
        render_selection_box = false;
        return;
    }

    if (mouse_pointer_active) {
        render_selection_box = true;
        selection_pos = selection_ray_start + selection_ray_dir * 15.f;

        // Convert world position to grid indices
        glm::vec3 local_pos = selection_pos - position;
        int grid_x = static_cast<int>(local_pos.x / cell_size);
        int grid_y = static_cast<int>(local_pos.y / cell_size);
        int grid_z = static_cast<int>(local_pos.z / cell_size);

        if (grid_x >= 0 && grid_x < width &&
            grid_y >= 0 && grid_y < height &&
            grid_z >= 0 && grid_z < depth) {
            selection_pos = get_voxel_world_pos(grid_x, grid_y, grid_z);
            if (mouse_clicked) {
                // uint8_t current_value = get_voxel_value(grid_x, grid_y, grid_z);
                // uint8_t new_value = (current_value == 0u) ? 15u : 0u; 
                set_voxel_value(grid_x, grid_y, grid_z, 15u);
            }
        } else {
            render_selection_box = false;
        }
    } else {
        render_selection_box = false;
    }
}

void VoxelGrid::init_instance_buffer() {
    num_occupied_voxels = 0;

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
    
    std::cout << "VoxelGrid [" << width << "x" << height << "x" << depth << "] " 
              << num_occupied_voxels << "/" << num_voxels << " occupied" << std::endl;

    GLuint vao = cube_model.vao;
    glBindVertexArray(vao);

    if (instanceVBO == 0) {
        glGenBuffers(1, &instanceVBO);
    }
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 instance_grid_indexes.size() * sizeof(glm::ivec3),
                 instance_grid_indexes.data(),
                 GL_DYNAMIC_DRAW);

    GLsizei ivec3_size = sizeof(glm::ivec3);
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 3, GL_INT, ivec3_size, (void*)0);
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
    // bounds check
    if (x < 0 || x >= width || y < 0 || y >= height || z < 0 || z >= depth) {
        return;
    }

    int index = height * width * z + width * y + x;
    if (index >= 0 && index < num_voxels) {
        voxels[index] = static_cast<uint8_t>(value);
    }
    _voxels_changed = true;
    _instances_dirty = true;
}

void VoxelGrid::set_bounds_visualization_enabled(const bool v) {
    _show_bounds = v;
}

void VoxelGrid::enqueue_bounds_visualization(Renderer& renderer) {
    // todo
}

glm::vec3 VoxelGrid::get_voxel_world_pos(int x, int y, int z) {
    // bounds check
    if (x < 0 || x >= width || y < 0 || y >= height || z < 0 || z >= depth) {
        return glm::vec3(0.0f);
    }

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

void VoxelGrid::resize_grid(int w, int h, int d, bool preserve_data) {
    if (w <= 0 || h <= 0 || d <= 0) return;

    const int old_w = width;
    const int old_h = height;
    const int old_d = depth;
    const std::vector<uint8_t> old_voxels = voxels;

    width = w;
    height = h;
    depth = d;

    num_voxels = width * height * depth;
    voxels.assign(static_cast<size_t>(num_voxels), static_cast<uint8_t>(0));
    num_occupied_voxels = 0;

    if (preserve_data) {
        preserve_voxel_data(old_w, old_h, old_d, old_voxels);
    }

    create_voxel_texture();
    _voxels_changed = true;
    _instances_dirty = true;
}

void VoxelGrid::preserve_voxel_data(int old_w, int old_h, int old_d, const std::vector<uint8_t>& old_voxels) {
    num_occupied_voxels = 0;

    int min_w = std::min(old_w, width);
    int min_h = std::min(old_h, height);
    int min_d = std::min(old_d, depth);

    for (int z = 0; z < min_d; z++) {
        for (int y = 0; y < min_h; y++) {
            for (int x = 0; x < min_w; x++) {
                int old_index = old_h * old_w * z + old_w * y + x;
                int new_index = height * width * z + width * y + x;
                voxels[new_index] = old_voxels[old_index];
                if (old_voxels[old_index] != 0u) {
                    num_occupied_voxels += 1;
                }
            }
        }
    }
}

void VoxelGrid::flood_fill(glm::ivec3 start_pos, uint8_t start_value) {
    if (start_pos.x < 0 || start_pos.x >= width ||
        start_pos.y < 0 || start_pos.y >= height ||
        start_pos.z < 0 || start_pos.z >= depth) {
        return;
    }

    if (start_value == 0u) {
        return;
    }

    uint8_t target_value = get_voxel_value(start_pos.x, start_pos.y, start_pos.z);
    if (target_value == start_value) {
        return;
    }

    struct FillNode {
        glm::ivec3 pos;
        uint8_t value;
    };

    std::queue<FillNode> to_fill;
    to_fill.push({ start_pos, start_value });

    while (!to_fill.empty()) {
        FillNode current = to_fill.front();
        to_fill.pop();

        const glm::ivec3& p = current.pos;
        if (p.x < 0 || p.x >= width ||
            p.y < 0 || p.y >= height ||
            p.z < 0 || p.z >= depth) {
            continue;
        }

        if (get_voxel_value(p.x, p.y, p.z) != target_value) {
            continue;
        }

        set_voxel_value(p.x, p.y, p.z, current.value);

        if (current.value <= 1u) {
            continue;
        }

        uint8_t next_value = static_cast<uint8_t>(current.value - 1u);
        to_fill.push({ p + glm::ivec3(1, 0, 0), next_value });
        to_fill.push({ p + glm::ivec3(-1, 0, 0), next_value });
        to_fill.push({ p + glm::ivec3(0, 1, 0), next_value });
        to_fill.push({ p + glm::ivec3(0, -1, 0), next_value });
        to_fill.push({ p + glm::ivec3(0, 0, 1), next_value });
        to_fill.push({ p + glm::ivec3(0, 0, -1), next_value });
    }
}