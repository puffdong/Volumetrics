#include "Shader.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <span>

Shader::Shader(const std::string& vertex_path, const std::string& fragment_path) 
: _rendering_id(0), _uniform_location_cache(), _uniform_block_index_cache(), _debug_output(false)
{
    _shader_name = std::filesystem::path(vertex_path).stem().string(); // vertex path is king, might make more robust later, we'll see
    if (_shader_name.empty()) _shader_name = "file not found ;_;";

    ShaderProgramSource source = parse_shader(vertex_path, fragment_path);
    _rendering_id = create_shader(source);
    if (_rendering_id == 0) {
        std::cout << "Shader could not be created: " << _shader_name << std::endl; 
    }

    _vertex_file = {vertex_path, std::filesystem::last_write_time(vertex_path), ShaderType::VERTEX};
    _fragment_file = {fragment_path, std::filesystem::last_write_time(fragment_path), ShaderType::FRAGMENT};
}

Shader::~Shader() {
    glDeleteProgram(_rendering_id);
}

ShaderProgramSource Shader::parse_shader(const std::string& vertex_path, const std::string& fragment_path) {
    std::string line;

    std::ifstream vertex_stream(vertex_path);
    std::stringstream v_source;
    while (getline(vertex_stream, line)) {
        v_source << line << '\n';
    }

    std::ifstream fragment_stream(fragment_path);
    std::stringstream f_source;
    while (getline(fragment_stream, line)) {
        f_source << line << '\n';
    }

    return { .vertex_source{v_source.str()}, .fragment_source{f_source.str()} };
}


unsigned int Shader::compile_shader(const std::string& source, unsigned int type) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result); // Get compilation info

    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "!!!: " << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") << " shader compilation failed!" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }
    return id;
}

