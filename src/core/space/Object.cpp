#include "Object.hpp"
#include "core/rendering/Texture.hpp"
#include "core/space/Space.hpp"
#include <iostream>

Object::Object(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, Base* parent,
           const std::string& shader_path, 
           const std::string& model_path, 
           const std::string& texture_path) 
           : Base(pos, rot, scale, parent) 
           {
            r_shader.asset_path = shader_path;
            r_model.asset_path = model_path;
            r_texture.asset_path = texture_path;
           }

void Object::init(ResourceManager& resources, Space* space) {
    Base::init(resources, space);
    if (r_model.asset_path.empty()) {
        r_model = resources.load_model("res://models/VoxelModels/defaultCube.obj"); // default to cube if no model path is supplied
    } else {
        r_model = resources.load_model(r_model.asset_path);
    }

    // object now loaded, now we construct the lines from the aabb
    init_collision_bounds_debug(resources, space);
    // not used for now 
    // if (r_texture.asset_path != "") { 
    //     _texture = new Texture(resources.get_full_path(r_texture.asset_path));
    // } else {
    //     _texture = nullptr;
    // }
    auto shader_path = r_shader.asset_path;
    r_shader = resources.load_shader(shader_path, shader_path.substr(0, shader_path.find_last_of('.'))+".fs");
}

void Object::tick(float delta) {

}

void Object::enqueue(Renderer& renderer, ResourceManager& resources) {


    if (auto shader = resources.get_shader(r_shader.id)) {
        glm::mat4 proj = renderer.get_proj();
        glm::mat4 view = renderer.get_view();
        glm::mat4 model = get_model_matrix();
        (*shader)->hot_reload_if_changed();
        (*shader)->bind();
        (*shader)->set_uniform_vec3("u_camera_pos", _space->get_camera()->get_position());
        (*shader)->set_uniform_vec3("u_sun_dir", _space->get_sun()->get_direction());
        (*shader)->set_uniform_vec3("u_sun_color", glm::vec3(1.0, 1.0, 1.0));
        (*shader)->set_uniform_mat4("u_mvp", proj * view * model);
        (*shader)->set_uniform_mat4("u_proj", renderer.get_proj());
        (*shader)->set_uniform_mat4("u_model", model);
        (*shader)->set_uniform_mat4("u_view", view);

        ModelGpuData gpu_model = resources.get_model_gpu_data(r_model.id);

        // STILL GOTTA HANDLE TEXTURE STUFF BUT IDC RIGHT NOW, WE MOVING FAST AND SWIFT, OUT WITH OLD, IN WITH NEW
        RenderCommand cmd{};
        cmd.vao = gpu_model.vao;
        cmd.draw_type = DrawType::Elements;
        cmd.count = gpu_model.index_count;
        cmd.shader = (*shader);
        cmd.attach_lights = true;

        renderer.submit(RenderPass::Forward, cmd);
    } else {
        std::cout << "Shader for resource ID: " << r_shader.id.value() << " not found!" << "\n";
        return;
    }

    if (draw_collision) {
        collision_lines->enqueue(renderer, resources);
    }
    
}

void Object::set_model(Res::Model res) {
    r_model = res;
}

void Object::init_collision_bounds_debug(ResourceManager& resources, Space* space) {
    ModelGpuData gpu_model = resources.get_model_gpu_data(r_model.id);
    
    glm::vec3 aabb_min = gpu_model.aabb_min + position;
    glm::vec3 aabb_max = gpu_model.aabb_max + position;

    std::vector<LinePrimitive> edges; edges.reserve(12);

    const glm::vec3 c000{aabb_min.x, aabb_min.y, aabb_min.z};
    const glm::vec3 c100{aabb_max.x, aabb_min.y, aabb_min.z};
    const glm::vec3 c010{aabb_min.x, aabb_max.y, aabb_min.z};
    const glm::vec3 c110{aabb_max.x, aabb_max.y, aabb_min.z};
    const glm::vec3 c001{aabb_min.x, aabb_min.y, aabb_max.z};
    const glm::vec3 c101{aabb_max.x, aabb_min.y, aabb_max.z};
    const glm::vec3 c011{aabb_min.x, aabb_max.y, aabb_max.z};
    const glm::vec3 c111{aabb_max.x, aabb_max.y, aabb_max.z};

    auto add_edge = [&edges](const glm::vec3& a, const glm::vec3& b) {
        edges.push_back(LinePrimitive{a, b});
    };

    // Bottom face (z = min)
    add_edge(c000, c100);
    add_edge(c100, c110);
    add_edge(c110, c010);
    add_edge(c010, c000);

    // Top face (z = max)
    add_edge(c001, c101);
    add_edge(c101, c111);
    add_edge(c111, c011);
    add_edge(c011, c001);

    // Vertical edges
    add_edge(c000, c001);
    add_edge(c100, c101);
    add_edge(c110, c111);
    add_edge(c010, c011);
    
    collision_lines = new Line(std::move(edges));
    collision_lines->init(resources, space);
}