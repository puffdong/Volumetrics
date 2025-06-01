#pragma once

#include "../OBJLoader.h"
#include "../rendering/Texture.h"

#include "glm/glm.hpp"
#include <vector>

#include "Camera.h"
#include "WorldObject.h"
#include "../feature/Skybox.h"

#include "../feature/Volumetrics/VoxelStructure.h"
#include "../feature/raymarcher/raymarcher.hpp"
#include "../feature/raymarcher/rayscene.hpp"

#include "../feature/water/WaterSurface.hpp"

#include "../utils/ButtonMap.h"
#include "../utils/LightSource.h"

#include <iostream>

class Space {
private:
	std::vector<WorldObject*> wObjects; 
	Skybox* skybox;

	float fov = 70.f;
	float near = 1.0f;
	float far =  256.0f;
	glm::mat4 proj = glm::perspective(glm::radians(70.f), 16.f / 9.0f, 1.0f, 256.0f);

	

	Camera* camera;

	float time = 0.0;

	WaterSurface* water_surface;
	// Sun* sun;

	VoxelStructure* vox;
	Raymarcher* raymarcher;
	RayScene* ray_scene;

	RaySphere* sphere1;
	RaySphere* sphere2;

public:
	Space();

	void tick(float delta, ButtonMap bm);

	void renderWorld(float delta);
	Camera* Space::get_camera() { return camera; }   // where `camera` is your member
	void change_fov(double xoffset, double yoffset);


private:
	void loadLevel1();
};