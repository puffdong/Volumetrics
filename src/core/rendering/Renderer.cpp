#include "Renderer.hpp"
#include <iostream>

// helper stuff
#ifdef __APPLE__
    #define ASSET_PATH "/Users/puff/Developer/graphics/Volumetrics/res/"
    #endif
#if defined _WIN32 || defined _WIN64
    #define ASSET_PATH "C:/Dev/OpenGL/Volumetrics/res/"
#endif 

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

std::string get_full_path(const std::string& asset_path) {
    std::string asset_handle = "res://";
    return std::string(ASSET_PATH) + std::string(asset_path.substr(asset_handle.size(), asset_path.size()));
}
// that's that! 

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

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LINE_SMOOTH);
    glDepthMask(GL_TRUE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    current.depth_test      = false;
    current.depth_write     = true;
    current.cull_face       = false;
    current.cull_front_back = GL_BACK;
    current.line_smooth     = false;
    
    composite_shader = new Shader(get_full_path("res://shaders/pipeline/composite.vs"), get_full_path("res://shaders/pipeline/composite.fs"));
    composite_shader->bind();
    upload_composite_shader_uniforms();

    copy_present_shader = new Shader(get_full_path("res://shaders/pipeline/copy_present.vs"), get_full_path("res://shaders/pipeline/copy_present.fs"));
    copy_present_shader->bind();
    copy_present_shader->set_uniform_int("u_src_color", 0);

    light_manager.init();
    init_quad();
    init_framebuffers(width, height);
    glViewport(0,0, width, height);
}

void Renderer::init_quad() {
    if (quad_vao) return;

    float verts[] = {
        // pos     // uv
       -1.f, -1.f, 0.f, 0.f,
        3.f, -1.f, 2.f, 0.f,
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

void Renderer::init_framebuffers(int width, int height)
{
    r_width  = width;
    r_height = height;

    if (render_fbo) {
        if (ping_color) glDeleteTextures(1, &ping_color);
        if (pong_color) glDeleteTextures(1, &pong_color);
        if (ping_pong_depth) glDeleteTextures(1, &ping_pong_depth);
        glDeleteFramebuffers(1, &render_fbo);
        render_fbo = 0; ping_color = 0; pong_color = 0; ping_pong_depth = 0;
    }

    glGenFramebuffers(1, &render_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);

    ping_color = create_color_attachment(width, height);
    pong_color = create_color_attachment(width, height);
    ping_pong_depth = create_depth_attachment(width, height);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "r_fbo incomplete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (volumetric_fbo) {
        if (raymarch_bounds_depth) glDeleteTextures(1, &raymarch_bounds_depth);
        if (volumetric_color) glDeleteTextures(1, &volumetric_color);
        glDeleteFramebuffers(1, &volumetric_fbo);
        volumetric_fbo = 0; volumetric_color = 0; raymarch_bounds_depth = 0;
    }

    glGenFramebuffers(1, &volumetric_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, volumetric_fbo);

    volumetric_color = create_color_attachment(r_width, r_height);
    raymarch_bounds_depth = create_depth_attachment(r_width, r_height);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "v_fbo incomplete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // bind default :)
}

unsigned int Renderer::create_color_attachment(int width, int height) {
    unsigned int color_attachment = 0;
    glGenTextures(1, &color_attachment);
    glBindTexture(GL_TEXTURE_2D, color_attachment);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, color_attachment, 0);
    return color_attachment;
}

unsigned int Renderer::create_depth_attachment(int width, int height) {
    unsigned int depth_attachment = 0;
    glGenTextures(1, &depth_attachment);
    glBindTexture(GL_TEXTURE_2D, depth_attachment);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D, depth_attachment, 0);

    return depth_attachment;
}

void Renderer::upload_composite_shader_uniforms() {
    composite_shader->bind();
    composite_shader->set_uniform_float("u_near", near);
    composite_shader->set_uniform_float("u_far", far);
    composite_shader->set_uniform_int("u_src_color",   0);
    composite_shader->set_uniform_int("u_volum_color", 1);
    composite_shader->set_uniform_int("u_scene_depth", 2);
    composite_shader->set_uniform_int("u_raymarch_depth", 3);
}

void Renderer::resize(int width, int height) {
    viewport_width = width;
    viewport_height = height;
    init_framebuffers(width, height); // the function re-initializes the framebuffer
}

