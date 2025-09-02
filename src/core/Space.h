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

class Space {
private:
	std::vector<WorldObject*> wObjects;

	float fov = 70.f;
	float near = 1.0f; // hmm... what is too close/too far? 
	float far = 256.0f;
	float aspect_ratio = 16.f / 9.0f;
	glm::mat4 proj;
	bool changes_made = true;

	Camera* camera;

	float time = 0.0;

	Sun* sun;
	Skybox* skybox;
	WaterSurface* water_surface;

	VoxelStructure* vox;
	Raymarcher* raymarcher;
	RayScene* ray_scene;
	RaySphere* sphere1;
	RaySphere* sphere2;
	Line* line;

public:
	Space();

	void tick(float delta, ButtonMap bm);
	void enqueue_renderables();
	Camera* get_camera();
	void change_fov(double xoffset, double yoffset); 
	void update_projection_matrix_aspect_ratio(float aspectRatio);

private: 
	void init_space();
	void update_projection_uniforms(); // ooooh
	
};