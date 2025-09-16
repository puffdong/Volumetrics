#include "WorldObject.hpp"

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

void WorldObject::draw(Renderer& renderer, glm::mat4 projMatrix, glm::mat4 worldMatrix, glm::mat4 modelMatrix) {
	shader->HotReloadIfChanged();
	shader->Bind();
	shader->SetUniformMat4("u_MVP", projMatrix * worldMatrix * modelMatrix);
	shader->SetUniformMat4("modelMatrix", modelMatrix);
	shader->SetUniformMat4("worldMatrix", worldMatrix);

	RenderCommand cmd{};
    cmd.vao        = model->getVAO();
    cmd.draw_type   = DrawType::Elements;
    cmd.count      = model->getIndexCount();
	cmd.model      = modelMatrix; // swap this tbh, doesn't even make any semantic sense, the mvp is different from modelmtrx
    cmd.shader     = shader;

    renderer.submit(RenderPass::Forward, cmd);
}

void WorldObject::setPosition(const glm::vec3& p) { position = p; }

void WorldObject::setRotation(const glm::vec3& r) { rotation = r; } 

void WorldObject::setScale   (const glm::vec3& s) { scale = s; }

glm::mat4 WorldObject::getModelMatrix() {
	glm::mat4 m(1.0f);
    // Translate
    m = glm::translate(m, position);
    // Rotate (angles are in RADIANS)
    m = glm::rotate(m, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)); // roll
    m = glm::rotate(m, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)); // yaw
    m = glm::rotate(m, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)); // pitch
    // Scale
    m = glm::scale(m, scale);
    return m;
}

Shader* WorldObject::getShader() {
	return shader;
}