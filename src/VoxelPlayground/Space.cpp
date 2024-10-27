#include "Space.h"
#include <iostream>

Space::Space()
{
	// player->setPosition(playerStartPos);

	glm::vec3 cameraDir(0.f, 0.f, 1.f);
	camera = new Camera(cameraDir);

	vox = new VoxelStructure(10, 25, 7, glm::ivec3(1, 1, 1), 1, 0.5f);
	vox->setVoxelValue(2, 2, 2, 3);
	std::cout << vox->getVoxelValue(2, 2, 2) << std::endl;
	std::cout << vox->getVoxelValue(2, 2, 3) << std::endl;

	loadLevel1();
}

void Space::tick(float delta, ButtonMap bm)
{
	camera->tick(delta, bm);
}

void Space::renderWorld(float delta)
{
	glm::mat4 viewMatrix = camera->getLookAt();

	skybox->draw(proj, camera);

	for (WorldObject* o : wObjects)
	{
		o->tick(delta);
		o->draw(proj, viewMatrix, o->getModelMatrix());
	}

	vox->drawVoxels(proj, viewMatrix);
}

void Space::loadLevel1()
{
	skybox = new Skybox(
		std::string("C:/Dev/OpenGL/Volumetrics/res/models/skybox-full-tweaked.obj"),
		std::string("C:/Dev/OpenGL/Volumetrics/res/shaders/Skybox.shader"),
		std::string("C:/Dev/OpenGL/Volumetrics/res/textures/skybox/cloud-landscape.tga")
	);


	// Setup shader with lighting
	Shader* worldShader = new Shader("C:/Dev/OpenGL/Volumetrics/res/shaders/WorldObject.shader");
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
	WorldObject* teapotObject = new WorldObject(worldShader, "C:/Dev/OpenGL/Volumetrics/res/models/teapot.obj", glm::vec3(-10.f, 0.f, 0.f), glm::vec3(0.f));
	wObjects.push_back(teapotObject);
}




