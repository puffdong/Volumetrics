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
    int num_lines;

    Shader* shader;
    unsigned int VAO, VBO, instanceVBO;
    bool instances_initialized = false;
    bool instances_dirty = false;
    int allocated_capacity = 0;

public:
    Line();
    ~Line();

    void init(const std::string& vertex_shader_path, const std::string& fragment_shader_path);
    
    void add_line(const LinePrimitive& line);
    void add_line(glm::vec3 start, glm::vec3 end, glm::vec4 color = glm::vec4(1.0f));
    void add_lines(const std::vector<LinePrimitive>& lines);
    void clear_lines();

    void enqueue(Renderer& renderer);

private:
    void init_instance_buffer();
    void update_instance_buffer();
};