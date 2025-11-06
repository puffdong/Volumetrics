#pragma once
#include <glm/gtc/matrix_transform.hpp> // get that perspective thing
#include "glm/glm.hpp"
#include <vector>
#include "core/resources/ResourceManager.hpp"
#include "RenderCommand.hpp"

#include "core/rendering/Shader.hpp"

// - - - Todo - - - //
// this shader is very much hard coded, its getting unweildly :o
// get an actual pipe-line set up
// look into touching up the rendercommands by adding uniform setting calls (just an idea :o)

// Error handling
// #define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    GLLogCall(#x, __FILE__, __LINE__);

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);

class Renderer {
private:
    ResourceManager& resources;
    Shader* composite_shader;
    Shader* copy_present_shader;

    RenderState current {};

    int viewport_width;
    int viewport_height;

    float fov = 70.f;
	float near = 1.0f;
	float far = 256.0f;
	float aspect_ratio = 16.f / 9.0f;
	glm::mat4 proj;
    glm::mat4 view;
	bool changes_made = true;

    unsigned int default_vao; // wasn't there a good reason for having a default vao or am I just imagining it?

public: 
    Renderer(ResourceManager& resources);

    void init_renderer(int width, int height);
    void resize(int width, int height);
    void destroy_renderer();

    void begin_frame();
    void submit(RenderPass pass, const RenderCommand& cmd);
    void execute_pipeline();
    void flush(RenderPass pass);
    

    // getters n' setters
    void set_projection_matrix(float aspect_ratio, float fov, float near_plane = 1.0, float far_plane = 512);
    void set_fov(float fov);
    void set_view(glm::mat4 v) { view = v; };
    
    glm::vec2 get_viewport_size();
    glm::vec2 get_framebuffer_size(RenderPass pass);

    inline glm::mat4 get_proj() const { return proj; };
    inline glm::mat4 get_view() const { return view; }
    inline float get_fov() const { return fov; };
    inline float get_near() const { return near; };
    inline float get_far() const { return far; };

private:
    void init_quad();
    unsigned int quad_vao = 0;
    unsigned int quad_vbo = 0;

    void init_framebuffers(int width, int height);

    std::vector<RenderCommand> queues[int(RenderPass::UI)+1];
    void apply_state(RenderState s);
    void execute_command(const RenderCommand& cmd);

    
    // Passes: Skypass, Forward
    void create_render_framebuffer(int width, int height); // prefix r_
    int r_width; int r_height;
    unsigned int r_fbo = 0;
    unsigned int r_color_texture = 0;
    unsigned int r_depth_texture = 0;

    // Pass: Volumetrics
    void create_volumetric_framebuffer(int width, int height); // prefix v_
    int v_width; int v_height; 
    unsigned int v_fbo = 0;
    unsigned int v_color_texture = 0;
    unsigned int v_depth_texture = 0;
    
    // Pass: UI
    void create_composite_framebuffer(int width, int height); // prefix c_
    int c_width; int c_height;
    unsigned int c_fbo = 0;
    unsigned int c_color_texture = 0;
    unsigned int c_depth_texture = 0;
};

