#pragma once
// OpenGL
#include <GL/glew.h>

// std
#include <string>
#include <filesystem>  
#include <unordered_map>

// glm
#include "glm/glm.hpp"

enum class ShaderType {
	NONE = -1, 
	VERTEX = 0, 
	FRAGMENT = 1
};

struct ShaderFile {
	std::string file_path;
	std::filesystem::file_time_type last_write_time;
	ShaderType type; // NONE means it is a .shader file!
};

struct ShaderProgramSource {
	std::string vertex_source;
	std::string fragment_source;
};

class Shader {
private:
	std::string _shader_name; // name derived from filepath
	std::vector<ShaderFile> _shader_files;
	std::unordered_map<std::string, int> _uniform_location_cache;
	std::unordered_map<std::string, int> _uniform_block_index_cache;
	unsigned int _rendering_id; // opengl program id
	
public:
	Shader(const std::string& file_path);
	Shader(const std::string& vertex_path, const std::string& fragment_path);
	~Shader();

	void bind() const;
	void unbind() const;
	void hot_reload_if_changed();
	unsigned int get_renderer_id() const { return _rendering_id; };

	// uniform stuff
	void set_uniform_float(const std::string& name, float f);
	void set_uniform_vec2(const std::string& name, glm::vec2 vec);
	void set_uniform_vec3(const std::string& name, glm::vec3 vec);
	void set_uniform_vec4(const std::string& name, glm::vec4 vec);

	void set_uniform_float_array(const std::string& name, std::vector<float> array);
	void set_uniform_vec2_array(const std::string& name, std::vector<glm::vec2> array);
	void set_uniform_vec3_array(const std::string& name, std::vector<glm::vec3> array);
	void set_uniform_vec4_array(const std::string& name, std::vector<glm::vec4> array);

	void set_uniform_int(const std::string& name, int i);
	void set_uniform_ivec2(const std::string& name, glm::ivec2 ivec);
	void set_uniform_ivec3(const std::string& name, glm::ivec3 ivec);

	void set_uniform_int_array(const std::string& name, std::vector<int> array);

	void set_uniform_mat3(const std::string& name, const glm::mat3& matrix);
	void set_uniform_mat4(const std::string& name, const glm::mat4& matrix);

	void set_uniform_block(const std::string& block_name, unsigned int binding_point);

private:
	ShaderProgramSource parse_shader(const std::string& file_path);
	ShaderProgramSource parse_shader(const std::string& vertex_path, const std::string& fragment_path);

	unsigned int create_shader(ShaderProgramSource source);
	unsigned int compile_shader(const std::string& source, unsigned int type);
	int get_uniform_location(const std::string& name);
	int get_uniform_block_index(const std::string& block_name);
};