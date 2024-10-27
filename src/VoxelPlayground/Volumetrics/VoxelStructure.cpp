#include "VoxelStructure.h"

#include "../../Renderer.h"

VoxelStructure::VoxelStructure(int h, int w, int d, glm::vec3 pos, int initValue, float cellSize)
	: height(h), width(w), depth(d), position(pos), cellSize(cellSize)
{
	shader = new Shader("C:/Dev/OpenGL/Volumetrics/res/shaders/VoxelShaders/VoxelDebug.shader");
	voxelCube = new ModelObject("C:/Dev/OpenGL/Volumetrics/res/models/VoxelModels/defaultCube.obj");

	voxels.resize(height * width * depth);
	length = height * width * depth;

	for (int i = 0; i < length; i++) {
		voxels[i] = initValue;
	}

	std::vector<glm::mat4> modelMatrices;
	for (int h = 0; h < height; h++) {
		for (int d = 0; d < depth; d++) {
			for (int w = 0; w < width; w++) {
				modelMatrices.push_back(getModelMatrix(h, d, w));
			}
		}
	}

}

void VoxelStructure::setVoxelValue(int x, int y, int z, int value) {
	int index = height * width * z + width * y + x;
	voxels[index] = value;
}

int VoxelStructure::getVoxelValue(int x, int y, int z) {
	int index = height * width * z + width * y + x;
	return voxels[index];
}

glm::vec3 VoxelStructure::getVoxelToWorldSpace(int x, int y, int z) {
	glm::vec3 voxelPos = glm::vec3(x, y, z) * cellSize - position;
	
	return voxelPos;
}

void VoxelStructure::drawVoxels(glm::mat4 projMatrix, glm::mat4 viewMatrix) {
	shader->Bind();
	shader->SetUniform4f("u_Color", 1.0f, 1.0f, 1.0f, 1.0f);

	shader->SetUniformMat4("proj", projMatrix);
	shader->SetUniformMat4("view", viewMatrix);
	shader->SetUniform1i("u_Texture", 0);
	shader->SetUniform3f("sunDir", glm::vec3(1.f, 0.5f, 0.f));
	shader->SetUniform3f("sunColor", glm::vec3(1.f, 1.f, 1.f));

	for (int h = 0; h < height; h++) {
		for (int d = 0; d < depth; d++) {
			for (int w = 0; w < width; w++) {
				shader->SetUniformMat4("model", getModelMatrix(h, d, w));
				voxelCube->render();
			}
		} 
	}
}

glm::mat4 VoxelStructure::getModelMatrix(int x, int y, int z) {
	return glm::scale(glm::translate(glm::mat4(1.f), getVoxelToWorldSpace(x, y, z)), glm::vec3(0.25, 0.25, 0.25));
}