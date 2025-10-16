#include "Skybox.hpp"
#include "core/space/Space.hpp"
#include "core/rendering/Renderer.hpp"

Skybox::Skybox() : Base()
{}

void Skybox::init(ResourceManager& resources, Space* space) {
	Base::init(resources, space);
	r_shader = resources.load_shader("res://shaders/Skybox.shader");

	model = new ModelObject(resources.get_full_path("res://models/skybox-full-tweaked.obj"));
	texture = new Texture(resources.get_full_path("res://textures/skybox/cloud-landscape.tga"));

	texture->Bind(0); // we gotta set up the parameters for this texture asap, so do it now and make it better later
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	texture->Unbind();

}

void Skybox::enqueue(Renderer& renderer, ResourceManager& resources)
{
	if (auto shader = resources.get_shader(r_shader.id)) {
		auto camera = _space->get_camera();
		glm::mat4 modelTrans = glm::scale(glm::translate(glm::mat4(1.f), camera->get_position()), glm::vec3(5.f, 5.f, 5.f));
		glm::mat4 mvp = renderer.get_proj() * camera->get_view_matrix() * modelTrans;
		
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
		cmd.shader     = (*shader);
		cmd.state.depth_test  = false;
		cmd.state.depth_write = false;

		cmd.textures.push_back(tex);

		renderer.submit(RenderPass::Skypass, cmd);
		
		(*shader)->bind();
		(*shader)->set_uniform_mat4("u_MVP", renderer.get_proj() * renderer.get_view());
		
	}

}

