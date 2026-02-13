#include "Line.hpp"
#include <iostream>
#include "core/rendering/Renderer.hpp"

Line::Line() = default;

Line::~Line() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &instance_vbo);
}

void Line::init(const std::string& vertex_shader_path, const std::string& fragment_shader_path) {
    shader = new Shader(vertex_shader_path, fragment_shader_path);
    
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &instance_vbo);

    glBindVertexArray(vao);

    float vertices[] = { 0.0f, 1.0f };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);

    glBindVertexArray(0); // unbind it
}

void Line::add_line(const LinePrimitive& line, bool permanent) {
    if (permanent) {
        permanent_line_primitives.push_back(line);
    } else {
        line_primitives.push_back(line);
    }
    refresh_line_count();
}

void Line::add_line(glm::vec3 start, glm::vec3 end, glm::vec4 color, bool permanent) {
    LinePrimitive line{start, end, color};
    add_line(line, permanent);
}

void Line::add_lines(const std::vector<LinePrimitive>& lines, bool permanent) {
    if (permanent) {
        for (const auto& line : lines) {
            permanent_line_primitives.push_back(line);
        }
    } else {
        for (const auto& line : lines) {
            line_primitives.push_back(line);
        }
    }
    refresh_line_count();
}

void Line::clear_lines() {
    line_primitives.clear();
    refresh_line_count();
}

void Line::clear_all_lines() {
    line_primitives.clear();
    permanent_line_primitives.clear();
    refresh_line_count();
}

void Line::init_instance_buffer() {
    if (num_lines == 0) return;

    rebuild_instance_data();

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(LinePrimitive) * num_lines, instance_data.data(), GL_DYNAMIC_DRAW);

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

    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
    rebuild_instance_data();
    
    if (num_lines > allocated_capacity) {
        glBufferData(GL_ARRAY_BUFFER, sizeof(LinePrimitive) * num_lines, instance_data.data(), GL_DYNAMIC_DRAW);
        allocated_capacity = num_lines;
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(LinePrimitive) * num_lines, instance_data.data());
    }
}

void Line::rebuild_instance_data() {
    instance_data.clear();
    instance_data.reserve(static_cast<std::size_t>(num_lines));
    instance_data.insert(instance_data.end(), permanent_line_primitives.begin(), permanent_line_primitives.end());
    instance_data.insert(instance_data.end(), line_primitives.begin(), line_primitives.end());
}

void Line::refresh_line_count() {
    num_lines = static_cast<int>(permanent_line_primitives.size() + line_primitives.size());
    instances_dirty = true;
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

    shader->bind();
    shader->set_uniform_mat4("u_projection", renderer.get_proj());
    shader->set_uniform_mat4("u_view", renderer.get_view());
    shader->set_uniform_int("u_depth_texture", 2);
    shader->set_uniform_vec2("u_resolution", renderer.get_viewport_size());
    shader->set_uniform_float("u_far", renderer.get_far());
    shader->set_uniform_float("u_near", renderer.get_near());
    
    RenderCommand cmd{};
    cmd.vao = vao;
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