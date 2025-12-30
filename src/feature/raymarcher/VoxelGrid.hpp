#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "core/resources/ResourceManager.hpp"
#include "core/rendering/Renderer.hpp"

struct Resource;

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
	ModelGpuData cube_model;

	unsigned int voxel_tex = 0;

	bool changed = false;
	bool _visible = true;
	
public:
	VoxelGrid(int w, int h, int d, uint8_t init_value, float cell_size, glm::vec3 pos, glm::vec3 scale);
	~VoxelGrid();

	void init(ResourceManager& resources);
    void tick(float delta);
    void enqueue(Renderer& renderer, ResourceManager& resources);
	
	// void resize_grid(int w, int h, int d);
	
	void set_voxel_value(int x, int y, int z, uint8_t value);
	void set_cell_size(float size) { cell_size = size; changed = true; };
	void set_position(const glm::vec3& p) { position = p; changed = true; };
	void set_visibility(const bool v) { _visible = v; };
	
	bool is_visible() const { return _visible; };
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
	
	
private:
	GLuint instanceVBO;

	void turn_on_corner_visualization();
	void init_instance_buffer();
	void re_init_instance_buffer();
	void create_voxel_texture();
	void delete_instance_buffer();
};