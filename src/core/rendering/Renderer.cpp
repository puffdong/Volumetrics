#include "Renderer.h"
#include <GL/glew.h>
#include <iostream>


void GLClearError() {
    while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line) {
    while (GLenum error = glGetError()) {
        std::cout << "[OpenGL Error] (" << error << "): " << function << " " << file << ":" << line << std::endl;
        return false;
    }
    return true;
}

std::vector<RenderCommand> Renderer::queues[int(RenderPass::UI)+1];

void Renderer::BeginFrame(const glm::vec4& clear)
{
    glClearColor(clear.r, clear.g, clear.b, clear.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (auto& q : queues) q.clear();
}

void Renderer::Submit(RenderPass pass, const RenderCommand& cmd)
{
    queues[int(pass)].push_back(cmd);
}

void Renderer::Flush(RenderPass pass)
{
    for (const auto& cmd : queues[int(pass)])
        executeCommand(cmd);
}

void Renderer::applyState(RenderState s) { // figure out what to do with this
    auto apply = [](bool desired, bool enabled_cap, int GL_FUNC)
    {
        if (desired && !enabled_cap) glEnable(GL_FUNC);
        if (!desired && enabled_cap) glDisable(GL_FUNC);
    };

    apply(s.depth_test, current.depth_test, GL_DEPTH_TEST);
    apply(s.cull_face, current.cull_face, GL_CULL_FACE); // what if i wanna do GL_FRONT culling, look into how this works
    apply(s.line_smooth, current.line_smooth, GL_LINE_SMOOTH);

    if (s.depth_write != current.depth_write)
        glDepthMask(s.depth_write ? GL_TRUE : GL_FALSE);
}

void Renderer::executeCommand(const RenderCommand& c)
{
    applyState(c.state); 

    c.shader->Bind();
    c.shader->SetUniformMat4("model", c.model);  

    glBindVertexArray(c.vao);

    for (const auto& t : c.textures) {
        glActiveTexture(GL_TEXTURE0 + t.unit);
        glBindTexture(t.target, t.id);

        if (t.uniform_name) {
            c.shader->SetUniform1i(t.uniform_name, t.unit);
        }
    }

    switch (c.draw_type)
    {
    case DrawType::Arrays:
        glDrawArrays(c.primitive, 0, c.count);
        break;
    case DrawType::Elements:
        glDrawElements(c.primitive, c.count, GL_UNSIGNED_INT, 0);
        break;
    case DrawType::ArraysInstanced:
        glDrawArraysInstanced(c.primitive, 0, c.count, c.instance_count);
        break;
    case DrawType::ElementsInstanced:
        glDrawElementsInstanced(c.primitive, c.count, GL_UNSIGNED_INT, 0, c.instance_count);
        break;
    }
}
