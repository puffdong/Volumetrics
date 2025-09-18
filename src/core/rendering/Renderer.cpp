#include "Renderer.hpp"
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

void Renderer::init_renderer(int width, int height)
{  
    GLCall(glDepthFunc(GL_LESS));
    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GLCall(glEnable(GL_DEPTH_TEST));
    GLCall(glEnable(GL_CULL_FACE));
    GLCall(glCullFace(GL_BACK));
    GLCall(glFrontFace(GL_CCW));

    GLuint globalVao = 0;
    GLCall(glGenVertexArrays(1, &globalVao));
    GLCall(glBindVertexArray(globalVao));

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LINE_SMOOTH);
    glDepthMask(GL_TRUE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW); // pick one and stick to it

    current.depth_test      = false;
    current.depth_write     = true;
    current.cull_face       = false;
    current.cull_front_back = GL_BACK;
    current.line_smooth     = false;
    init_quad();
    init_framebuffer(width, height);
    test_shader = new Shader("/Users/puff/Developer/graphics/Volumetrics/res/shaders/test_shader.shader");
    
    glViewport(0,0, width, height);
}

void Renderer::init_framebuffer(int width, int height)
{
    // clean up old resources if they exist
    if (sceneFBO)
    {
        glDeleteFramebuffers(1, &sceneFBO);
        glDeleteTextures   (1, &sceneColorTex);
        glDeleteRenderbuffers(1, &sceneDepthRBO);
    }
        // clean up old resources if they exist
    if (volumetrics_fbo)
    {
        glDeleteFramebuffers(1, &volumetrics_fbo);
        glDeleteTextures   (1, &volumetrics_fbo_color);
        glDeleteRenderbuffers(1, &volumetrics_fbo_depth);
    }

    // 1. framebuffer
    glGenFramebuffers(1, &sceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

    // 2. color attachment (texture)
    glGenTextures(1, &sceneColorTex);
    glBindTexture(GL_TEXTURE_2D, sceneColorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           sceneColorTex,
                           0);

    // 3. depth attachment (renderbuffer)
    glGenRenderbuffers(1, &sceneDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, sceneDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER,
                          GL_DEPTH24_STENCIL8,
                          width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER,
                              sceneDepthRBO);

    // 4. sanity check
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Renderer::recreateSceneFBO -> Framebuffer incomplete!" << std::endl;
    }
    
    if (volumetrics_fbo)
    {
        glDeleteFramebuffers(1, &volumetrics_fbo);
        glDeleteTextures   (1, &volumetrics_fbo_color);
        glDeleteRenderbuffers(1, &volumetrics_fbo_depth);
    }

    // 1. framebuffer
    glGenFramebuffers(1, &volumetrics_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, volumetrics_fbo);

    // 2. color attachment (texture)
    glGenTextures(1, &volumetrics_fbo_color);
    glBindTexture(GL_TEXTURE_2D, volumetrics_fbo_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width / 2, height / 2, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           volumetrics_fbo_color,
                           0);

    // 3. depth attachment (renderbuffer)
    glGenRenderbuffers(1, &volumetrics_fbo_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, volumetrics_fbo_depth);
    glRenderbufferStorage(GL_RENDERBUFFER,
                          GL_DEPTH24_STENCIL8,
                          width / 2, height / 2);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER,
                              volumetrics_fbo_depth);

    // 4. sanity check
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Renderer::volumetrics_fbo -> Framebuffer incomplete!" << std::endl;
    }
    else {
        std::cout << "complete" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::init_quad() {
    if (quadVAO) return; // already done

    float verts[] = {
        //   pos   // uv
        -1.f, -1.f, 0.f, 0.f,
            3.f, -1.f, 2.f, 0.f,  // single-triangle trick
        -1.f,  3.f, 0.f, 2.f
    };
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
}

void Renderer::present_to_screen() {
    // 1. back to default framebuffer
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // 2. assume window size viewport already set by the platform layer
    glClear(GL_COLOR_BUFFER_BIT);
    test_shader->HotReloadIfChanged();
    test_shader->Bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneColorTex);
    test_shader->SetUniform1i("u_Scene", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, volumetrics_fbo_color);
    test_shader->SetUniform1i("volumetrics_tex", volumetrics_fbo_color);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Renderer::resize(int width, int height) {
    init_framebuffer(width, height); // the function re-initializes the framebuffer
}

void Renderer::begin_frame(const glm::vec4& clear)
{
    // glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
    // Establish a known baseline before clearing
    glDisable(GL_SCISSOR_TEST);          // scissor clips clears; kill it unless you mean it
    glDepthMask(GL_TRUE);                // allow depth clear to actually write
    glClearDepth(1.0);                   // depth clear value
    glClearColor(clear.r, clear.g, clear.b, clear.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto& q : queues) q.clear();
}

void Renderer::submit(RenderPass pass, const RenderCommand& cmd)
{
    queues[int(pass)].push_back(cmd);
}

void Renderer::flush(RenderPass pass)
{
    for (const auto& cmd : queues[int(pass)])
        execute_command(cmd);
}

void Renderer::execute_pipeline() {
    for (const auto& cmd : queues[int(RenderPass::Skypass)]) {
        execute_command(cmd);
    }

    for (const auto& cmd : queues[int(RenderPass::Forward)]) {
        execute_command(cmd);
    }
    
    for (const auto& cmd : queues[int(RenderPass::Transparent)]) {
        execute_command(cmd);
    }

    for (const auto& cmd : queues[int(RenderPass::Volumetrics)]) {
        execute_command(cmd);
    }
}

void Renderer::apply_state(RenderState s) { // figure out what to do with this
    auto apply = [](bool desired, bool enabled_cap, int GL_FUNC)
    {
        if (desired && !enabled_cap) glEnable(GL_FUNC);
        if (!desired && enabled_cap) glDisable(GL_FUNC);
    };

    apply(s.depth_test, current.depth_test, GL_DEPTH_TEST);
    apply(s.cull_face, current.cull_face, GL_CULL_FACE); // what if i wanna do GL_FRONT culling, look into how this works
    apply(s.line_smooth, current.line_smooth, GL_LINE_SMOOTH);

    glCullFace(s.cull_front_back);

    if (s.depth_write != current.depth_write)
        glDepthMask(s.depth_write ? GL_TRUE : GL_FALSE);
    
    current = s;
}

void Renderer::execute_command(const RenderCommand& c)
{
    apply_state(c.state);

    c.shader->Bind();
    c.shader->SetUniformMat4("model", c.model);

    for (const auto& t : c.textures) {
        glActiveTexture(GL_TEXTURE0 + t.unit);
        glBindTexture(t.target, t.id);
        if (t.uniform_name) c.shader->SetUniform1i(t.uniform_name, t.unit);
    }

    switch (c.draw_type)
    {
    case DrawType::Arrays:
        glBindVertexArray(c.vao);
        glDrawArrays(c.primitive, 0, c.count);
        break;

    case DrawType::Elements:
        glBindVertexArray(c.vao);
        glDrawElements(c.primitive, c.count, GL_UNSIGNED_INT, 0);
        break;

    case DrawType::ArraysInstanced:
        glBindVertexArray(c.vao);
        glDrawArraysInstanced(c.primitive, 0, c.count, c.instance_count);
        break;

    case DrawType::ElementsInstanced:
        glBindVertexArray(c.vao);
        glDrawElementsInstanced(c.primitive, c.count, GL_UNSIGNED_INT, 0, c.instance_count);
        break;

    case DrawType::Framebuffer:
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        break;
    }
}