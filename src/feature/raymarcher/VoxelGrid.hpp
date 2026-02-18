#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "core/resources/ResourceManager.hpp"
#include "core/rendering/Renderer.hpp"

class VoxelGrid {
private:
	glm::vec3 _position;
	glm::vec3 _scale;

	int _height;
	int _width;
	int _depth;

	int _num_voxels;
	int _num_occupied_voxels;
	float _cell_size;
	std::vector<uint8_t> _voxels;

	Shader* _shader = nullptr;
	Shader* _selection_box_shader = nullptr;
	ModelGpuData _cube_model;

	unsigned int _voxel_tex = 0;
	bool _voxels_changed = false;
	bool _instances_dirty = false;

	bool _debug_visible = false;

	// for the lil selection box thingy!
	bool _selection_box_enabled = false;
	bool _render_selection_box = false;
	glm::vec3 _selection_pos;
	uint8_t _selection_value = 255u;
	
public:
	VoxelGrid() = default;
	VoxelGrid(int w, int h, int d, uint8_t init_value, float cell_size, glm::vec3 pos);
	~VoxelGrid();

	void init(ResourceManager& resources);
    void tick(float delta, glm::vec3 selection_ray_start, glm::vec3 selection_ray_dir, bool mouse_pointer_active, bool mouse_clicked);
    void enqueue(Renderer& renderer, glm::vec3 camera_pos, glm::vec3 sun_direction, glm::vec4 sun_color);

	void selection_box_tick(float delta, glm::vec3 selection_ray_start, glm::vec3 selection_ray_dir, bool mouse_pointer_active, bool mouse_clicked);
	
	void resize_grid(int w, int h, int d, bool preserve_data = false);
	void set_voxel_value(int x, int y, int z, uint8_t value);
	void set_cell_size(float size) { _cell_size = size; };
	void set_position(const glm::vec3& p) { _position = p; };
	void set_debug_visibility(const bool v) { _debug_visible = v; };
	void set_selection_box_enabled(const bool v) { _selection_box_enabled = v; };
	
	inline bool is_debug_view_visible() const { return _debug_visible; };
	inline bool is_selection_box_enabled() const { return _selection_box_enabled; }
	uint8_t get_selection_value() const { return _selection_value; };
	void set_selection_value(uint8_t value) { _selection_value = value; };


	uint8_t get_voxel_value(int x, int y, int z);
	glm::vec3 get_position() const { return _position; };

	void update_voxel_data();
	void bind_voxel_texture(GLint unit);
	unsigned int get_voxel_texture_id() const { return _voxel_tex; };

	glm::ivec3 get_grid_dim() const { return glm::ivec3(_width, _height, _depth); }; // width, height, depth
	float get_cell_size() const { return _cell_size; };

	glm::vec3 get_voxel_world_pos(int x, int y, int z); // origin is at (0, 0, 0)
	
	void flood_fill(glm::ivec3 origin, float radius, uint8_t start_value);
	void flood_fill2(glm::ivec3 origin, float radius, uint8_t start_value, float threshold_ratio, float decay_power, float cutoff);
	
	
private:
	GLuint _instance_vbo = 0;
	bool _instance_buffer_ready = false;
	std::vector<glm::ivec3> _instance_grid_indexes;
	std::vector<int> _instance_index_map;

	int get_index(int x, int y, int z) const;
	void update_instance_for_voxel_change(int x, int y, int z, int flat_index, uint8_t old_value, uint8_t new_value);
	void add_instance(int x, int y, int z, int flat_index);
	void remove_instance(int flat_index);
	void init_instance_buffer();
	void re_init_instance_buffer();
	void create_voxel_texture();
	void delete_instance_buffer();
	void preserve_voxel_data(int old_w, int old_h, int old_d, const std::vector<uint8_t>& old_voxels);
};