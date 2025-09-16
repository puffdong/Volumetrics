#pragma once
#include "core/OBJLoader.hpp"
#include "core/rendering/Shader.hpp"
#include "core/rendering/Texture.hpp"
#include "core/Camera.hpp"

struct Skybox {
	ModelObject* model;
	Shader* shader;
	Texture* texture;

	Skybox(const std::string& modelPath, const std::string& shaderPath, const std::string& texturePath);

	void draw(Renderer& renderer, glm::mat4 projMatrix, Camera* camera);
	void enqueue(Renderer& renderer, RenderPass pass = RenderPass::Forward) const;
	void update_static_uniforms(glm::mat4 proj, float near, float far);
};