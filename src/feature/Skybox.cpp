#include "Skybox.hpp"

#include "core/rendering/Renderer.hpp"

Skybox::Skybox(const std::string& modelPath, const std::string& shaderPath, const std::string& texturePath)
{
	model = new ModelObject(modelPath);
	shader = new Shader(shaderPath);
	texture = new Texture(texturePath);

	texture->Bind(0); // we gotta set up the parameters for this texture asap, so do it now and make it better later
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	texture->Unbind();
}

void Skybox::draw(Renderer& renderer, glm::mat4 projMatrix, Camera* camera)
{
	shader->Bind();
	glm::mat4 modelTrans = glm::scale(glm::translate(glm::mat4(1.f), camera->get_position()), glm::vec3(5.f, 5.f, 5.f));
	glm::mat4 mvp = projMatrix * camera->get_view_matrix() * modelTrans;
    shader->SetUniformMat4("u_MVP", projMatrix * camera->get_view_matrix());

	TextureBinding tex{};
	tex.id = texture->get_id();
	tex.target = GL_TEXTURE_2D;
	tex.unit = 0;
	tex.uniform_name = "u_Texture";

    RenderCommand cmd{};
    cmd.vao        = model->getVAO();
    cmd.draw_type   = DrawType::Elements;
    cmd.count      = model->getIndexCount();
	cmd.model      = mvp;
    cmd.shader     = shader;
	cmd.state.depth_test  = false;
    cmd.state.depth_write = false;

    cmd.textures.push_back(tex);

    renderer.submit(RenderPass::Skypass, cmd);
}

void Skybox::enqueue(Renderer& renderer, RenderPass pass) const
{	
	TextureBinding tex{};
	tex.id = texture->get_id();
	tex.target = GL_TEXTURE_2D;
	tex.unit = 0;
	tex.uniform_name = "u_Texture";

    RenderCommand cmd{};
    cmd.vao        = model->getVAO();
    cmd.draw_type   = DrawType::Elements;
    cmd.count      = model->getIndexCount();
    cmd.shader     = shader;
	cmd.state.depth_test  = false;
    cmd.state.depth_write = false;

    cmd.textures.push_back(tex);

    renderer.submit(RenderPass::Forward, cmd);
}

void Skybox::update_static_uniforms(glm::mat4 proj, float near, float far) {

}

