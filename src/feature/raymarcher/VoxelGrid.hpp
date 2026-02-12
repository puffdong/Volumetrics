#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "core/resources/ResourceManager.hpp"
#include "core/rendering/Renderer.hpp"

class VoxelGrid {
private:
	glm::vec3 position;
	glm::vec3 scale;

	int height;
	int width;
	int depth;

	int num_voxels;
	int num_occupied_voxels;
	float cell_size;
	std::vector<uint8_t> voxels;

	Resource r_shader;
	Shader* shader;
	ModelGpuData cube_model;

	unsigned int voxel_tex = 0;
	bool _voxels_changed = false;
	bool _instances_dirty = false;

	bool _debug_visible = false;
	bool _show_bounds = false;

	// for the lil selection box thingy!
	bool render_selection_box = false;
	glm::vec3 selection_pos;
	Shader* selection_box_shader;
	
public:
	VoxelGrid() = default;
	VoxelGrid(int w, int h, int d, uint8_t init_value, float cell_size, glm::vec3 pos, glm::vec3 scale);
	~VoxelGrid();

	void init(ResourceManager& resources);
    void tick(float delta, glm::vec3 selection_ray_start, glm::vec3 selection_ray_dir, bool mouse_pointer_active, bool mouse_clicked);
    void enqueue(Renderer& renderer, ResourceManager& resources, glm::vec3 camera_pos);
	
	void resize_grid(int w, int h, int d, bool preserve_data = false);
	
	void set_voxel_value(int x, int y, int z, uint8_t value);
	void set_cell_size(float size) { cell_size = size; };
	void set_position(const glm::vec3& p) { position = p; };
	void set_debug_visibility(const bool v) { _debug_visible = v; };
	void set_bounds_visualization_enabled(const bool v);
	
	inline bool is_debug_view_visible() const { return _debug_visible; };
	inline bool is_bounds_visualization_enabled() const { return _show_bounds; }


	uint8_t get_voxel_value(int x, int y, int z);
    glm::vec3 get_scale() const { return scale; };
    void set_scale(const glm::vec3& s) { scale = s; };
	glm::vec3 get_position() const { return position; };

	void update_voxel_data();
	void bind_voxel_texture(GLint unit);
	unsigned int get_voxel_texture_id() const { return voxel_tex; };

	glm::ivec3 get_grid_dim() const { return glm::ivec3(width, height, depth); }; // width, height, depth
	float get_cell_size() const { return cell_size; };

	glm::vec3 get_voxel_world_pos(int x, int y, int z); // origin is at (0, 0, 0)
	
	void add_cube(glm::ivec3 position, int width, int height, int depth, uint8_t value);
	void flood_fill(glm::ivec3 start_pos, uint8_t start_value);
	
	
private:
	GLuint instanceVBO = 0;

	void enqueue_bounds_visualization(Renderer& renderer);
	void init_instance_buffer();
	void re_init_instance_buffer();
	void create_voxel_texture();
	void delete_instance_buffer();
	void preserve_voxel_data(int old_w, int old_h, int old_d, const std::vector<uint8_t>& old_voxels);
};