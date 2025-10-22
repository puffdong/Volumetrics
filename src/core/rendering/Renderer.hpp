#pragma once
#include <glm/gtc/matrix_transform.hpp> // get that perspective thing
#include "glm/glm.hpp"
#include <vector>
#include "RenderCommand.hpp"
#include "Shader.hpp"

// - - - Todo - - - //
// get an actual pipe-line set up
// look into touching up the rendercommands by adding uniform setting calls (just an idea :o)

// Error handling
// #define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    GLLogCall(#x, __FILE__, __LINE__);

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);

class Renderer
{
public: 
    
    void init_renderer(int width, int height);
    void resize(int width, int height);
    void begin_frame();
    void submit(RenderPass pass, const RenderCommand& cmd);
    void execute_pipeline();
    void flush(RenderPass pass);
    
    void destroy();

    // getters n' setters
    void set_projection_matrix(float aspect_ratio, float fov, float near_plane = 1.0, float far_plane = 256);
    void set_fov(float fov);
    void set_view(glm::mat4 v) { view = v; };
    
    glm::ivec2 get_viewport_size();
    glm::ivec2 get_framebuffer_size(RenderPass pass);

    inline glm::mat4 get_proj() const { return proj; };
    inline glm::mat4 get_view() const { return view; }
    inline float get_fov() const { return fov; };
    inline float get_near() const { return near; };
    inline float get_far() const { return far; };

private:
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
    
    std::vector<RenderCommand> queues[int(RenderPass::UI)+1];
    void apply_state(RenderState s);
    void execute_command(const RenderCommand& cmd);
    
    void init_quad();
    GLuint quad_vao = 0;
    GLuint quad_vbo = 0;
    
    void init_framebuffers(int width, int height);

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

    Shader* composite_shader = nullptr;
    Shader* copy_present_shader = nullptr;
};

