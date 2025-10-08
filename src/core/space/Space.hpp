#pragma once
#include <vector>
#include <string>
#include <memory>

#include "glm/glm.hpp"

#include "core/resources/ResourceManager.hpp"
#include "core/rendering/Renderer.hpp"

#include "core/Base.hpp"
#include "core/space/Object.hpp"

#include "core/Camera.hpp"
#include "core/Line.hpp"

#include "feature/Sun.hpp"
#include "feature/Skybox.hpp"
#include "feature/raymarcher/raymarcher.hpp"
#include "feature/water/WaterSurface.hpp"

#include "core/utils/ButtonMap.hpp"
#include "core/UUID.hpp"

class Space {
private:
	ResourceManager& resources;
	
	std::vector<std::unique_ptr<Base>> uninitialized_objects;
	std::vector<std::unique_ptr<Base>> objects;
	
	Camera* camera;
	
	float time = 0.0;

	Sun* sun; // keeping track of this whilst still keeping it within the objects list would be cool? 

	Raymarcher* raymarcher;


public:
	Space(ResourceManager& resources);

	void tick(float delta, ButtonMap bm);
	void enqueue_renderables(Renderer& renderer);
	
	Camera* get_camera() const { return camera; };
	Sun* get_sun() const { return sun; }
	
private:
	void init_space();
	void process_init_queue();
	
};