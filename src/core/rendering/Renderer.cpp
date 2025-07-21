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

// Renderer.cpp  (pseudo‑implementation)
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

void Renderer::executeCommand(const RenderCommand& c)
{
    c.shader->Bind();
    c.shader->SetUniformMat4("model", c.model);  // basic per‑object uniform

    glBindVertexArray(c.vao);

    switch (c.drawType)
    {
    case DrawType::Arrays:
        glDrawArrays(c.primitive, 0, c.count);
        break;
    case DrawType::Elements:
        glDrawElements(c.primitive, c.count, GL_UNSIGNED_INT, 0);
        break;
    case DrawType::ArraysInstanced:
        glDrawArraysInstanced(c.primitive, 0, c.count, c.instanceCount);
        break;
    case DrawType::ElementsInstanced:
        glDrawElementsInstanced(c.primitive, c.count, GL_UNSIGNED_INT, 0, c.instanceCount);
        break;
    }
}
