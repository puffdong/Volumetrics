#include "Skybox.hpp"
#include "core/OBJLoader.hpp"
#include "core/rendering/Renderer.hpp"

Skybox::Skybox() {

}

void Skybox::init(ResourceManager& resources) {
	r_shader = resources.load_shader("res://shaders/skybox.vs", "res://shaders/skybox.fs");
	r_model = resources.load_model("res://models/skybox-full-tweaked.obj");

	TextureData top;
	LoadTGATextureData(resources.get_full_path("res://textures/skybox/top.tga").c_str(), &top);
	TextureData bottom;
	LoadTGATextureData(resources.get_full_path("res://textures/skybox/bottom.tga").c_str(), &bottom);
	TextureData middle;
	LoadTGATextureData(resources.get_full_path("res://textures/skybox/middle.tga").c_str(), &middle);
	TextureData leftmost;
	LoadTGATextureData(resources.get_full_path("res://textures/skybox/leftmost.tga").c_str(), &leftmost);
	TextureData rightmost;
	LoadTGATextureData(resources.get_full_path("res://textures/skybox/rightmost.tga").c_str(), &rightmost);
	TextureData rightofmiddle;
	LoadTGATextureData(resources.get_full_path("res://textures/skybox/rightofmiddle.tga").c_str(), &rightofmiddle);
	
	TextureData faces[6] = { rightofmiddle, leftmost, bottom, top, middle, rightmost };

	glGenTextures(1, &skybox_tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex);

	for (int i = 0; i < 6; ++i) {
		auto& img = faces[i];
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, img.width, img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.imageData);
		free(img.imageData);
	}

	// filtering / wrapping
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Skybox::enqueue(Renderer& renderer, ResourceManager& resources, glm::vec3 camera_pos)
{
	if (auto shader = resources.get_shader(r_shader.id)) {
		glm::mat4 model_matrix = glm::scale(glm::translate(glm::mat4(1.f), camera_pos), glm::vec3(5.f, 5.f, 5.f));
		glm::mat4 mvp = renderer.get_proj() * renderer.get_view() * model_matrix;

		(*shader)->hot_reload_if_changed();
		(*shader)->bind();
		(*shader)->set_uniform_mat4("u_proj", renderer.get_proj());
		(*shader)->set_uniform_mat4("u_view", renderer.get_view());
		
		TextureBinding tex{};
		tex.id = skybox_tex;
		tex.target = GL_TEXTURE_CUBE_MAP;
		tex.unit = 5;
		tex.uniform_name = "u_texture";

		ModelGpuData model_gpu = resources.get_model_gpu_data(r_model.id);

		RenderCommand cmd{};
		cmd.vao        = model_gpu.vao;
		cmd.draw_type   = DrawType::Elements;
		cmd.count      = model_gpu.index_count;
		cmd.shader     = (*shader);
		cmd.state.depth_test  = false;
		cmd.state.depth_write = false;

		cmd.textures.push_back(tex);

		renderer.submit(RenderPass::Skypass, cmd);
		

		
	}

}

