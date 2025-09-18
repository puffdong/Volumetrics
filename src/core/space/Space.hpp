#pragma once

#include "core/OBJLoader.hpp"
#include "core/rendering/Texture.hpp"

#include "glm/glm.hpp"
#include <vector>

#include "core/resources/ResourceManager.hpp"

#include "core/Camera.hpp"
#include "core/WorldObject.hpp"
#include "core/Line.hpp"

#include "feature/Sun.hpp"
#include "feature/Skybox.hpp"
#include "feature/volumetrics/VoxelStructure.hpp"
#include "feature/raymarcher/raymarcher.hpp"
#include "feature/raymarcher/rayscene.hpp"
#include "feature/water/WaterSurface.hpp"

#include "core/utils/ButtonMap.hpp"

class Space {
private:
	ResourceManager& resources;

	std::vector<WorldObject*> wObjects;

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
	Space(ResourceManager& resources);

	void tick(float delta, ButtonMap bm);
	void enqueue_renderables(Renderer& renderer);
	Camera* get_camera();

private: 
	void init_space();
	
};