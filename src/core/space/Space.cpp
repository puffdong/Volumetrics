#include "Space.hpp"
#include <iostream>
#include <string>
#include "core/Base.hpp"
#include "core/space/Object.hpp"
#include "core/ui/ui_dumptruck.hpp"
#include "core/Line.hpp"

// feature imports :)
#include "feature/glass/Glass.hpp"
#include "feature/Sun.hpp"
#include "feature/Skybox.hpp"
#include "feature/raymarcher/raymarcher.hpp"
#include "feature/water/WaterSurface.hpp"

Space::Space(ResourceManager& resources) : resources(resources)
{	
	init_space();
}

void Space::init_space() {
	// Standard stuff, always there, need em for the getters and setters
	camera = new Camera(glm::vec3(0.0, 10.0f, 0.0));
	sun = new Sun(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // gotta work more on this tho
	sun->init(resources, this); 

	// world grid lines (currently parallell to all axis at 0)
	std::vector<LinePrimitive> lines = {{glm::vec3(256.f, 0.0f, 0.0f), glm::vec3(-256.0f, 0.0f, 0.0f), glm::vec4(0.86f, 0.08f, 0.24f, 1.0f)},    // X : R (crimson red)
										{glm::vec3(0.f, 256.0f, 0.0f), glm::vec3(0.0f, -256.0f, 0.0f), glm::vec4(0.196f, 0.754f, 0.196f, 1.0f)}, // Y : G (forest green)
										{glm::vec3(0.f, 0.0f, 256.0f), glm::vec3(0.0f, 0.0f, -256.0f), glm::vec4(0.118f, 0.565f, 1.0f, 1.0f)}};  // Z : B (dodger blue)
	
	uninitialized_objects.push_back(std::make_unique<Line>(std::move(lines)));


	uninitialized_objects.push_back(std::make_unique<Object>(glm::vec3(-10.f, 0.f, 10.f), glm::vec3(0.f), glm::vec3(1.f), nullptr, "res://shaders/core/default_ground.vs", "res://models/teapot.obj", ""));
	uninitialized_objects.push_back(std::make_unique<Raymarcher>());
	// uninitialized_objects.push_back(std::make_unique<WaterSurface>(glm::vec3(5.f, -10.f, 5.f), glm::vec3(0.f), glm::vec3(1.f), nullptr, 20.f, 20.f));
	uninitialized_objects.push_back(std::make_unique<Skybox>());
	// uninitialized_objects.push_back(std::make_unique<Glass>());

	// THIS IS STINKY; EWWW
	auto base_ground = std::make_unique<Object>(glm::vec3(-10.f, 0.f, 10.f), glm::vec3(0.f), glm::vec3(1.f), nullptr, "res://shaders/core/default_ground.vs", "res://models/teapot.obj", "");
	base_ground->init(resources, this);
	ModelObject* ground_model = new ModelObject(350, 350, 50, 50); // kinda hacky but eh it works!
	base_ground->swap_model(ground_model);
	objects.push_back(std::move(base_ground));
	// THIS IS STINKY; BLEEEH

	// light stuff
	Light l{};
	l.position = glm::vec3(0.0f, 10.0f, 0.0);
	l.radius = 10.f;
	l.color = glm::vec3(1.0f, 0.8515625, 0.4765625);
	l.intensity = 1.0f;
	l.direction = glm::vec3(0.0f, -1.0f, 0.0f);
	l.volumetric_intensity = 1.0f;
	l.type = LightType::Point;

	lights.push_back(l);
}

void Space::process_init_queue() {
	for (auto& obj : uninitialized_objects) {
        obj->init(resources, this);
		std::cout << "Initialized object. UUID: " << obj->get_id() << std::endl;
    }   

    objects.insert(objects.end(),
                   std::make_move_iterator(uninitialized_objects.begin()),
                   std::make_move_iterator(uninitialized_objects.end()));

    uninitialized_objects.clear();
}

void Space::tick(float delta, ButtonMap bm)
{
	time += delta;
	this_frames_button_map = bm;

	process_init_queue();

	camera->tick(delta, bm);
	sun->tick(delta);

	for (auto& o : objects) {
		o->tick(delta);
		// std::string title = std::to_string(o->get_id());
		// ui::transform_window(*o, title.c_str());
	}
}

void Space::enqueue_renderables(Renderer& renderer) {
	glm::mat4 view_matrix = camera->get_view_matrix();
	glm::vec3 cam_pos = camera->get_position();
	renderer.set_view(view_matrix); // renderer should have all the knowledge! maybe a better way to do this?!
	renderer.submit_lighting_data(lights);

	for (auto& o : objects) {
		o->enqueue(renderer, resources);
	}
	
	sun->enqueue(renderer, resources); // skybox prio is just a coincidence because it sun enqueues after skybox does it. gotta get some prio thing into the renderer tbh tbh tbh 
}




