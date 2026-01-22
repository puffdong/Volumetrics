#include "Space.hpp"
#include <iostream>
#include <string>
#include "core/rendering/Renderer.hpp"
#include "core/space/Object.hpp"
#include "core/ui/ui_dumptruck.hpp"

#include "core/utils/ModelGenerator.hpp"

Space::Space(ResourceManager& resources, Renderer& renderer)
 : resources(resources), renderer(renderer)
{	
	
}

void Space::init_space() {
	camera = new Camera(glm::vec3(0.0, 10.0f, 0.0));

	init_skybox();
	init_raymarcher_and_voxelgrid();
	init_glass();
	init_lights();
	init_lines();

	add_base_entity(std::make_unique<Object>(glm::vec3(-10.f, 0.f, 10.f), glm::vec3(0.f), glm::vec3(1.f), "res://shaders/core/default_shader.vs", "res://models/teapot.obj", ""));

	// THIS IS STINKY; EWWW
	auto base_ground = std::make_unique<Object>(glm::vec3(-10.f, 0.f, 10.f), glm::vec3(0.f), glm::vec3(1.f), "res://shaders/core/default_shader.vs");
	base_ground->init(resources, this);
	ModelGpuData ground_model_2 = ModelGenerator::create_flat_ground(350, 350, 50, 50);
	Res::Model r_ground_model = resources.upload_model(std::move(ground_model_2));
	base_ground->set_model(std::move(r_ground_model));
	base_objects.push_back(std::move(base_ground));
	// THIS IS STINKY; BLEEEH

	std::string sphere_path = "res://models/sphere.obj";
	create_object(glm::vec3(7.0f, -1.0f, -8.0f), glm::vec3(0.0f), glm::vec3(5.0f), sphere_path);
	create_object(glm::vec3(-18.0f, 5.0f, 28.0f), glm::vec3(0.0f), glm::vec3(3.5f), sphere_path);
	create_object(glm::vec3(35.0f, 2.0f, 14.0f), glm::vec3(0.0f), glm::vec3(2.0f), sphere_path);
	create_object(glm::vec3(17.0f, 3.5f, -2.0f), glm::vec3(0.0f), glm::vec3(7.0f), sphere_path);


}

void Space::init_skybox() {
	skybox = Skybox();
	skybox.init(resources);
	sun = Sun();
	sun.set_direction(glm::vec3(1.0f, 1.0f, 1.0f));
	sun.set_color(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	sun.init(resources);
}

void Space::init_raymarcher_and_voxelgrid() {
	voxel_grid = VoxelGrid(30, 30, 30, 0, 1.5f, glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.f)); 
    voxel_grid.init(resources);
	voxel_grid.set_debug_visibility(false);
	raymarcher = Raymarcher();
	raymarcher.init(resources);
}

void Space::init_glass() {
	glass.init(resources);
	glass.set_visibility(true);
}

void Space::init_lights() {
	lights.reserve(3); light_spheres.reserve(3);
	add_light(glm::vec3(0.0f, 10.0f, 0.0), 200.f, glm::vec3(1.0f, 0.3f, 0.2f), 25.0f, glm::vec3(0.0f, -1.0f, 0.0f), 1.0f, LightType::Point);
	add_light(glm::vec3(0.0f, 10.0f, 0.0), 200.f, glm::vec3(0.0f, 0.58f, 1.0f), 25.0f, glm::vec3(0.0f, -1.0f, 0.0f), 1.0f, LightType::Point);
	add_light(glm::vec3(0.0f, 10.0f, 0.0), 200.f, glm::vec3(0.31f, 0.78f, 0.48f), 25.0f, glm::vec3(0.0f, -1.0f, 0.0f), 1.0f, LightType::Point);	
}

void Space::init_lines() {
	// world grid lines (currently parallell to all axis at 0)
	line_manager = Line();
	line_manager.init(resources.get_full_path("res://shaders/line.vs"), resources.get_full_path("res://shaders/line.fs"));
	std::vector<LinePrimitive> world_grid_lines = {{glm::vec3(256.f, 0.0f, 0.0f), glm::vec3(-256.0f, 0.0f, 0.0f), glm::vec4(0.86f, 0.08f, 0.24f, 1.0f)}, // X : R (crimson red)
	{glm::vec3(0.f, 256.0f, 0.0f), glm::vec3(0.0f, -256.0f, 0.0f), glm::vec4(0.196f, 0.754f, 0.196f, 1.0f)}, // Y : G (forest green)
	{glm::vec3(0.f, 0.0f, 256.0f), glm::vec3(0.0f, 0.0f, -256.0f), glm::vec4(0.118f, 0.565f, 1.0f, 1.0f)}};  // Z : B (dodger blue)
	line_manager.add_lines(world_grid_lines);
}

