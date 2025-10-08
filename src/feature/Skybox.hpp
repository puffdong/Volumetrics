#pragma once
#include "core/Base.hpp"
#include "core/OBJLoader.hpp"
#include "core/rendering/Texture.hpp"
#include "core/Camera.hpp"

class Space; //fwd decl

struct Skybox : public Base {
	Resource r_shader;
	ModelObject* model;
	Texture* texture;

	Skybox();
	
	void init(ResourceManager& resources, Space* space) override;
    void enqueue(Renderer& renderer, ResourceManager& resources) override;
};