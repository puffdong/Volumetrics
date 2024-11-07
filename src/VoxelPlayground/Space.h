#pragma once

#include "../OBJLoader.h"
#include "../Texture.h"

#include "glm/glm.hpp"
#include <vector>

#include "Camera.h"
#include "WorldObject.h"
#include "Skybox.h"

#include "Volumetrics/VoxelStructure.h"
#include "raymarcher/raymarcher.hpp"


#include "../Utils/ButtonMap.h"
#include "../Utils/LightSource.h"

class Space {
private:
	std::vector<WorldObject*> wObjects; 
	Skybox* skybox;

	glm::mat4 proj = glm::perspective(glm::radians(70.f), 16.f / 9.0f, 1.0f, 256.0f);
	Camera* camera;

	VoxelStructure* vox;
	Raymarcher* raymarcher;

public:
	Space();

	void tick(float delta, ButtonMap bm);

	void renderWorld(float delta);

private:
	void loadLevel1();
};