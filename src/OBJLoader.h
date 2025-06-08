#pragma once
#include <GL/glew.h>
#include <string>
#include <vector>
#include "glm/glm.hpp"
#include "LoadTGA.h"

class ModelObject {
private:
	// Store the VAO and other related information in your ModelObject class, assuming you have the following member variables:
	GLuint m_VAO;
	GLuint m_VBO;
	GLuint m_EBO;
	GLsizei m_indexCount;
	std::vector<glm::vec3> m_vertexArray;
	std::vector<glm::vec3> m_normalArray;
	std::vector<glm::vec2> m_texCoordArray;

public:
	unsigned int numIndices;

	ModelObject(const std::string& filepath);
	ModelObject(float widht, float depth);
	ModelObject(float width, float depth, int numRows, int numCols);
	ModelObject(float width, float depth, float height, TextureData* mapTexture);
	~ModelObject();
	void loadThroughTiny(const std::string& filepath);

	void render();

	GLuint getVAO();
	GLsizei getIndexCount() const;
	void Bind() const;
	void Unbind() const;
	void createFlatGround(float width, float depth);
	void createFlatGround(float width, float depth, int numRows, int numCols);
	void createHeightmap(float width, float depth, float height, TextureData* mapTexture);
	std::vector<glm::vec3> getVertexArray() { return m_vertexArray; }
	std::vector<glm::vec3> getNormalArray() { return m_normalArray; }
	std::vector<glm::vec2> getTexCoordArray() { return m_texCoordArray; }
};