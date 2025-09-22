#include "Space.hpp"
#include <iostream>
#include "core/ui/ui_dumptruck.hpp"

Space::Space(ResourceManager& resources) : resources(resources)
{	
	camera = new Camera();

	init_space();
}

void Space::init_space() {
	water_surface = new WaterSurface(glm::vec3(5.f, -10.f, 5.f), 20.f, 20.f);
	sun = new Sun(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	vox = new VoxelStructure(20, 20, 20, glm::vec3(20, 1, 1), 1, 0.75f);
	
	ray_scene = new RayScene(glm::vec3(0.0, 0.0, 0.0));
	raymarcher = new Raymarcher(ray_scene);
	sphere1 = ray_scene->add_sphere(glm::vec3(-5.0f, -3.0f, -10.0f), 15.0f, glm::vec4(1.0, 0.98, 0.92, 1.0));
	// sphere2 = ray_scene->add_sphere(glm::vec3(-10.0f, 0.0f, 0.0f), 3.0f, glm::vec4(1.0, 0.98, 0.92, 1.0));

		skybox = new Skybox(
		std::string("/Users/puff/Developer/graphics/Volumetrics/res/models/skybox-full-tweaked.obj"),
		std::string("/Users/puff/Developer/graphics/Volumetrics/res/shaders/Skybox.shader"),
		std::string("/Users/puff/Developer/graphics/Volumetrics/res/textures/skybox/cloud-landscape.tga")
	);

	Shader* worldShader = new Shader("/Users/puff/Developer/graphics/Volumetrics/res/shaders/WorldObject.shader");
	
	WorldObject* teapotObject = new WorldObject(worldShader, "/Users/puff/Developer/graphics/Volumetrics/res/models/teapot.obj", glm::vec3(-10.f, 0.f, 10.f), glm::vec3(0.f));
	wObjects.push_back(teapotObject);

	std::vector<LinePrimitive> lines = {{glm::vec3(5.f, 0.0f, 0.0f), glm::vec3(-5.0f, 0.0f, 0.0f)},
										{glm::vec3(0.f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, -5.0f)},
										{glm::vec3(0.f, 5.0f, 0.0f), glm::vec3(0.0f, -5.0f, 0.0f)},
										{glm::vec3(2.f, 2.0f, 2.0f), glm::vec3(-2.0f, -2.0f, -2.0f)}};
	uninitialized_objects.push_back(std::make_unique<Line>(std::move(lines)));
	
}

void Space::process_init_queue() {
	for (auto& obj : uninitialized_objects) {
        obj->init(resources); // virtual call resolves to actual subclass
    }

    // Move them to the main objects vector
    objects.insert(objects.end(),
                   std::make_move_iterator(uninitialized_objects.begin()),
                   std::make_move_iterator(uninitialized_objects.end()));

    uninitialized_objects.clear();
}

void Space::tick(float delta, ButtonMap bm)
{
	time += delta;

	process_init_queue();

	for (auto& o : objects) {
		o->tick(delta, bm);
	}

	for (WorldObject* o : wObjects)
	{
		o->tick(delta);
		ui::transform_window(*o, "teapot");
	}

	camera->tick(delta, bm);
	sun->tick(delta);

	// sphere1->pos.x = 13 * sin(time * 0.1f);
	// sphere1->pos.z = 13 * cos(time * 0.1f);

	// sphere2->pos.x = 7 * sin(-time * 0.15f);
	// sphere2->pos.z = 7 * cos(-time * 0.15f);

	raymarcher->tick(delta);


}

void Space::enqueue_renderables(Renderer& renderer) {
	glm::mat4 view_matrix = camera->get_view_matrix();
	glm::vec3 cam_pos = camera->get_position();
	glm::vec3 sun_dir = sun->get_direction();

	renderer.set_view(view_matrix); // renderer should have all the knowledge! maybe a better way to do this?!

	// water_surface->render(proj, view_matrix, cam_pos); // tbh this one is transparent, and also not really working...
	
	for (auto& o : objects) {
		o->enqueue(renderer, resources);
	}

	for (WorldObject* o : wObjects)
	{
		o->draw(renderer, view_matrix, o->getModelMatrix());
	}
	
	skybox->draw(renderer, camera); // draw prio u know
	vox->drawVoxels(renderer, view_matrix);
	
	sun->render(renderer, camera);
	// line->render(renderer, camera->get_view_matrix()); // when to do this tho, prolly late...?

	// raymarcher->enqueue(renderer, RenderPass::Volumetrics, camera, sun_dir);
}

Camera* Space::get_camera() { 
	return camera; 
};





