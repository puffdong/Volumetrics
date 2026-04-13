#include "Space.hpp"
#include <iostream>
#include <string>
#include <algorithm>
#include "core/rendering/Renderer.hpp"
#include "core/ui/ui_dumptruck.hpp"

#include "core/utils/ModelGenerator.hpp"

Space::Space(ResourceManager& resources, Renderer& renderer)
 : resources(resources), renderer(renderer)
{

}

Space::~Space() {
	for (auto* obj : objects) {
		delete obj;
	}
	objects.clear();
}

void Space::init_space() {
	camera.set_position(glm::vec3(0.0f, 11.0f, 37.5f));

	init_skybox();
	init_raymarcher_and_voxelgrid();
	init_glass();
	init_lights();
	init_lines();

	// THIS IS STINKY;
	Object* base_ground = new Object(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f), glm::vec3(1.f), "res://shaders/core/default_shader.vs");
	base_ground->init(resources, "Ground Plane");
	ModelGpuData ground_model_2 = ModelGenerator::create_flat_ground(350, 350, 50, 50);
	Res::Model r_ground_model = resources.upload_model(std::move(ground_model_2));
	base_ground->set_model(std::move(r_ground_model));
	objects.push_back(base_ground);
	// THIS IS STINKY;
	
	std::string sphere_path = "res://models/sphere.obj";
	create_object(glm::vec3(-10.f, 0.f, 10.f), glm::vec3(0.f), glm::vec3(1.f), "res://models/teapot.obj", "Teapot");
	create_object(glm::vec3(7.0f, -1.0f, -8.0f), glm::vec3(0.0f), glm::vec3(5.0f), sphere_path, "Sphere 1");
	create_object(glm::vec3(-18.0f, 5.0f, 28.0f), glm::vec3(0.0f), glm::vec3(3.5f), sphere_path, "Sphere 2");
	create_object(glm::vec3(35.0f, 2.0f, 14.0f), glm::vec3(0.0f), glm::vec3(2.0f), sphere_path, "Sphere 3");
	create_object(glm::vec3(17.0f, 3.5f, -2.0f), glm::vec3(0.0f), glm::vec3(7.0f), sphere_path, "Sphere 4");
	// create_object(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f), "res://models/Sponza/glTF/Sponza.gltf", "Sponza");
	// create_object(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f), "res://models/crystal_ball_table/fortune_teller_table.glb", "Fortune Table");
	// for (int i = 0; i < 20; ++i) { // testing the shadows
	// 	float x = (rand() % 200 - 100) * 1.2f;
	// 	float y = (rand() % 40);
	// 	float z = (rand() % 200 - 100) * 1.2f;
	// 	float scale = 0.5f + (rand() % 100) * 0.05f;
		
	// 	create_object(glm::vec3(x, y, z), glm::vec3(0.f), glm::vec3(scale), sphere_path, "Sphere " + std::to_string(i + 5));
	// }

	auto* new_object = new Object(glm::vec3(52.0f, 3.5f, -10.0f), glm::vec3(0.0f), glm::vec3(2.0f, 15.0f, 20.0f), "res://shaders/core/default_shader.vs", "res://models/cube.obj");
	new_object->init(resources, "Default Cube");
	objects.push_back(new_object);
	auto* new_object2 = new Object(glm::vec3(37.0f, 3.5f, -28.0f), glm::vec3(0.0f), glm::vec3(20.0f, 15.0f, 2.0f), "res://shaders/core/default_shader.vs", "res://models/cube.obj");
	new_object2->init(resources, "Default Cube 2");
	objects.push_back(new_object2);
}

void Space::init_skybox() {
	skybox = Skybox();
	skybox.init(resources);
	sun = Sun();
	sun.set_direction(glm::vec3(1.0f, 1.0f, 1.0f));
	sun.set_color(glm::vec4(1.0f, 1.0f, 0.930f, 1.0f));
	sun.init(resources);
}

