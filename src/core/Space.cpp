#include "Space.h"
#include <iostream>

Space::Space()
{
	proj = glm::perspective(glm::radians(70.f), aspect_ratio, 1.0f, 256.0f);
	
	camera = new Camera();

	water_surface = new WaterSurface(glm::vec3(5.f, -10.f, 5.f), 20.f, 20.f);
	sun = new Sun(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	// voxel stuff -> in progress 
	vox = new VoxelStructure(20, 20, 20, glm::vec3(20, 1, 1), 1, 0.75f);
	vox->setVoxelValue(2, 2, 2, 3);
	std::cout << vox->getVoxelValue(2, 2, 2) << std::endl;
	std::cout << vox->getVoxelValue(2, 2, 3) << std::endl;

	// raymarcher
	ray_scene = new RayScene(glm::vec3(0.0, 0.0, 0.0));
	raymarcher = new Raymarcher(ray_scene);
	sphere1 = ray_scene->add_sphere(glm::vec3(10.0f, 0.0f, 0.0f), 5.0f, glm::vec4(1.0, 0.4, 0.1, 0.8));
	sphere2 = ray_scene->add_sphere(glm::vec3(-10.0f, 0.0f, 0.0f), 3.0f, glm::vec4(1.0, 0.4, 0.1, 0.8));

		skybox = new Skybox(
		std::string("/Users/puff/Developer/graphics/Volumetrics/res/models/skybox-full-tweaked.obj"),
		std::string("/Users/puff/Developer/graphics/Volumetrics/res/shaders/Skybox.shader"),
		std::string("/Users/puff/Developer/graphics/Volumetrics/res/textures/skybox/cloud-landscape.tga")
	);


	// Setup shader with lighting
	Shader* worldShader = new Shader("/Users/puff/Developer/graphics/Volumetrics/res/shaders/WorldObject.shader");
	LightSource newLightSources[] = {
				LightSource(glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, 1.f, 0.f), true)
	};

	std::vector<LinePrimitive> lines = {{glm::vec3(5.f, 0.0f, 0.0f), glm::vec3(-5.0f, 0.0f, 0.0f)},
										{glm::vec3(0.f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, -5.0f)},
										{glm::vec3(0.f, 5.0f, 0.0f), glm::vec3(0.0f, -5.0f, 0.0f)},
										{glm::vec3(2.f, 2.0f, 2.0f), glm::vec3(-2.0f, -2.0f, -2.0f)}};
	line = new Line(lines);
	

	// std::vector<glm::vec3> lightColors;
	// std::vector<glm::vec3> lightDirs;
	// std::vector<int> isDirectional;
	// for (LightSource& light : newLightSources)
	// {
	// 	lightColors.push_back(light.color);
	// 	lightDirs.push_back(light.dir);
	// 	isDirectional.push_back((int)light.isDirectional);
	// }
	// worldShader->Bind();
	// worldShader->SetUniform1i("numLights", lightColors.size());
	// worldShader->SetUniform3fv("lightColors", lightColors);
	// worldShader->SetUniform3fv("lightDirs", lightDirs);
	// worldShader->SetUniform1iv("isDirectional", isDirectional);

	// load all the world objects and set up the world
	WorldObject* teapotObject = new WorldObject(worldShader, "/Users/puff/Developer/graphics/Volumetrics/res/models/teapot.obj", glm::vec3(-10.f, 0.f, 0.f), glm::vec3(0.f));
	wObjects.push_back(teapotObject);
}

void Space::tick(float delta, ButtonMap bm)
{
	time += delta;

	camera->tick(delta, bm);
	sun->tick(delta);

	// sphere1->pos.x = 13 * sin(time * 0.1f);
	// sphere1->pos.z = 13 * cos(time * 0.1f);

	sphere2->pos.x = 7 * sin(-time * 0.15f);
	sphere2->pos.z = 7 * cos(-time * 0.15f);


}

void Space::renderWorld(float delta)
{
	glm::mat4 view_matrix = camera->get_view_matrix();
	glm::vec3 cam_pos = camera->get_position();

	skybox->draw(proj, camera);
	sun->render(proj, camera);

	sun->render(proj, camera);

	water_surface->render(proj, view_matrix, cam_pos);

	for (WorldObject* o : wObjects)
	{
		o->tick(delta);
		o->draw(proj, view_matrix, o->getModelMatrix());
	}

	// raymarcher

	// std::cout << "x y z : " << cam_pos.x << " " << cam_pos.y << " " << cam_pos.z << std::endl;

	// voxel stuff
	vox->drawVoxels(proj, view_matrix);

	line->render(proj, view_matrix);
	// raymarcher->render(cam_pos, view_matrix, proj, delta, near, far);


}

void Space::change_fov(double xoffset, double yoffset) {
	fov -= (float)yoffset;
	proj = glm::perspective(glm::radians(fov), aspect_ratio, near, far);
}

void Space::update_projection_matrix_aspect_ratio(float aspectRatio) {
    if (aspectRatio <= 0) {
		aspectRatio = 16.f / 9.f;
	} else {
		aspect_ratio = aspectRatio;
	}
    change_fov(0.0, 0.0);
}

Camera* Space::get_camera() { 
	return camera; 
};




