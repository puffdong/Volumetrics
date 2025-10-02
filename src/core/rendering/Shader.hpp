#pragma once
#include <GL/glew.h>
#include <string>
#include <filesystem>  
#include <unordered_map>
#include "glm/glm.hpp"

struct ShaderProgramSource {
	std::string VertexSource;
	std::string FragmentSource;
};

class Shader
{
private:
	std::string _file_path;
	unsigned int _rendering_id;
	std::filesystem::file_time_type _last_write_time;
	std::unordered_map<std::string, int> uniform_location_cache;
public:
	Shader(const std::string& file_path);
	~Shader();

	void bind() const;
	void unbind() const;
	void hot_reload_if_changed();
	unsigned int get_renderer_id() const { return _rendering_id; };

	// Set uniforms
	void SetUniform1f(const std::string& name, GLfloat v0);
	void SetUniform4f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
	void SetUniform4fv(const std::string& name, std::vector<glm::vec4> inVector);
	void SetUniform3fv(const std::string& name, std::vector<glm::vec3> inVector);
	void SetUniform2fv(const std::string& name, std::vector<glm::vec2> inVector);
	void SetUniform1fv(const std::string& name, std::vector<float> inVector);
	void SetUniform1i(const std::string& name, GLint i);
	void SetUniform3i(const std::string& name, glm::ivec3 ivec);
	void SetUniform1iv(const std::string& name, std::vector<int> inVector);
	void SetUniformMat4(const std::string& name, const glm::mat4& matrix);
	void SetUniform3f(const std::string& name, glm::vec3 vec);
	void SetUniform2f(const std::string& name, glm::vec2 vec);


private:
	ShaderProgramSource parse_shader(const std::string& file_path);
	unsigned int create_shader(const std::string& vertexShader, const std::string& fragmentShader);
	unsigned int compile_shader(const std::string& source, unsigned int type);
	int get_uniform_location(const std::string& name);
};