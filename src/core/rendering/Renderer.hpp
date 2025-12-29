#pragma once
#include <glm/gtc/matrix_transform.hpp> // get that perspective thing
#include "glm/glm.hpp"
#include "RenderCommand.hpp"
#include "core/space/LightManager.hpp"

// Error handling
// #define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    GLLogCall(#x, __FILE__, __LINE__);

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);

class Renderer {
private:
    LightManager light_manager;
    Shader* composite_shader;
    Shader* copy_present_shader;

    RenderState current {};

    int viewport_width;
    int viewport_height;

    float fov = 75.f;
	float near = 0.1f;
	float far = 512.0f;
	float aspect_ratio = 16.f / 9.0f;
	glm::mat4 proj;
    glm::mat4 view;

public: 
    Renderer() = default;

    void init_renderer(int width, int height);
    void resize(int width, int height);
    void destroy_renderer();

    void begin_frame();
    void submit(RenderPass pass, const RenderCommand& cmd);
    void submit_lighting_data(std::vector<Light> lights); // this sucks because we are copying crap
    void execute_pipeline();
    void flush(RenderPass pass);
    
    void set_projection_matrix(float aspect_ratio, float fov, float near_plane = 0.1, float far_plane = 512.0f);
    void set_fov(float fov);
    void set_view(glm::mat4 v) { view = v; };
    
    glm::vec2 get_viewport_size() const;

    inline glm::mat4 get_proj() const { return proj; };
    inline glm::mat4 get_view() const { return view; }
    inline float get_fov() const { return fov; };
    inline float get_near() const { return near; };
    inline float get_far() const { return far; };

private:
    void init_quad();
    unsigned int quad_vao = 0;
    unsigned int quad_vbo = 0;

    std::vector<RenderCommand> queues[int(RenderPass::UI)+1];
    void apply_state(RenderState s);
    void execute_command(const RenderCommand& cmd);

    std::vector<Light> current_frame_light_list;

    // framebuffers :)
    void init_framebuffers(int width, int height);
    unsigned int create_color_attachment(int width, int height);
    unsigned int create_depth_attachment(int width, int height);
    int r_width; 
    int r_height;
    unsigned int render_fbo = 0;
    unsigned int ping_color = 0;
    unsigned int pong_color = 0;
    unsigned int ping_pong_depth = 0;
    
    int v_width; 
    int v_height; 
    unsigned int volumetric_fbo = 0;
    unsigned int volumetric_color = 0;
    unsigned int volumetric_depth = 0;
};