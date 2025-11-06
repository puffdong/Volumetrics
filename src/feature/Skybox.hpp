#pragma once
#include "core/Base.hpp"
#include "core/rendering/Texture.hpp"

class Space; //fwd decl
class ModelObject;

struct Skybox : public Base {
	Resource r_shader;
	ModelObject* model;
	Texture* texture;

	GLuint skybox_tex;

	Skybox();
	
	void init(ResourceManager& resources, Space* space) override;
    void enqueue(Renderer& renderer, ResourceManager& resources) override;
};