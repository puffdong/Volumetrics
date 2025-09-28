#pragma once
#include <vector>
#include <string>
#include <memory>

#include "glm/glm.hpp"

#include "core/resources/ResourceManager.hpp"
#include "core/rendering/Renderer.hpp"

#include "core/Base.hpp"

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
#include "core/UUID.hpp"

class Space {
private:
	ResourceManager& resources;

	std::vector<WorldObject*> wObjects;
	
	float time = 0.0;
	
	std::vector<std::unique_ptr<Base>> uninitialized_objects;
	std::vector<std::unique_ptr<Base>> objects;
	
	Camera* camera;

	Sun* sun;
	Skybox* skybox;
	WaterSurface* water_surface;

	VoxelStructure* vox;
	Raymarcher* raymarcher;
	RayScene* ray_scene;
	RaySphere* sphere1;
	RaySphere* sphere2;
	// Line* line;

public:
	Space(ResourceManager& resources);

	void tick(float delta, ButtonMap bm);
	void enqueue_renderables(Renderer& renderer);

	void process_init_queue();
	Camera* get_camera();

private: 
	void init_space();
	
};