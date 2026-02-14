#include "VoxelGrid.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

VoxelGrid::VoxelGrid(int w, int h, int d, uint8_t init_value, float cell_size, glm::vec3 pos, glm::vec3 scale)
    : _width(w), _height(h), _depth(d), _cell_size(cell_size), _num_occupied_voxels(0), _position(pos), _scale(scale)
{   
    _num_voxels = h * w * d;
    _voxels = std::vector<uint8_t>(_num_voxels, static_cast<uint8_t>(init_value));

    flood_fill(glm::ivec3(50, 50, 50), 15.0f, 255u);

    _voxels_changed = false;
    _instances_dirty = false;
}

void VoxelGrid::init(ResourceManager& resources) {
    _shader = new Shader(resources.get_full_path("res:://shaders/VoxelShaders/VoxelDebug.vs"), resources.get_full_path("res:://shaders/VoxelShaders/VoxelDebug.fs"));
    _selection_box_shader = new Shader(resources.get_full_path("res:://shaders/core/default_shader.vs"), resources.get_full_path("res:://shaders/core/default_shader.fs"));

    Res::Model r_model = resources.load_model("res://models/VoxelModels/defaultCube.obj");
    _cube_model = resources.get_model_gpu_data(r_model.id);
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

void VoxelGrid::enqueue(Renderer& renderer, glm::vec3 camera_pos, glm::vec3 sun_direction, glm::vec4 sun_color) {
    _shader->hot_reload_if_changed();
    _shader->bind();
    _shader->set_uniform_mat4("u_proj", renderer.get_proj());
    _shader->set_uniform_mat4("u_view", renderer.get_view());
    _shader->set_uniform_ivec3("u_grid_dim", glm::ivec3(_width, _height, _depth));
    _shader->set_uniform_vec3("u_grid_origin", _position);
    _shader->set_uniform_float("u_voxel_size", _cell_size);

    TextureBinding bind{ _voxel_tex, GL_TEXTURE_3D, 5, "u_voxels" };

    RenderCommand cmd{};
    cmd.vao        = _cube_model.vao;
    cmd.draw_type   = DrawType::ElementsInstanced;
    cmd.primitive = GL_TRIANGLES;
    cmd.count      = _cube_model.index_count;
    cmd.instance_count = _num_occupied_voxels;
    cmd.shader     = _shader;
    cmd.attach_lights = true;
    cmd.textures.push_back(bind);

    renderer.submit(RenderPass::RaymarchBounds, cmd);

    if (_render_selection_box) {
        _selection_box_shader->hot_reload_if_changed();
        _selection_box_shader->bind();
        
        glm::mat4 model = glm::translate(glm::mat4(1.f), _selection_pos) * glm::scale(glm::mat4(1.f), glm::vec3(_cell_size * 0.5f));
        glm::mat4 mvp = renderer.get_proj() * renderer.get_view() * model;
        
        _selection_box_shader->set_uniform_mat4("u_mvp", mvp);
        _selection_box_shader->set_uniform_mat4("u_model", model);
        _selection_box_shader->set_uniform_mat4("u_proj", renderer.get_proj());
        _selection_box_shader->set_uniform_mat4("u_view", renderer.get_view());
        _selection_box_shader->set_uniform_vec3("u_camera_pos", camera_pos);
        _selection_box_shader->set_uniform_vec3("u_sun_dir", sun_direction);
        _selection_box_shader->set_uniform_vec3("u_sun_color", sun_color);

        RenderCommand cmd{};
        cmd.vao = _cube_model.vao;
        cmd.draw_type = DrawType::Elements;
        cmd.count = _cube_model.index_count;
        cmd.shader = _selection_box_shader;
        cmd.attach_lights = true;

        renderer.submit(RenderPass::Forward, cmd);
    }
}

void VoxelGrid::selection_box_tick(float delta, glm::vec3 selection_ray_start, glm::vec3 selection_ray_dir, bool mouse_pointer_active, bool mouse_clicked) {
    if (!_selection_box_enabled || !mouse_pointer_active) {
        _render_selection_box = false;
        return;
    }

    _render_selection_box = true;
    _selection_pos = selection_ray_start + selection_ray_dir * 15.f;

    // figure out if we are within the bounds
    glm::vec3 local_pos = _selection_pos - _position;
    int grid_x = static_cast<int>(local_pos.x / _cell_size);
    int grid_y = static_cast<int>(local_pos.y / _cell_size);
    int grid_z = static_cast<int>(local_pos.z / _cell_size);

    if (grid_x >= 0 && grid_x < _width &&
        grid_y >= 0 && grid_y < _height &&
        grid_z >= 0 && grid_z < _depth) {
        _selection_pos = get_voxel_world_pos(grid_x, grid_y, grid_z); // snap it to voxel
        if (mouse_clicked) {
            set_voxel_value(grid_x, grid_y, grid_z, _selection_value);
        }
    } else {
        _render_selection_box = false;
    }
}

void VoxelGrid::init_instance_buffer() {
    _num_occupied_voxels = 0;
	_instance_grid_indexes.clear();
	_instance_grid_indexes.reserve(static_cast<size_t>(_num_voxels));
	_instance_index_map.assign(static_cast<size_t>(_num_voxels), -1);

    for (int w = 0; w < _width; w++) {
        for (int h = 0; h < _height; h++) {
            for (int d = 0; d < _depth; d++) {
                if (get_voxel_value(w, h, d) != 0u) {
                    int flat_index = get_index(w, h, d);
                    _instance_index_map[flat_index] = _num_occupied_voxels;
                    _instance_grid_indexes.push_back(glm::ivec3(w, h, d));
                    _num_occupied_voxels += 1;
                }
            }
        }
    }
    
    std::cout << "VoxelGrid [" << _width << "x" << _height << "x" << _depth << "] " 
              << _num_occupied_voxels << "/" << _num_voxels << " occupied" << std::endl;

    GLuint vao = _cube_model.vao;
    glBindVertexArray(vao);

    if (_instance_vbo == 0) {
        glGenBuffers(1, &_instance_vbo);
    }
    glBindBuffer(GL_ARRAY_BUFFER, _instance_vbo);
    size_t buffer_size = static_cast<size_t>(_num_voxels) * sizeof(glm::ivec3);
    glBufferData(GL_ARRAY_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);
    if (_num_occupied_voxels > 0) {
        glBufferSubData(GL_ARRAY_BUFFER,
                        0,
                        static_cast<GLsizeiptr>(_num_occupied_voxels) * sizeof(glm::ivec3),
                        _instance_grid_indexes.data());
    }

    GLsizei ivec3_size = sizeof(glm::ivec3);
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 3, GL_INT, ivec3_size, (void*)0);
    glVertexAttribDivisor(3, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
	_instance_buffer_ready = true;
}

void VoxelGrid::delete_instance_buffer() {
    if (_instance_vbo != 0) {
        glDeleteBuffers(1, &_instance_vbo);
    }
	_instance_buffer_ready = false;
	_instance_grid_indexes.clear();
	_instance_index_map.clear();
}

void VoxelGrid::create_voxel_texture() {
    if (!_voxel_tex) glGenTextures(1, &_voxel_tex);

    glBindTexture(GL_TEXTURE_3D, _voxel_tex);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, _width, _height, _depth, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, _voxels.data());
    glBindTexture(GL_TEXTURE_3D, 0);
}

