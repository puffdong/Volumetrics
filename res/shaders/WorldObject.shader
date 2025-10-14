#shader vertex
#version 330 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_tex_coord;

out vec3 v_pos;
out vec3 v_normal;
out vec2 v_tex_coord;

uniform mat4 u_mvp;

void main()
{
	v_pos = a_pos;
	v_normal = a_normal;
	v_tex_coord = a_tex_coord;
	gl_Position = u_mvp * vec4(a_pos, 1.0f);
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec3 v_pos;
in vec3 v_normal;
in vec2 v_tex_coord;

uniform mat4 u_model_matrix;
uniform mat4 u_world_matrix;
uniform mat4 u_mvp;
uniform sampler2D u_texture1;

uniform vec3 u_sun_dir;
uniform vec3 u_sun_color;
uniform vec3 u_camera_pos;

// uniform float textureScale;
// uniform float specularStrength;
// uniform float diffuseStrength;
// uniform int shininess;

const float diffuse_strength = 0.9;
const float specular_strength = 0.4;
const int   shininess = 32;
const float texture_scale = 1.0;

void main()
{
	vec3 surface_pos = vec3(u_world_matrix * u_model_matrix * vec4(v_pos, 1.0));
	mat3 normal_matrix = transpose(inverse(mat3(u_world_matrix * u_model_matrix)));
	vec3 norm = normal_matrix * v_normal;
	norm = normalize(norm);

	vec3 result_color = vec3(0.0, 0.0, 0.0);

	vec3 light_dir = vec3(0.0, 1.0, 0.0);
	light_dir = normalize(mat3(u_world_matrix) * u_sun_dir);
	vec3 light_color = vec3(0.9, 0.9, 0.9);
	vec3 view_dir = normalize(-surface_pos);

	// Diffuse lighting
	float shade = max(dot(norm, light_dir), 0.0);
	result_color = result_color + light_color * shade * diffuse_strength;

	// Specular lighting
	vec3 reflect_dir = reflect(-light_dir, norm);  
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), float(shininess));
	result_color = result_color + light_color * spec * specular_strength ;

	// color = texture(u_texture1, v_tex_coord * textureScale) * vec4(result_color, 1.0);
	//color = texture(u_Texture, v_texCoord * 100) * vec4(result_color, 1.0);
	color = vec4(result_color, 1.0);
}
