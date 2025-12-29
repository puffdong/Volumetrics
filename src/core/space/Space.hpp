#pragma once
#include <vector>
#include <string>
#include <memory>

#include "glm/glm.hpp"

#include "core/utils/ButtonMap.hpp"
#include "core/resources/ResourceManager.hpp"

#include "core/UUID.hpp"

#include "core/Camera.hpp"
#include "feature/Sun.hpp"

#include "core/space/Light.hpp"

class Raymarcher; // fwd decl
class Object;

class Space {
private:
	ResourceManager& resources;
	Renderer& renderer;
	
	std::vector<std::unique_ptr<Base>> base_objects;
	
	// various doohickeys
	Camera* camera;
	Sun* sun;
	Raymarcher* raymarcher;
	
	float time = 0.0;
	ButtonMap this_frames_button_map;

	// placeholder for create_object feature
	UUID<Base> id_1;
	UUID<Base> id_2;
	UUID<Base> id_3;
	UUID<Base> id_4;

	Light light1;
	Light light2;
	Light light3;
	Object* light_sphere1; 
	Object* light_sphere2; 
	Object* light_sphere3; 


public:
	Space(ResourceManager& resources, Renderer& renderer);
	
	void tick(float delta, ButtonMap bm);
	void enqueue_renderables();
	
	Camera* get_camera() const { return camera; };
	Sun* get_sun() const { return sun; };
	const ButtonMap& get_button_map() const { return this_frames_button_map; };
	float get_time() const { return time; };

	
	UUID<Base> create_object(glm::vec3 position = glm::vec3(0.0f), 
							 glm::vec3 rotation = glm::vec3(0.0f), 
							 glm::vec3 scale = glm::vec3(1.0f), 
							 const std::string& model_asset = "");



	UUID<Base> add_base_entity(std::unique_ptr<Base> base);
	Base* get_base_entity(const UUID<Base>& id);
	
	void cast_ray();

private:
	void init_space();
	
};