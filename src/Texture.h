#pragma once

#include "Renderer.h"
#include "LoadTGA.h"

class Texture
{
private:
	unsigned int m_RendererID;
	std::string m_FilePath;
	int m_Width, m_Height, m_BPP;

public:
	TextureData tgaData;
	Texture(const std::string& path);
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline int GetWidth() const { return m_Width; }
	inline int GetHeight() const { return m_Height; }
};