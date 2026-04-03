#include "Space.hpp"
#include <iostream>
#include <string>
#include <algorithm>
#include "core/rendering/Renderer.hpp"
#include "core/ui/ui_dumptruck.hpp"

#include "core/resources/adapters/ModelAdapter.hpp"
#include "core/utils/ModelGenerator.hpp"
#include "glm/gtx/string_cast.hpp"

Space::Space(ResourceManager& resources, Renderer& renderer)
 : resources(resources), renderer(renderer)
{	
	
}

Space::~Space() {
	for (auto* obj : objects) {
		delete obj;
	}
	objects.clear();
}

void Space::init_space() {
	camera.set_position(glm::vec3(0.0f, 11.0f, 37.5f));

	init_skybox();
	init_raymarcher_and_voxelgrid();
	init_glass();
	init_lights();
	init_lines();
	init_gltf_models();

	// THIS IS STINKY;
	Object* base_ground = new Object(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f), glm::vec3(1.f), "res://shaders/core/default_shader.vs");
	base_ground->init(resources, "Ground Plane");
	ModelGpuData ground_model_2 = ModelGenerator::create_flat_ground(350, 350, 50, 50);
	Res::Model r_ground_model = resources.upload_model(std::move(ground_model_2));
	base_ground->set_model(std::move(r_ground_model));
	objects.push_back(base_ground);
	// THIS IS STINKY;
	
	std::string sphere_path = "res://models/sphere.obj";
	create_object(glm::vec3(-10.f, 0.f, 10.f), glm::vec3(0.f), glm::vec3(1.f), "res://models/teapot.obj", "Teapot");
	create_object(glm::vec3(7.0f, -1.0f, -8.0f), glm::vec3(0.0f), glm::vec3(5.0f), sphere_path, "Sphere 1");
	create_object(glm::vec3(-18.0f, 5.0f, 28.0f), glm::vec3(0.0f), glm::vec3(3.5f), sphere_path, "Sphere 2");
	create_object(glm::vec3(35.0f, 2.0f, 14.0f), glm::vec3(0.0f), glm::vec3(2.0f), sphere_path, "Sphere 3");
	create_object(glm::vec3(17.0f, 3.5f, -2.0f), glm::vec3(0.0f), glm::vec3(7.0f), sphere_path, "Sphere 4");
	// for (int i = 0; i < 20; ++i) { // testing the shadows
	// 	float x = (rand() % 200 - 100) * 1.2f;
	// 	float y = (rand() % 40);
	// 	float z = (rand() % 200 - 100) * 1.2f;
	// 	float scale = 0.5f + (rand() % 100) * 0.05f;
		
	// 	create_object(glm::vec3(x, y, z), glm::vec3(0.f), glm::vec3(scale), sphere_path, "Sphere " + std::to_string(i + 5));
	// }

	auto* new_object = new Object(glm::vec3(52.0f, 3.5f, -10.0f), glm::vec3(0.0f), glm::vec3(2.0f, 15.0f, 20.0f), "res://shaders/core/default_shader.vs", "res://models/cube.obj");
	new_object->init(resources, "Default Cube");
	objects.push_back(new_object);
	auto* new_object2 = new Object(glm::vec3(37.0f, 3.5f, -28.0f), glm::vec3(0.0f), glm::vec3(20.0f, 15.0f, 2.0f), "res://shaders/core/default_shader.vs", "res://models/cube.obj");
	new_object2->init(resources, "Default Cube 2");
	objects.push_back(new_object2);
}

void Space::init_skybox() {
	skybox = Skybox();
	skybox.init(resources);
	sun = Sun();
	sun.set_direction(glm::vec3(1.0f, 1.0f, 1.0f));
	sun.set_color(glm::vec4(1.0f, 1.0f, 0.930f, 1.0f));
	sun.init(resources);
}

