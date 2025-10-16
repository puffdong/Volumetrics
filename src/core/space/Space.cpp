#include "Space.hpp"
#include <iostream>
#include <string>
#include "core/ui/ui_dumptruck.hpp"

Space::Space(ResourceManager& resources) : resources(resources)
{	
	init_space();
}

void Space::init_space() {
	camera = new Camera(glm::vec3(0.0, 10.0, 0.0));
	sun = new Sun(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	sun->init(resources, this);

	std::vector<LinePrimitive> lines = {{glm::vec3(5.f, 0.0f, 0.0f), glm::vec3(-5.0f, 0.0f, 0.0f)},
										{glm::vec3(0.f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, -5.0f)},
										{glm::vec3(0.f, 5.0f, 0.0f), glm::vec3(0.0f, -5.0f, 0.0f)},
										{glm::vec3(2.f, 2.0f, 2.0f), glm::vec3(-2.0f, -2.0f, -2.0f)}};
	
	uninitialized_objects.push_back(std::make_unique<Line>(std::move(lines)));
	uninitialized_objects.push_back(std::make_unique<Object>(glm::vec3(-10.f, 0.f, 10.f), glm::vec3(0.f), glm::vec3(1.f), nullptr, "res://shaders/WorldObject.shader", "res://models/teapot.obj", ""));
	uninitialized_objects.push_back(std::make_unique<Raymarcher>());
	// uninitialized_objects.push_back(std::make_unique<WaterSurface>(glm::vec3(5.f, -10.f, 5.f), glm::vec3(0.f), glm::vec3(1.f), nullptr, 20.f, 20.f));
	uninitialized_objects.push_back(std::make_unique<Skybox>());

	// THIS IS STINKY; EWWW
	auto base_ground = std::make_unique<Object>(glm::vec3(-10.f, 0.f, 10.f), glm::vec3(0.f), glm::vec3(1.f), nullptr, "res://shaders/WorldObject.shader", "res://models/teapot.obj", "");
	base_ground->init(resources, this);
	ModelObject* ground_model = new ModelObject(256, 256, 50, 50);
	base_ground->swap_model(ground_model);
	objects.push_back(std::move(base_ground));
	// THIS IS STINKY; BLEEEH
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
	
	for (auto& o : objects) {
		o->enqueue(renderer, resources);
	}
	
	sun->enqueue(renderer, resources); // skybox prio is just a coincidence because it sun enqueues after skybox does it. gotta get some prio thing into the renderer tbh tbh tbh 
}