void Renderer::destroy_renderer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    for (auto& q : queues) q.clear();
}

void Renderer::set_projection_matrix(float new_aspect_ratio, float new_fov, float near_plane, float far_plane) {
    if (new_aspect_ratio <= 0) {
		aspect_ratio = 16.f / 9.f; // a bit of sanity checking
	} else {
		aspect_ratio = new_aspect_ratio;
	}
    near = near_plane;
    far = far_plane;

    fov = new_fov;
    proj = glm::perspective(glm::radians(fov), aspect_ratio, 0.1f, far);
}

void Renderer::set_fov(float fov) {
    set_projection_matrix(aspect_ratio, fov, near, far); // jank but honestly, pretty lush
}

glm::vec2 Renderer::get_viewport_size() const {
    return glm::vec2(viewport_width, viewport_height);
}

void Renderer::begin_frame()
{
    for (auto& q : queues) q.clear();
}

void Renderer::submit(RenderPass pass, const RenderCommand& cmd)
{
    queues[int(pass)].push_back(cmd);
}

void Renderer::submit_lighting_data(std::vector<Light> lights) { // I ADMIT, THIS IS NOT NICE, BUT WE HACKING AND THEN WE SLASHING
    current_frame_light_list = lights; // THAT IS HOW I ROLL, WE GLUE STUFF TOGETHER THEN WE ARCHITECHT IT LATER WHEN I FIGURE OUT THE DETAILS
}

void Renderer::flush(RenderPass pass)
{
    for (const auto& cmd : queues[int(pass)])
        execute_command(cmd);
}

void Renderer::execute_pipeline(bool voxel_grid_debug_view) {
    // --- Ping-pong roles ------------------------------------------------------
    GLuint src_color = ping_color;
    GLuint dst_color = pong_color;

    // Skypass, Forward
    glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);
    glViewport(0, 0, r_width, r_height);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, src_color, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ping_pong_depth, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glDisable(GL_SCISSOR_TEST);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glClearDepth(1.0);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    flush(RenderPass::Skypass);
    flush(RenderPass::Forward);

    // Raymarch Bounds
    glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, raymarch_bounds_depth, 0);
    if (!voxel_grid_debug_view) glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glClear(GL_DEPTH_BUFFER_BIT);
    flush(RenderPass::RaymarchBounds);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // Volumetrics
    glBindFramebuffer(GL_FRAMEBUFFER, volumetric_fbo);
    glViewport(0, 0, r_width, r_height);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0); // we want to read from this one! disable it!

    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT); // de-attachment of depth means we can skip clearing it

    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, ping_pong_depth);
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, raymarch_bounds_depth);

    flush(RenderPass::Volumetrics);

    // Composite (skypass and forward) + (volumetrics)
    glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);
    glViewport(0, 0, r_width, r_height);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dst_color, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    // Don't write to depth (what?)
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, src_color);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, volumetric_color);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, ping_pong_depth);
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, raymarch_bounds_depth);

    bool changed = composite_shader->hot_reload_if_changed();
    composite_shader->bind();
    if (changed) {
        upload_composite_shader_uniforms();
    }

    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);
    glViewport(0, 0, r_width, r_height);

    for (const auto& cmd : queues[int(RenderPass::UI)]) {
        { GLuint t = src_color; src_color = dst_color; dst_color = t; } // ping pong

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ping_pong_depth, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dst_color, 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, src_color);
        glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, ping_pong_depth);

        copy_present_shader->bind();

        glBindVertexArray(quad_vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        execute_command(cmd);
    }

    { GLuint t = src_color; src_color = dst_color; dst_color = t; }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ping_pong_depth, 0);

    // present to screen!
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, viewport_width, viewport_height);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, src_color);

    copy_present_shader->bind();

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

    unsigned int program_id = c.shader->get_renderer_id();
    glUseProgram(program_id);

    for (const auto& t : c.textures) {
        glActiveTexture(GL_TEXTURE0 + t.unit);
        glBindTexture(t.target, t.id);
        if (t.uniform_name) c.shader->set_uniform_int(t.uniform_name, t.unit);
    }

    if (c.attach_lights) { // attach light info if desired
        light_manager.upload(current_frame_light_list);
        light_manager.bind(0);
        c.shader->set_uniform_block("b_light_block", 0);
        c.shader->set_uniform_int("u_light_count", light_manager.get_light_count());
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