void Space::init_raymarcher_and_voxelgrid() {
	const float cell_size = 1.5f;
	const glm::vec3 pos = glm::vec3(-30.0f, -14.45f, -22.3f);// * cell_size;
	voxel_grid = VoxelGrid(80, 40, 40, 0, cell_size, pos); 
    voxel_grid.init(resources);
	voxel_grid.set_debug_visibility(false);
	raymarcher = Raymarcher();
	raymarcher.init(new Shader(resources.get_full_path("res://shaders/raymarching/raymarcher.vs"), resources.get_full_path("res://shaders/raymarching/raymarcher.fs")));
}

void Space::init_glass() {
	glass.init(resources);
	glass.set_visibility(false);
}

void Space::init_lights() {
	lights.reserve(3); light_spheres.reserve(3);
	add_light(glm::vec3(14.0f, 13.0f, 20.5), 50.f, glm::vec3(1.0f, 0.3f, 0.2f), 84.0f, glm::vec3(0.0f, -1.0f, 0.0f), 1.0f, LightType::Point);
	add_light(glm::vec3(-10.0f, 11.0f, -22.0), 100.f, glm::vec3(0.0f, 0.58f, 1.0f), 271.0f, glm::vec3(0.0f, -1.0f, 0.0f), 1.0f, LightType::Point);
	add_light(glm::vec3(-10.0f, 12.0f, 10.0), 50.f, glm::vec3(0.31f, 0.78f, 0.48f), 21.0f, glm::vec3(0.0f, -1.0f, 0.0f), 1.0f, LightType::Point);	
}

void Space::init_lines() {
	line_manager = Line();
	line_manager.init(resources.get_full_path("res://shaders/line.vs"), resources.get_full_path("res://shaders/line.fs"));
	std::vector<LinePrimitive> world_grid_lines = {{glm::vec3(512.f, 0.0f, 0.0f), glm::vec3(-512.0f, 0.0f, 0.0f), glm::vec4(0.86f, 0.08f, 0.24f, 1.0f)},    // X : R (crimson red)
												   {glm::vec3(0.f, 512.0f, 0.0f), glm::vec3(0.0f, -512.0f, 0.0f), glm::vec4(0.196f, 0.754f, 0.196f, 1.0f)}, // Y : G (forest green)
												   {glm::vec3(0.f, 0.0f, 512.0f), glm::vec3(0.0f, 0.0f, -512.0f), glm::vec4(0.118f, 0.565f, 1.0f, 1.0f)}};  // Z : B (dodger blue)
	line_manager.add_lines(world_grid_lines, true);
}

void Space::init_gltf_models() {
	gltf_model_shader = new Shader(resources.get_full_path("res://shaders/core/default_shader.vs"), resources.get_full_path("res://shaders/core/default_shader.fs"));
	std::string model_path = "res://models/crystal_ball_table/fortune_teller_table.glb";
	std::string model_path2 = "res://models/Sponza/glTF/Sponza.gltf";
	std::string full_model_path = resources.get_full_path(model_path2);
	ModelGpuData2 model_data = ModelAdapter::load_gltf(full_model_path);
	new_models.push_back(std::move(model_data));
	std::cout << "Loaded glTF model with " << new_models[0].meshes.size() << " meshes and " << new_models[0].instances.size() << " instances." << std::endl;
}

