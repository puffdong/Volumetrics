#include "WorldObject.h"

WorldObject::WorldObject(Shader* s, const std::string& modelPath, glm::vec3 pos, glm::vec3 rot)
{
	model = new ModelObject(modelPath);
	shader = s;
	position = pos;
	rotation = rot;
}

WorldObject::WorldObject(Shader* s, ModelObject* m, glm::vec3 pos, glm::vec3 rot) {
	model = m;
	shader = s;
	position = pos;
	rotation = rot;
}

void WorldObject::tick(float deltaTIme) {

}

void WorldObject::draw(glm::mat4 projMatrix, glm::mat4 worldMatrix, glm::mat4 modelMatrix) {
	shader->Bind();
	shader->SetUniformMat4("u_MVP", projMatrix * worldMatrix * modelMatrix);
	shader->SetUniformMat4("modelMatrix", modelMatrix);
	shader->SetUniformMat4("worldMatrix", worldMatrix);
	shader->SetUniform1i("u_Texture", 8);
	shader->SetUniform1f("textureScale", 1.f);
	model->render();
}

glm::vec3 WorldObject::getPosition() {
	return position;
}

glm::mat4 WorldObject::getModelMatrix() {
	return glm::translate(glm::mat4(1.f), position);
}

Shader* WorldObject::getShader() {
	return shader;
}