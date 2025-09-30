#include "Space.hpp"
#include <iostream>
#include <string>
#include "core/ui/ui_dumptruck.hpp"

Space::Space(ResourceManager& resources) : resources(resources)
{	
	init_space();
}

void Space::init_space() {
	water_surface = new WaterSurface(glm::vec3(5.f, -10.f, 5.f), 20.f, 20.f);
	sun = new Sun(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	// vox = new VoxelStructure(20, 20, 20, glm::vec3(20, 1, 1), 1, 0.75f);
	
	camera = new Camera();	
	// sphere2 = ray_scene->add_sphere(glm::vec3(-10.0f, 0.0f, 0.0f), 3.0f, glm::vec4(1.0, 0.98, 0.92, 1.0));

		skybox = new Skybox(
		std::string(resources.get_full_path("res://models/skybox-full-tweaked.obj")),
		std::string(resources.get_full_path("res://shaders/Skybox.shader")),
		std::string(resources.get_full_path("res://textures/skybox/cloud-landscape.tga"))
	);

	std::vector<LinePrimitive> lines = {{glm::vec3(5.f, 0.0f, 0.0f), glm::vec3(-5.0f, 0.0f, 0.0f)},
										{glm::vec3(0.f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, -5.0f)},
										{glm::vec3(0.f, 5.0f, 0.0f), glm::vec3(0.0f, -5.0f, 0.0f)},
										{glm::vec3(2.f, 2.0f, 2.0f), glm::vec3(-2.0f, -2.0f, -2.0f)}};
	
	uninitialized_objects.push_back(std::make_unique<Line>(std::move(lines)));
	uninitialized_objects.push_back(std::make_unique<Object>(glm::vec3(-10.f, 0.f, 10.f), glm::vec3(0.f), glm::vec3(1.f), nullptr, "res://shaders/WorldObject.shader", "res://models/teapot.obj", ""));
	uninitialized_objects.push_back(std::make_unique<Raymarcher>());

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
		std::string title = std::to_string(o->get_id());
		ui::transform_window(*o, title.c_str());
	}


}

void Space::enqueue_renderables(Renderer& renderer) {
	glm::mat4 view_matrix = camera->get_view_matrix();
	glm::vec3 cam_pos = camera->get_position();
	renderer.set_view(view_matrix); // renderer should have all the knowledge! maybe a better way to do this?!

	skybox->draw(renderer, camera); // draw prio u know

	// water_surface->render(proj, view_matrix, cam_pos); // tbh this one is transparent, and also not really working...
	
	for (auto& o : objects) {
		o->enqueue(renderer, resources);
	}
	
	// vox->drawVoxels(renderer, view_matrix);
	
	sun->render(renderer, camera);

	// raymarcher->enqueue(renderer, RenderPass::Volumetrics, camera, sun_dir);
}




