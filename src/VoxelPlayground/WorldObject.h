#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "../OBJLoader.h"
#include "../Texture.h"

#include <string>
#include "../Shader.h"

class WorldObject {
protected:
	Shader* shader;
	ModelObject* model;
	Texture* texture;

	glm::vec3 position;
	glm::vec3 rotation;
	float specularExponent = 200.f;

public:
	WorldObject(Shader* s, const std::string& modelPath, glm::vec3 pos, glm::vec3 rot);
	WorldObject(Shader* s, ModelObject* m, glm::vec3 pos, glm::vec3 rot);
	glm::vec3 getPosition();
	virtual void tick(float deltaTime);
	virtual void draw(glm::mat4 projMatrix, glm::mat4 worldMatrix, glm::mat4 modelMatrix);
	glm::mat4 getModelMatrix();

	Shader* getShader();
};