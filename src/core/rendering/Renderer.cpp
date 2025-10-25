#include "Renderer.hpp"
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
 
Renderer::Renderer(ResourceManager& resources) : resources(resources) {}

void Renderer::init_renderer(int width, int height)
{  
    viewport_width = width;
    viewport_height = height;

    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    default_vao = 0;
    GLCall(glGenVertexArrays(1, &default_vao)); // keep these GL calls, if the gl context is wonky we know it early
    GLCall(glBindVertexArray(default_vao));

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LINE_SMOOTH);
    glDepthMask(GL_TRUE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW); // pick one and stick to it, CCW: counter clock wise

    current.depth_test      = false;
    current.depth_write     = true;
    current.cull_face       = false;
    current.cull_front_back = GL_BACK;
    current.line_smooth     = false;
    
    // init shaders, and set the uniforms that stay static during run-time
    composite_shader = new Shader(resources.get_full_path("res://shaders/pipeline/composite.vs"), resources.get_full_path("res://shaders/pipeline/composite.fs"));
    composite_shader->bind();
    composite_shader->set_uniform_int("u_src_color",   0);
    composite_shader->set_uniform_int("u_volum_color", 1);
    composite_shader->set_uniform_int("u_scene_depth", 2); // kept for later depth-aware composite


    copy_present_shader = new Shader(resources.get_full_path("res://shaders/pipeline/copy_present.vs"), resources.get_full_path("res://shaders/pipeline/copy_present.fs"));
    copy_present_shader->bind();
    copy_present_shader->set_uniform_int("u_src_color", 0);
    copy_present_shader->set_uniform_int("u_depth_texture", 2);

    init_quad();
    init_framebuffers(width, height);
    glViewport(0,0, width, height);
}

void Renderer::init_framebuffers(int width, int height)
{
    create_render_framebuffer(width, height);
    create_volumetric_framebuffer(width, height);
    create_composite_framebuffer(width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // default framebuffer
}

void Renderer::destroy_renderer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Todo: Clear framebuffers

    for (auto& q : queues) q.clear();
}

