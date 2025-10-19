#include "ShaderAdapter.hpp"
#include "core/resources/ResourceManager.hpp"
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>

namespace ShaderAdapter {
    struct ShaderProgramSource {
        std::string vertex_source;
        std::string fragment_source;
        };

    std::optional<std::string> parse_shader_file(const std::string& shader_path) {
        std::ifstream shader_stream(shader_path);
        if (!shader_stream.is_open() || shader_stream.fail()) return std::nullopt;
        
        std::stringstream source;

        std::string line;
        while (getline(shader_stream, line)) {
            source << line << '\n';
        }

        return source.str();
    }

    void print_compile_info(unsigned int id, unsigned int type) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "!!!: " << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") << " shader compilation failed!" << std::endl;
        std::cout << message << std::endl;
    }

    std::optional<unsigned int> compile_shader(const std::string& source, unsigned int type) {
        unsigned int id = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);

        int result;
        glGetShaderiv(id, GL_COMPILE_STATUS, &result); 
        if (result == GL_FALSE) {
            print_compile_info(id, type);
            glDeleteShader(id);
            return std::nullopt;
        }

        return id;
    }
    
    std::optional<ShaderResource> load_shader(const std::string& vertex_path, const std::string& fragment_path) {
        std::string shader_name = std::filesystem::path(vertex_path).stem().string();
        
        std::optional<std::string> v_source = parse_shader_file(vertex_path);
        std::optional<std::string> f_source = parse_shader_file(fragment_path);

        if (!v_source || !f_source) {
            std::cout << shader_name << " was not parsed because it couldn't be found :/" << std::endl; 
            return std::nullopt;
        }
        
        std::optional<unsigned int> v_id = compile_shader(v_source.value(), GL_VERTEX_SHADER);
        std::optional<unsigned int> f_id = compile_shader(f_source.value(), GL_FRAGMENT_SHADER);
        
        if (!v_id && !f_id) {
            return std::nullopt;
        }

        unsigned int program = glCreateProgram();
        
        glAttachShader(program, v_id.value());
        glAttachShader(program, f_id.value());
        
        glLinkProgram(program);

        int link_success;
        glGetProgramiv(program, GL_LINK_STATUS, &link_success);
        if (link_success == false) {
            int length;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            char* message = (char*)alloca(length * sizeof(char));
            glGetProgramInfoLog(program, length, &length, message);
            std::cout << "!!!: " << shader_name << " failed linking!" << std::endl;
            std::cout << message << std::endl;
            glDeleteProgram(program);
            return std::nullopt;
        }

        glDeleteShader(v_id.value());
        glDeleteShader(f_id.value());

        ShaderResource res;
        res.fragment_file_path = fragment_path;
        res.vertex_file_path = vertex_path;
        res.rendering_id = program; 
        res.id = {};
        res.name = shader_name;
        res.uniform_block_index_cache = {};
        res.uniform_location_cache = {};

        auto vertex_last_change = std::filesystem::last_write_time(vertex_path); 
        auto fragment_last_change = std::filesystem::last_write_time(fragment_path);

        if (vertex_last_change <= fragment_last_change) {
            res.shader_last_changed = fragment_last_change;
        } else {
            res.shader_last_changed = vertex_last_change;
        }

        return res;

        // return {
        //     .id{},
        //     .rendering_id{program}
        // };
    }
};