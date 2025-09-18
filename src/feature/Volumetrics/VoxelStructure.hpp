#pragma once
#include "glm/glm.hpp"
#include <GL/glew.h>
#include "glm/gtc/matrix_transform.hpp"
#include "core/rendering/Renderer.hpp"
#include <vector>
#include "core/OBJLoader.hpp"
#include "core/rendering/Shader.hpp" 

class VoxelStructure {
private:
	int height;
	int width;
	int depth;

	glm::vec3 position; 

	std::vector<int> voxels;
	int numVoxels;
	float cellSize;

	Shader* shader;
	ModelObject* cube;
	
	public:
	VoxelStructure(int h, int w, int d, glm::vec3 pos, int initValue, float cellSize);
	~VoxelStructure();
	
	void resize_grid(int h, int w, int d);

	void setVoxelValue(int x, int y, int z, int value);
	int getVoxelValue(int x, int y, int z);
	
	glm::vec3 getVoxelToWorldSpace(int x, int y, int z);
	void drawVoxels(Renderer& renderer, glm::mat4 viewMatrix);
	glm::mat4 getModelMatrix(int x, int y, int z);
	
private:
	GLuint instanceVBO;

	void init_instance_buffer(int h, int w, int d);
	void delete_instance_buffer();
};