void Space::draw_gltf_models() {
	glm::mat4 proj = renderer.get_proj();
    glm::mat4 view = renderer.get_view();
    glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(glm::mat4(1.0f)))); // set it to identity for
    
    gltf_model_shader->hot_reload_if_changed();
    gltf_model_shader->bind();
    
    // Set material uniforms
    gltf_model_shader->set_uniform_vec4("u_diffuse_color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    gltf_model_shader->set_uniform_vec4("u_specular_color", glm::vec4(1.0f, 1.0f, 1.0f, 0.6f));
    gltf_model_shader->set_uniform_vec4("u_material_params", glm::vec4(64.0f, 0.0f, 0.0f, 0.0f));
    gltf_model_shader->set_uniform_int("u_is_selected", 0);
    gltf_model_shader->set_uniform_vec3("u_sun_dir", sun.get_direction());
    gltf_model_shader->set_uniform_vec4("u_sun_color", sun.get_color());
    // gltf_model_shader->set_uniform_mat4("u_mvp", proj * view * model);
    gltf_model_shader->set_uniform_mat4("u_proj", renderer.get_proj());
    gltf_model_shader->set_uniform_mat4("u_view", view);
    gltf_model_shader->set_uniform_mat3("u_normal_matrix", normal_matrix);
    gltf_model_shader->set_uniform_mat4("u_light_space_matrix", renderer.get_light_space_matrix());

    TextureBinding shadow_bind{ renderer.get_shadow_map_texture_id(), GL_TEXTURE_2D, 5, "u_shadow_map" };

	const auto& loaded_model = new_models[0];
	for (const auto& instance : loaded_model.instances) {
		const Mesh& mesh = loaded_model.meshes[instance.mesh_index];

		for (const auto& primitive : mesh.primitives) {
			RenderCommand cmd;
			cmd.textures.push_back(shadow_bind);
			cmd.vao = primitive.vao;
			cmd.shader = gltf_model_shader;
			cmd.draw_type = DrawType::Elements;
			cmd.count = primitive.index_count;
			cmd.index_type = primitive.index_type;
			cmd.index_offset = primitive.index_byte_offset;
			glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(instance.transform)));
			gltf_model_shader->set_uniform_mat3("u_normal_matrix", normal_matrix);
			cmd.model_matrix = instance.transform; // The global transform we calculated!
			// cmd.state.cull_face = false;
			cmd.attach_lights = true;

			// --- NEW: Bind the Material ---
            if (primitive.material_index >= 0) {
                const auto& mat = loaded_model.materials[primitive.material_index];
                
                // If we have a texture, add it to the command's texture bindings
                if (mat.base_color_texture != 0) {
                    // Assuming your TextureBinding takes: { id, target, texture_unit, uniform_name }
                    cmd.textures.push_back({ mat.base_color_texture, GL_TEXTURE_2D, 8, "u_diffuse_texture" });
                    
                    // Tell the shader we ARE using a texture
                    gltf_model_shader->set_uniform_int("u_use_diffuse_texture", 1);
                } else {
                    // Tell the shader we are NOT using a texture
                    gltf_model_shader->set_uniform_int("u_use_diffuse_texture", 0);
                }
                
                // Always pass the base color factor (tint)
                gltf_model_shader->set_uniform_vec4("u_diffuse_color", mat.base_color_factor);
            } else {
				// No material, use defaults
				gltf_model_shader->set_uniform_int("u_use_diffuse_texture", 0);
				gltf_model_shader->set_uniform_vec4("u_diffuse_color", glm::vec4(1.0f));
			}

			renderer.submit(RenderPass::Forward, cmd);
		}
	}
}

void Space::tick(float delta, ButtonMap bm)
{
	time += delta;
	this_frames_button_map = bm;
	if (bm.MousePointerActive) {
		selection_ray_cast(bm.MousePosX, bm.MousePosY);
	}

	camera.tick(delta, bm);
	sun.tick(delta);

	for (auto& b : objects) {
		b->tick(delta);
	}

	voxel_grid.tick(delta, selection_ray_start, selection_ray_dir, bm.MousePointerActive, bm.MouseLeft);
	raymarcher.tick(delta);

	glass.tick(delta, bm);
        
	ui::stats_overlay(camera, renderer);
	ui::settings_panel(*this, raymarcher, raymarcher.get_raymarch_settings(), voxel_grid, sun, lights, glass, line_manager, objects);

	const std::size_t count = std::min(lights.size(), light_spheres.size());
	for (std::size_t i = 0; i < count; ++i) {
		light_spheres[i]->set_position(lights[i].position);
	}
}

