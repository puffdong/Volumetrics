#include "Texture.h"
#include <iostream>

#include "Renderer.h"

Texture::Texture(const std::string& path)
	: m_RendererID(0), m_FilePath(path), m_Width(0), m_Height(0), m_BPP(0)
{
	GLCall(glGenTextures(1, &m_RendererID));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));

	bool ok = LoadTGATextureData(path.c_str(), &tgaData);
	if (!ok) {
		std::cout << "fuck" << std::endl;
	}

	m_Width = tgaData.width;
	m_Height = tgaData.height;
	m_BPP = tgaData.bpp;

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tgaData.width, tgaData.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tgaData.imageData));
	GLCall(glBindTexture(GL_TEXTURE_2D, 0)); // unbind in the end
}


Texture::~Texture() {

}

void Texture::Bind(unsigned int slot) const {
	GLCall(glActiveTexture(GL_TEXTURE0 + slot));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
}

void Texture::Unbind() const {
	glBindTexture(GL_TEXTURE_2D, 0);
}

