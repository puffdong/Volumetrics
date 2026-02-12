#include "Line.hpp"
#include <iostream>
#include "core/rendering/Renderer.hpp"

Line::Line() {
    num_lines = 0;
}

Line::~Line() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &instanceVBO);
}

void Line::init(const std::string& vertex_shader_path, const std::string& fragment_shader_path) {
    shader = new Shader(vertex_shader_path, fragment_shader_path);
    shader->set_debug_output(true);
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(VAO);

    float vertices[] = { 0.0f, 1.0f };

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);

    glBindVertexArray(0); // unbind it
}

void Line::add_line(const LinePrimitive& line) {
    line_primitives.push_back(line);
    num_lines = line_primitives.size();
    instances_dirty = true;
}

void Line::add_line(glm::vec3 start, glm::vec3 end, glm::vec4 color) {
    LinePrimitive line{start, end, color};
    line_primitives.push_back(line);
    num_lines = line_primitives.size();
    instances_dirty = true;
}

void Line::add_lines(const std::vector<LinePrimitive>& lines) {
    for (const auto& line : lines) {
        line_primitives.push_back(line);
    }
    num_lines = line_primitives.size();
    instances_dirty = true;
}

void Line::clear_lines() {
    line_primitives.clear();
    num_lines = 0;
}

void Line::init_instance_buffer() {
    if (num_lines == 0) return;

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(LinePrimitive) * num_lines, &line_primitives[0], GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LinePrimitive), (void*)offsetof(LinePrimitive, start));
    glVertexAttribDivisor(1, 1); 

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(LinePrimitive), (void*)offsetof(LinePrimitive, end));
    glVertexAttribDivisor(2, 1); 

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(LinePrimitive), (void*)offsetof(LinePrimitive, color));
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);
    
    allocated_capacity = num_lines;
}

void Line::update_instance_buffer() {
    if (num_lines == 0) return;

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    
    // If we need more space, reallocate
    if (num_lines > allocated_capacity) {
        glBufferData(GL_ARRAY_BUFFER, sizeof(LinePrimitive) * num_lines, &line_primitives[0], GL_DYNAMIC_DRAW);
        allocated_capacity = num_lines;
    } else {
        // Otherwise just update existing data
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(LinePrimitive) * num_lines, &line_primitives[0]);
    }
}

void Line::enqueue(Renderer& renderer) { 
    if (num_lines == 0) return;

    if (!instances_initialized) {
        init_instance_buffer();
        instances_initialized = true;
    }
    
    if (instances_dirty) {
        update_instance_buffer();
        instances_dirty = false;
    }

    shader->hot_reload_if_changed();
    shader->bind();
    shader->set_uniform_mat4("u_projection", renderer.get_proj());
    shader->set_uniform_mat4("u_view", renderer.get_view());
    shader->set_uniform_int("u_depth_texture", 2);
    shader->set_uniform_vec2("u_resolution", renderer.get_viewport_size());
    shader->set_uniform_float("u_far", renderer.get_far());
    shader->set_uniform_float("u_near", renderer.get_near());
    
    RenderCommand cmd{};
    cmd.vao = VAO;
    cmd.draw_type = DrawType::ArraysInstanced;
    cmd.primitive = GL_LINES;
    cmd.count = 2;
    cmd.instance_count = num_lines;
    cmd.shader = shader;
    cmd.state.depth_write = false;
    cmd.state.depth_test = true;
    cmd.state.line_smooth = true;

    renderer.submit(RenderPass::UI, cmd);
}