void Space::enqueue_renderables() {
	glm::mat4 view_matrix = camera.get_view_matrix();
	glm::vec3 camera_pos = camera.get_position();
	renderer.set_view(view_matrix); // renderer should have all the knowledge! maybe a better way to do this?!
	renderer.set_camera_pos(camera_pos);
	renderer.update_light_matrix(-sun.get_direction(), glm::vec3(0.0f));
	renderer.begin_frame();

	// lights
	renderer.submit_lighting_data(lights);
	for (auto& light_sphere : light_spheres) { // the light objects (hard to see em otherwise)
		light_sphere->enqueue(renderer, resources, camera_pos, sun.get_direction(), sun.get_color());
	}

	skybox.enqueue(renderer, resources, camera_pos);
	sun.enqueue(renderer, camera_pos);

	for (auto& b : objects) {
		b->enqueue(renderer, resources, camera_pos, sun.get_direction(), sun.get_color());
	}
	
	voxel_grid.enqueue(renderer, resources, camera_pos, sun.get_direction(), sun.get_color());
	raymarcher.enqueue(renderer, camera_pos, sun.get_direction(), sun.get_color(), voxel_grid.get_voxel_texture_id(), voxel_grid.get_grid_dim(), voxel_grid.get_position(), voxel_grid.get_cell_size());
	
	line_manager.enqueue(renderer);
	glass.enqueue(renderer, this_frames_button_map);

	draw_gltf_models();
	renderer.execute_pipeline(voxel_grid.is_debug_view_visible());
}

void Space::create_object(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, const std::string& model_asset, const std::string& name) {
	auto* new_object = new Object(position, rotation, scale, "res://shaders/core/default_shader.vs", model_asset);
	new_object->init(resources, name);
	objects.push_back(new_object);
}

void Space::add_light(glm::vec3 position, float radius, glm::vec3 color, float intensity, 
					  glm::vec3 direction, float volumetric_intensity, LightType type) {
	Light new_light{position, radius, color, intensity, direction, volumetric_intensity, type};
	lights.push_back(std::move(new_light));
	
																	// invert the scale to invert the normals x) now its lit up! :)
	Object* new_light_object = new Object(position, glm::vec3(0.0f), glm::vec3(-0.4f), "res://shaders/core/default_shader.vs", "res://models/sphere.obj");
	new_light_object->init(resources);
	light_spheres.push_back(new_light_object);
}

void Space::remove_light(std::size_t index) {
	if (index >= lights.size() || index >= light_spheres.size()) {
		return;
	}

	delete light_spheres[index];
	light_spheres.erase(light_spheres.begin() + static_cast<std::ptrdiff_t>(index));
	lights.erase(lights.begin() + static_cast<std::ptrdiff_t>(index));
}

void Space::cast_ray(float mouse_x, float mouse_y) {
	glm::vec2 viewport_dims = renderer.get_viewport_size();
	float x = (2.0f * mouse_x) / viewport_dims.x - 1.0f;
    float y = 1.0f - (2.0f * mouse_y) / viewport_dims.y;

    glm::vec4 ray_clip = glm::vec4(x, y, -1.0f, 1.0f);
    glm::vec4 ray_eye = glm::inverse(renderer.get_proj()) * ray_clip;

    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

    glm::vec3 ray_dir = glm::vec3(glm::inverse(renderer.get_view()) * ray_eye);
    ray_dir = glm::normalize(ray_dir);
	
	glm::vec3 start = camera.get_position();
	glm::vec3 end = start + ray_dir * 25.0f;

	// line_manager.add_line(start, end, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // todo: add a button to clear the lines
}

void Space::selection_ray_cast(float mouse_x, float mouse_y) {
	glm::vec2 viewport_dims = renderer.get_viewport_size();
	float x = (2.0f * mouse_x) / viewport_dims.x - 1.0f;
    float y = 1.0f - (2.0f * mouse_y) / viewport_dims.y;

    glm::vec4 ray_clip = glm::vec4(x, y, -1.0f, 1.0f);
    glm::vec4 ray_eye = glm::inverse(renderer.get_proj()) * ray_clip;

    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

    glm::vec3 ray_dir = glm::vec3(glm::inverse(renderer.get_view()) * ray_eye);
    ray_dir = glm::normalize(ray_dir);
	
	selection_ray_start = camera.get_position();
	selection_ray_dir = ray_dir;
}


