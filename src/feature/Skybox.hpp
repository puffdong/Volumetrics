#pragma once
#include "core/resources/ResourceManager.hpp"
#include "core/rendering/Texture.hpp"

class Space; //fwd decl

struct Skybox {
	Resource r_shader;
	Res::Model r_model;
	
	Texture* texture;

	GLuint skybox_tex;

	Skybox();
	
	void init(ResourceManager& resources);
    void enqueue(Renderer& renderer, ResourceManager& resources, glm::vec3 camera_pos);
};