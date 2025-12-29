#include "Space.hpp"
#include <iostream>
#include <string>
#include "core/Base.hpp"
#include "core/space/Object.hpp"
#include "core/ui/ui_dumptruck.hpp"
#include "core/Line.hpp"

#include "core/utils/ModelGenerator.hpp"

// feature imports :)
#include "feature/glass/Glass.hpp"
#include "feature/Sun.hpp"
#include "feature/Skybox.hpp"
#include "feature/raymarcher/raymarcher.hpp"

Space::Space(ResourceManager& resources) : resources(resources)
{	
	init_space();
}

void Space::init_space() {
	camera = new Camera(glm::vec3(0.0, 10.0f, 0.0));
	sun = new Sun(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // gotta work more on this tho
	sun->init(resources, this); 

	// world grid lines (currently parallell to all axis at 0)
	std::vector<LinePrimitive> lines = {{glm::vec3(256.f, 0.0f, 0.0f), glm::vec3(-256.0f, 0.0f, 0.0f), glm::vec4(0.86f, 0.08f, 0.24f, 1.0f)},    // X : R (crimson red)
	{glm::vec3(0.f, 256.0f, 0.0f), glm::vec3(0.0f, -256.0f, 0.0f), glm::vec4(0.196f, 0.754f, 0.196f, 1.0f)}, // Y : G (forest green)
	{glm::vec3(0.f, 0.0f, 256.0f), glm::vec3(0.0f, 0.0f, -256.0f), glm::vec4(0.118f, 0.565f, 1.0f, 1.0f)}};  // Z : B (dodger blue)
	
	add_base_entity(std::make_unique<Line>(std::move(lines)));
	add_base_entity(std::make_unique<Object>(glm::vec3(-10.f, 0.f, 10.f), glm::vec3(0.f), glm::vec3(1.f), nullptr, "res://shaders/core/default_shader.vs", "res://models/teapot.obj", ""));
	add_base_entity(std::make_unique<Raymarcher>());
	add_base_entity(std::make_unique<Skybox>());
	add_base_entity(std::make_unique<Glass>());

	// THIS IS STINKY; EWWW
	auto base_ground = std::make_unique<Object>(glm::vec3(-10.f, 0.f, 10.f), glm::vec3(0.f), glm::vec3(1.f), nullptr, "res://shaders/core/default_shader.vs");
	base_ground->init(resources, this);
	ModelGpuData ground_model_2 = ModelGenerator::create_flat_ground(350, 350, 50, 50);
	Res::Model r_ground_model = resources.upload_model(std::move(ground_model_2));
	base_ground->set_model(std::move(r_ground_model));
	base_objects.push_back(std::move(base_ground));
	// THIS IS STINKY; BLEEEH

	std::string sphere_path = "res://models/sphere.obj";
	id_1 = create_object(glm::vec3(7.0f, -1.0f, -8.0f), glm::vec3(0.0f), glm::vec3(5.0f), sphere_path);
	id_2 = create_object(glm::vec3(-18.0f, 5.0f, 28.0f), glm::vec3(0.0f), glm::vec3(3.5f), sphere_path);
	id_3 = create_object(glm::vec3(35.0f, 2.0f, 14.0f), glm::vec3(0.0f), glm::vec3(2.0f), sphere_path);
	id_4 = create_object(glm::vec3(17.0f, 3.5f, -2.0f), glm::vec3(0.0f), glm::vec3(7.0f), sphere_path);

	// light stuff
	light1.position = glm::vec3(0.0f, 10.0f, 0.0);
	light1.radius = 200.f;
	light1.color = glm::vec3(1.0f, 0.3f, 0.2f);
	light1.intensity = 25.0f;
	light1.direction = glm::vec3(0.0f, -1.0f, 0.0f);
	light1.volumetric_intensity = 1.0f;
	light1.type = LightType::Point;

	light2 = light1;
	light2.color = glm::vec3(0.0f, 0.58f, 1.0f);
	light3 = light1;
	light3.color = glm::vec3(0.31f, 0.78f, 0.48f);

							   								 //invert the scale to invert the normals x) now its lit up! 
	light_sphere1 = new Object(light1.position, glm::vec3(0.0f), glm::vec3(-0.4f), nullptr, "res://shaders/core/default_shader.vs", "res://models/sphere.obj");
	light_sphere2 = new Object(light2.position, glm::vec3(0.0f), glm::vec3(-0.4f), nullptr, "res://shaders/core/default_shader.vs", "res://models/sphere.obj");
	light_sphere3 = new Object(light3.position, glm::vec3(0.0f), glm::vec3(-0.4f), nullptr, "res://shaders/core/default_shader.vs", "res://models/sphere.obj");
	light_sphere1->init(resources, this);
	light_sphere2->init(resources, this);
	light_sphere3->init(resources, this);
}

void Space::tick(float delta, ButtonMap bm)
{
	time += delta;
	this_frames_button_map = bm;

	camera->tick(delta, bm);
	sun->tick(delta);

	glm::vec3 light_pos1 = glm::vec3(10.0f * sin(time * 0.12), 10.f, 10.0f * cos(time * 0.12));
	glm::vec3 light_pos2 = glm::vec3(10.0f * sin(time * 0.12 + 3 * PI/2), 10.f, 10.0f * cos(time * 0.12 + 3 * PI/2));
	glm::vec3 light_pos3 = glm::vec3(10.0f * sin(time * 0.12 + PI), 10.f, 10.0f * cos(time * 0.12 + PI));
	
	light_sphere1->set_position(light_pos1);
	light_sphere2->set_position(light_pos2);
	light_sphere3->set_position(light_pos3);
	light1.position = light_sphere1->get_position();
	light2.position = light_sphere2->get_position();
	light3.position = light_sphere3->get_position();

	for (auto& b : base_objects) {
		b->tick(delta);
		// std::string title = std::to_string(o->get_id());
		// ui::transform_window(*o, title.c_str());
	}

	// quick and dirty way to get the light stuff editable during run-time
	// ui_dumptruck stuff here
}

void Space::enqueue_renderables(Renderer& renderer) {
	glm::mat4 view_matrix = camera->get_view_matrix();
	glm::vec3 cam_pos = camera->get_position();
	renderer.set_view(view_matrix); // renderer should have all the knowledge! maybe a better way to do this?!
	renderer.submit_lighting_data(std::vector<Light>{light1, light2, light3});
	light_sphere1->enqueue(renderer, resources);
	light_sphere2->enqueue(renderer, resources);
	light_sphere3->enqueue(renderer, resources);

	for (auto& b : base_objects) {
		b->enqueue(renderer, resources);
	}
	
	sun->enqueue(renderer, resources); // skybox prio is just a coincidence because it sun enqueues after skybox does it. gotta get some prio thing into the renderer tbh tbh tbh 
}

UUID<Base> Space::create_object(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, const std::string& model_asset) {
	auto new_object = std::make_unique<Object>(position, rotation, scale, nullptr, "res://shaders/core/default_shader.vs", model_asset);
	new_object->init(resources, this);
	auto id = new_object->get_id();
	base_objects.push_back(std::move(new_object));

	return id;
}

UUID<Base> Space::add_base_entity(std::unique_ptr<Base> base) {
	auto id = base->get_id();
	base->init(resources, this); 
	base_objects.push_back(std::move(base));
	return id;
}

Base* Space::get_base_entity(const UUID<Base>& id) {
    for (auto& b : base_objects) {
        if (b->get_id() == id) {
            return b.get();
        }
    }
    return nullptr;
}

void Space::cast_ray() {
	glm::vec3 view_dir = camera->get_front();
	
	glm::vec3 start = camera->get_position();
	glm::vec3 end = start + view_dir * 25.0f;

	add_base_entity(std::make_unique<Line>(start, end, glm::vec4(1.0f)));
}


// void Space::add_light_object()