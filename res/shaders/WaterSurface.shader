#shader vertex
#version 330 core
layout(location = 0) in vec3 a_pos;      // Vertex position
layout(location = 1) in vec3 a_normal;   // Vertex normal
layout(location = 2) in vec2 a_texcoord; // Vertex texture coordinates

out vec3 v_pos;
out vec3 v_normal;
out vec2 v_texCoord;

uniform mat4 u_mvp;

void main()
{
	v_pos = a_pos;
	v_normal = a_normal;
	v_texCoord = a_texcoord;
	gl_Position = u_mvp * vec4(a_pos, 1.0f);
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec3 v_pos;
in vec3 v_normal;
in vec2 v_texCoord;

uniform mat4 modelMatrix;
uniform mat4 worldMatrix;
uniform sampler2D u_Texture;
uniform int numLights;
uniform vec3 lightColors[10];
uniform vec3 lightDirs[10];
uniform bool isDirectional[10];
uniform float textureScale;
uniform float specularStrength;
uniform float diffuseStrength;
uniform int shininess;

void main()
{
	vec3 surfacePos = vec3(worldMatrix * modelMatrix * vec4(v_pos, 1.0));

	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	vec3 norm = mat3(worldMatrix) * normalMatrix * v_normal;
	normalize(norm);

	vec3 result_color = vec3(0.0, 0.0, 0.0);

	for (int i = 0; i < numLights; i++) {
		vec3 lightDir;
		if (isDirectional[i]) {
			lightDir = normalize(mat3(worldMatrix) * lightDirs[i]);
		} else {
			vec3 lightPosition = vec3(worldMatrix * vec4(lightDirs[i], 1.0));
			lightDir = normalize(lightPosition - surfacePos);
		}
		vec3 lightColor = lightColors[i];
		vec3 viewDir = normalize(-surfacePos);

		// Diffuse lighting
		float shade = max(dot(norm, lightDir), 0.0);
		result_color = result_color + lightColor * shade * diffuseStrength;

		// Specular lighting
		vec3 reflectDir = reflect(-lightDir, norm);  
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
		result_color = result_color + lightColor * spec * specularStrength ;
	}

	color = texture(u_Texture, v_texCoord * textureScale) * vec4(result_color, 1.0);
	//color = texture(u_Texture, v_texCoord * 100) * vec4(result_color, 1.0);
	//color = vec4(result_color, 1.0);
}