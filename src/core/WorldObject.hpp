#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "core/OBJLoader.hpp"
#include "core/rendering/Texture.hpp"

#include <string>
#include "core/rendering/Shader.hpp"

class WorldObject {
protected:
	Shader* shader;
	ModelObject* model;
	Texture* texture;

	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale = glm::vec3(1.0f);

public:
	WorldObject(Shader* s, const std::string& modelPath, glm::vec3 pos, glm::vec3 rot);
	WorldObject(Shader* s, ModelObject* m, glm::vec3 pos, glm::vec3 rot);
    glm::vec3 getPosition() const { return position; };
    glm::vec3 getRotation() const { return rotation; };
    glm::vec3 getScale() const { return scale; };

    void setPosition(const glm::vec3& p);
    void setRotation(const glm::vec3& r);
    void setScale(const glm::vec3& s);

	virtual void tick(float deltaTime);
	virtual void draw(Renderer& renderer, glm::mat4 worldMatrix, glm::mat4 modelMatrix);
	glm::mat4 getModelMatrix();                               

	Shader* getShader();
};