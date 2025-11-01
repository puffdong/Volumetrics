#include "Line.hpp"
#include <iostream>

Line::Line(const glm::vec3& start, const glm::vec3& end,
           const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& sc)
  : Base(pos, rot, sc) {
    line_primitives.reserve(2);
    line_primitives.push_back({start, end});
    num_lines = 1;
}

Line::Line(std::vector<LinePrimitive> lines, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& sc)
  : Base(pos, rot, sc) {
    for (int i = 0; i < lines.size(); i++) {
        line_primitives.push_back(lines[i]);
    }
    num_lines = lines.size();
}

Line::~Line() {
    // delete shader; // TODO: fix later
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &instanceVBO);
}

void Line::init(ResourceManager& resources, Space* space) {
    Base::init(resources, _space);
    r_shader = resources.load_shader("res://shaders/Line.shader");
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(VAO);

    float vertices[] = {
        0.0f,
        1.0f
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(LinePrimitive) * num_lines, &line_primitives[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LinePrimitive), (void*)offsetof(LinePrimitive, start));
    glVertexAttribDivisor(1, 1); 

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(LinePrimitive), (void*)offsetof(LinePrimitive, end));
    glVertexAttribDivisor(2, 1); 

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(LinePrimitive), (void*)offsetof(LinePrimitive, color));
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0); // unbind it
}

void Line::tick(float delta) {

}

void Line::enqueue(Renderer& renderer, ResourceManager& resources) {
    if (auto shader = resources.get_shader(r_shader.id)) {
        (*shader)->hot_reload_if_changed();
        (*shader)->bind();
        (*shader)->set_uniform_mat4("projection", renderer.get_proj());
        (*shader)->set_uniform_mat4("view", renderer.get_view());
        (*shader)->set_uniform_int("u_depth_texture", 2);
        (*shader)->set_uniform_vec2("u_resolution", renderer.get_framebuffer_size(RenderPass::UI));
        (*shader)->set_uniform_float("u_far", renderer.get_far());
        (*shader)->set_uniform_float("u_near", renderer.get_near());
        

        RenderCommand cmd{};
        cmd.vao            = VAO;
        cmd.draw_type       = DrawType::ArraysInstanced;
        cmd.primitive      = GL_LINES;
        cmd.count          = 2;
        cmd.instance_count  = num_lines;
        cmd.shader         = (*shader);
        cmd.state.depth_write = false;
        cmd.state.depth_test = false;
        cmd.state.line_smooth = true;

        renderer.submit(RenderPass::UI, cmd);
    } else {
        std::cout << "Shader for resource ID " << r_shader.id.value() << " not found!" << "\n";
        return;
    }
}
