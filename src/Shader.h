#pragma once
#include <string>
#include <GL/glew.h>
#include <unordered_map>
#include "glm/glm.hpp"

struct ShaderProgramSource {
	std::string VertexSource;
	std::string FragmentSource;
};

class Shader
{
private:
	std::string m_FilePath;
	unsigned int m_RendererID;
	// caching for uniforms
	std::unordered_map<std::string, int> m_UniformLocationCache;
public:
	Shader(const std::string& filepath);
	~Shader();

	void Bind() const;
	void Unbind() const;
	unsigned int GetRendererID() const;

	// Set uniforms
	void SetUniform1f(const std::string& name, GLfloat v0);
	void SetUniform4f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
	void SetUniform3fv(const std::string& name, std::vector<glm::vec3> inVector);
	void SetUniform1i(const std::string& name, GLint i);
	void SetUniform1iv(const std::string& name, std::vector<int> inVector);
	void SetUniformMat4(const std::string& name, const glm::mat4& matrix);
	void SetUniform3f(const std::string& name, glm::vec3 vec);


private:
	ShaderProgramSource ParseShader(const std::string& filepath);
	unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
	unsigned int CompileShader(const std::string& source, unsigned int type);
	int GetUniformLocation(const std::string& name);
};