#pragma once

#include "../OBJLoader.h"
#include "../core/rendering/Texture.h"

#include "glm/glm.hpp"
#include <vector>

#include "Camera.h"
#include "WorldObject.h"
#include "Line.hpp"

#include "../feature/Sun.hpp"
#include "../feature/Skybox.h"
#include "../feature/volumetrics/VoxelStructure.h"
#include "../feature/raymarcher/raymarcher.hpp"
#include "../feature/raymarcher/rayscene.hpp"
#include "../feature/water/WaterSurface.hpp"

#include "../utils/ButtonMap.h"
#include "../utils/LightSource.h"

class Space {
private:
	std::vector<WorldObject*> wObjects;

	Sun* sun;
	Skybox* skybox;

	float fov = 70.f;
	float near = 1.0f; // hmm... what is too close/too far? 
	float far = 256.0f;
	float aspect_ratio = 16.f / 9.0f;
	glm::mat4 proj;

	Camera* camera;

	float time = 0.0;

	WaterSurface* water_surface;
	// Sun* sun;

	VoxelStructure* vox;
	Raymarcher* raymarcher;
	RayScene* ray_scene;

	RaySphere* sphere1;
	RaySphere* sphere2;

	Line* line;

public:
	Space();

	void tick(float delta, ButtonMap bm);

	void renderWorld(float delta);
	void enqueue_renderables();
	Camera* get_camera();
	void change_fov(double xoffset, double yoffset); 
	void update_projection_matrix_aspect_ratio(float aspectRatio);
};