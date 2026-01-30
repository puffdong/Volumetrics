#pragma once
#include <vector>
#include <string>
#include <memory>

#include "glm/glm.hpp"

#include "core/utils/ButtonMap.hpp"
#include "core/resources/ResourceManager.hpp"

#include "core/space/Light.hpp"
#include "core/UUID.hpp"

#include "core/space/Object.hpp"
#include "core/Camera.hpp"
#include "feature/Sun.hpp"
#include "feature/Skybox.hpp"
#include "feature/raymarcher/VoxelGrid.hpp"
#include "feature/raymarcher/raymarcher.hpp"
#include "feature/glass/Glass.hpp"
#include "core/Line.hpp"

class Space {
private:
	ResourceManager& resources;
	Renderer& renderer;

	std::vector<std::unique_ptr<Object>> objects;
	
	Camera camera;
	Skybox skybox;
	Sun sun;
	VoxelGrid voxel_grid;
	Raymarcher raymarcher;
	Glass glass;
	Line line_manager;
	
	float time = 0.0;
	ButtonMap this_frames_button_map;

	std::vector<Light> lights;
	std::vector<Object*> light_spheres;


public:
	Space(ResourceManager& resources, Renderer& renderer);
	void init_space();

	void tick(float delta, ButtonMap bm);
	void enqueue_renderables();
	
	Camera& get_camera() { return camera; };
	Sun& get_sun() { return sun; };
	const ButtonMap& get_button_map() const { return this_frames_button_map; };

	void create_object(glm::vec3 position = glm::vec3(0.0f), 
							 glm::vec3 rotation = glm::vec3(0.0f), 
							 glm::vec3 scale = glm::vec3(1.0f), 
							 const std::string& model_asset = "",
							 const std::string& name = "");
	void add_light(glm::vec3 position, float radius, glm::vec3 color, float intensity, 
					  glm::vec3 direction, float volumetric_intensity, LightType type);
	void remove_light(std::size_t index);

	void add_object(std::unique_ptr<Object> object);
	void cast_ray();

private:
	void init_skybox();
	void init_raymarcher_and_voxelgrid();
	void init_glass();
	void init_lights();
	void init_lines();
};