#pragma once
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "core/Base.hpp"
#include "core/resources/ResourceManager.hpp"
#include "core/OBJLoader.hpp"

class Space;

class VoxelGrid : public Base {
private:
	int height;
	int width;
	int depth;

	int num_voxels;
	float cell_size;
	std::vector<uint8_t> voxels;

	Resource r_shader;
	ModelObject* cube;

	GLuint voxel_tex = 0;
	
public:
	VoxelGrid(int w, int h, int d, uint8_t init_value = 0, float cell_size = 1,
			  glm::vec3 pos = glm::vec3(0.f),
			  glm::vec3 rot = glm::vec3(0.f),
			  glm::vec3 scale = glm::vec3(1.f), 
			  Base* parent = nullptr);
	~VoxelGrid();

	void init(ResourceManager& resources, Space* space) override;
    void tick(float delta) override;
    void enqueue(Renderer& renderer, ResourceManager& resources) override;
	
	// void resize_grid(int h, int w, int d);
	
	uint8_t get_voxel_value(int x, int y, int z);
	void set_voxel_value(int x, int y, int z, uint8_t value);
	void set_cell_size(float size) { cell_size = size; };

	void update_voxel_data();
	void bind_voxel_texture(GLint unit);
	GLuint get_voxel_texture_id() const { return voxel_tex; };
	glm::ivec3 get_grid_dim() const { return glm::ivec3(width, height, depth); }; // width, height, depth
	float get_cell_size() const { return cell_size; };

	glm::mat4 get_model_matrix(int x, int y, int z);
	glm::vec3 get_voxel_world_pos(int x, int y, int z);
	

private:
	GLuint instanceVBO;

	void init_instance_buffer();
	void create_voxel_texture();
	void delete_instance_buffer();
};