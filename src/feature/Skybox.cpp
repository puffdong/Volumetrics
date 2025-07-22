#include "Skybox.h"

#include "../core/rendering/Renderer.h"

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
	glm::mat4 modelTrans = glm::scale(glm::translate(glm::mat4(1.f), camera->get_position()), glm::vec3(5.f, 5.f, 5.f));
	glm::mat4 mvp = projMatrix * camera->get_view_matrix() * modelTrans;
	shader->SetUniformMat4("u_MVP", mvp);

	// To draw the skybox it need special treatment to avoid drawing it wrong
	GLCall(glDisable(GL_DEPTH_TEST));
	glDepthMask(GL_FALSE);
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	model->render();
	GLCall(glEnable(GL_DEPTH_TEST));
	glDepthMask(GL_TRUE);
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

	// START HERE

	shader->Bind();
	glm::mat4 modelTrans = glm::scale(glm::translate(glm::mat4(1.f), camera->get_position()), glm::vec3(5.f, 5.f, 5.f));
	glm::mat4 mvp = projMatrix * camera->get_view_matrix() * modelTrans;
    shader->SetUniformMat4("u_MVP", projMatrix * camera->get_view_matrix());

	enqueue()


}

void Skybox::enqueue(RenderPass pass) const
{
    RenderCommand cmd{};
    cmd.vao        = model->getVAO();
    cmd.draw_type   = DrawType::Elements;
    cmd.count      = model->getIndexCount();
    cmd.shader     = shader;
    cmd.state.depth_test  = false;
    cmd.state.depth_write = false;

    cmd.textures.push_back({ texture->get_id(), GL_TEXTURE_CUBE_MAP, 0, "u_Texture" });

    Renderer::Submit(RenderPass::Forward, cmd);
}