void Space::init_raymarcher_and_voxelgrid() {
	const float cell_size = 1.5f;
	const glm::vec3 pos = glm::vec3(-30.0f, -14.45f, -22.3f);// * cell_size;
	voxel_grid = VoxelGrid(80, 40, 40, 0, cell_size, pos); 
    voxel_grid.init(resources);
	voxel_grid.set_debug_visibility(false);
	raymarcher = Raymarcher();
	raymarcher.init(new Shader(resources.get_full_path("res://shaders/raymarching/raymarcher.vs"), resources.get_full_path("res://shaders/raymarching/raymarcher.fs")));
}

void Space::init_glass() {
	glass.init(resources);
	glass.set_visibility(false);
}

void Space::init_lights() {
	lighting_data.sun_direction = glm::vec3(1.0f, 1.0f, 1.0f);
	lighting_data.sun_color = glm::vec3(1.0f, 1.0f, 0.930f);
	lighting_data.sun_intensity = 1.0f;
	lighting_data.ambient_color = glm::vec3(0.1f);
	lighting_data.ambient_intensity = 1.0f;

	lights.reserve(MAX_LIGHTS); light_spheres.reserve(MAX_LIGHTS);
	add_light(glm::vec3(14.0f, 13.0f, 20.5), 50.f, glm::vec3(1.0f, 0.3f, 0.2f), 84.0f, glm::vec3(0.0f, -1.0f, 0.0f), 1.0f, 1.0f, LightType::Point);
	add_light(glm::vec3(-10.0f, 11.0f, -22.0), 100.f, glm::vec3(0.0f, 0.58f, 1.0f), 271.0f, glm::vec3(0.0f, -1.0f, 0.0f), 1.0f, 1.0f, LightType::Point);
	add_light(glm::vec3(-10.0f, 12.0f, 10.0), 50.f, glm::vec3(0.31f, 0.78f, 0.48f), 21.0f, glm::vec3(0.0f, -1.0f, 0.0f), 1.0f, 1.0f, LightType::Point);	
}

void Space::init_lines() {
	line_manager = Line();
	line_manager.init(resources.get_full_path("res://shaders/line.vs"), resources.get_full_path("res://shaders/line.fs"));
	std::vector<LinePrimitive> world_grid_lines = {{glm::vec3(512.f, 0.0f, 0.0f), glm::vec3(-512.0f, 0.0f, 0.0f), glm::vec4(0.86f, 0.08f, 0.24f, 1.0f)},    // X : R (crimson red)
												   {glm::vec3(0.f, 512.0f, 0.0f), glm::vec3(0.0f, -512.0f, 0.0f), glm::vec4(0.196f, 0.754f, 0.196f, 1.0f)}, // Y : G (forest green)
												   {glm::vec3(0.f, 0.0f, 512.0f), glm::vec3(0.0f, 0.0f, -512.0f), glm::vec4(0.118f, 0.565f, 1.0f, 1.0f)}};  // Z : B (dodger blue)
	line_manager.add_lines(world_grid_lines, true);
}

void Space::tick(float delta, ButtonMap bm)
{
	time += delta;
	this_frames_button_map = bm;
	if (bm.MousePointerActive) {
		selection_ray_cast(bm.MousePosX, bm.MousePosY);
	}

	camera.tick(delta, bm);
	sun.tick(delta);

	for (auto& b : objects) {
		b->tick(delta);
	}

	voxel_grid.tick(delta, selection_ray_start, selection_ray_dir, bm.MousePointerActive, bm.MouseLeft);
	raymarcher.tick(delta);

	glass.tick(delta, bm);
        
	ui::stats_overlay(camera, renderer);
	ui::settings_panel(*this, raymarcher, raymarcher.get_raymarch_settings(), voxel_grid, sun, lighting_data, lights, glass, line_manager, objects);

	lighting_data.sun_direction = sun.get_direction();
	lighting_data.sun_color = sun.get_color();
	lighting_data.sun_intensity = sun.get_intensity();

	const std::size_t count = std::min(lights.size(), light_spheres.size());
	for (std::size_t i = 0; i < count; ++i) {
		light_spheres[i]->set_position(lights[i].position);
	}
}