unsigned int Shader::create_shader(const ShaderProgramSource& source) {
	unsigned int program = glCreateProgram();
	unsigned int vs = compile_shader(source.vertex_source, GL_VERTEX_SHADER);
	unsigned int fs = compile_shader(source.fragment_source, GL_FRAGMENT_SHADER);

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);
    int link_success;
    glGetProgramiv(program, GL_LINK_STATUS, &link_success);
    if (link_success == GL_FALSE) {
        int length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetProgramInfoLog(program, length, &length, message);
        std::cout << "!!!: Failed to link " << _shader_name << " program!" << std::endl;
        std::cout << message << std::endl;
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void Shader::bind() const {
    glUseProgram(_rendering_id);
}

void Shader::unbind() const {
    glUseProgram(0);
}

bool Shader::hot_reload_if_changed() {
    namespace fs = std::filesystem;
    bool changed = false;
    
    if (_vertex_file.last_write_time != fs::last_write_time(_vertex_file.file_path)) changed = true;
    if (_fragment_file.last_write_time != fs::last_write_time(_fragment_file.file_path)) changed = true;

    if (changed) {
        block_error_count = 0;
        _debug_output = true; // hot-reloading triggers debug output

        ShaderProgramSource source = parse_shader(_vertex_file.file_path, _fragment_file.file_path);
        unsigned int new_id = create_shader(source);

        if (new_id) {
            glDeleteProgram(_rendering_id);
            _rendering_id = new_id;
            _uniform_location_cache.clear();
            _uniform_block_index_cache.clear();
            
            _vertex_file.last_write_time = fs::last_write_time(_vertex_file.file_path);
            _fragment_file.last_write_time = fs::last_write_time(_fragment_file.file_path);
            std::cout << "hot-reloaded shader: " << _shader_name << '\n';
        }
        else {
            std::cout << "!!!: hot-reloading " << _shader_name << " failed!";
        }
    }
    
    return changed;
}

void Shader::set_uniform_float(const std::string& name, float f)
{
	glUniform1f(get_uniform_location(name), f);
}

void Shader::set_uniform_vec2(const std::string& name, const glm::vec2& vec) {
	glUniform2f(get_uniform_location(name), vec.x, vec.y);
}

void Shader::set_uniform_vec3(const std::string& name, const glm::vec3& vec) {
	glUniform3f(get_uniform_location(name), vec.x, vec.y, vec.z);
}

void Shader::set_uniform_vec4(const std::string& name, const glm::vec4& vec)
{
	glUniform4f(get_uniform_location(name), vec.x, vec.y, vec.z, vec.w);
}

void Shader::set_uniform_float_array(const std::string& name, std::span<const float> array)
{
	glUniform1fv(get_uniform_location(name), (GLint)array.size(), &array[0]);
}

void Shader::set_uniform_vec2_array(const std::string& name, std::span<const glm::vec2> array)
{
	glUniform2fv(get_uniform_location(name), (GLint)array.size(), &array[0].x);
}

void Shader::set_uniform_vec3_array(const std::string& name, std::span<const glm::vec3> array)
{
	glUniform3fv(get_uniform_location(name), (GLint)array.size(), &array[0].x);
}

void Shader::set_uniform_vec4_array(const std::string& name, std::span<const glm::vec4> array)
{
	glUniform4fv(get_uniform_location(name), (GLint)array.size(), &array[0].x);
}

void Shader::set_uniform_int(const std::string& name, int i)
{
	glUniform1i(get_uniform_location(name), i);
}

void Shader::set_uniform_ivec2(const std::string& name, const glm::ivec2& ivec) {
	glUniform2i(get_uniform_location(name), ivec.x, ivec.y);
}

void Shader::set_uniform_ivec3(const std::string& name, const glm::ivec3& ivec) {
	glUniform3i(get_uniform_location(name), ivec.x, ivec.y, ivec.z);
}

void Shader::set_uniform_int_array(const std::string& name, std::span<const int> array)
{
	glUniform1iv(get_uniform_location(name), (GLint)array.size(), &array[0]);
}

void Shader::set_uniform_mat3(const std::string& name, const glm::mat3& matrix) {
	glUniformMatrix3fv(get_uniform_location(name), 1, GL_FALSE, &matrix[0][0]);
}

void Shader::set_uniform_mat4(const std::string& name, const glm::mat4& matrix) {
	glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE, &matrix[0][0]);
}

void Shader::set_uniform_block(const std::string& block_name, unsigned int binding_point) {
	glUniformBlockBinding(_rendering_id, get_uniform_block_index(block_name), binding_point);
	if (_debug_output) {
		while (GLenum error = glGetError()) {
            block_error_count++;
            if (block_error_count > 5) break; // prevent spam
			std::cout << "[OpenGL Error] (" << error << ") <- prolly some block error in -> " << _shader_name << std::endl;
		}
	} else {
		glGetError(); // clear error queue silently
	}
}

int Shader::get_uniform_location(const std::string& uniform_name) {
	if (_uniform_location_cache.find(uniform_name) != _uniform_location_cache.end()) {
		return _uniform_location_cache[uniform_name];
	}

	int location = glGetUniformLocation(_rendering_id, uniform_name.c_str());
	if (location == -1 && _debug_output) {
		std::cout << "'" << uniform_name << "' in " << _shader_name << " doesn't exist!" << std::endl;
	}
	_uniform_location_cache[uniform_name] = location;
	return location;
}

int Shader::get_uniform_block_index(const std::string& block_name) {
	if (_uniform_block_index_cache.find(block_name) != _uniform_block_index_cache.end()) {
		return _uniform_block_index_cache[block_name];
	}

	int block_index = glGetUniformBlockIndex(_rendering_id, block_name.c_str());
	if (block_index == GL_INVALID_INDEX && _debug_output) {
		std::cout << "'" << block_name << "' block in " << _shader_name << " doesn't exist!" << std::endl;
	}

	_uniform_block_index_cache[block_name] = block_index;
	return block_index;
}