void Space::tick(float delta, ButtonMap bm)
{
	time += delta;
	this_frames_button_map = bm;

	camera->tick(delta, bm);
	sun.tick(delta);

	process_lights();

	for (auto& b : base_objects) {
		b->tick(delta);
	}
	voxel_grid.tick(delta);
	raymarcher.tick(delta);
    ui::raymarcher_panel(raymarcher, raymarcher.get_raymarch_settings(), voxel_grid);
	glass.tick(delta, bm);

}

void Space::process_lights() {
	// not pretty but we aint all perfect alright?
	const float speed = 0.12f;

	glm::vec3 light_pos1 = glm::vec3(10.0f * sin(time * speed), 10.f, 10.0f * cos(time * speed));
	glm::vec3 light_pos2 = glm::vec3(10.0f * sin(time * speed + 3 * PI/2), 10.f, 10.0f * cos(time * speed + 3 * PI/2));
	glm::vec3 light_pos3 = glm::vec3(10.0f * sin(time * speed + PI), 10.f, 10.0f * cos(time * speed + PI));
	
	light_spheres[0]->set_position(light_pos1);
	light_spheres[1]->set_position(light_pos2);
	light_spheres[2]->set_position(light_pos3);
	lights[0].position = light_pos1;
	lights[1].position = light_pos2;
	lights[2].position = light_pos3;
}

void Space::enqueue_renderables() {
	glm::mat4 view_matrix = camera->get_view_matrix();
	glm::vec3 camera_pos = camera->get_position();
	renderer.set_view(view_matrix); // renderer should have all the knowledge! maybe a better way to do this?!
	
	// lights
	renderer.submit_lighting_data(lights);
	for (auto& light_sphere : light_spheres) { // the light objects (hard to see em otherwise)
		light_sphere->enqueue(renderer, resources);
	}

	skybox.enqueue(renderer, resources, camera_pos);
	sun.enqueue(renderer, resources, camera_pos);

	for (auto& b : base_objects) {
		b->enqueue(renderer, resources);
	}
	
	voxel_grid.enqueue(renderer, resources);
	raymarcher.enqueue(renderer, resources, camera->get_position(), sun.get_direction(), sun.get_color(), voxel_grid.get_voxel_texture_id(), voxel_grid.get_grid_dim(), voxel_grid.get_position(), voxel_grid.get_cell_size());
	glass.enqueue(renderer, resources, this_frames_button_map);

	line_manager.enqueue(renderer);

	renderer.execute_pipeline(voxel_grid.is_debug_view_visible());
}

void Space::create_object(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, const std::string& model_asset) {
	auto new_object = std::make_unique<Object>(position, rotation, scale, "res://shaders/core/default_shader.vs", model_asset);
	new_object->init(resources, this);
	base_objects.push_back(std::move(new_object));
}

void Space::add_base_entity(std::unique_ptr<Base> base) {
	base->init(resources, this); 
	base_objects.push_back(std::move(base));
}

void Space::add_light(glm::vec3 position, float radius, glm::vec3 color, float intensity, 
					  glm::vec3 direction, float volumetric_intensity, LightType type) {
	Light new_light{position, radius, color, intensity, direction, volumetric_intensity, type};
	lights.push_back(std::move(new_light));
	
																	// invert the scale to invert the normals x) now its lit up! :)
	Object* new_light_object = new Object(position, glm::vec3(0.0f), glm::vec3(-0.4f), "res://shaders/core/default_shader.vs", "res://models/sphere.obj");
	new_light_object->init(resources, this);
	light_spheres.push_back(new_light_object);
}



void Space::cast_ray() {
	glm::vec3 view_dir = camera->get_front();
	
	glm::vec3 start = camera->get_position();
	glm::vec3 end = start + view_dir * 25.0f;

	line_manager.add_line(start, end, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
}

