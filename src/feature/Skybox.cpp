#include "Skybox.hpp"
#include "core/rendering/Renderer.hpp"

Skybox::Skybox() {

}

void Skybox::init(ResourceManager& resources) {
	shader = new Shader(resources.get_full_path("res://shaders/skybox.vs"), resources.get_full_path("res://shaders/skybox.fs"));
	
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

void Skybox::enqueue(Renderer& renderer, ResourceManager& resources)
{
	shader->hot_reload_if_changed();
	shader->bind();
	
	TextureBinding tex{};
	tex.id = skybox_tex;
	tex.target = GL_TEXTURE_CUBE_MAP;
	tex.unit = 5;
	tex.uniform_name = "u_texture";

	auto model_gpu = resources.get_model_gpu_data(r_model);
	if (model_gpu.meshes.empty() || model_gpu.meshes[0].primitives.empty()) {
		return;
	}
	const Primitive& primitive = model_gpu.meshes[0].primitives[0];

	RenderCommand cmd{};
	cmd.vao = primitive.vao;
	cmd.draw_type = primitive.index_count > 0 ? DrawType::Elements : DrawType::Arrays;
	cmd.count = primitive.index_count > 0 ? primitive.index_count : primitive.vertex_count;
	cmd.index_type = primitive.index_type;
	cmd.index_offset = primitive.index_byte_offset;
	cmd.shader     = shader;
	cmd.state.depth_test  = false;
	cmd.state.depth_write = false;

	cmd.textures.push_back(tex);

	renderer.submit(RenderPass::Skypass, cmd);
}

