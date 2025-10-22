#pragma once
#include <vector>
#include <string>
#include <memory>

#include "glm/glm.hpp"

#include "core/utils/ButtonMap.hpp"
#include "core/resources/ResourceManager.hpp"
#include "core/rendering/Renderer.hpp"

#include "core/UUID.hpp"

#include "core/Camera.hpp"
#include "feature/Sun.hpp"

class Raymarcher; // fwd decl

class Space {
private:
	ResourceManager& resources;
	
	std::vector<std::unique_ptr<Base>> uninitialized_objects;
	std::vector<std::unique_ptr<Base>> objects;
	
	Camera* camera;
	Sun* sun;
	
	float time = 0.0;
	ButtonMap this_frames_button_map;

	Raymarcher* raymarcher;


public:
	Space(ResourceManager& resources);

	void tick(float delta, ButtonMap bm);
	void enqueue_renderables(Renderer& renderer);
	
	Camera* get_camera() const { return camera; };
	Sun* get_sun() const { return sun; };
	const ButtonMap& get_button_map() const { return this_frames_button_map; };
	
private:
	void init_space();
	void process_init_queue();
	
};