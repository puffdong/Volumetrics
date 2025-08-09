#pragma once
#include "glm/glm.hpp"
#include <GL/glew.h>
#include "glm/gtc/matrix_transform.hpp"
#include <vector>
#include "../../OBJLoader.h"
#include "../../core/rendering/Shader.h" 

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
	ModelObject* voxelCube;
	GLuint instanceVBO;

public:
	VoxelStructure(int h, int w, int d, glm::vec3 pos, int initValue, float cellSize);
	~VoxelStructure();

	void setVoxelValue(int x, int y, int z, int value);
	int getVoxelValue(int x, int y, int z);

	glm::vec3 getVoxelToWorldSpace(int x, int y, int z);
	void drawVoxels(glm::mat4 projMatrix, glm::mat4 viewMatrix);
	glm::mat4 getModelMatrix(int x, int y, int z);
};