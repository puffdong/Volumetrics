#pragma once
#include "core/Base.hpp"
#include "core/OBJLoader.hpp"
#include "core/rendering/Shader.hpp"
#include "core/rendering/Texture.hpp"
#include "core/Camera.hpp"

struct Skybox {
	Resource r_shader;
	ModelObject* model;
	Shader* shader;
	Texture* texture;

	Skybox(const std::string& modelPath, const std::string& shaderPath, const std::string& texturePath);
	
	void draw(Renderer& renderer, Camera* camera);
	void enqueue(Renderer& renderer, RenderPass pass = RenderPass::Forward) const;
};