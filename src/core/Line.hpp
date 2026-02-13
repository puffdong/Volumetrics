#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include "core/rendering/Shader.hpp"

class Renderer;

struct LinePrimitive {
    glm::vec3 start;
    glm::vec3 end;
    glm::vec4 color = glm::vec4(1.0f);
};

class Line {
private:
    std::vector<LinePrimitive> line_primitives;
    std::vector<LinePrimitive> permanent_line_primitives;
    std::vector<LinePrimitive> instance_data;
    int num_lines = 0;

    Shader* shader = nullptr;
    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int instance_vbo = 0;
    bool instances_initialized = false;
    bool instances_dirty = false;
    int allocated_capacity = 0;

public:
    Line();
    ~Line();

    void init(const std::string& vertex_shader_path, const std::string& fragment_shader_path);
    
    void add_line(const LinePrimitive& line, bool permanent = false);
    void add_line(glm::vec3 start, glm::vec3 end, glm::vec4 color = glm::vec4(1.0f), bool permanent = false);
    void add_lines(const std::vector<LinePrimitive>& lines, bool permanent = false);
    void clear_lines();
    void clear_all_lines();

    void enqueue(Renderer& renderer);

private:
    void init_instance_buffer();
    void update_instance_buffer();
    void rebuild_instance_data();
    void refresh_line_count();
};