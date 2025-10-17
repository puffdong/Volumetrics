#pragma once

#include "Renderer.hpp"
#include "core/LoadTGA.hpp"

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

	void bind(unsigned int slot = 0) const;
	void unbind() const;

	int get_id();

	inline int get_width() const { return m_Width; }
	inline int get_height() const { return m_Height; }
};