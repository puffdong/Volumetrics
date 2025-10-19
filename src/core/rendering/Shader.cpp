#include "Shader.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.hpp"

Shader::Shader(const std::string& file_path) 
: _shader_files(), _rendering_id(0), _uniform_location_cache(), _uniform_block_index_cache()
{
    _shader_name = std::filesystem::path(file_path).filename().string();
    if (_shader_name.empty()) _shader_name = "file not found ;_;";

    ShaderProgramSource source = parse_shader(file_path);
    _rendering_id = create_shader(source);
    std::filesystem::file_time_type last_write_time = std::filesystem::last_write_time(file_path);
    
    _shader_files.push_back({file_path, last_write_time, ShaderType::NONE}); 

    std::cout << _shader_name << " successfully compiled!" << std::endl;
}

Shader::Shader(const std::string& vertex_path, const std::string& fragment_path) 
: _shader_files(), _rendering_id(0), _uniform_location_cache(), _uniform_block_index_cache()
{
    _shader_name = std::filesystem::path(vertex_path).stem().string(); // vertex path is king, might make more robust later, we'll see
    if (_shader_name.empty()) _shader_name = "file not found ;_;";

    std::cout << "Vertex path: " << vertex_path << std::endl;
    std::cout << "Fragment path: " << fragment_path << std::endl;

    ShaderProgramSource source = parse_shader(vertex_path, fragment_path);
    _rendering_id = create_shader(source);

    ShaderFile v{vertex_path, std::filesystem::last_write_time(vertex_path), ShaderType::VERTEX};
    ShaderFile f{fragment_path, std::filesystem::last_write_time(fragment_path), ShaderType::FRAGMENT};

    _shader_files.push_back(v);
    _shader_files.push_back(f);

    std::cout << _shader_name << " successfully compiled! Yay!" << std::endl;
}

Shader::~Shader() {
    glDeleteProgram(_rendering_id);
}

ShaderProgramSource Shader::parse_shader(const std::string& file_path) {
    std::ifstream stream(file_path);

    std::string line;
    std::stringstream ss[2]; // two types of shaders (hard coded slop heuheu)
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos) {
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos) {
                type = ShaderType::FRAGMENT;
            }
        }
        else {
            if (type != ShaderType::NONE) 
                {
                    ss[(int)type] << line << '\n';
                }
        }
    }

    return { ss[0].str(), ss[1].str() }; // ShaderProgramSource struct
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

