#include "Skybox.h"

#include "../Renderer.h"

Skybox::Skybox(const std::string& modelPath, const std::string& shaderPath, const std::string& texturePath)
{
	model = new ModelObject(modelPath);
	shader = new Shader(shaderPath);
	texture = new Texture(texturePath);
}

void Skybox::draw(glm::mat4 projMatrix, Camera* camera)
{
	shader->Bind();
	texture->Bind(0);
	shader->SetUniform1i("u_Texture", 0);
	glm::mat4 modelTrans = glm::translate(glm::mat4(1.f), camera->getPosition());
	glm::mat4 mvp = projMatrix * camera->getLookAt() * modelTrans;
	shader->SetUniformMat4("u_MVP", mvp);

	// To draw the skybox it need special treatment to avoid drawing it wrong
	GLCall(glDisable(GL_DEPTH_TEST));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	model->render();
	GLCall(glEnable(GL_DEPTH_TEST));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
}
