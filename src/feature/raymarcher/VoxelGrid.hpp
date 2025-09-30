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
	
public:
	VoxelGrid(int h, int w, int d, uint8_t init_value = 0, float cell_size = 1,
			  glm::vec3 pos = glm::vec3(0.f),
			  glm::vec3 rot = glm::vec3(0.f),
			  glm::vec3 scale = glm::vec3(1.f), 
			  Base* parent = nullptr);
	~VoxelGrid();

	void init(ResourceManager& resources, Space* space) override;
    void tick(float delta) override;
    void enqueue(Renderer& renderer, ResourceManager& resources) override;
	
	void resize_grid(int h, int w, int d);
	
	uint8_t get_voxel_value(int x, int y, int z);
	void set_voxel_value(int x, int y, int z, uint8_t value);

	
	glm::mat4 get_model_matrix(int x, int y, int z);
	glm::vec3 get_voxel_world_pos(int x, int y, int z);
	

private:
	GLuint instanceVBO;

	void init_instance_buffer();
	void delete_instance_buffer();
};