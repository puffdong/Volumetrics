#pragma once
#include "../OBJLoader.h"
#include "../Shader.h"
#include "../Texture.h"
#include "Camera.h"

struct Skybox {
	ModelObject* model;
	Shader* shader;
	Texture* texture;

	Skybox(const std::string& modelPath, const std::string& shaderPath, const std::string& texturePath);

	void draw(glm::mat4 projMatrix, Camera* camera);
};