void Space::enqueue_renderables() {
	glm::mat4 view_matrix = camera.get_view_matrix();
	glm::vec3 camera_pos = camera.get_position();
	renderer.submit_frame_data(view_matrix, camera_pos, lighting_data, lights);

	for (auto& light_sphere : light_spheres) { // the light objects (hard to see em otherwise)
		light_sphere->enqueue(renderer, resources);
	}

	skybox.enqueue(renderer, resources);
	sun.enqueue(renderer);

	for (auto& b : objects) {
		b->enqueue(renderer, resources);
	}
	
	voxel_grid.enqueue(renderer, resources, camera_pos);
	raymarcher.enqueue(renderer, camera_pos, voxel_grid.get_voxel_texture_id());
	
	line_manager.enqueue(renderer);
	glass.enqueue(renderer, this_frames_button_map);
}

void Space::create_object(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, const std::string& model_asset, const std::string& name) {
	auto* new_object = new Object(position, rotation, scale, "res://shaders/core/default_shader.vs", model_asset);
	new_object->init(resources, name);
	objects.push_back(new_object);
}

void Space::add_light(glm::vec3 position, float radius, glm::vec3 color, float intensity, 
					  glm::vec3 direction, float angle, float volumetric_intensity, LightType type) {
	if (lights.size() >= MAX_LIGHTS) {
		std::cerr << "Max light count reached, cannot add more lights." << std::endl;
		return;
	}
	Light light{};
	light.position = position;
	light.radius = radius;
	light.color = color;
	light.intensity = intensity;
	light.direction = direction;
	light.angle = angle;
	light.type = type;
	light.volumetric_multiplier = volumetric_intensity;
	
	lights.push_back(std::move(light));
	
																	// invert the scale to invert the normals x) now its lit up! :)
	Object* new_light_object = new Object(position, glm::vec3(0.0f), glm::vec3(-0.4f), "res://shaders/core/default_shader.vs", "res://models/sphere.obj");
	new_light_object->init(resources);
	light_spheres.push_back(new_light_object);
}

void Space::remove_light(std::size_t index) {
	if (index >= lights.size() || index >= light_spheres.size()) { // TODO: look over this logic
		return;
	}

	delete light_spheres[index];
	light_spheres.erase(light_spheres.begin() + static_cast<std::ptrdiff_t>(index));
	lights.erase(lights.begin() + static_cast<std::ptrdiff_t>(index));
}

void Space::cast_ray(float mouse_x, float mouse_y) {
	glm::vec2 viewport_dims = renderer.get_viewport_size();
	float x = (2.0f * mouse_x) / viewport_dims.x - 1.0f;
    float y = 1.0f - (2.0f * mouse_y) / viewport_dims.y;

    glm::vec4 ray_clip = glm::vec4(x, y, -1.0f, 1.0f);
    glm::vec4 ray_eye = glm::inverse(renderer.get_proj()) * ray_clip;

    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

    glm::vec3 ray_dir = glm::vec3(glm::inverse(renderer.get_view()) * ray_eye);
    ray_dir = glm::normalize(ray_dir);
	
	glm::vec3 start = camera.get_position();
	glm::vec3 end = start + ray_dir * 25.0f;

	// line_manager.add_line(start, end, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // todo: add a button to clear the lines
}

void Space::selection_ray_cast(float mouse_x, float mouse_y) {
	glm::vec2 viewport_dims = renderer.get_viewport_size();
	float x = (2.0f * mouse_x) / viewport_dims.x - 1.0f;
    float y = 1.0f - (2.0f * mouse_y) / viewport_dims.y;

    glm::vec4 ray_clip = glm::vec4(x, y, -1.0f, 1.0f);
    glm::vec4 ray_eye = glm::inverse(renderer.get_proj()) * ray_clip;

    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

    glm::vec3 ray_dir = glm::vec3(glm::inverse(renderer.get_view()) * ray_eye);
    ray_dir = glm::normalize(ray_dir);
	
	selection_ray_start = camera.get_position();
	selection_ray_dir = ray_dir;
}