// unsigned int Shader::create_shader(const std::string& vertexShader, const std::string& fragmentShader) {
unsigned int Shader::create_shader(ShaderProgramSource source) {
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

void Shader::hot_reload_if_changed() {
    namespace fs = std::filesystem;

    // this is hacky but bear with me okay?

    // if only 1 file, then we got the old .shader format
    if (_shader_files.size() == 1) { 
        fs::file_time_type now = fs::last_write_time(_shader_files[0].file_path);
        if (now == _shader_files[0].last_write_time) return; // nothing changed

        ShaderProgramSource source = parse_shader(_shader_files[0].file_path);
        unsigned int new_id = create_shader(source);

        if (new_id) {
            glDeleteProgram(_rendering_id);
            _rendering_id = new_id;
            _uniform_location_cache.clear();
            _shader_files[0].last_write_time = now;
            std::cout << "hot-reloaded " << _shader_name << std::endl;;
        }
        else {
            std::cout << "!!: hot-reloading " << _shader_name << " failed!";
        }
    }
    // if there are more, then we know there are at least two files to look through
    else {
        // check both the vertex and fragment timestamps
        bool changed = false;
        for (ShaderFile file : _shader_files) {
            fs::file_time_type now = fs::last_write_time(file.file_path);
            if (now != file.last_write_time) {
                changed = true; // ding ding!

            }
            
        }

        if (changed) {
            ShaderProgramSource source = parse_shader(_shader_files[0].file_path, _shader_files[1].file_path); // [0] -> vertex, [1] -> fragment 
            unsigned int new_id = create_shader(source);

            if (new_id) {
                glDeleteProgram(_rendering_id);
                _rendering_id = new_id;
                _uniform_location_cache.clear();
                
                _shader_files[0].last_write_time = fs::last_write_time(_shader_files[0].file_path);
                _shader_files[1].last_write_time = fs::last_write_time(_shader_files[1].file_path);
                std::cout << "hot-reloaded shader: " << _shader_name << '\n';
            }
            else {
                std::cout << "!!!: hot-reloading " << _shader_name << " failed!";
            }
        }
    }
    // we did some major work on this shader class this evening, but I am at my wits end
    // refactoring this won't be a pain, surely :))
}

void Shader::set_uniform_float(const std::string& name, float f)
{
    GLCall(glUniform1f(get_uniform_location(name), f));
}

void Shader::set_uniform_vec2(const std::string& name, glm::vec2 vec) {
    GLCall(glUniform2f(get_uniform_location(name), vec.x, vec.y));
}

void Shader::set_uniform_vec3(const std::string& name, glm::vec3 vec) {
    GLCall(glUniform3f(get_uniform_location(name), vec.x, vec.y, vec.z));
}

void Shader::set_uniform_vec4(const std::string& name, glm::vec4 vec)
{
    GLCall(glUniform4f(get_uniform_location(name), vec.x, vec.y, vec.z, vec.w));
}

void Shader::set_uniform_float_array(const std::string& name, std::vector<float> array)
{
    GLCall(glUniform1fv(get_uniform_location(name), (GLint)array.size(), &array[0]));
}

void Shader::set_uniform_vec2_array(const std::string& name, std::vector<glm::vec2> array)
{
    GLCall(glUniform2fv(get_uniform_location(name), (GLint)array.size(), &array[0].x));
}

void Shader::set_uniform_vec3_array(const std::string& name, std::vector<glm::vec3> array)
{
    GLCall(glUniform3fv(get_uniform_location(name), (GLint)array.size(), &array[0].x));
}

void Shader::set_uniform_vec4_array(const std::string& name, std::vector<glm::vec4> array)
{
    GLCall(glUniform3fv(get_uniform_location(name), (GLint)array.size(), &array[0].x));
}

void Shader::set_uniform_int(const std::string& name, int i)
{
    GLCall(glUniform1i(get_uniform_location(name), i));
}

void Shader::set_uniform_ivec2(const std::string& name, glm::ivec2 ivec) {
    GLCall(glUniform2i(get_uniform_location(name), ivec.x, ivec.y));
}

void Shader::set_uniform_ivec3(const std::string& name, glm::ivec3 ivec) {
    GLCall(glUniform3i(get_uniform_location(name), ivec.x, ivec.y, ivec.z));
}

void Shader::set_uniform_int_array(const std::string& name, std::vector<int> array)
{
    GLCall(glUniform1iv(get_uniform_location(name), (GLint)array.size(), &array[0]));
}

void Shader::set_uniform_mat3(const std::string& name, const glm::mat3& matrix) {
    GLCall(glUniformMatrix3fv(get_uniform_location(name), 1, GL_FALSE, &matrix[0][0]));
}

void Shader::set_uniform_mat4(const std::string& name, const glm::mat4& matrix) {
    GLCall(glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE, &matrix[0][0]));
}

void Shader::set_uniform_block(const std::string& block_name, unsigned int binding_point) {
    GLCall(glUniformBlockBinding(_rendering_id, get_uniform_block_index(block_name), binding_point));
}

int Shader::get_uniform_location(const std::string& uniform_name) {
    if (_uniform_location_cache.find(uniform_name) != _uniform_location_cache.end()) {
        return _uniform_location_cache[uniform_name];
    }

    int location = glGetUniformLocation(_rendering_id, uniform_name.c_str());
    if (location == -1) {
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
    if (block_index == GL_INVALID_INDEX) {
        std::cout << "'" << block_name << "' block in " << _shader_name << " doesn't exist!" << std::endl;
    }

    _uniform_block_index_cache[block_name] = block_index;
    return block_index;
}
