#pragma once
#include "../OBJLoader.h"
#include "../core/rendering/Shader.h"
#include "../core/rendering/Texture.h"
#include "../core/Camera.h"

struct Skybox {
	ModelObject* model;
	Shader* shader;
	Texture* texture;

	Skybox(const std::string& modelPath, const std::string& shaderPath, const std::string& texturePath);

	void draw(glm::mat4 projMatrix, Camera* camera);
	void enqueue(RenderPass pass = RenderPass::Forward) const;
};