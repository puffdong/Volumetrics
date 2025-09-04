#include "Space.h"
#include <iostream>
#include "ui/ui_dumptruck.hpp"

Space::Space()
{
	proj = glm::perspective(glm::radians(70.f), aspect_ratio, 1.0f, 256.0f);
	
	camera = new Camera();

	water_surface = new WaterSurface(glm::vec3(5.f, -10.f, 5.f), 20.f, 20.f);
	sun = new Sun(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	vox = new VoxelStructure(20, 20, 20, glm::vec3(20, 1, 1), 1, 0.75f);
	vox->setVoxelValue(2, 2, 2, 3);
	std::cout << vox->getVoxelValue(2, 2, 2) << std::endl;
	std::cout << vox->getVoxelValue(2, 2, 3) << std::endl;

	ray_scene = new RayScene(glm::vec3(0.0, 0.0, 0.0));
	raymarcher = new Raymarcher(ray_scene);
	sphere1 = ray_scene->add_sphere(glm::vec3(-5.0f, -3.0f, -10.0f), 15.0f, glm::vec4(1.0, 0.98, 0.92, 1.0));
	// sphere2 = ray_scene->add_sphere(glm::vec3(-10.0f, 0.0f, 0.0f), 3.0f, glm::vec4(1.0, 0.98, 0.92, 1.0));

		skybox = new Skybox(
		std::string("/Dev/OpenGL/Volumetrics/res/models/skybox-full-tweaked.obj"),
		std::string("/Dev/OpenGL/Volumetrics/res/shaders/Skybox.shader"),
		std::string("/Dev/OpenGL/Volumetrics/res/textures/skybox/cloud-landscape.tga")
	);

	Shader* worldShader = new Shader("/Dev/OpenGL/Volumetrics/res/shaders/WorldObject.shader");

	std::vector<LinePrimitive> lines = {{glm::vec3(5.f, 0.0f, 0.0f), glm::vec3(-5.0f, 0.0f, 0.0f)},
										{glm::vec3(0.f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, -5.0f)},
										{glm::vec3(0.f, 5.0f, 0.0f), glm::vec3(0.0f, -5.0f, 0.0f)},
										{glm::vec3(2.f, 2.0f, 2.0f), glm::vec3(-2.0f, -2.0f, -2.0f)}};
	line = new Line(lines);
	
	WorldObject* teapotObject = new WorldObject(worldShader, "/Dev/OpenGL/Volumetrics/res/models/teapot.obj", glm::vec3(-10.f, 0.f, 10.f), glm::vec3(0.f));
	wObjects.push_back(teapotObject);
}

void Space::tick(float delta, ButtonMap bm)
{
	if (changes_made) { // whenever aspect ratio and fov changes, this has to propogate somehow, right :)
		update_projection_uniforms();
		changes_made = false;
	}
	time += delta;

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

void Space::enqueue_renderables() {
	glm::mat4 view_matrix = camera->get_view_matrix();
	glm::vec3 cam_pos = camera->get_position();
	glm::vec3 sun_dir = sun->get_direction();

	// water_surface->render(proj, view_matrix, cam_pos); // tbh this one is transparent, and also not really working...
	
	for (WorldObject* o : wObjects)
	{
		o->draw(proj, view_matrix, o->getModelMatrix());
	}
	
	skybox->draw(proj, camera); // draw prio u know
	vox->drawVoxels(proj, view_matrix);
	
	sun->render(proj, camera);
	line->render(proj, camera->get_view_matrix()); // when to do this tho, prolly late...?

	raymarcher->enqueue(RenderPass::Volumetrics, camera, sun_dir);
}

void Space::change_fov(double xoffset, double yoffset) { 
	fov -= (float)yoffset;
	proj = glm::perspective(glm::radians(fov), aspect_ratio, near, far);
	changes_made = true;
}

void Space::update_projection_matrix_aspect_ratio(float aspectRatio) {
    if (aspectRatio <= 0) {
		aspectRatio = 16.f / 9.f;
	} else {
		aspect_ratio = aspectRatio;
	}
    change_fov(0.0, 0.0); // a bit dumb that we rely on "fov func" to update the proj matrix
	changes_made = true;
}

Camera* Space::get_camera() { 
	return camera; 
};

void Space::update_projection_uniforms() {
	raymarcher->update_static_uniforms(proj, near, far);
	line->update_static_uniforms(proj, near, far);
	std::cout << "<Updated static uniforms>" << std::endl;
}





