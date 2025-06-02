#include "Space.h"
#include <iostream>

Space::Space()
{
	// player->setPosition(playerStartPos);

	glm::vec3 cameraDir(1.f, 0.f, 0.f);
	camera = new Camera(cameraDir);

	water_surface = new WaterSurface(glm::vec3(5.f, -10.f, 5.f), 20.f, 20.f);
	// sun = new Sun(glm::vec3(1.f, 0.0f, 0.0f));

	// voxel stuff -> in progress 
	vox = new VoxelStructure(10, 25, 7, glm::vec3(-20, 1, 1), 1, 0.5f);
	vox->setVoxelValue(2, 2, 2, 3);
	std::cout << vox->getVoxelValue(2, 2, 2) << std::endl;
	std::cout << vox->getVoxelValue(2, 2, 3) << std::endl;

	// raymarcher
	ray_scene = new RayScene(glm::vec3(0.0, 0.0, 0.0));
	raymarcher = new Raymarcher(ray_scene);
	sphere1 = ray_scene->add_sphere(glm::vec3(10.0f, 0.0f, 0.0f), 5.0f, glm::vec4(1.0, 0.4, 0.1, 0.8));
	sphere2 = ray_scene->add_sphere(glm::vec3(-10.0f, 0.0f, 0.0f), 3.0f, glm::vec4(1.0, 0.4, 0.1, 0.8));

	loadLevel1();
}

void Space::tick(float delta, ButtonMap bm)
{
	time += delta;

	camera->tick(delta, bm);

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

	// sun->render(camera, proj);

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

	raymarcher->render(cam_pos, view_matrix, proj, delta, near, far);


}

void Space::change_fov(double xoffset, double yoffset) {
	fov -= (float)yoffset;
	proj = glm::perspective(glm::radians(fov), 16.f / 9.0f, 1.0f, 256.0f);
}

void Space::loadLevel1()
{
	skybox = new Skybox(
		std::string("C:/Dev/OpenGL/volumetrics/res/models/skybox-full-tweaked.obj"),
		std::string("C:/Dev/OpenGL/volumetrics/res/shaders/Skybox.shader"),
		std::string("C:/Dev/OpenGL/volumetrics/res/textures/skybox/cloud-landscape.tga")
	);


	// Setup shader with lighting
	Shader* worldShader = new Shader("C:/Dev/OpenGL/volumetrics/res/shaders/WorldObject.shader");
	LightSource newLightSources[] = {
				LightSource(glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, 1.f, 0.f), true)
	};

	std::vector<glm::vec3> lightColors;
	std::vector<glm::vec3> lightDirs;
	std::vector<int> isDirectional;
	for (LightSource& light : newLightSources)
	{
		lightColors.push_back(light.color);
		lightDirs.push_back(light.dir);
		isDirectional.push_back((int)light.isDirectional);
	}
	worldShader->Bind();
	worldShader->SetUniform1i("numLights", lightColors.size());
	worldShader->SetUniform3fv("lightColors", lightColors);
	worldShader->SetUniform3fv("lightDirs", lightDirs);
	worldShader->SetUniform1iv("isDirectional", isDirectional);

	// load all the world objects and set up the world
	WorldObject* teapotObject = new WorldObject(worldShader, "C:/Dev/OpenGL/volumetrics/res/models/teapot.obj", glm::vec3(-10.f, 0.f, 0.f), glm::vec3(0.f));
	wObjects.push_back(teapotObject);
}




