#include "Shader.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.hpp"

Shader::Shader(const std::string& filepath) : _file_path(filepath), _rendering_id(0), uniform_location_cache()
{
    ShaderProgramSource source = parse_shader(filepath);
    _rendering_id = create_shader(source.VertexSource, source.FragmentSource);
    _last_write_time = std::filesystem::last_write_time(filepath);
    std::cout << "Shader compiled: " << filepath << std::endl;
}

Shader::~Shader() {
    GLCall(glDeleteProgram(_rendering_id));
}

ShaderProgramSource Shader::parse_shader(const std::string& filepath) {
    std::ifstream stream(filepath);

    enum class ShaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2]; // two types of shaders
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
            {
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos)
            {
                type = ShaderType::FRAGMENT;
            }
        }
        else
        {
            if (type != ShaderType::NONE) 
                {
                    ss[(int)type] << line << '\n';
                }
        }
    }

    return { ss[0].str(), ss[1].str() }; // ShaderProgramSource struct
}


unsigned int Shader::compile_shader(const std::string& source, unsigned int type) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str(); // Pointer to beginning of data
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result); // Get compilation info

    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed " << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") << " shader compilation!" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }
    return id;
}

unsigned int Shader::create_shader(const std::string& vertexShader, const std::string& fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = compile_shader(vertexShader, GL_VERTEX_SHADER);
    unsigned int fs = compile_shader(fragmentShader, GL_FRAGMENT_SHADER);

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);
    int linkSuccess;
    glGetProgramiv(program, GL_LINK_STATUS, &linkSuccess);
    if (linkSuccess == GL_FALSE) {
        int length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetProgramInfoLog(program, length, &length, message);
        std::cout << "Failed to link shader program!" << _file_path << std::endl;
        std::cout << message << std::endl;
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void Shader::bind() const {
    GLCall(glUseProgram(_rendering_id));
}

void Shader::unbind() const {
    glUseProgram(0);
}

void Shader::hot_reload_if_changed()
{
    namespace fs = std::filesystem;
    fs::file_time_type now = fs::last_write_time(_file_path);
    if (now == _last_write_time) return;

    ShaderProgramSource s = parse_shader(_file_path);
    unsigned int newID = create_shader(s.VertexSource, s.FragmentSource);

    if (newID) {
        glDeleteProgram(_rendering_id);
        _rendering_id = newID;
        uniform_location_cache.clear();
        _last_write_time = now;
        std::cout << "Hot-reloaded shader: " << _file_path << '\n';
    }
    else {
        std::cout << "Shader hot reload failed.";
    }
}

void Shader::SetUniform1f(const std::string& name, GLfloat v0)
{
    GLCall(glUniform1f(get_uniform_location(name), v0));
}

void Shader::SetUniform4f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    GLCall(glUniform4f(get_uniform_location(name), v0, v1, v2, v3));
}

void Shader::SetUniform4fv(const std::string& name, std::vector<glm::vec4> inVector)
{
    GLCall(glUniform3fv(get_uniform_location(name), (GLint)inVector.size(), &inVector[0].x));
}

void Shader::SetUniform3fv(const std::string& name, std::vector<glm::vec3> inVector)
{
    GLCall(glUniform3fv(get_uniform_location(name), (GLint)inVector.size(), &inVector[0].x));
}

void Shader::SetUniform2fv(const std::string& name, std::vector<glm::vec2> inVector)
{
    GLCall(glUniform2fv(get_uniform_location(name), (GLint)inVector.size(), &inVector[0].x));
}

void Shader::SetUniform1fv(const std::string& name, std::vector<float> inVector)
{
    GLCall(glUniform1fv(get_uniform_location(name), (GLint)inVector.size(), &inVector[0]));
}

void Shader::SetUniform1i(const std::string& name, GLint i)
{
    GLCall(glUniform1i(get_uniform_location(name), i));
}

void Shader::SetUniform1iv(const std::string& name, std::vector<int> inVector)
{
    GLCall(glUniform1iv(get_uniform_location(name), (GLint)inVector.size(), &inVector[0]));
}

void Shader::SetUniformMat4(const std::string& name, const glm::mat4& matrix) {
    GLCall(glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE, &matrix[0][0]));
}

void Shader::SetUniform3f(const std::string& name, glm::vec3 vec) {
    GLCall(glUniform3f(get_uniform_location(name), vec.x, vec.y, vec.z));
}

void Shader::SetUniform2f(const std::string& name, glm::vec2 vec) {
    GLCall(glUniform2f(get_uniform_location(name), vec.x, vec.y));
}

int Shader::get_uniform_location(const std::string& name) {
    if (uniform_location_cache.find(name) != uniform_location_cache.end()) {
        return uniform_location_cache[name];
    }

    GLCall(int location = glGetUniformLocation(_rendering_id, name.c_str()));
    if (location == -1) {
        std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
    }
    uniform_location_cache[name] = location;
    return location;
}