void Renderer::init_quad() {
    if (quad_vao) return; // already done

    float verts[] = {
        //   pos   // uv
        -1.f, -1.f, 0.f, 0.f,
            3.f, -1.f, 2.f, 0.f,  // single-triangle trick
        -1.f,  3.f, 0.f, 2.f
    };
    glGenVertexArrays(1, &quad_vao);
    glGenBuffers(1, &quad_vbo);
    glBindVertexArray(quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
}

void Renderer::create_render_framebuffer(int width, int height) {
    // Store size
    r_width  = width;
    r_height = height;

    // Clean up previous
    if (r_fbo) {
        if (r_color_texture) glDeleteTextures(1, &r_color_texture);
        if (r_depth_texture) glDeleteTextures(1, &r_depth_texture);
        glDeleteFramebuffers(1, &r_fbo);
        r_fbo = 0; r_color_texture = 0; r_depth_texture = 0;
    }

    // FBO
    glGenFramebuffers(1, &r_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, r_fbo);

    // Color (LDR; switch to GL_SRGB8_ALPHA8 or GL_RGBA16F later if desired)
    glGenTextures(1, &r_color_texture);
    glBindTexture(GL_TEXTURE_2D, r_color_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, r_color_texture, 0);

    // Depth (texture so we can sample it later)
    glGenTextures(1, &r_depth_texture);
    glBindTexture(GL_TEXTURE_2D, r_depth_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Important: read numeric depth, not shadow compare:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D, r_depth_texture, 0);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    // Sanity check
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[Renderer] r_fbo incomplete: 0x" << std::hex << status << std::dec << "\n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::create_volumetric_framebuffer(int width, int height) {
    v_width  = width;
    v_height = height;

    // Clean previous
    if (v_fbo) {
        if (v_color_texture) glDeleteTextures(1, &v_color_texture);
        if (v_depth_texture) glDeleteTextures(1, &v_depth_texture);
        glDeleteFramebuffers(1, &v_fbo);
        v_fbo = 0; v_color_texture = 0; v_depth_texture = 0;
    }

    glGenFramebuffers(1, &v_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, v_fbo);

    // Color: HDR to avoid banding in fog
    glGenTextures(1, &v_color_texture);
    glBindTexture(GL_TEXTURE_2D, v_color_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, v_width, v_height, 0,
                 GL_RGBA, GL_HALF_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, v_color_texture, 0);

    // Depth is optional for pure raymarch; keeping it gives flexibility (e.g., slicing)
    glGenTextures(1, &v_depth_texture);
    glBindTexture(GL_TEXTURE_2D, v_depth_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, v_width, v_height, 0,
                 GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D, v_depth_texture, 0);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[Renderer] v_fbo incomplete: 0x" << std::hex << status << std::dec << "\n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::create_composite_framebuffer(int width, int height) {
    c_width  = width;
    c_height = height;
    // Clean previous
    if (c_fbo) {
        if (c_color_texture) glDeleteTextures(1, &c_color_texture);
        if (c_depth_texture) glDeleteTextures(1, &c_depth_texture);
        glDeleteFramebuffers(1, &c_fbo);
        c_fbo = 0; c_color_texture = 0; c_depth_texture = 0;
    }

    glGenFramebuffers(1, &c_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, c_fbo);

    // Color (LDR by default; switch to RGBA16F if you build an HDR post chain)
    glGenTextures(1, &c_color_texture);
    glBindTexture(GL_TEXTURE_2D, c_color_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, c_color_texture, 0);

    // Depth (often not needed for pure fullscreen composites, but you exposed it)
    glGenTextures(1, &c_depth_texture);
    glBindTexture(GL_TEXTURE_2D, c_depth_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D, c_depth_texture, 0);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[Renderer] c_fbo incomplete: 0x" << std::hex << status << std::dec << "\n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::resize(int width, int height) {
    viewport_width = width;
    viewport_height = height;
    init_framebuffers(width, height); // the function re-initializes the framebuffer
}

void Renderer::set_projection_matrix(float new_aspect_ratio, float new_fov, float near_plane, float far_plane) {
    if (aspect_ratio <= 0) {
		aspect_ratio = 16.f / 9.f; // a bit of sanity checking
	} else {
		aspect_ratio = new_aspect_ratio;
	}
    near = near_plane;
    far = far_plane;

    fov = new_fov;
    proj = glm::perspective(glm::radians(fov), aspect_ratio, near, far);
	changes_made = true;
}

void Renderer::set_fov(float fov) {
    set_projection_matrix(aspect_ratio, fov, near, far); // jank but honestly, pretty lush
}



glm::vec2 Renderer::get_viewport_size() {
    return glm::ivec2(viewport_width, viewport_height);
}

glm::vec2 Renderer::get_framebuffer_size(RenderPass pass) {
    switch (pass) { // This is shitty
        case RenderPass::Skypass:
            return glm::vec2(r_width, r_height);
        case RenderPass::Forward:
            return glm::vec2(r_width, r_height);
        case RenderPass::Volumetrics:
            return glm::vec2(v_width, v_height);
        case RenderPass::UI:
            return glm::vec2(c_width, c_height);
    }
}

void Renderer::begin_frame()
{
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
    // --- Ping-pong roles ------------------------------------------------------
    GLuint src_color = r_color_texture;   // current scene buffer we READ from
    GLuint dst_color = c_color_texture;   // destination buffer we WRITE into

    // --- 1) Skypass + Forward onto r_fbo (with depth) ------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, r_fbo);
    glViewport(0, 0, r_width, r_height);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, src_color, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,   GL_TEXTURE_2D, r_depth_texture, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glDisable(GL_SCISSOR_TEST);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glClearDepth(1.0);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    flush(RenderPass::Skypass);
    flush(RenderPass::Forward);

    // --- 2) Volumetrics into v_fbo -------------------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, v_fbo);
    glViewport(0, 0, v_width, v_height);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Volumetric shaders will pull depth on whatever units they expect; we just make it handy on TU2 as convention.
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, r_depth_texture);

    flush(RenderPass::Volumetrics);

    // --- 3) Composite volumes → dst_color on r_fbo ---------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, r_fbo);
    glViewport(0, 0, r_width, r_height);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dst_color, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    // Detach depth while sampling it (avoids any read/write ambiguity)
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    // Bind inputs to well-known units
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, src_color);       // scene color
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, v_color_texture); // volumetrics
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, r_depth_texture); // scene depth

    // Bind composite program and set sampler uniforms to the *unit indices*
    composite_shader->hot_reload_if_changed();
    composite_shader->bind();

    // Optional (for later when we use depth): composite_shader->set_uniform_float("u_near", near); composite_shader->set_uniform_float("u_far",  far);

    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Swap: composited scene now in dst_color → becomes new src
    { GLuint t = src_color; src_color = dst_color; dst_color = t; }

    // Re-attach depth for any later geometry-type passes (kept consistent)
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, r_depth_texture, 0);

    // --- 4) UI OVER SCENE (BEFORE present) -----------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, r_fbo);
    glViewport(0, 0, r_width, r_height);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dst_color, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    // Detach depth attachment while we *sample* the depth texture
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);  // 4th argument == 0 (ie, we detached depth tex)

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    // 4a) Base copy src_color → dst_color
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, src_color);       // in here, both opaque and volumetrics are combined :)
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, r_depth_texture);

    copy_present_shader->hot_reload_if_changed();
    copy_present_shader->bind();

    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // 4b) Make scene color + depth available to UI shaders (by convention)
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, src_color);       // in here, both opaque and volumetrics are combined :)
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, r_depth_texture); // scene depth

    flush(RenderPass::UI);

    // 4c) Swap so UI-composited image becomes the new src
    { GLuint t = src_color; src_color = dst_color; dst_color = t; }

    // Restore depth attachment (not strictly needed for present)
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, r_depth_texture, 0);

    // --- 5) Present the current scene (now includes UI) ----------------------
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, viewport_width, viewport_height);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, src_color);

    copy_present_shader->bind();
    copy_present_shader->set_uniform_int("u_src_color", 0);

    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Renderer::apply_state(RenderState s) { // figure out what to do with this
    auto apply = [](bool desired, bool enabled_cap, int GL_FUNC)
    {
        if (desired && !enabled_cap) glEnable(GL_FUNC);
        if (!desired && enabled_cap) glDisable(GL_FUNC);
    };

    apply(s.depth_test, current.depth_test, GL_DEPTH_TEST);
    apply(s.cull_face, current.cull_face, GL_CULL_FACE); // what if i wanna do GL_FRONT culling tho
    apply(s.line_smooth, current.line_smooth, GL_LINE_SMOOTH);
    apply(s.scissor_test, current.scissor_test, GL_SCISSOR_TEST);

    glCullFace(s.cull_front_back);

    if (s.depth_write != current.depth_write)
        glDepthMask(s.depth_write ? GL_TRUE : GL_FALSE);
    
    current = s;
}

void Renderer::execute_command(const RenderCommand& c)
{
    apply_state(c.state);

    c.shader->bind();

    for (const auto& t : c.textures) {
        glActiveTexture(GL_TEXTURE0 + t.unit);
        glBindTexture(t.target, t.id);
        if (t.uniform_name) c.shader->set_uniform_int(t.uniform_name, t.unit);
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

    case DrawType::FullscreenQuad:
        glBindVertexArray(quad_vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        break;
    }
}