void VoxelGrid::update_voxel_data() {
    glBindTexture(GL_TEXTURE_3D, _voxel_tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, _width, _height, _depth,
                    GL_RED_INTEGER, GL_UNSIGNED_BYTE, _voxels.data());
    glBindTexture(GL_TEXTURE_3D, 0);
}

void VoxelGrid::re_init_instance_buffer() {
    init_instance_buffer();
}

VoxelGrid::~VoxelGrid() {
    if (_instance_vbo != 0) {
        glDeleteBuffers(1, &_instance_vbo);
    }
    if (_voxel_tex != 0) {
        glDeleteTextures(1, &_voxel_tex);
    }
    if (_shader) delete _shader;
    if (_selection_box_shader) delete _selection_box_shader;
}

int VoxelGrid::get_index(int x, int y, int z) const {
    return _height * _width * z + _width * y + x;
}

void VoxelGrid::add_instance(int x, int y, int z, int flat_index) {
    if (!_instance_buffer_ready || _instance_vbo == 0) {
        return;
    }
    if (_num_occupied_voxels >= _num_voxels) {
        return;
    }

    glm::ivec3 grid_pos(x, y, z);
    int instance_index = _num_occupied_voxels;
    _instance_grid_indexes.push_back(grid_pos);
    _instance_index_map[flat_index] = instance_index;
    _num_occupied_voxels += 1;

    glBindBuffer(GL_ARRAY_BUFFER, _instance_vbo);
    glBufferSubData(GL_ARRAY_BUFFER,
                    static_cast<GLintptr>(instance_index) * sizeof(glm::ivec3),
                    sizeof(glm::ivec3),
                    &grid_pos);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VoxelGrid::remove_instance(int flat_index) {
    if (!_instance_buffer_ready || _instance_vbo == 0) {
        return;
    }
    int instance_index = _instance_index_map[flat_index];
    if (instance_index < 0) {
        return;
    }

    int last_index = _num_occupied_voxels - 1;
    if (instance_index != last_index) {
        const glm::ivec3& last_pos = _instance_grid_indexes[last_index];
        _instance_grid_indexes[instance_index] = last_pos;
        int last_flat = get_index(last_pos.x, last_pos.y, last_pos.z);
        _instance_index_map[last_flat] = instance_index;

        glBindBuffer(GL_ARRAY_BUFFER, _instance_vbo);
        glBufferSubData(GL_ARRAY_BUFFER,
                        static_cast<GLintptr>(instance_index) * sizeof(glm::ivec3),
                        sizeof(glm::ivec3),
                        &last_pos);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    _instance_grid_indexes.pop_back();
    _instance_index_map[flat_index] = -1;
    _num_occupied_voxels -= 1;
}

void VoxelGrid::update_instance_for_voxel_change(int x, int y, int z, int flat_index, uint8_t old_value, uint8_t new_value) {
    bool was_occupied = old_value != 0u;
    bool is_occupied = new_value != 0u;
    if (was_occupied == is_occupied) {
        return;
    }
    if (!_instance_buffer_ready) {
        return;
    }

    if (is_occupied) {
        add_instance(x, y, z, flat_index);
    } else {
        remove_instance(flat_index);
    }
}

uint8_t VoxelGrid::get_voxel_value(int x, int y, int z) {
    if (x < 0 || x >= _width || y < 0 || y >= _height || z < 0 || z >= _depth) {
        return 0;
    }
    return _voxels[get_index(x, y, z)];
}

void VoxelGrid::set_voxel_value(int x, int y, int z, uint8_t value) {
    if (x < 0 || x >= _width || y < 0 || y >= _height || z < 0 || z >= _depth) {
        return;
    }
    int flat_index = get_index(x, y, z);
    uint8_t old_value = _voxels[flat_index];
    if (old_value == value) {
        return;
    }

    _voxels[flat_index] = value;
    _voxels_changed = true;
	update_instance_for_voxel_change(x, y, z, flat_index, old_value, value);
}

void VoxelGrid::bind_voxel_texture(GLint unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_3D, _voxel_tex);
}

glm::vec3 VoxelGrid::get_voxel_world_pos(int x, int y, int z) {
    if (x < 0 || x >= _width || y < 0 || y >= _height || z < 0 || z >= _depth) {
        std::cerr << "VoxelGrid::get_voxel_world_pos: out of bounds (" << x << ", " << y << ", " << z << ")" << std::endl;
        return _position;
    }
    glm::vec3 local_pos = glm::vec3(x, y, z) * _cell_size;
    return _position + local_pos; 
}

void VoxelGrid::resize_grid(int w, int h, int d, bool preserve_data) {
    if (w <= 0 || h <= 0 || d <= 0) return;

    const int old_w = _width;
    const int old_h = _height;
    const int old_d = _depth;
    const std::vector<uint8_t> old_voxels = _voxels;

    _width = w;
    _height = h;
    _depth = d;

    _num_voxels = _width * _height * _depth;
    _voxels.assign(static_cast<size_t>(_num_voxels), static_cast<uint8_t>(0));
    _num_occupied_voxels = 0;

    if (preserve_data) {
        preserve_voxel_data(old_w, old_h, old_d, old_voxels);
    }

    _instance_grid_indexes.clear();
    _instance_index_map.clear();
    _instance_buffer_ready = false;

    create_voxel_texture();
    _voxels_changed = true;
    _instances_dirty = true;
}

void VoxelGrid::preserve_voxel_data(int old_w, int old_h, int old_d, const std::vector<uint8_t>& old_voxels) {
    _num_occupied_voxels = 0;

    int min_w = std::min(old_w, _width);
    int min_h = std::min(old_h, _height);
    int min_d = std::min(old_d, _depth);

    for (int z = 0; z < min_d; z++) {
        for (int y = 0; y < min_h; y++) {
            for (int x = 0; x < min_w; x++) {
                int old_index = old_h * old_w * z + old_w * y + x;
                int new_index = get_index(x, y, z);
                _voxels[new_index] = old_voxels[old_index];
                if (old_voxels[old_index] != 0u) {
                    _num_occupied_voxels += 1;
                }
            }
        }
    }
}

void VoxelGrid::flood_fill(glm::ivec3 origin, float radius, uint8_t start_value) {
    if (radius < 0.0f) {
        return;
    }

    if (radius < 0.001f) {
        set_voxel_value(origin.x, origin.y, origin.z, start_value);
        return;
    }

    float radius_grid = radius / _cell_size;
    int radius_int = static_cast<int>(std::ceil(radius_grid));

    int min_x = std::max(0, origin.x - radius_int);
    int max_x = std::min(_width - 1, origin.x + radius_int);
    int min_y = std::max(0, origin.y - radius_int);
    int max_y = std::min(_height - 1, origin.y + radius_int);
    int min_z = std::max(0, origin.z - radius_int);
    int max_z = std::min(_depth - 1, origin.z + radius_int);

    if (min_x > max_x || min_y > max_y || min_z > max_z) {
        return;
    }

    float inv_radius = 1.0f / radius_grid;

    for (int z = min_z; z <= max_z; z++) {
        for (int y = min_y; y <= max_y; y++) {
            for (int x = min_x; x <= max_x; x++) {
                float dx = static_cast<float>(x - origin.x);
                float dy = static_cast<float>(y - origin.y);
                float dz = static_cast<float>(z - origin.z);
                float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
                if (dist > radius_grid) {
                    continue;
                }

                float t = 1.0f - (dist * inv_radius);
                t = glm::clamp(t, 0.0f, 1.0f);
                float value_f = t * static_cast<float>(start_value);
                uint8_t value = static_cast<uint8_t>(std::round(value_f));
                set_voxel_value(x, y, z, value);
            }
        }
    }
}